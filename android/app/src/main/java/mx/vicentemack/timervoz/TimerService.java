package mx.vicentemack.timervoz;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ServiceInfo;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.ParcelUuid;
import android.os.PowerManager;
import android.os.SystemClock;

import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.UUID;

/**
 * Servicio en primer plano: busca el timer StickS3, se conecta solo (tambien cuando lo vuelves
 * a prender) y dice los avisos por los audifonos, aunque la pantalla este apagada.
 * Protocolo del timer v3.31 y v3.32 (Bluetooth de bajo consumo). App 1.1 (desde la 1.2 se llama PRS Timer Voz): la primera vez
 * se queda con el primer timer que encuentra y despues solo se conecta a ese.
 */
public class TimerService extends Service {
    static final UUID SERVICIO = UUID.fromString("6f3c0001-5b1a-4c7e-9d2a-3e1f5a7b9c10");
    static final UUID AVISOS = UUID.fromString("6f3c0002-5b1a-4c7e-9d2a-3e1f5a7b9c10");
    static final UUID CCCD = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
    static final String ACCION_PROBAR = "mx.vicentemack.timervoz.PROBAR";
    static final String ACCION_DETENER = "mx.vicentemack.timervoz.DETENER";
    static final String ACCION_AJUSTES = "mx.vicentemack.timervoz.AJUSTES";
    static final String ACCION_CAMBIAR = "mx.vicentemack.timervoz.CAMBIAR";

    /** Lo que dice en cada aviso del timer (escrito como se pronuncia: "taimer", "erre o"). */
    static final Map<String, String> FRASES = new HashMap<>();

    static {
        FRASES.put("LISTO", "Prepárate. Cinco segundos para que se arme el táimer");
        FRASES.put("ARMADO", "Armado. Ya puedes decirle listo al erre o");
        FRASES.put("Q60", "sesenta");
        FRASES.put("Q45", "cuarenta y cinco");
        FRASES.put("Q25", "veinticinco");
        String[] n = {"uno", "dos", "tres", "cuatro", "cinco", "seis", "siete", "ocho", "nueve", "diez"};
        for (int i = 1; i <= 10; i++) FRASES.put("Q" + i, n[i - 1]);
    }

    /** La pantalla de la app escucha los cambios. */
    interface Escucha {
        void alCambiar();
    }

    static volatile Escucha escucha;
    static volatile boolean corriendo = false;
    static volatile String estadoConexion = "Detenido";
    static volatile String fase = "—";
    static volatile String segundos = "—";
    static volatile String tiros = "—";
    static volatile String bateria = "—";
    static final LinkedList<String> bitacora = new LinkedList<>();

    private static final Object PRUEBA = new Object();
    private final Handler h = new Handler(Looper.getMainLooper());
    private final Runnable buscarR = this::buscar;
    private final Runnable tiempoFuera = () -> {
        anotar("No conectó a tiempo; vuelvo a buscar");
        cerrarGatt();
        programarBusqueda(1000);
    };

    private SharedPreferences prefs;
    private Voz voz;
    private BluetoothAdapter adaptador;
    private BluetoothLeScanner escaner;
    private BluetoothGatt gatt;
    private String nombreConectado = "";
    private boolean buscando = false;
    private boolean detenido = false;
    private long ultimaBusqueda = -100000;
    private PowerManager.WakeLock candado;

    @Override
    public void onCreate() {
        super.onCreate();
        prefs = getSharedPreferences("ajustes", MODE_PRIVATE);
        voz = new Voz(this);
        BluetoothManager bm = getSystemService(BluetoothManager.class);
        adaptador = bm != null ? bm.getAdapter() : null;
        PowerManager pm = getSystemService(PowerManager.class);
        candado = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "timervoz:conectado");
        candado.setReferenceCounted(false);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        String accion = intent != null ? intent.getAction() : null;
        if (ACCION_DETENER.equals(accion)) {
            detener();
            return START_NOT_STICKY;
        }
        try {
            Notification n = aviso(estadoConexion.equals("Detenido") ? "Buscando el timer…" : estadoConexion);
            if (Build.VERSION.SDK_INT >= 29) {
                startForeground(1, n, ServiceInfo.FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE);
            } else {
                startForeground(1, n);
            }
        } catch (Exception e) {
            estadoConexion = "Faltan permisos de Bluetooth";
            anotar("No pude arrancar: " + e.getClass().getSimpleName());
            stopSelf();
            return START_NOT_STICKY;
        }
        corriendo = true;
        detenido = false;
        if (ACCION_CAMBIAR.equals(accion)) {      // otro timer: olvida el nombre y busca el primero que aparezca
            prefs.edit().remove("nombre").apply();
            pararBusqueda();
            cerrarGatt();
            if (candado.isHeld()) candado.release();
            anotar("Buscando otro timer: deja prendido solo el tuyo");
        }
        voz.mantenerDespierto(prefs.getBoolean("despierto", true));
        if (ACCION_PROBAR.equals(accion)) probar();
        if (gatt == null && !buscando) programarBusqueda(0);
        avisar();
        return START_STICKY;
    }

    // ---------- busqueda ----------

    private String nombreBuscado() {
        return prefs.getString("nombre", "").trim();
    }

    private String nombreParaMostrar() {
        String n = nombreBuscado();
        return n.isEmpty() ? "tu timer" : n;
    }

    private void programarBusqueda(long ms) {
        h.removeCallbacks(buscarR);
        if (!detenido) h.postDelayed(buscarR, ms);
    }

    @SuppressLint("MissingPermission")
    private void buscar() {
        if (detenido || gatt != null || buscando) return;
        if (adaptador == null || !adaptador.isEnabled()) {
            estado("Prende el Bluetooth del teléfono");
            programarBusqueda(5000);
            return;
        }
        long espera = 6500 - (SystemClock.elapsedRealtime() - ultimaBusqueda);   // Android limita 5 busquedas cada 30 s
        if (espera > 0) {
            programarBusqueda(espera);
            return;
        }
        escaner = adaptador.getBluetoothLeScanner();
        if (escaner == null) {
            programarBusqueda(3000);
            return;
        }
        ScanFilter filtro = new ScanFilter.Builder().setServiceUuid(new ParcelUuid(SERVICIO)).build();
        ScanSettings ajustes = new ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build();
        try {
            escaner.startScan(Collections.singletonList(filtro), ajustes, alBuscar);
            buscando = true;
            ultimaBusqueda = SystemClock.elapsedRealtime();
            estado("Buscando " + nombreParaMostrar() + "…");
        } catch (SecurityException e) {
            estado("Faltan permisos de Bluetooth");
        }
    }

    private final ScanCallback alBuscar = new ScanCallback() {
        @Override
        public void onScanResult(int tipo, ScanResult r) {
            h.post(() -> revisar(r));
        }

        @Override
        public void onBatchScanResults(List<ScanResult> rs) {
            for (ScanResult r : rs) h.post(() -> revisar(r));
        }

        @Override
        public void onScanFailed(int codigo) {
            h.post(() -> {
                buscando = false;
                anotar("La búsqueda falló (" + codigo + "); vuelvo a intentar");
                programarBusqueda(5000);
            });
        }
    };

    @SuppressLint("MissingPermission")
    private void revisar(ScanResult r) {
        if (!buscando || gatt != null) return;
        String nombre = r.getScanRecord() != null ? r.getScanRecord().getDeviceName() : null;
        if (nombre == null) {
            try {
                nombre = r.getDevice().getName();
            } catch (SecurityException e) {
                nombre = null;
            }
        }
        if (nombre == null) return;          // todavia no llega el nombre: espera el siguiente anuncio
        String buscado = nombreBuscado();
        boolean sirve = buscado.isEmpty() ? nombre.startsWith("TimerS3") : nombre.equalsIgnoreCase(buscado);
        if (!sirve) return;
        pararBusqueda();
        conectar(r.getDevice(), nombre);
    }

    @SuppressLint("MissingPermission")
    private void pararBusqueda() {
        if (buscando && escaner != null) {
            try {
                escaner.stopScan(alBuscar);
            } catch (Exception e) {
                // el Bluetooth ya estaba apagado
            }
        }
        buscando = false;
    }

    // ---------- conexion ----------

    @SuppressLint("MissingPermission")
    private void conectar(BluetoothDevice d, String nombre) {
        nombreConectado = nombre;
        estado("Conectando a " + nombre + "…");
        try {
            gatt = d.connectGatt(this, false, alGatt, BluetoothDevice.TRANSPORT_LE);
            h.removeCallbacks(tiempoFuera);
            h.postDelayed(tiempoFuera, 15000);
        } catch (SecurityException e) {
            gatt = null;
            estado("Faltan permisos de Bluetooth");
            programarBusqueda(5000);
        }
    }

    private final BluetoothGattCallback alGatt = new BluetoothGattCallback() {
        @Override
        @SuppressLint("MissingPermission")
        public void onConnectionStateChange(BluetoothGatt g, int status, int nuevo) {
            h.post(() -> {
                if (g != gatt) {
                    try {
                        g.close();
                    } catch (Exception e) {
                        // ya estaba cerrado
                    }
                    return;
                }
                if (nuevo == BluetoothProfile.STATE_CONNECTED) {
                    try {
                        g.discoverServices();
                    } catch (SecurityException e) {
                        anotar("Faltan permisos de Bluetooth");
                    }
                } else if (nuevo == BluetoothProfile.STATE_DISCONNECTED) {
                    desconectado();
                }
            });
        }

        @Override
        @SuppressLint("MissingPermission")
        public void onServicesDiscovered(BluetoothGatt g, int status) {
            h.post(() -> {
                if (g != gatt) return;
                BluetoothGattService s = g.getService(SERVICIO);
                BluetoothGattCharacteristic c = s != null ? s.getCharacteristic(AVISOS) : null;
                BluetoothGattDescriptor d = c != null ? c.getDescriptor(CCCD) : null;
                if (d == null) {
                    anotar("Ese aparato no trae los avisos del timer");
                    cerrarGatt();
                    programarBusqueda(5000);
                    return;
                }
                try {
                    g.setCharacteristicNotification(c, true);
                    if (Build.VERSION.SDK_INT >= 33) {
                        g.writeDescriptor(d, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                    } else {
                        escribirViejo(g, d);
                    }
                } catch (SecurityException e) {
                    anotar("Faltan permisos de Bluetooth");
                }
            });
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt g, BluetoothGattDescriptor d, int status) {
            h.post(() -> {
                if (g != gatt) return;
                if (status == BluetoothGatt.GATT_SUCCESS) {
                    conectado();
                } else {
                    anotar("No pude activar los avisos (" + status + "); vuelvo a intentar");
                    cerrarGatt();
                    programarBusqueda(3000);
                }
            });
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt g, BluetoothGattCharacteristic c, byte[] valor) {
            String m = new String(valor, StandardCharsets.US_ASCII).trim();
            h.post(() -> recibir(m));
        }

        @Override
        @SuppressWarnings("deprecation")
        public void onCharacteristicChanged(BluetoothGatt g, BluetoothGattCharacteristic c) {
            if (Build.VERSION.SDK_INT >= 33) return;          // en Android 13+ llega por el metodo de arriba
            byte[] v = c.getValue();
            if (v == null) return;
            String m = new String(v, StandardCharsets.US_ASCII).trim();
            h.post(() -> recibir(m));
        }
    };

    @SuppressWarnings("deprecation")
    @SuppressLint("MissingPermission")
    private static void escribirViejo(BluetoothGatt g, BluetoothGattDescriptor d) {
        d.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
        g.writeDescriptor(d);
    }

    private void conectado() {
        h.removeCallbacks(tiempoFuera);
        if (nombreBuscado().isEmpty()) {          // la primera vez se queda con este timer
            prefs.edit().putString("nombre", nombreConectado).apply();
            anotar("Este teléfono se quedó con " + nombreConectado);
        }
        estado("Conectado a " + nombreConectado);
        anotar("Conectado a " + nombreConectado);
        if (prefs.getBoolean("decirConectado", true)) voz.decir("conectado");
        if (!candado.isHeld()) candado.acquire(4L * 60 * 60 * 1000);   // maximo 4 horas
    }

    private void desconectado() {
        boolean estaba = estadoConexion.startsWith("Conectado");
        cerrarGatt();
        if (candado.isHeld()) candado.release();
        if (estaba) anotar("Se desconectó el timer");
        fase = "Sin timer";
        programarBusqueda(1500);
        avisar();
    }

    @SuppressLint("MissingPermission")
    private void cerrarGatt() {
        h.removeCallbacks(tiempoFuera);
        if (gatt != null) {
            BluetoothGatt g = gatt;
            gatt = null;
            try {
                g.disconnect();
                g.close();
            } catch (Exception e) {
                // ya estaba cerrado
            }
        }
    }

    // ---------- avisos del timer ----------

    private void recibir(String m) {
        if (m.startsWith("S,")) {                 // estado cada segundo: S,segundos,estado,tiros,barras
            String[] p = m.split(",");
            if (p.length >= 5) {
                segundos = p[1];
                fase = nombreFase(p[2]);
                tiros = p[3];
                bateria = p[4].startsWith("-") ? "—" : p[4] + " de 5";
                avisar();
            }
            return;
        }
        anotar(m);
        String clave = m.split(",")[0];
        String frase = FRASES.get(clave);
        if (frase != null) {
            voz.decir(frase);
        } else if (clave.equals("INICIO")) {        // arranco el tiempo: 6 pitidos cortos
            voz.callar();
            voz.pitidos(6, 120, 120);
            fase = "Corriendo";
        } else if (clave.equals("FIN")) {           // se acabo el tiempo: 6 pitidos largos
            voz.callar();
            voz.pitidos(6, 500, 250);
            fase = "Terminó";
        } else if (clave.equals("APAGADO")) {       // se va a apagar: lo espero para la siguiente etapa
            cerrarGatt();
            if (candado.isHeld()) candado.release();
            fase = "Timer apagado";
            estado("Timer apagado; lo espero");
            programarBusqueda(1500);
        }
        if (clave.equals("LISTO")) fase = "Espera";
        if (clave.equals("ARMADO")) fase = "Armado";
        avisar();
    }

    private static String nombreFase(String e) {
        switch (e) {
            case "L": return "Listo";
            case "E": return "Espera";
            case "A": return "Armado";
            case "C": return "Corriendo";
            case "T": return "Terminó";
            default: return e;
        }
    }

    // ---------- prueba de voz y pitidos ----------

    private void probar() {
        anotar("Prueba de voz y pitidos");
        h.removeCallbacksAndMessages(PRUEBA);
        voz.callar();
        long t = 300;
        t = paso(t, () -> voz.decir(FRASES.get("LISTO")), 3800);
        t = paso(t, () -> voz.decir(FRASES.get("ARMADO")), 3500);
        t = paso(t, () -> voz.pitidos(6, 120, 120), 2300);
        t = paso(t, () -> voz.decir("sesenta"), 1500);
        t = paso(t, () -> voz.decir("cuarenta y cinco"), 1800);
        t = paso(t, () -> voz.decir("veinticinco"), 1800);
        for (int s = 10; s >= 1; s--) {
            final String f = FRASES.get("Q" + s);
            t = paso(t, () -> voz.decir(f), 1000);
        }
        paso(t, () -> voz.pitidos(6, 500, 250), 0);
    }

    private long paso(long en, Runnable r, long dura) {
        h.postAtTime(r, PRUEBA, SystemClock.uptimeMillis() + en);
        return en + dura;
    }

    // ---------- estado para la pantalla ----------

    private Notification aviso(String texto) {
        NotificationManager nm = getSystemService(NotificationManager.class);
        nm.createNotificationChannel(new NotificationChannel("timer", "PRS Timer Voz", NotificationManager.IMPORTANCE_LOW));
        PendingIntent abrir = PendingIntent.getActivity(this, 0,
                new Intent(this, MainActivity.class), PendingIntent.FLAG_IMMUTABLE);
        return new Notification.Builder(this, "timer")
                .setContentTitle("PRS Timer Voz")
                .setContentText(texto)
                .setSmallIcon(android.R.drawable.stat_sys_data_bluetooth)
                .setContentIntent(abrir)
                .setOngoing(true)
                .build();
    }

    private void estado(String t) {
        estadoConexion = t;
        if (corriendo) {
            NotificationManager nm = getSystemService(NotificationManager.class);
            if (nm != null) nm.notify(1, aviso(t));
        }
        avisar();
    }

    private void anotar(String t) {
        String hora = new SimpleDateFormat("HH:mm:ss", Locale.US).format(new Date());
        synchronized (bitacora) {
            bitacora.addFirst(hora + "  " + t);
            while (bitacora.size() > 40) bitacora.removeLast();
        }
        avisar();
    }

    private static void avisar() {
        Escucha e = escucha;
        if (e != null) e.alCambiar();
    }

    private void detener() {
        detenido = true;
        h.removeCallbacksAndMessages(null);
        pararBusqueda();
        cerrarGatt();
        if (candado.isHeld()) candado.release();
        corriendo = false;
        estadoConexion = "Detenido";
        fase = "—";
        segundos = "—";
        tiros = "—";
        bateria = "—";
        avisar();
        stopForeground(STOP_FOREGROUND_REMOVE);
        stopSelf();
    }

    @Override
    public void onDestroy() {
        detenido = true;
        h.removeCallbacksAndMessages(null);
        pararBusqueda();
        cerrarGatt();
        if (candado != null && candado.isHeld()) candado.release();
        if (voz != null) voz.cerrar();
        corriendo = false;
        estadoConexion = "Detenido";
        avisar();
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent i) {
        return null;
    }
}

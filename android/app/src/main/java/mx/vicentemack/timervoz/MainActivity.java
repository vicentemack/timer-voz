package mx.vicentemack.timervoz;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.Typeface;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

/** Pantalla de la app: conectar, probar la voz, ajustes y los avisos que van llegando. */
public class MainActivity extends Activity {
    private SharedPreferences prefs;
    private TextView estado, fase, segundos, detalle, bitacora, tuTimer;
    private Button conectar;
    private String pendiente;          // lo que se queria hacer antes de pedir permisos

    @Override
    protected void onCreate(Bundle guardado) {
        super.onCreate(guardado);
        prefs = getSharedPreferences("ajustes", MODE_PRIVATE);

        ScrollView marco = new ScrollView(this);
        LinearLayout col = new LinearLayout(this);
        col.setOrientation(LinearLayout.VERTICAL);
        int m = dp(16);
        col.setPadding(m, m, m, m);
        marco.addView(col);

        col.addView(texto("PRS Timer Voz", 26, true));
        col.addView(texto("Timer y voz para el StickS3", 15, false));
        estado = texto("Detenido", 16, false);
        col.addView(estado);

        fase = texto("—", 18, true);
        fase.setGravity(Gravity.CENTER);
        col.addView(fase, margen());
        segundos = texto("—", 72, true);
        segundos.setGravity(Gravity.CENTER);
        col.addView(segundos);
        detalle = texto("Tiros —   Batería —", 16, false);
        detalle.setGravity(Gravity.CENTER);
        col.addView(detalle);

        conectar = boton("Conectar con el timer", v -> alConectar());
        col.addView(conectar, margen());
        col.addView(boton("Probar voz y pitidos", v -> pedirOIniciar(TimerService.ACCION_PROBAR)), margen());

        tuTimer = texto("", 16, true);
        col.addView(tuTimer, margen());
        col.addView(boton("Cambiar de timer", v -> cambiarDeTimer()));

        col.addView(casilla("Mantener despiertos los audífonos, para que no se coman la primera sílaba", "despierto"), margen());
        col.addView(casilla("Decir “conectado” cuando se conecta el timer", "decirConectado"));
        col.addView(boton("Quitar el ahorro de batería a esta app",
                v -> startActivity(new Intent(Settings.ACTION_IGNORE_BATTERY_OPTIMIZATION_SETTINGS))), margen());

        col.addView(texto("Qué dice: al picar el azul, “Prepárate. Cinco segundos para que se arme el timer”; "
                + "a los 5 s, “Armado. Ya puedes decirle listo al R. O.”; al arrancar, 6 pitidos cortos; "
                + "“sesenta”, “cuarenta y cinco” y “veinticinco”; la cuenta de diez a uno; y en 0, 6 pitidos largos. "
                + "Funciona con la pantalla apagada: deja la app conectada. Cuando vuelves a prender el timer, se conecta solo. "
                + "La primera vez, prende solo tu timer: la app se queda con él (su nombre sale abajo en la pantalla del timer).",
                14, false), margen());

        col.addView(texto("Avisos recibidos", 16, true), margen());
        bitacora = texto("", 13, false);
        col.addView(bitacora);

        setContentView(marco);
    }

    @Override
    protected void onResume() {
        super.onResume();
        TimerService.escucha = () -> runOnUiThread(this::refrescar);
        refrescar();
    }

    @Override
    protected void onPause() {
        TimerService.escucha = null;
        super.onPause();
    }

    private void refrescar() {
        estado.setText(TimerService.estadoConexion);
        fase.setText(TimerService.fase);
        segundos.setText(TimerService.segundos);
        detalle.setText("Tiros " + TimerService.tiros + "   Batería " + TimerService.bateria);
        conectar.setText(TimerService.corriendo ? "Detener" : "Conectar con el timer");
        String n = prefs.getString("nombre", "").trim();
        tuTimer.setText(n.isEmpty() ? "Tu timer: se escoge solo la primera vez" : "Tu timer: " + n);
        StringBuilder sb = new StringBuilder();
        synchronized (TimerService.bitacora) {
            for (String linea : TimerService.bitacora) sb.append(linea).append('\n');
        }
        bitacora.setText(sb.toString());
    }

    // ---------- conectar y permisos ----------

    private void alConectar() {
        if (TimerService.corriendo) {
            Intent i = new Intent(this, TimerService.class);
            i.setAction(TimerService.ACCION_DETENER);
            startService(i);
        } else {
            pedirOIniciar(null);
        }
    }

    private void pedirOIniciar(String accion) {
        List<String> faltan = new ArrayList<>();
        for (String p : permisos()) {
            if (checkSelfPermission(p) != PackageManager.PERMISSION_GRANTED) faltan.add(p);
        }
        if (faltan.isEmpty()) {
            iniciar(accion);
        } else {
            pendiente = accion == null ? "" : accion;
            requestPermissions(faltan.toArray(new String[0]), 7);
        }
    }

    @Override
    public void onRequestPermissionsResult(int codigo, String[] permisos, int[] resultados) {
        super.onRequestPermissionsResult(codigo, permisos, resultados);
        if (codigo != 7) return;
        if (bluetoothConcedido()) {
            iniciar(pendiente == null || pendiente.isEmpty() ? null : pendiente);
        } else {
            estado.setText("Sin el permiso de Bluetooth (Dispositivos cercanos) no puedo buscar el timer");
        }
        pendiente = null;
    }

    private String[] permisos() {
        List<String> l = new ArrayList<>();
        if (Build.VERSION.SDK_INT >= 31) {
            l.add(Manifest.permission.BLUETOOTH_SCAN);
            l.add(Manifest.permission.BLUETOOTH_CONNECT);
        } else {
            l.add(Manifest.permission.ACCESS_FINE_LOCATION);
        }
        if (Build.VERSION.SDK_INT >= 33) l.add(Manifest.permission.POST_NOTIFICATIONS);
        return l.toArray(new String[0]);
    }

    private boolean bluetoothConcedido() {
        if (Build.VERSION.SDK_INT >= 31) {
            return checkSelfPermission(Manifest.permission.BLUETOOTH_SCAN) == PackageManager.PERMISSION_GRANTED
                    && checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED;
        }
        return checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED;
    }

    private void iniciar(String accion) {
        Intent i = new Intent(this, TimerService.class);
        if (accion != null) i.setAction(accion);
        startForegroundService(i);
    }

    private void cambiarDeTimer() {
        prefs.edit().remove("nombre").apply();
        if (TimerService.corriendo) {
            Intent i = new Intent(this, TimerService.class);
            i.setAction(TimerService.ACCION_CAMBIAR);
            startForegroundService(i);
        }
        refrescar();
    }

    private void avisarAjustes() {
        if (!TimerService.corriendo) return;
        Intent i = new Intent(this, TimerService.class);
        i.setAction(TimerService.ACCION_AJUSTES);
        startForegroundService(i);
    }

    // ---------- piezas de la pantalla ----------

    private TextView texto(String t, int sp, boolean negritas) {
        TextView v = new TextView(this);
        v.setText(t);
        v.setTextSize(TypedValue.COMPLEX_UNIT_SP, sp);
        if (negritas) v.setTypeface(Typeface.DEFAULT_BOLD);
        return v;
    }

    private Button boton(String t, View.OnClickListener alTocar) {
        Button b = new Button(this);
        b.setText(t);
        b.setAllCaps(false);
        b.setTextSize(TypedValue.COMPLEX_UNIT_SP, 17);
        b.setOnClickListener(alTocar);
        return b;
    }

    private CheckBox casilla(String t, String clave) {
        CheckBox c = new CheckBox(this);
        c.setText(t);
        c.setTextSize(TypedValue.COMPLEX_UNIT_SP, 15);
        c.setChecked(prefs.getBoolean(clave, true));
        c.setOnCheckedChangeListener((caja, si) -> {
            prefs.edit().putBoolean(clave, si).apply();
            avisarAjustes();
        });
        return c;
    }

    private LinearLayout.LayoutParams margen() {
        LinearLayout.LayoutParams p = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        p.topMargin = dp(12);
        return p;
    }

    private int dp(int v) {
        return (int) (v * getResources().getDisplayMetrics().density + 0.5f);
    }
}

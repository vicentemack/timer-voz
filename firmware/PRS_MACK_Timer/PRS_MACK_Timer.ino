// PRS MACK - Timer y contador M5StickS3 - v3.32 (26-nov-2026)
// PANTALLA DE TIMER (la normal), en vertical: arriba los tiros en grande,
// abajo los segundos que quedan y hasta abajo 5 barritas de bateria.
//  - Tiempo de la etapa: 105 o 120 s; se escoge en la pantalla de prueba.
//  - Boton azul del frente:
//      listo (blanco, en 105 o 120): un clic suena 3 bips y abajo cuenta ESPERA 5 a 1:
//        tienes 5 s para acomodarte y quedarte quieto (v3.29). Luego suenan otros 3 bips,
//        los numeros se ponen verdes y abajo dice ARMADO: el primer movimiento del rifle de
//        1 g o mas (al bajarlo para acomodarlo en la estacion) arranca el timer con 6 bips
//        cortitos. En la espera o armado, otro clic arranca a mano y apretado 1.5 s lo desarma;
//      corriendo: dejarlo apretado 1.5 s regresa al inicio (un roce no borra nada);
//      en 0: un clic regresa al inicio.
//    Al regresar al inicio los tiros quedan en 0 y espera otro clic para armarlo.
//  - Bips (v3.29): 3 al picar el azul y 3 cuando queda armado, de 0.12 s con 0.12 s de pausa;
//    6 cortitos al arrancar el tiempo (0.07 s con 0.075 s de pausa, ~0.8 s); y 6 al acabarse
//    el tiempo, de 0.12 s con 0.12 s de pausa (como desde la v3.23).
//    Ya no suenan a los 30, 60 ni 90 s: mientras suena un bip el microfono se apaga y un
//    tiro en ese momento no contaria.
//  - Color de toda la pantalla, tiros y tiempo (v3.21): verde los primeros 60 s y
//    amarillo hasta que quedan 15 s, con numeros negros; rojo los ultimos 15 s y
//    en 0, con numeros blancos. Listo para arrancar: numeros blancos sobre negro.
//    Una franja negra separa la bateria, para no confundir los colores.
//  - Parpadeo desde el minuto (v3.28): en amarillo y en rojo, cada medio segundo se
//    invierte: fondo negro con los numeros del color (amarillo o rojo). Los numeros
//    siempre se leen. En 0 ya no parpadea.
//  - Tiros: del 11 en adelante el numero parpadea, un cuarto de segundo se ve y un
//    cuarto de segundo no (v3.23), para saber que ya te pasaste (ya no valen).
//  - Tiros una linea de su letra mas abajo (v3.22): 12 px con una cifra y 11 px con
//    dos, para que no queden pegados al borde de arriba.
//  - Bateria: 5 barritas; verdes con 4 o 5, amarillas con 2 o 3, roja con 1.
//    Usa la lectura de porcentaje de M5Stack, que todavia no esta comprobada.
//  - Solo cuenta tiros mientras corre el tiempo; al terminar, el numero se queda.
// PANTALLA DE PRUEBA (para calibrar en el campo), en horizontal: un clic al boton
// lateral cambia de pantalla. Se refresca 4 veces por segundo.
//  - Ahi un clic al azul borra maximos, tiros y ANT.
//  - Botones sin ruido (v3.25): el clic al apretar y al soltar cualquier boton satura
//    el microfono (32,752 aun con ganancia de 20%, medido el 25-nov-2026). Ahora no
//    oye 0.3 s despues de cada uno y quita lo que oyo justo antes, asi que la M, el
//    ANT y el registro ya no se llenan con el clic del boton.
//  - Doble clic al azul baja la ganancia del microfono 20%: 100, 80, 60, 40, 20
//    y regresa a 100 (cambia ~1.5 s despues del doble clic). En realidad son
//    100, 81, 62, 38 y 19%, porque M5Unified solo acepta pasos enteros.
//  - Dejar apretado el azul 1.5 s cambia el tiempo de la etapa entre 105 y 120 s
//    (solo si el timer no esta corriendo).
//  - Dejar apretado el lateral 1 s sube el umbral de golpe 1 g
//    (1, 2, 3 ... 10; despues de 10 regresa a 1).
//  - Triple clic al azul (con el timer parado) pregunta BORRAR REGISTRO?: un clic al
//    lateral lo borra; el azul o 5 s sin tocar nada lo cancelan.
//  - En amarillo (v3.24), lo que se cambia en el aparato: el umbral de golpe (g),
//    el tiempo de la etapa y la ganancia.
//    Umbral, tiempo y ganancia se guardan en la memoria del aparato aunque lo
//    apagues, y solo se cambian en esta pantalla, para que no cambien sin verlos.
//  - En verde clarito (v3.26), la linea ULT: sonido y golpe del ultimo tiro que conto.
//  - Linea TIROS: tambien muestra REG, cuantos eventos hay en el registro.
//  - Linea SEG: tiempo de la etapa y ganancia (GAN).
//  - Linea ANT: que tan fuerte venia el sonido en los 10 ms antes del ultimo
//    cruce arriba de 30,000, y si lo acepto (RAPIDO) o no (LENTO).
//  - Linea IMU: lecturas del acelerometro por segundo, voltaje de la bateria y %.
//    M5Unified saca el % del voltaje en linea recta: 3.30 V = 0%, 4.10 V = 100%.
// REGISTRO DE TIROS (v3.17): cada evento se guarda en la memoria del aparato
// (archivo /tiros.csv; no se borra al apagarlo), cuente o no cuente como tiro.
// Un evento se abre con un sonido RAPIDO o con un golpe de 2 g o mas, y 80 ms
// despues se guarda con: numero, etapa, segundo dentro de la etapa, pantalla,
// si conto, tiros, sonido maximo, nivel de antes, RAPIDO/LENTO, g maximo,
// umbral de g y ganancia. Un ruido LENTO sin golpe no se guarda (viento, voz).
// Para bajarlo: cable USB + monitor serie de Arduino, escribir LOG y Enter.
// BORRAR (y Enter) borra el registro y reinicia el numero de etapa; desde el aparato,
// triple clic al azul en la pantalla de prueba y confirmar con el lateral (v3.20).
// Desde la v3.25, borrar el registro tambien borra las M (MIC y ACC), ULT y ANT.
// APAGADO AUTOMATICO (v3.27): se apaga 5 minutos despues de prenderlo, haga lo que haga,
// para cuidar la bateria (250 mAh). Una etapa, desde que entras a la estacion hasta que
// terminas, no pasa de 5 minutos: prenderlo es parte de la rutina de entrada. Nunca se
// apaga a media etapa: si a los 5 minutos el timer sigue corriendo, se apaga 30 s despues
// de que termine. No se apaga con el cable USB conectado (computadora, cargador o power
// bank). Se prende con un clic al boton de encendido; prendido, ese clic lo reinicia y
// empiezan otros 5 minutos. (De la v3.18 a la v3.26 eran 5 minutos sin uso.)
// Tampoco se apaga armado (v3.28), hasta 5 minutos despues de armarlo.
// Desde la v3.31, al terminar la etapa se apaga 30 s despues de los bips del final (para que
// alcances a ver tus tiros), aunque no hayan pasado 5 minutos; con cable USB no se apaga.
// Para la siguiente etapa lo prendes y el telefono se vuelve a conectar solo.
// REGISTRO DE BATERIA (v3.19): sin cable USB, cada minuto anota en /bateria.csv el
// voltaje y el % de la bateria (no anota mientras corre una etapa, para no distraer
// al programa); tambien al prender y al apagarse solo. Por el cable
// USB: BAT lo baja, BORRARBAT lo borra y PRUEBABAT quita el apagado automatico hasta
// que se vuelva a prender (para medir cuanto dura la bateria).
// Deteccion de tiro: sonido + golpe dentro de 50 ms, en cualquier orden
// (el golpe llega primero; el sonido tarda ~2.9 ms por metro).
//  - Sonido: cruza arriba de 30,000 con un brinco de por lo menos 10,000 en menos
//    de 2 ms, empiece donde empiece: la muestra que cruza menos lo mas alto de los
//    10 ms de antes (sin contar los 2 ms justo antes). Un ruido que sube despacio
//    no cuenta como sonido.
//  - Golpe: arriba del umbral elegido (de fabrica 4 g).
// Umbrales elegidos por Vicente el 25-nov-2026 (sonido 30,000 con brinco de 10,000 en 2 ms: v3.14).
// MEDICION: por el cable USB avisa cada sonido fuerte (SON, con su nivel de antes
// y si fue RAPIDO o LENTO) y cada golpe (GOL) con su hora en ms, y en cada tiro
// el retraso del sonido (dt).
// Mientras suena un bip el microfono se apaga: comparte el codec con la bocina.
// Procesador a 80 MHz para ahorrar bateria (de fabrica corre a 240 MHz).
// NOMBRE (v3.32): cada timer se anuncia con su propio nombre, TimerS3-XXXX, con las ultimas 4
// cifras de su direccion Bluetooth; en la pantalla del timer, en listo, se ve abajo en gris.
// Asi varios timers en la misma escuadra no se confunden. Para un nombre fijo, ponlo en BLE_NOMBRE.
// TELEFONO (v3.30): por Bluetooth de bajo consumo (BLE) se anuncia por su nombre y le avisa
// al telefono: LISTO al picar el azul, ARMADO a los 5 s, INICIO al arrancar, Q60, Q45, Q25
// y Q10 cuando quedan 60, 45, 25 y 10 s, y desde la v3.31 Q9 a Q1 (la cuenta de los ultimos
// 10 s), FIN en 0, ALTO al regresar al inicio y APAGADO antes de apagarse solo. Cada segundo
// manda el estado: S,segundos,estado,tiros,barras
// (estado L listo, E espera de 5 s, A armado, C corriendo, T termino).
// El telefono (pagina o app) lo dice por los audifonos Bluetooth (v3.31): "preparate" al
// picar el azul, "armado" a los 5 s, 6 pitidos cortos al arrancar, "sesenta", "cuarenta y
// cinco", "veinticinco", la cuenta de diez a uno y 6 pitidos largos en 0. Mandar avisos no
// apaga el microfono: solo los bips de la bocina del StickS3 lo apagan. Con el telefono
// conectado sale un punto azul abajo a la derecha, en la franja negra.
#include <M5Unified.h>
#include <Preferences.h>
#include <LittleFS.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <esp_mac.h>

// ===== Ajustes (los umbrales se confirman en el campo) =====
const int      TIEMPOS_ETAPA[]  = {105, 120};   // segundos de la etapa que se pueden escoger
const int      N_TIEMPOS        = 2;
int            iTiempo          = 0;        // 105 s, si nunca se ha cambiado
const int      GANANCIAS[]      = {100, 80, 60, 40, 20};   // ganancia del microfono en % (100 = de fabrica)
const uint8_t  MAGNIFICACION[]  = {16, 13, 10, 6, 3};      // valor para M5Unified (16 = de fabrica)
const int      N_GANANCIAS      = 5;
int            iGanancia        = 0;        // 100%, si nunca se ha cambiado
const uint32_t AMARILLO_MS      = 60000;    // de verde a amarillo a los 60 s
const uint32_t ROJO_ULTIMOS_MS  = 15000;    // rojo los ultimos 15 s
const int      TIROS_VALIDOS    = 10;       // del 11 en adelante el numero parpadea
const uint32_t PARPADEO_MS      = 250;      // un cuarto de segundo se ve, un cuarto de segundo no
const uint32_t PARPADEO_FONDO_MS = 500;     // v3.28: desde el minuto, los colores se invierten cada medio segundo
const float    UMBRAL_ARRANQUE  = 1.0;      // v3.28: armado, un movimiento de esto o mas (g) arranca el timer
const uint32_t ESPERA_MS        = 5000;     // v3.29: despues de los 3 bips del clic, 5 s para acomodarte (no siente el movimiento)
const uint32_t BATERIA_CADA_MS  = 5000;     // cada cuanto revisa la bateria
const uint16_t REGRESAR_MS      = 1500;     // azul apretado este tiempo: regresa al inicio o cambia el tiempo
const uint32_t PRUEBA_CADA_MS   = 250;      // refresco de la pantalla de prueba
const uint32_t CONFIRMAR_MS     = 5000;     // tiempo para confirmar el borrado del registro
const uint32_t APAGADO_MS       = 5UL * 60UL * 1000UL;   // v3.27: se apaga 5 minutos despues de prenderlo, haga lo que haga
const uint32_t GRACIA_MS        = 30000;    // si la etapa seguia a los 5 minutos, se apaga 30 s despues de que termine
const int      USB_MV           = 4000;     // arriba de esto (mV) hay cable USB: no se apaga solo
const uint32_t BAT_CADA_MS      = 60000;    // bateria: anota el voltaje cada minuto sin cable USB
const char*    ARCHIVO_BAT      = "/bateria.csv";
const size_t   BAT_MAX          = 100000;   // bateria: deja de anotar arriba de ~100 kB (~50 horas)
const int      UMBRAL_SONIDO    = 30000;    // pico del microfono interno (tope ~32,767)
const int      SUBIDA_MS        = 2;        // el brinco debe pasar en menos de esto...
const int      ANTES_MS         = 10;       // ...comparado con lo mas alto de estos ms de antes
const int      SALTO_MIN        = 10000;    // brinco minimo: lo que cruza menos lo de antes
const float    UMBRALES_GOLPE[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};  // en g, de 1 en 1
const int      N_UMBRALES       = 10;
const int      UMBRAL_FABRICA   = 3;        // 4 g, si nunca se ha cambiado
int            iUmbral          = UMBRAL_FABRICA;
const int      VENTANA_MS       = 50;
const int      TIEMPO_MUERTO    = 300;
const float    EVENTO_G         = 2.0;      // registro: un golpe de esto o mas abre un evento
const uint32_t EVENTO_MS        = 80;       // registro: el evento se guarda 80 ms despues de abrirse
const char*    ARCHIVO          = "/tiros.csv";
const size_t   REGISTRO_MAX     = 1000000;  // registro: deja de guardar arriba de ~1 MB (~15,000 eventos)
const int      REGISTRO_SONIDO  = 15000;    // medicion: avisa sonidos arriba de esto
const float    REGISTRO_GOLPE   = 1.0;      // medicion: avisa golpes arriba de esto (g)
const uint32_t BOTON_MS         = 300;      // v3.25: no oye esto despues de apretar o soltar un boton
const uint16_t VERDE_CLARO      = 0x87F0;   // verde clarito (128, 255, 128) para la linea ULT (v3.26)
const uint16_t AZUL_BLE         = 0x2D1F;   // v3.30: azul clarito (40, 160, 255): telefono conectado
const char*    BLE_NOMBRE       = "";          // v3.32: vacio = TimerS3-XXXX (unico); o un nombre fijo, ej. "TimerS3R"
char           bleNombre[20]    = "TimerS3";   // v3.32: el nombre con el que lo encuentra el telefono
#define BLE_SERVICIO "6f3c0001-5b1a-4c7e-9d2a-3e1f5a7b9c10"   // v3.30: servicio de avisos al telefono
#define BLE_AVISOS   "6f3c0002-5b1a-4c7e-9d2a-3e1f5a7b9c10"   // v3.30: avisos (lectura y notificacion)
const int      AVISOS_SEG[]     = {60, 45, 25, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};   // v3.31: 60, 45, 25 y la cuenta de 10 a 1
const int      N_AVISOS         = sizeof(AVISOS_SEG) / sizeof(AVISOS_SEG[0]);
const int      BIP_HZ           = 2500;
const int      BIP_CORTO_MS     = 120;      // cada bip dura 0.12 s
const int      PAUSA_BIP_MS     = 120;      // y 0.12 s de pausa entre bips
const int      BIPS_AVISO       = 3;        // v3.29: 3 bips al picar el azul y 3 cuando queda armado
const int      BIPS_ARRANQUE    = 6;        // v3.29: 6 bips cortitos al arrancar el tiempo (~0.8 s)
const int      BIP_ARRANQUE_MS  = 70;       // v3.29: cada bip del arranque dura 0.07 s...
const int      PAUSA_ARRANQUE_MS = 75;      // ...con 0.075 s de pausa
const int      BIPS_FIN         = 6;        // bips al acabarse el tiempo (v3.23; antes uno largo)
const uint8_t  VOLUMEN          = 190;      // ~74%: el maximo que M5Stack recomienda con bateria (menos de 75%)
const uint8_t  GIRO_TIMER       = 0;        // vertical con el USB abajo (2 = al reves)
const uint8_t  GIRO_PRUEBA      = 1;        // horizontal
const int      Y_TIROS          = 0;        // pantalla del timer (alto 240): de 0 a 198 un solo
const int      ALTO_TIROS       = 96;       // color, con los tiros arriba
const int      BAJA_TIROS       = 1;        // los tiros bajan 1 linea de su letra (v3.22)
const int      Y_TIEMPO         = 102;      // y los segundos abajo;
const int      ALTO_TIEMPO      = 96;
const int      Y_BATERIA        = 220;      // franja negra de 198 a 220 y bateria hasta abajo
const int      ALTO_BATERIA     = 18;
const uint32_t CPU_MHZ          = 80;       // 80, 160 o 240
// ====================================================================

Preferences memoria;                        // guarda umbral, tiempo, ganancia y etapa aunque se apague

const uint32_t I2C_FREQ  = 400000;
const float    LSB_POR_G = 2048.0f;         // rango +/-16 g
uint8_t bmi = 0;
bool imuRapido = false;

const int N_MUESTRAS = 256;                 // 16 ms de audio a 16 kHz
const int FRECUENCIA = 16000;
const int N_SUBIDA   = SUBIDA_MS * FRECUENCIA / 1000;   // 32 muestras
const int N_ANTES    = ANTES_MS * FRECUENCIA / 1000;    // 160 muestras
const int N_HIST     = N_SUBIDA + N_ANTES;              // lo que se guarda del bloque anterior
int16_t audio[N_MUESTRAS];
uint16_t nivel[N_HIST + N_MUESTRAS];        // sonido sin signo: final del bloque anterior + este
int  antesBloque = -1;                      // nivel de antes del cruce en este bloque (-1 = no cruzo)
int  antesUlt = -1;                         // lo mismo del ultimo cruce, para la pantalla de prueba
bool rapidoUlt = false;                     // si ese cruce fue un brinco grande y rapido

enum Estado { LISTO, ARMADO, CORRIENDO, TERMINADO };   // ARMADO (v3.28): espera el movimiento del rifle
Estado estado = LISTO;
bool modoPrueba = false;
bool redibujar = true;
uint32_t tInicio = 0;

int   picoMic = 0, maxMic = 0, micVentana = 0, tiroMic = 0;
float golpe = 0, maxGolpe = 0, golpeVentana = 0, tiroGolpe = 0;
int   maxMicPrevio = 0, antesUltPrevio = -1;   // v3.25: como estaban antes de lo ultimo que oyo,
float maxGolpePrevio = 0;                      // para quitar el clic de un boton
bool  rapidoUltPrevio = false;
bool  haySonido = false, hayGolpe = false;
uint32_t tSonido = 0, tGolpe = 0, tTiro = 0, silencioHasta = 0;
uint32_t tPrimerSonido = 0, tPrimerGolpe = 0, tRegSon = 0, tRegGol = 0;
int   tiros = 0, ultTiros = -1, ultSeg = -1, ultBarras = -1;
uint16_t ultColor = 0, ultColorBat = 0, ultLetra = 0;
int   ultFranja = -1;                      // v3.28: la franja negra dice ARMADO (1), PRUEBA BATERIA (2) o nada (0)
bool  ultVisibles = true;                   // parpadeo de los tiros (del 11 en adelante)
int16_t previo[3] = {0, 0, 0};
uint32_t lecturas = 0, hzImu = 0, tCuenta = 0, tPantalla = 0, tSerie = 0, tBateria = 0;
uint32_t tFinEtapa = 0;                     // v3.27: cuando termino la ultima etapa (30 s para ver los tiros)
uint32_t tNoApagar = 0;                     // v3.27: no revisa el apagado antes de esto
bool  avisoUsbFin = false;                  // v3.31: ya aviso por el cable que con USB no se apaga al terminar
uint32_t tArmado = 0;                       // v3.28: cuando se armo
bool  movArranque = false;                  // v3.28: ya llego el movimiento que arranca el timer
uint32_t tMovArranque = 0;                  // v3.28: cuando llego ese movimiento
float gArranque = 0;                        // v3.28: de cuantos g fue
bool  avisoArmado = false;                  // v3.29: ya pasaron los 5 s (numeros verdes y ARMADO)
uint32_t tSiente = 0xFFFFFFFF;              // v3.29: desde cuando el movimiento arranca (al terminar los 3 bips)
BLECharacteristic* bleCarac = nullptr;      // v3.30: avisos al telefono por Bluetooth
volatile bool bleConectado = false;         // v3.30: el telefono esta conectado
volatile bool bleReanunciar = false;        // v3.30: se desconecto: anunciarse otra vez
bool  bleAntes = false;                     // v3.30: para avisar por el cable cuando cambia
int   avisosDados = 0;                      // v3.30: avisos de tiempo ya mandados en esta etapa
uint32_t tBleEstado = 0;                    // v3.30: ultima vez que mando el estado al telefono

// Registro de eventos
bool fsOk = false;                          // si la memoria de archivos arranco bien
int  etapaN = 0;                            // numero de etapa (sube cada vez que arranca el timer)
int  eventosGuardados = 0;                  // cuantos eventos hay en el archivo
bool evAbierto = false, evCruce = false, evRapido = false, evContado = false;
uint32_t tEvento = 0;
int  evMic = 0, evAntes = -1;
float evG = 0;

// Registro de bateria
int  batSesion = 0;                         // numero de encendido (sube cada vez que prende)
uint32_t tBat = 0;                          // ultima vez que anoto la bateria
bool sinApagado = false;                    // PRUEBABAT: sin apagado automatico hasta que se vuelva a prender
bool confirmarBorrar = false;               // esperando el si o el no para borrar el registro de tiros
uint32_t tConfirmar = 0;

int decimasDeG(int i) { return (int)(UMBRALES_GOLPE[i] * 10 + 0.5f); }   // 4 g -> 40
uint32_t totalMs() { return (uint32_t)TIEMPOS_ETAPA[iTiempo] * 1000; }   // 105 s -> 105000
bool hayUsb() { return M5.Power.getVBUSVoltage() > USB_MV; }              // cable USB conectado

class AvisosBle : public BLEServerCallbacks {   // v3.30: el telefono se conecta o se va
  void onConnect(BLEServer* s) override { bleConectado = true; }
  void onDisconnect(BLEServer* s) override { bleConectado = false; bleReanunciar = true; }
};

void iniciarBle() {                         // v3.30: se anuncia como TimerS3 para que lo encuentre el telefono
  if (BLE_NOMBRE[0]) {                      // v3.32: nombre fijo
    snprintf(bleNombre, sizeof(bleNombre), "%s", BLE_NOMBRE);
  } else {                                  // v3.32: TimerS3-XXXX con el final de su direccion Bluetooth
    uint8_t mac[6] = {0, 0, 0, 0, 0, 0};
    esp_read_mac(mac, ESP_MAC_BT);
    snprintf(bleNombre, sizeof(bleNombre), "TimerS3-%02X%02X", mac[4], mac[5]);
  }
  BLEDevice::init(bleNombre);
  BLEServer* servidor = BLEDevice::createServer();
  servidor->setCallbacks(new AvisosBle());
  BLEService* servicio = servidor->createService(BLE_SERVICIO);
  bleCarac = servicio->createCharacteristic(BLE_AVISOS,
               BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  bleCarac->addDescriptor(new BLE2902());
  bleCarac->setValue((uint8_t*)"HOLA", 4);
  servicio->start();
  BLEAdvertising* anuncio = BLEDevice::getAdvertising();
  anuncio->addServiceUUID(BLE_SERVICIO);
  anuncio->setScanResponse(true);
  BLEDevice::startAdvertising();
}

void bleMandar(const char* m, bool imprimir) {   // v3.30: manda un aviso al telefono, si esta conectado
  if (bleCarac == nullptr) return;
  bleCarac->setValue((uint8_t*)m, strlen(m));
  if (bleConectado) bleCarac->notify();
  if (imprimir) Serial.printf("BLE> %s%s\n", m, bleConectado ? "" : " (sin telefono)");
}

void bleAvisar(const char* m) { bleMandar(m, true); }

void aplicarGanancia() {                    // se aplica la proxima vez que arranca el microfono
  auto mc = M5.Mic.config();
  mc.magnification = MAGNIFICACION[iGanancia];
  M5.Mic.config(mc);
}

bool iniciarImuRapido() {
  const uint8_t dirs[2] = {0x68, 0x69};
  for (int i = 0; i < 2; i++) {
    if (M5.In_I2C.readRegister8(dirs[i], 0x00, I2C_FREQ) == 0x24) {  // CHIP_ID del BMI270
      bmi = dirs[i];
      break;
    }
  }
  if (bmi == 0) return false;
  M5.In_I2C.writeRegister8(bmi, 0x7C, 0x00, I2C_FREQ);         // PWR_CONF: sin ahorro avanzado
  delay(2);
  uint8_t pwr = M5.In_I2C.readRegister8(bmi, 0x7D, I2C_FREQ);
  M5.In_I2C.writeRegister8(bmi, 0x7D, pwr | 0x04, I2C_FREQ);   // PWR_CTRL: acelerometro encendido
  delay(2);
  M5.In_I2C.writeRegister8(bmi, 0x41, 0x03, I2C_FREQ);         // ACC_RANGE: +/-16 g
  M5.In_I2C.writeRegister8(bmi, 0x40, 0xAC, I2C_FREQ);         // ACC_CONF: 1600 Hz, modo rendimiento
  delay(2);
  return M5.In_I2C.readRegister8(bmi, 0x40, I2C_FREQ) == 0xAC &&
         M5.In_I2C.readRegister8(bmi, 0x41, I2C_FREQ) == 0x03;
}

int contarEventos() {                       // cuenta las lineas del archivo (menos la de titulos)
  if (!fsOk || !LittleFS.exists(ARCHIVO)) return 0;
  File f = LittleFS.open(ARCHIVO, FILE_READ);
  if (!f) return 0;
  int lineas = 0;
  uint8_t buf[256];
  while (f.available()) {
    int n = f.read(buf, sizeof(buf));
    for (int i = 0; i < n; i++) {
      if (buf[i] == '\n') lineas++;
    }
  }
  f.close();
  return lineas > 0 ? lineas - 1 : 0;
}

void guardarEvento() {                      // agrega una linea al archivo del registro
  if (!fsOk) return;
  bool nuevo = !LittleFS.exists(ARCHIVO);
  File f = LittleFS.open(ARCHIVO, FILE_APPEND);
  if (!f) return;
  if (f.size() > REGISTRO_MAX) {           // memoria llena: ya no guarda
    f.close();
    return;
  }
  if (nuevo) f.println("n,etapa,seg,pantalla,conto,tiros,sonido,antes,tipo,g,umbral_g,ganancia");
  eventosGuardados++;
  char seg[12] = "";
  if (estado == CORRIENDO && tEvento >= tInicio) {
    snprintf(seg, sizeof(seg), "%.2f", (tEvento - tInicio) / 1000.0f);
  }
  f.printf("%d,%d,%s,%s,%s,%d,%d,%d,%s,%.2f,%d,%d\n", eventosGuardados,
           estado == CORRIENDO ? etapaN : 0, seg, modoPrueba ? "PRUEBA" : "TIMER",
           evContado ? "SI" : "NO", tiros, evMic, evAntes,
           evRapido ? "RAPIDO" : (evCruce ? "LENTO" : "-"), evG,
           (int)UMBRALES_GOLPE[iUmbral], GANANCIAS[iGanancia]);
  f.close();
}

void registrarEvento(uint32_t ahora, int subidaCiclo, int antesCiclo, bool contadoAhora) {
  if (!evAbierto && ahora > silencioHasta && (subidaCiclo >= 0 || golpe >= EVENTO_G)) {   // abre un evento
    evAbierto = true;
    tEvento = ahora;
    evMic = 0;
    evAntes = -1;
    evCruce = false;
    evRapido = false;
    evG = 0;
    evContado = false;
  }
  if (!evAbierto) return;
  if (picoMic > evMic) evMic = picoMic;
  if (golpe > evG) evG = golpe;
  if (antesCiclo >= 0 && !evRapido) {      // nivel de antes del cruce de 30,000
    evCruce = true;
    evAntes = antesCiclo;
  }
  if (subidaCiclo >= 0) evRapido = true;
  if (contadoAhora) evContado = true;
  if (ahora - tEvento >= EVENTO_MS) {      // se cierra y se guarda
    guardarEvento();
    evAbierto = false;
  }
}

void borrarMaximos() {                      // borra las M (MIC y ACC), ULT y ANT de la pantalla de prueba
  maxMic = 0;
  maxGolpe = 0;
  tiroMic = 0;
  tiroGolpe = 0;
  antesUlt = -1;
}

void borrarRegistro() {                     // borra el registro de tiros y reinicia el numero de etapa
  if (fsOk) LittleFS.remove(ARCHIVO);
  eventosGuardados = 0;
  etapaN = 0;
  memoria.putInt("etapa_n", etapaN);
  borrarMaximos();                          // v3.25: tambien la M, ULT y ANT
  Serial.println("REGISTRO BORRADO");
}

void guardarBateria(const char* nota) {     // agrega una linea al registro de bateria
  if (!fsOk) return;
  bool nuevo = !LittleFS.exists(ARCHIVO_BAT);
  File f = LittleFS.open(ARCHIVO_BAT, FILE_APPEND);
  if (!f) return;
  if (f.size() > BAT_MAX) {                // ya no cabe: deja de anotar
    f.close();
    return;
  }
  if (nuevo) f.println("sesion,seg,mV,pct,usb,carga,estado,nota");
  const char* est = (estado == CORRIENDO) ? "CORRIENDO" : (estado == TERMINADO ? "TERMINADO" :
                    (estado == ARMADO ? "ARMADO" : "LISTO"));
  f.printf("%d,%lu,%d,%d,%d,%d,%s,%s\n", batSesion, (unsigned long)(millis() / 1000),
           (int)M5.Power.getBatteryVoltage(), (int)M5.Power.getBatteryLevel(),
           hayUsb() ? 1 : 0, M5.Power.isCharging() ? 1 : 0, est, nota);
  f.close();
}

void revisarBateria() {                     // sin cable USB, anota la bateria cada minuto
  if (millis() - tBat < BAT_CADA_MS) return;
  tBat = millis();
  if (estado != CORRIENDO && !hayUsb()) guardarBateria("MIN");   // durante la etapa no escribe
}

void imprimirArchivo(const char* nombre) {  // manda un archivo por el cable USB
  if (!fsOk || !LittleFS.exists(nombre)) return;
  File f = LittleFS.open(nombre, FILE_READ);
  uint8_t buf[256];
  while (f && f.available()) {
    int n = f.read(buf, sizeof(buf));
    Serial.write(buf, n);
  }
  if (f) f.close();
}

void revisarComandos() {                    // ordenes por el cable USB: LOG, BORRAR, BAT, BORRARBAT, PRUEBABAT, REINICIAR
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toUpperCase();
  if (cmd == "LOG") {
    Serial.printf("=== REGISTRO: %d eventos ===\n", eventosGuardados);
    imprimirArchivo(ARCHIVO);
    Serial.println("=== FIN DEL REGISTRO ===");
  } else if (cmd == "BORRAR") {
    borrarRegistro();
  } else if (cmd == "BAT") {
    Serial.printf("=== BATERIA: sesion %d, ahora %d mV %d%% usb=%d carga=%d ===\n", batSesion,
                  (int)M5.Power.getBatteryVoltage(), (int)M5.Power.getBatteryLevel(),
                  hayUsb() ? 1 : 0, M5.Power.isCharging() ? 1 : 0);
    imprimirArchivo(ARCHIVO_BAT);
    Serial.println("=== FIN BATERIA ===");
  } else if (cmd == "BORRARBAT") {
    if (fsOk) LittleFS.remove(ARCHIVO_BAT);
    batSesion = 0;
    memoria.putInt("bat_sesion", batSesion);
    Serial.println("BATERIA BORRADA");
  } else if (cmd == "PRUEBABAT") {
    sinApagado = true;
    redibujar = true;
    Serial.println("PRUEBA DE BATERIA: sin apagado automatico hasta que se vuelva a prender");
  } else if (cmd == "REINICIAR") {          // v3.30: para probar que arranca bien varias veces
    Serial.println("REINICIANDO");
    delay(100);
    ESP.restart();
  }
}

void revisarApagado() {                     // v3.27: se apaga 5 minutos despues de prenderlo, haga lo que haga
  uint32_t ahora = millis();
  if (sinApagado || ahora < tNoApagar) return;   // PRUEBABAT, o lo vuelve a revisar mas tarde
  if (estado == CORRIENDO) return;          // nunca a media etapa
  bool porFin = (estado == TERMINADO);      // v3.31: termino la etapa: se apaga 30 s despues, aunque no pasen 5 minutos
  if (porFin) {
    if (ahora - tFinEtapa < GRACIA_MS) return;                    // 30 s para ver los tiros
  } else {
    if (ahora < APAGADO_MS) return;                               // todavia no pasan 5 minutos
    if (estado == ARMADO && ahora - tArmado < APAGADO_MS) return; // v3.28: armado tampoco (hasta 5 minutos)
    if (tFinEtapa > 0 && ahora - tFinEtapa < GRACIA_MS) return;   // acaba de terminar una etapa: 30 s para ver los tiros
  }
  if (hayUsb()) {                           // con cable USB conectado no se apaga
    if (porFin && !avisoUsbFin) {           // v3.31: lo avisa una vez por el cable, para probar en la mesa
      avisoUsbFin = true;
      Serial.println("FIN + 30 s: aqui se apagaria; con cable USB no se apaga");
    }
    tNoApagar = ahora + 5000;               // lo vuelve a revisar en 5 s
    return;
  }
  Serial.println(porFin ? "APAGADO AUTOMATICO: 30 s despues de terminar la etapa"
                        : "APAGADO AUTOMATICO: 5 minutos desde que se prendio");
  bleAvisar("APAGADO");                     // v3.30: el telefono sabe que se apago
  if (evAbierto) {
    guardarEvento();
    evAbierto = false;
  }
  guardarBateria("APAGADO");
  M5.Display.setRotation(GIRO_TIMER);
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(19, 110);
  M5.Display.print("APAGANDO");
  delay(1500);
  M5.Power.powerOff();                     // con M5Unified 0.3.0 corta la corriente y no regresa
  M5.Display.wakeup();                     // si algun dia regresa (no se apago), sigue trabajando
  tNoApagar = millis() + 60000;            // y no lo vuelve a intentar en 1 minuto
  M5.Display.setRotation(modoPrueba ? GIRO_PRUEBA : GIRO_TIMER);
  M5.Display.fillScreen(TFT_BLACK);
  redibujar = true;
}

void procesarGolpe(float gx, float gy, float gz) {
  lecturas++;
  float imp = fabsf(sqrtf(gx * gx + gy * gy + gz * gz) - 1.0f);  // resta 1 g de gravedad
  if (imp > golpe) golpe = imp;
  uint32_t ahora = millis();
  if (estado == ARMADO && avisoArmado && !movArranque && imp >= UMBRAL_ARRANQUE && ahora >= tSiente) {
    movArranque = true;                                      // v3.28: el primer movimiento arranca el timer
    tMovArranque = ahora;                                    // (lo atiende el loop)
    gArranque = imp;
  }
  if (imp > REGISTRO_GOLPE && ahora - tRegGol > 200) {        // medicion: primer instante del golpe
    tRegGol = ahora;
    Serial.printf("GOL t=%lu g=%.2f\n", (unsigned long)ahora, imp);
  }
  if (imp > UMBRALES_GOLPE[iUmbral]) {
    if (!hayGolpe) tPrimerGolpe = ahora;
    if (!hayGolpe || imp > golpeVentana) golpeVentana = imp;
    hayGolpe = true;
    tGolpe = ahora;
  }
}

void leerImu() {
  if (imuRapido) {
    uint8_t b[6];
    if (!M5.In_I2C.readRegister(bmi, 0x0C, b, 6, I2C_FREQ)) return;
    int16_t x = (int16_t)(b[0] | (b[1] << 8));
    int16_t y = (int16_t)(b[2] | (b[3] << 8));
    int16_t z = (int16_t)(b[4] | (b[5] << 8));
    if (x == previo[0] && y == previo[1] && z == previo[2]) return;  // aun no hay dato nuevo
    previo[0] = x;
    previo[1] = y;
    previo[2] = z;
    procesarGolpe(x / LSB_POR_G, y / LSB_POR_G, z / LSB_POR_G);
  } else if (M5.Imu.update()) {
    auto d = M5.Imu.getImuData();
    procesarGolpe(d.accel.x, d.accel.y, d.accel.z);
  }
}

void esperarLeyendo(uint32_t ms) {
  uint32_t t0 = millis();
  while (millis() - t0 < ms) leerImu();
}

// Busca en el bloque de audio el primer cruce arriba del umbral que venga de un brinco:
// la muestra que cruza supera por SALTO_MIN o mas a lo mas alto de los ANTES_MS previos
// (sin contar los SUBIDA_MS justo antes del cruce). Regresa la muestra, o -1 si no hubo.
int buscarSubida() {
  for (int i = 0; i < N_MUESTRAS; i++) {
    int v = audio[i];
    nivel[N_HIST + i] = (uint16_t)(v < 0 ? -v : v);
  }
  int subida = -1;
  antesBloque = -1;
  for (int i = N_HIST; i < N_HIST + N_MUESTRAS && subida < 0; i++) {
    if (nivel[i] <= UMBRAL_SONIDO || nivel[i - 1] > UMBRAL_SONIDO) continue;   // solo donde cruza hacia arriba
    int antes = 0;
    for (int j = i - N_HIST; j < i - N_SUBIDA; j++) {
      if (nivel[j] > antes) antes = nivel[j];
    }
    antesBloque = antes;
    if ((int)nivel[i] - antes >= SALTO_MIN) subida = i - N_HIST;   // brinco grande y rapido
  }
  memmove(nivel, nivel + N_MUESTRAS, N_HIST * sizeof(nivel[0]));   // el final queda para el siguiente bloque
  return subida;
}

void pitar(int veces, int duracion, int pausa) {
  M5.Mic.end();                    // microfono y bocina comparten el codec
  M5.Speaker.begin();
  M5.Speaker.setVolume(VOLUMEN);
  for (int i = 0; i < veces; i++) {
    M5.Speaker.tone(BIP_HZ, duracion);
    esperarLeyendo(duracion);
    if (i < veces - 1) esperarLeyendo(pausa);
  }
  while (M5.Speaker.isPlaying()) leerImu();
  M5.Speaker.end();
  M5.Mic.begin();
  haySonido = false;
  silencioHasta = millis() + 100;  // ignora el ruido del arranque del microfono
}

int segundosQuedan() {
  if (estado == LISTO || estado == ARMADO) return TIEMPOS_ETAPA[iTiempo];
  if (estado == TERMINADO) return 0;
  uint32_t total = totalMs();
  uint32_t pasado = millis() - tInicio;
  if (pasado >= total) return 0;
  return (total - pasado + 999) / 1000;   // redondea hacia arriba
}

void coloresPantalla(uint16_t &fondo, uint16_t &letra) {   // color de la pantalla (tiros y tiempo) y de los numeros
  if (estado == LISTO)     { fondo = TFT_BLACK; letra = TFT_WHITE; return; }
  if (estado == ARMADO)    { fondo = TFT_BLACK; letra = avisoArmado ? TFT_GREEN : TFT_WHITE; return; }   // v3.29: verde ya armado
  if (estado == TERMINADO) { fondo = TFT_RED;   letra = TFT_WHITE; return; }
  uint32_t pasado = millis() - tInicio;
  if (pasado < AMARILLO_MS) { fondo = TFT_GREEN; letra = TFT_BLACK; return; }   // primeros 60 s
  uint16_t color = TFT_RED;                                       // ultimos 15 s: rojo con numeros blancos
  letra = TFT_WHITE;
  if (pasado < totalMs() - ROJO_ULTIMOS_MS) {                     // hasta que quedan 15 s: amarillo con negros
    color = TFT_YELLOW;
    letra = TFT_BLACK;
  }
  fondo = color;
  if (((pasado - AMARILLO_MS) / PARPADEO_FONDO_MS) % 2 == 1) {    // v3.28: medio segundo al reves:
    fondo = TFT_BLACK;                                            // fondo negro y numeros del color
    letra = color;
  }
}

int segundosEspera() {             // v3.29: segundos que faltan para que quede armado (5 a 1)
  uint32_t pasado = millis() - tArmado;
  if (pasado >= ESPERA_MS) return 0;
  return (ESPERA_MS - pasado + 999) / 1000;
}

void numeroGrande(int valor, int y, int alto, int tamano, uint16_t color, uint16_t fondo, bool visible, int lineasAbajo) {
  char txt[8];
  int n = snprintf(txt, sizeof(txt), "%d", valor);
  int ancho = M5.Display.width();
  if (n * 6 * tamano > ancho) tamano = ancho / (n * 6);   // si no cabe, cifra mas chica
  M5.Display.fillRect(0, y, ancho, alto, fondo);         // fondo de lado a lado
  if (!visible) return;                                  // parpadeo: un cuarto de segundo sin numero
  M5.Display.setTextSize(tamano);                        // cada cifra mide 6 x 8 px por tamano
  M5.Display.setTextColor(color);                        // sin fondo: el bloque ya quedo pintado
  M5.Display.setCursor((ancho - n * 6 * tamano) / 2,
                       y + (alto - 8 * tamano) / 2 + lineasAbajo * tamano);   // cada linea de la letra mide tamano px
  M5.Display.print(txt);
}

void barrasBateria(bool forzar) {
  if (!forzar && millis() - tBateria < BATERIA_CADA_MS) return;
  tBateria = millis();
  int nivelBat = M5.Power.getBatteryLevel();       // 0 a 100 (lectura aproximada)
  if (nivelBat < 0) nivelBat = 0;
  if (nivelBat > 100) nivelBat = 100;
  int barras = (nivelBat + 19) / 20;               // 0 a 5 barritas
  uint16_t color = TFT_GREEN;                      // 4 o 5 barritas
  if (barras <= 3) color = TFT_YELLOW;             // a la mitad
  if (barras <= 1) color = TFT_RED;                // una barrita
  if (!forzar && barras == ultBarras && color == ultColorBat) return;
  ultBarras = barras;
  ultColorBat = color;
  const int y = Y_BATERIA, alto = ALTO_BATERIA, ancho = 22, hueco = 4;
  int x0 = (M5.Display.width() - (5 * ancho + 4 * hueco)) / 2;
  for (int i = 0; i < 5; i++) {
    int x = x0 + i * (ancho + hueco);
    if (i < barras) {
      M5.Display.fillRect(x, y, ancho, alto, color);
    } else {
      M5.Display.fillRect(x, y, ancho, alto, TFT_BLACK);
      M5.Display.drawRect(x, y, ancho, alto, TFT_DARKGREY);   // barrita vacia
    }
  }
}

void pantallaTimer() {
  int seg = segundosQuedan();
  uint16_t fondo, letra;
  coloresPantalla(fondo, letra);
  bool cambioColor = redibujar || fondo != ultColor || letra != ultLetra;
  bool visibles = tiros <= TIROS_VALIDOS ||                                 // del 11 en adelante parpadea,
                  ((millis() - tTiro) / PARPADEO_MS) % 2 == 0;              // y se ve justo al contar el tiro
  if (cambioColor) {                                                        // hueco entre tiros y tiempo
    M5.Display.fillRect(0, Y_TIROS + ALTO_TIROS, M5.Display.width(), Y_TIEMPO - Y_TIROS - ALTO_TIROS, fondo);
  }
  if (cambioColor || tiros != ultTiros || visibles != ultVisibles) {
    numeroGrande(tiros, Y_TIROS, ALTO_TIROS, 12, letra, fondo, visibles, BAJA_TIROS);   // tiros: arriba (con 2 cifras baja a 11)
    ultTiros = tiros;
    ultVisibles = visibles;
  }
  if (cambioColor || seg != ultSeg) {
    numeroGrande(seg, Y_TIEMPO, ALTO_TIEMPO, 7, letra, fondo, true, 0);    // segundos: abajo
    ultSeg = seg;
  }
  ultColor = fondo;
  ultLetra = letra;
  barrasBateria(redibujar);                              // bateria: hasta abajo, en la franja negra
  int aviso = (estado == ARMADO) ? (avisoArmado ? 1 : 10 + segundosEspera()) : (sinApagado ? 2 : 0);   // v3.29
  bool telefono = bleConectado;                          // v3.30: punto azul si el telefono esta conectado
  bool nombre = (estado == LISTO && aviso == 0);         // v3.32: en listo se ve el nombre del timer
  int franja = (aviso * 2 + (telefono ? 1 : 0)) * 2 + (nombre ? 1 : 0);
  if (redibujar || franja != ultFranja) {
    int yFranja = Y_TIEMPO + ALTO_TIEMPO;
    M5.Display.fillRect(0, yFranja, M5.Display.width(), Y_BATERIA - yFranja, TFT_BLACK);
    if (aviso >= 10) {                                   // v3.29: ESPERA 5 a 1, para acomodarte
      char txt[12];
      snprintf(txt, sizeof(txt), "ESPERA %d", aviso - 10);
      M5.Display.setTextSize(2);
      M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
      M5.Display.setCursor((M5.Display.width() - 8 * 12) / 2, yFranja + 3);
      M5.Display.print(txt);
    } else if (aviso == 1) {                             // armado: espera el movimiento del rifle
      M5.Display.setTextSize(2);
      M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
      M5.Display.setCursor((M5.Display.width() - 6 * 12) / 2, yFranja + 3);
      M5.Display.print("ARMADO");
    } else if (aviso == 2) {                             // PRUEBABAT: aviso chico
      M5.Display.setTextSize(1);
      M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
      M5.Display.setCursor((M5.Display.width() - 14 * 6) / 2, 205);
      M5.Display.print("PRUEBA BATERIA");
    }
    if (nombre) {                                        // v3.32: el nombre con el que lo encuentra el telefono
      M5.Display.setTextSize(1);
      M5.Display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      M5.Display.setCursor(4, yFranja + 7);
      M5.Display.print(bleNombre);
    }
    if (telefono) M5.Display.fillCircle(M5.Display.width() - 6, yFranja + 11, 4, AZUL_BLE);   // v3.30
    ultFranja = franja;
  }
  redibujar = false;
}

void colorTexto(uint16_t c) { M5.Display.setTextColor(c, TFT_BLACK); }   // pantalla de prueba: letras sobre negro

void pantallaPrueba() {
  M5.Display.setTextSize(2);                             // 8 lineas de 16 px
  M5.Display.setCursor(0, 2);
  if (confirmarBorrar) {                                 // pregunta antes de borrar el registro
    int quedan = (int)CONFIRMAR_MS - (int)(millis() - tConfirmar);
    if (quedan < 0) quedan = 0;
    M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5.Display.printf("BORRAR REGISTRO?\n");
    M5.Display.printf("%5d eventos\n\n", eventosGuardados);
    M5.Display.printf("LATERAL = SI\n");
    M5.Display.printf("AZUL    = NO\n\n");
    M5.Display.printf("%d s para decidir ", (quedan + 999) / 1000);
    return;
  }
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.printf("TIROS %3d REG %5d\n", tiros, eventosGuardados);
  M5.Display.printf("MIC %5d M%5d \n", picoMic, maxMic);
  M5.Display.printf("ACC %5.2f M%5.2f\n", golpe, maxGolpe);
  colorTexto(VERDE_CLARO);                               // la linea ULT (ultimo tiro) en verde clarito (v3.26)
  M5.Display.printf("ULT %5d %5.2f \n", tiroMic, tiroGolpe);
  colorTexto(TFT_WHITE);
  M5.Display.printf("UMB %5d ", UMBRAL_SONIDO);
  colorTexto(TFT_YELLOW);                                // en amarillo, lo que se cambia en el aparato (v3.24)
  M5.Display.printf("%4.1fg", UMBRALES_GOLPE[iUmbral]);
  colorTexto(TFT_WHITE);
  M5.Display.printf(" \nSEG ");
  colorTexto(TFT_YELLOW);
  M5.Display.printf("%3d s", TIEMPOS_ETAPA[iTiempo]);
  colorTexto(TFT_WHITE);
  M5.Display.printf(" GAN ");
  colorTexto(TFT_YELLOW);
  M5.Display.printf("%3d%%", GANANCIAS[iGanancia]);
  colorTexto(TFT_WHITE);
  M5.Display.printf("\n");
  if (antesUlt < 0) M5.Display.printf("ANT     - ------ \n");
  else M5.Display.printf("ANT %5d %s \n", antesUlt, rapidoUlt ? "RAPIDO" : "LENTO ");
  M5.Display.printf("IMU %4d%c%4.2fV %3d%%", (int)hzImu, imuRapido ? ' ' : '*',
                    M5.Power.getBatteryVoltage() / 1000.0f, (int)M5.Power.getBatteryLevel());
}

void limpiarPrueba() {                      // limpia la pantalla de prueba para redibujarla
  M5.Display.fillScreen(TFT_BLACK);
  tPantalla = 0;
}

void cambiarPantalla() {
  modoPrueba = !modoPrueba;
  M5.Display.setRotation(modoPrueba ? GIRO_PRUEBA : GIRO_TIMER);
  M5.Display.fillScreen(TFT_BLACK);
  redibujar = true;
  tPantalla = 0;
}

void iniciar(uint32_t t0) {        // t0: cuando arranco (el movimiento o el clic)
  tiros = 0;
  etapaN++;                        // etapa nueva para el registro
  memoria.putInt("etapa_n", etapaN);
  tInicio = t0;
  estado = CORRIENDO;
  movArranque = false;
  haySonido = false;
  hayGolpe = false;
  avisosDados = 0;                 // v3.31: avisos de 60, 45, 25 y de 10 a 1 s otra vez
  Serial.printf("INICIO etapa %d\n", etapaN);
  char m[16];
  snprintf(m, sizeof(m), "INICIO,%d", TIEMPOS_ETAPA[iTiempo]);
  bleAvisar(m);                    // v3.30: el telefono sabe que arranco
  if (!modoPrueba) pantallaTimer();
  pitar(BIPS_ARRANQUE, BIP_ARRANQUE_MS, PAUSA_ARRANQUE_MS);   // v3.29: 6 bips cortitos al arrancar
}

void regresarAlInicio() {
  estado = LISTO;                  // queda en 105 o 120 esperando otro clic
  tiros = 0;
  haySonido = false;
  hayGolpe = false;
  Serial.printf("EN %d, ESPERANDO\n", TIEMPOS_ETAPA[iTiempo]);
  bleAvisar("ALTO");               // v3.30: el telefono regresa a esperar
}

void armar() {                     // v3.29: 3 bips, 5 s para acomodarte, 3 bips y queda armado
  estado = ARMADO;
  avisoArmado = false;
  tSiente = 0xFFFFFFFF;            // todavia no siente el movimiento
  movArranque = false;
  tiros = 0;
  tArmado = millis();
  Serial.println("ESPERA: 5 s para acomodarte; luego 3 bips y queda armado");
  bleAvisar("LISTO");              // v3.30: el telefono dice "listo"
  if (!modoPrueba) pantallaTimer();
  pitar(BIPS_AVISO, BIP_CORTO_MS, PAUSA_BIP_MS);   // 3 bips: empiezan los 5 s
  tArmado = millis();              // los 5 s cuentan desde que terminan los bips
}

void quedarArmado() {              // v3.29: pasaron los 5 s: verde, ARMADO y 3 bips
  avisoArmado = true;
  tSiente = 0xFFFFFFFF;            // mientras suenan los bips no siente el movimiento
  movArranque = false;
  bleAvisar("ARMADO");             // v3.30: el telefono dice "armado"
  if (!modoPrueba) pantallaTimer();
  pitar(BIPS_AVISO, BIP_CORTO_MS, PAUSA_BIP_MS);
  tSiente = millis();              // desde aqui, el primer movimiento de 1 g arranca el timer
  Serial.printf("ARMADO: arranca con el primer movimiento de %.1f g o mas\n", UMBRAL_ARRANQUE);
}

void manejarBotones() {
  if (M5.BtnA.wasChangePressed() || M5.BtnB.wasChangePressed()) {   // v3.25: se apreto o se solto un boton;
    maxMic = maxMicPrevio;                                          // su clic satura el microfono: quita lo
    maxGolpe = maxGolpePrevio;                                      // que oyo desde la ultima revision...
    antesUlt = antesUltPrevio;
    rapidoUlt = rapidoUltPrevio;
    evAbierto = false;                                              // (ese evento no es tiro)
    haySonido = false;
    uint32_t hasta = millis() + BOTON_MS;                           // ...y no oye los siguientes 0.3 s
    if ((int32_t)(hasta - silencioHasta) > 0) silencioHasta = hasta;
  }
  if (confirmarBorrar) {                                          // esperando el si o el no para borrar el registro
    if (M5.BtnB.wasClicked()) {                                   // lateral: si, lo borra
      borrarRegistro();
      confirmarBorrar = false;
      limpiarPrueba();
    } else if (M5.BtnA.wasPressed() || millis() - tConfirmar > CONFIRMAR_MS) {   // azul o 5 s sin tocar: no borra
      confirmarBorrar = false;
      Serial.println("BORRADO CANCELADO");
      limpiarPrueba();
    }
    return;
  }
  if (M5.BtnB.wasClicked()) cambiarPantalla();                    // lateral: cambia de pantalla
  if (modoPrueba && M5.BtnB.wasHold()) {                          // lateral 1 s (solo en prueba): sube 1 g
    iUmbral = (iUmbral + 1) % N_UMBRALES;                         // despues de 10 g regresa a 1 g
    memoria.putInt("umbral_dg", decimasDeG(iUmbral));             // se queda guardado al apagar
    Serial.printf("UMBRAL %.1f g guardado\n", UMBRALES_GOLPE[iUmbral]);
  }
  if (modoPrueba) {
    if (M5.BtnA.wasPressed()) {                                   // azul: borra maximos, tiros y ANT
      tiros = 0;
      borrarMaximos();
    }
    if (M5.BtnA.wasDoubleClicked()) {                             // azul doble clic: baja 20% la ganancia
      iGanancia = (iGanancia + 1) % N_GANANCIAS;                  // despues de 20% regresa a 100%
      memoria.putInt("ganancia", GANANCIAS[iGanancia]);           // se queda guardado al apagar
      M5.Mic.end();
      aplicarGanancia();
      M5.Mic.begin();
      haySonido = false;
      silencioHasta = millis() + 100;                             // ignora el arranque del microfono
      Serial.printf("GANANCIA %d%% guardada\n", GANANCIAS[iGanancia]);
    }
    if (M5.BtnA.wasDecideClickCount() && M5.BtnA.getClickCount() == 3 && estado != CORRIENDO) {   // azul triple clic
      confirmarBorrar = true;                                     // pregunta si borra el registro de tiros
      tConfirmar = millis();
      limpiarPrueba();
      Serial.println("BORRAR REGISTRO? lateral = SI; azul o 5 s = NO");
    }
    if (M5.BtnA.wasHold() && estado != CORRIENDO) {               // azul 1.5 s: cambia entre 105 y 120 s
      iTiempo = (iTiempo + 1) % N_TIEMPOS;
      memoria.putInt("etapa_s", TIEMPOS_ETAPA[iTiempo]);          // se queda guardado al apagar
      Serial.printf("ETAPA %d s guardada\n", TIEMPOS_ETAPA[iTiempo]);
    }
  } else if (estado == LISTO) {
    if (M5.BtnA.wasClicked()) armar();                            // listo: un clic lo arma (v3.28)
  } else if (estado == ARMADO) {
    if (M5.BtnA.wasHold()) regresarAlInicio();                    // armado: apretado 1.5 s lo desarma
    else if (M5.BtnA.wasClicked()) iniciar(millis());             // armado: otro clic arranca a mano
  } else if (estado == TERMINADO) {
    if (M5.BtnA.wasClicked()) regresarAlInicio();                 // en 0: un clic regresa al inicio (al soltar)
  } else if (M5.BtnA.wasHold() && millis() - tInicio > 2000) {
    regresarAlInicio();                                           // corriendo: apretado 1.5 s regresa al inicio
  }
}

void revisarTimer() {
  if (estado != CORRIENDO) return;       // v3.23: ya no hay bips a los 30, 60 ni 90 s
  uint32_t pasado = millis() - tInicio;
  while (avisosDados < N_AVISOS && pasado + (uint32_t)AVISOS_SEG[avisosDados] * 1000 >= totalMs()) {
    char m[8];                           // v3.31: el telefono dice cuanto queda (60, 45, 25 y de 10 a 1)
    snprintf(m, sizeof(m), "Q%d", AVISOS_SEG[avisosDados]);
    bleAvisar(m);
    avisosDados++;
  }
  if (pasado >= totalMs()) {
    estado = TERMINADO;
    tFinEtapa = millis();                // v3.27: para el apagado automatico
    avisoUsbFin = false;                 // v3.31
    bleAvisar("FIN");                    // v3.30: el telefono da un pitido
    if (!modoPrueba) pantallaTimer();
    Serial.printf("FIN tiros=%d\n", tiros);
    pitar(BIPS_FIN, BIP_CORTO_MS, PAUSA_BIP_MS);   // 6 bips: se acabo el tiempo
  }
}

void revisarBle(uint32_t ahora) {        // v3.30: estado al telefono cada segundo; si se fue, anunciarse otra vez
  if (bleConectado != bleAntes) {
    bleAntes = bleConectado;
    Serial.println(bleConectado ? "BLE: telefono conectado" : "BLE: telefono desconectado");
  }
  if (bleReanunciar) {
    bleReanunciar = false;
    BLEDevice::startAdvertising();
  }
  if (bleConectado && ahora - tBleEstado >= 1000) {
    tBleEstado = ahora;
    char m[24];
    char est = (estado == ARMADO && !avisoArmado) ? 'E' : "LACT"[estado];   // E: en la espera de 5 s
    snprintf(m, sizeof(m), "S,%d,%c,%d,%d", segundosQuedan(), est, tiros, ultBarras);
    bleMandar(m, false);
  }
}

void setup() {
  iniciarBle();                          // v3.30: el Bluetooth primero, a 240 MHz y antes de pantalla, sensores y microfono
  setCpuFrequencyMhz(CPU_MHZ);           // ahorra bateria
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  Serial.setTimeout(50);                 // las ordenes LOG y BORRAR no traban el programa
  M5.Display.setRotation(GIRO_TIMER);
  M5.Display.fillScreen(TFT_BLACK);
  M5.BtnA.setHoldThresh(REGRESAR_MS);
  M5.BtnB.setHoldThresh(1000);
  memoria.begin("timer", false);
  int guardado = memoria.getInt("umbral_dg", decimasDeG(UMBRAL_FABRICA));   // ultimo umbral elegido
  for (int i = 0; i < N_UMBRALES; i++) {
    if (decimasDeG(i) == guardado) iUmbral = i;
  }
  int etapaGuardada = memoria.getInt("etapa_s", TIEMPOS_ETAPA[0]);          // ultimo tiempo elegido
  for (int i = 0; i < N_TIEMPOS; i++) {
    if (TIEMPOS_ETAPA[i] == etapaGuardada) iTiempo = i;
  }
  int ganGuardada = memoria.getInt("ganancia", GANANCIAS[0]);               // ultima ganancia elegida
  for (int i = 0; i < N_GANANCIAS; i++) {
    if (GANANCIAS[i] == ganGuardada) iGanancia = i;
  }
  etapaN = memoria.getInt("etapa_n", 0);                                     // ultima etapa registrada
  batSesion = memoria.getInt("bat_sesion", 0) + 1;                           // otro encendido
  memoria.putInt("bat_sesion", batSesion);
  fsOk = LittleFS.begin(true);           // memoria de archivos (la primera vez se formatea)
  eventosGuardados = contarEventos();
  guardarBateria("ARRANQUE");
  tBat = millis();
  imuRapido = iniciarImuRapido();
  int motivo = (int)esp_reset_reason();  // v3.30: por que arranco (4 = se trabo y se reinicio solo)
  int panicos = memoria.getInt("panicos", 0);
  if (motivo == (int)ESP_RST_PANIC) memoria.putInt("panicos", ++panicos);
  Serial.printf("v3.32 BMI270 dir=0x%02X rapido=%d umbral=%.1f g sonido=%d salto=%d en %d ms etapa=%d s gan=%d%% arranque=%.1f g espera=%d s ble=%s reset=%d panicos=%d\n",
                bmi, imuRapido, UMBRALES_GOLPE[iUmbral], UMBRAL_SONIDO, SALTO_MIN, SUBIDA_MS,
                TIEMPOS_ETAPA[iTiempo], GANANCIAS[iGanancia], UMBRAL_ARRANQUE, (int)(ESPERA_MS / 1000), bleNombre,
                motivo, panicos);
  Serial.printf("REGISTRO fs=%d eventos=%d ultima_etapa=%d (escribe LOG para bajarlo) apagado=%lu min usb=%d mV\n",
                fsOk, eventosGuardados, etapaN, (unsigned long)(APAGADO_MS / 60000),
                (int)M5.Power.getVBUSVoltage());
  Serial.printf("BATERIA %d mV %d%% carga=%d sesion=%d (escribe BAT para bajar su registro)\n",
                (int)M5.Power.getBatteryVoltage(), (int)M5.Power.getBatteryLevel(),
                M5.Power.isCharging() ? 1 : 0, batSesion);
  M5.Speaker.end();
  aplicarGanancia();
  M5.Mic.begin();
  tCuenta = millis();
}

void loop() {
  M5.update();
  manejarBotones();
  revisarComandos();
  maxMicPrevio = maxMic;               // v3.25: como estaban, por si en este ciclo se aprieta un boton
  maxGolpePrevio = maxGolpe;
  antesUltPrevio = antesUlt;
  rapidoUltPrevio = rapidoUlt;

  golpe = 0;
  leerImu();
  int subidaCiclo = -1;                // brinco RAPIDO en este ciclo (-1 = no hubo)
  int antesCiclo = -1;                 // nivel de antes si cruzo 30,000 (-1 = no cruzo)
  if (M5.Mic.record(audio, N_MUESTRAS, FRECUENCIA)) {
    while (M5.Mic.isRecording()) {
      leerImu();                       // lee el acelerometro mientras graba
    }
    uint32_t tBloque = millis();
    picoMic = 0;
    for (int i = 0; i < N_MUESTRAS; i++) {
      int v = abs(audio[i]);
      if (v > picoMic) picoMic = v;
    }
    int subida = buscarSubida();       // muestra donde hubo un brinco grande y rapido (-1 = no hubo)
    if (subida >= 0 || (picoMic > REGISTRO_SONIDO && tBloque - tRegSon > 200)) {   // medicion
      tRegSon = tBloque;
      Serial.printf("SON t=%lu pico=%d antes=%d %s\n", (unsigned long)tBloque, picoMic, antesBloque,
                    subida >= 0 ? "RAPIDO" : (antesBloque >= 0 ? "LENTO" : "-"));
    }
    if (tBloque > silencioHasta) {     // v3.25: ni bips ni botones llenan la M ni el ANT
      subidaCiclo = subida;
      antesCiclo = antesBloque;
      if (picoMic > maxMic) maxMic = picoMic;
      if (antesBloque >= 0) {          // ANT: el ultimo cruce de 30,000
        antesUlt = antesBloque;
        rapidoUlt = (subida >= 0);
      }
    }
    if (subida >= 0 && tBloque > silencioHasta) {
      uint32_t tSubida = tBloque - (uint32_t)(N_MUESTRAS - subida) * 1000 / FRECUENCIA;  // hora del brinco
      if (!haySonido) tPrimerSonido = tSubida;
      if (!haySonido || picoMic > micVentana) micVentana = picoMic;
      haySonido = true;
      tSonido = tBloque;
    }
  }
  if (estado == ARMADO && !avisoArmado && millis() - tArmado >= ESPERA_MS) {   // v3.29: pasaron los 5 s
    quedarArmado();
  }
  if (estado == ARMADO && movArranque) {   // v3.28: arranca con el primer movimiento del rifle
    Serial.printf("ARRANQUE POR MOVIMIENTO: %.2f g\n", gArranque);
    iniciar(tMovArranque);
  }
  uint32_t ahora = millis();
  if (golpe > maxGolpe && ahora > silencioHasta) maxGolpe = golpe;
  bool contadoAhora = false;
  if (haySonido && ahora - tSonido > VENTANA_MS) haySonido = false;
  if (hayGolpe && ahora - tGolpe > VENTANA_MS) hayGolpe = false;
  if (haySonido && hayGolpe && ahora - tTiro > TIEMPO_MUERTO) {   // sonido + golpe = tiro
    tTiro = ahora;
    tiroMic = micVentana;
    tiroGolpe = golpeVentana;
    haySonido = false;
    hayGolpe = false;
    if (estado == CORRIENDO || modoPrueba) {
      tiros++;
      contadoAhora = true;
      Serial.printf("TIRO %d mic=%d golpe=%.2f dt=%ld ms\n", tiros, tiroMic, tiroGolpe,
                    (long)tPrimerSonido - (long)tPrimerGolpe);
    }
  }
  registrarEvento(ahora, subidaCiclo, antesCiclo, contadoAhora);   // guarda el evento en la memoria

  revisarTimer();
  revisarBle(ahora);                   // v3.30: estado al telefono y anunciarse otra vez
  revisarBateria();
  revisarApagado();

  if (ahora - tCuenta >= 1000) {
    hzImu = lecturas;
    lecturas = 0;
    tCuenta = ahora;
  }
  if (modoPrueba) {
    if (ahora - tPantalla >= PRUEBA_CADA_MS) {
      tPantalla = ahora;
      pantallaPrueba();
    }
  } else {
    pantallaTimer();
  }
  if (ahora - tSerie >= 1000) {
    tSerie = ahora;
    Serial.printf("seg=%d tiros=%d mic=%d acc=%.2f imuHz=%d cpu=%d bat=%d vbat=%d carga=%d umb=%.1f gan=%d reg=%d est=%c ble=%d\n",
                  segundosQuedan(), tiros, picoMic, golpe, (int)hzImu, (int)getCpuFrequencyMhz(),
                  ultBarras, (int)M5.Power.getBatteryVoltage(), M5.Power.isCharging() ? 1 : 0,
                  UMBRALES_GOLPE[iUmbral], GANANCIAS[iGanancia], eventosGuardados, "LACT"[estado],
                  bleConectado ? 1 : 0);
  }
}

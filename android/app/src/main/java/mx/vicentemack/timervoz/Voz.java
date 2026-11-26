package mx.vicentemack.timervoz;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioTrack;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;

import java.util.Locale;

/** Voz en español de México y pitidos, por el audio del teléfono (los audífonos Bluetooth). */
final class Voz {
    private static final int HZ = 44100;
    private static final AudioAttributes ATRIBUTOS_VOZ = new AudioAttributes.Builder()
            .setUsage(AudioAttributes.USAGE_MEDIA)
            .setContentType(AudioAttributes.CONTENT_TYPE_SPEECH)
            .build();
    private static final AudioAttributes ATRIBUTOS_TONO = new AudioAttributes.Builder()
            .setUsage(AudioAttributes.USAGE_MEDIA)
            .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
            .build();

    private TextToSpeech tts;
    private volatile boolean lista = false;
    private String pendiente = null;      // lo que se pidio decir antes de que la voz estuviera lista
    private AudioTrack pitidos;
    private AudioTrack despertador;       // tono casi en silencio: los audifonos no se duermen

    Voz(Context ctx) {
        tts = new TextToSpeech(ctx.getApplicationContext(), this::alIniciar);
    }

    private void alIniciar(int estado) {
        if (estado != TextToSpeech.SUCCESS) return;
        int r = tts.setLanguage(new Locale("es", "MX"));
        if (r == TextToSpeech.LANG_MISSING_DATA || r == TextToSpeech.LANG_NOT_SUPPORTED) {
            r = tts.setLanguage(new Locale("es", "US"));
        }
        if (r == TextToSpeech.LANG_MISSING_DATA || r == TextToSpeech.LANG_NOT_SUPPORTED) {
            tts.setLanguage(new Locale("es"));
        }
        tts.setSpeechRate(1.05f);
        tts.setAudioAttributes(ATRIBUTOS_VOZ);
        lista = true;
        String p;
        synchronized (this) {
            p = pendiente;
            pendiente = null;
        }
        if (p != null) decir(p);
    }

    /** Dice el texto; corta lo que estuviera diciendo para no atrasarse. */
    void decir(String texto) {
        if (!lista) {
            synchronized (this) {
                pendiente = texto;
            }
            return;
        }
        Bundle p = new Bundle();
        p.putFloat(TextToSpeech.Engine.KEY_PARAM_VOLUME, 1.0f);
        tts.speak(texto, TextToSpeech.QUEUE_FLUSH, p, "aviso");
    }

    void callar() {
        if (lista) tts.stop();
    }

    /** Pitidos de 1,000 Hz: cuantos, cuanto dura cada uno y la pausa entre ellos (ms). */
    void pitidos(int veces, int duraMs, int pausaMs) {
        int nSi = HZ * duraMs / 1000;
        int nNo = HZ * pausaMs / 1000;
        int suave = HZ * 5 / 1000;        // 5 ms de subida y bajada: sin chasquidos
        short[] pcm = new short[(nSi + nNo) * veces];
        int k = 0;
        for (int v = 0; v < veces; v++) {
            for (int i = 0; i < nSi; i++) {
                double env = Math.min(1.0, Math.min(i, nSi - 1 - i) / (double) suave);
                pcm[k++] = (short) (Math.sin(2 * Math.PI * 1000 * i / HZ) * 0.8 * 32767 * env);
            }
            k += nNo;
        }
        synchronized (this) {
            soltar(pitidos);
            AudioTrack t = pista(pcm.length);
            t.write(pcm, 0, pcm.length);
            t.play();
            pitidos = t;
        }
    }

    /** Un tono de 30 Hz casi en cero, en ciclo: no se oye y mantiene despiertos los audifonos. */
    void mantenerDespierto(boolean si) {
        synchronized (this) {
            if (si && despertador == null) {
                int n = HZ;                   // 1 s exacto: 30 ciclos completos
                short[] pcm = new short[n];
                for (int i = 0; i < n; i++) {
                    pcm[i] = (short) (Math.sin(2 * Math.PI * 30 * i / HZ) * 20);
                }
                AudioTrack t = pista(n);
                t.write(pcm, 0, n);
                t.setLoopPoints(0, n, -1);
                t.play();
                despertador = t;
            } else if (!si && despertador != null) {
                soltar(despertador);
                despertador = null;
            }
        }
    }

    void cerrar() {
        synchronized (this) {
            soltar(pitidos);
            pitidos = null;
            soltar(despertador);
            despertador = null;
        }
        tts.stop();
        tts.shutdown();
    }

    private static AudioTrack pista(int muestras) {
        return new AudioTrack.Builder()
                .setAudioAttributes(ATRIBUTOS_TONO)
                .setAudioFormat(new AudioFormat.Builder()
                        .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                        .setSampleRate(HZ)
                        .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                        .build())
                .setTransferMode(AudioTrack.MODE_STATIC)
                .setBufferSizeInBytes(muestras * 2)
                .build();
    }

    private static void soltar(AudioTrack t) {
        if (t == null) return;
        try {
            t.stop();
        } catch (Exception e) {
            // ya estaba parada
        }
        t.release();
    }
}

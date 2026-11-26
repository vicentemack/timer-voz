// Timer voz: guarda la página en el celular para que abra aunque no haya señal en el campo.
// Primero intenta la red (3 s) para traer cambios; si no hay, usa la copia guardada.
const CACHE = 'timer-voz-v1';
const ARCHIVOS = ['./', './index.html', './manifest.webmanifest', './icono-192.png', './icono-512.png'];

self.addEventListener('install', (e) => {
  e.waitUntil(caches.open(CACHE).then((c) => c.addAll(ARCHIVOS)).then(() => self.skipWaiting()));
});

self.addEventListener('activate', (e) => {
  e.waitUntil(caches.keys()
    .then((ks) => Promise.all(ks.filter((k) => k !== CACHE).map((k) => caches.delete(k))))
    .then(() => self.clients.claim()));
});

self.addEventListener('fetch', (e) => {
  if (e.request.method !== 'GET' || new URL(e.request.url).origin !== location.origin) return;
  e.respondWith((async () => {
    const control = new AbortController();
    const reloj = setTimeout(() => control.abort(), 3000);
    try {
      const r = await fetch(e.request, { signal: control.signal });
      clearTimeout(reloj);
      if (r.ok) {
        const copia = r.clone();
        caches.open(CACHE).then((c) => c.put(e.request, copia));
      }
      return r;
    } catch (err) {
      clearTimeout(reloj);
      const guardada = await caches.match(e.request, { ignoreSearch: true });
      return guardada || caches.match('./index.html');
    }
  })());
});

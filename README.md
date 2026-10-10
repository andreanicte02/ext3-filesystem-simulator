# Simulador de sistema de archivos EXT3

Programa en C que simula un sistema de archivos tipo **EXT3** sobre discos virtuales (archivos `.disk`):

- Crear, eliminar y aumentar discos; crear particiones (MBR) y montarlas
- Dar formato EXT3: superbloque, tabla de inodos, bloques de carpetas, archivos e indirectos, bitmaps y journaling
- Usuarios y grupos (`users.txt` dentro del propio sistema de archivos), inicio de sesión como `root`
- Crear carpetas, mostrar, eliminar, renombrar y mover archivos y directorios
- Reportes con Graphviz: directorios, archivos, general, inodos, bloques; y en texto: bitmaps y journaling

**Demo:** https://andreanicte02.github.io/archivos-fase1/

La demo corre el mismo programa compilado a WebAssembly con Emscripten. Los discos viven en la memoria del
navegador (se pierden al recargar) y los reportes se dibujan en la página con [viz.js](https://github.com/mdaines/viz-js).
Tiene una demo guiada que crea un disco, lo particiona, lo formatea, crea carpetas y genera los reportes.

## Cómo funciona la versión web

El código en C es el original; para el navegador se agregan tres piezas en `web/`:

| Archivo | Para qué |
|---|---|
| `compat.c` | Redirige `system()` a JavaScript (`-Dsystem=web_system`) y quita el buffer de `stdout` |
| `terminal-lib.js` | Reemplaza la lectura de stdin: con Asyncify el programa se pausa en `scanf` hasta que el usuario escribe una línea |
| `app.js` | Terminal, demo guiada y reportes: `dot -Tpng` se dibuja con viz.js y `xdg-open` de un `.txt` se muestra en la página |

La opción 16 (sincronizar) cargaba los usuarios a Oracle con `sqlldr`; eso no está disponible en el navegador.

## Compilar

```bash
# nativo (Linux/macOS)
gcc -std=gnu99 -o ext3 *.c -lm

# web (requiere Emscripten)
web/build.sh          # genera web/publicar/
```

Cada push a `master` publica la página con `.github/workflows/pages.yml`.

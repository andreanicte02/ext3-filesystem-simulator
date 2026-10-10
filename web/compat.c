/*
 * Capa de compatibilidad para correr el simulador en el navegador (Emscripten).
 * El codigo original no se modifica: se compila con -Dsystem=web_system.
 */

#include <stdio.h>
#include <emscripten.h>

// system() no existe en el navegador. Los comandos que usa el programa
// (dot, xdg-open, rm -r, sudo su) los resuelve app.js.
EM_JS(int, web_system, (const char *comando), {
    return Module.ejecutarComando(UTF8ToString(comando));
});

// Sin buffer: los mensajes sin salto de linea ("Ingrese el usuario:") se ven de inmediato
__attribute__((constructor)) static void stdout_sin_buffer(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
}

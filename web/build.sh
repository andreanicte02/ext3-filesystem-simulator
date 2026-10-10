#!/usr/bin/env bash
# Compila el simulador a WebAssembly y arma la pagina en web/publicar/
# Requiere Emscripten (emcc) en el PATH.
set -euo pipefail

cd "$(dirname "$0")/.."
SALIDA=web/publicar
rm -rf "$SALIDA"
mkdir -p "$SALIDA"

emcc -O2 -std=gnu99 -w \
    -Dsystem=web_system \
    *.c web/compat.c \
    --js-library web/terminal-lib.js \
    -sASYNCIFY -sASYNCIFY_STACK_SIZE=65536 \
    -sSTACK_SIZE=8MB -sALLOW_MEMORY_GROWTH \
    -sFORCE_FILESYSTEM -sEXPORTED_RUNTIME_METHODS=FS \
    -sENVIRONMENT=web \
    -lm \
    -o "$SALIDA/ext3.js"

cp web/index.html web/app.js web/estilos.css "$SALIDA/"
echo "Listo: $SALIDA"

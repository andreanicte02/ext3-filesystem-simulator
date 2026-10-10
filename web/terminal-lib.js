// Libreria de Emscripten: conecta stdin/stdout del programa con la terminal de la pagina.
// stdin se lee de forma asincrona (Asyncify): el programa se pausa en scanf hasta
// que el usuario escribe una linea, sin bloquear el navegador.

addToLibrary({
    // como texto: se evalua en el navegador, no al compilar
    $terminalEstado: "{ pendiente: new Uint8Array(0), codificador: new TextEncoder() }",

    fd_read__async: true,
    fd_read__deps: ['$doReadv', '$terminalEstado'],
    fd_read: (fd, iov, iovcnt, pnum) => {
        const copiarPendiente = () => {
            let leidos = 0;
            for (let i = 0; i < iovcnt && terminalEstado.pendiente.length > 0; i++) {
                const ptr = HEAPU32[(iov + i * 8) >> 2];
                const len = HEAPU32[(iov + i * 8 + 4) >> 2];
                const n = Math.min(len, terminalEstado.pendiente.length);
                HEAPU8.set(terminalEstado.pendiente.subarray(0, n), ptr);
                terminalEstado.pendiente = terminalEstado.pendiente.subarray(n);
                leidos += n;
            }
            HEAPU32[pnum >> 2] = leidos;
            return 0;
        };

        if (fd !== 0) {
            try {
                const stream = SYSCALLS.getStreamFromFD(fd);
                HEAPU32[pnum >> 2] = doReadv(stream, iov, iovcnt);
                return 0;
            } catch (e) {
                if (!(e instanceof FS.ErrnoError)) throw e;
                return e.errno;
            }
        }

        if (terminalEstado.pendiente.length > 0) {
            return copiarPendiente();
        }

        return Asyncify.handleSleep((despertar) => {
            Module.leerLinea().then((linea) => {
                terminalEstado.pendiente = terminalEstado.codificador.encode(linea + '\n');
                despertar(copiarPendiente());
            });
        });
    },

    fd_write__deps: ['$doWritev'],
    fd_write: (fd, iov, iovcnt, pnum) => {
        try {
            if (fd === 1 || fd === 2) {
                let escritos = 0;
                for (let i = 0; i < iovcnt; i++) {
                    const ptr = HEAPU32[(iov + i * 8) >> 2];
                    const len = HEAPU32[(iov + i * 8 + 4) >> 2];
                    Module.escribirTerminal(HEAPU8.slice(ptr, ptr + len));
                    escritos += len;
                }
                HEAPU32[pnum >> 2] = escritos;
                return 0;
            }
            const stream = SYSCALLS.getStreamFromFD(fd);
            HEAPU32[pnum >> 2] = doWritev(stream, iov, iovcnt);
            return 0;
        } catch (e) {
            if (!(e instanceof FS.ErrnoError)) throw e;
            return e.errno;
        }
    },
});

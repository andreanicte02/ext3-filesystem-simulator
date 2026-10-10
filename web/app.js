// Conecta el simulador (ext3.js, compilado con Emscripten) con la pagina:
// terminal, demo guiada y reportes.

const RUTA_DISCOS = '/home/nicte/Escritorio/discos';

const terminal = document.getElementById('terminal');
const formulario = document.getElementById('formulario');
const entrada = document.getElementById('entrada');
const botonEnviar = document.getElementById('enviar');

// ---------------------------------------------------------------- terminal

const decodificador = new TextDecoder();

function agregarTexto(texto, clase) {
    const nodo = document.createElement('span');
    if (clase) {
        nodo.className = clase;
    }
    nodo.textContent = texto;
    terminal.appendChild(nodo);
    terminal.scrollTop = terminal.scrollHeight;
}

// lineas que el programa todavia no ha pedido (las escribe el usuario o la demo)
const lineasPendientes = [];
let esperandoLinea = null;

function enviarLinea(linea) {
    agregarTexto(linea + '\n', 'escrito');
    if (esperandoLinea) {
        const resolver = esperandoLinea;
        esperandoLinea = null;
        resolver(linea);
    } else {
        lineasPendientes.push(linea);
    }
}

formulario.addEventListener('submit', (evento) => {
    evento.preventDefault();
    enviarLinea(entrada.value);
    entrada.value = '';
});

// ---------------------------------------------------------------- reportes

const NOMBRES_REPORTES = {
    'juego.dot': 'Directorios',
    'juego2.dot': 'Archivos',
    'juego3.dot': 'General (inodos, bloques y apuntadores)',
    'juego4.dot': 'Archivos de un directorio',
    'reporte_inodos.dot': 'Inodos',
    'reportes_bloques.dot': 'Bloques',
    'bm_inodos.txt': 'Bitmap de inodos',
    'bm_bloques.txt': 'Bitmap de bloques',
    'reporte_journaling.txt': 'Journaling',
};

const listaReportes = document.getElementById('lista-reportes');
const vizListo = typeof Viz !== 'undefined' ? Viz.instance() : Promise.reject(new Error('no cargo viz.js'));

function agregarReporte(archivo, contenido) {
    document.getElementById('sin-reportes').hidden = true;
    const tarjeta = document.createElement('details');
    tarjeta.className = 'reporte';
    tarjeta.open = true;
    const titulo = document.createElement('summary');
    titulo.textContent = `${NOMBRES_REPORTES[archivo] || archivo} · ${new Date().toLocaleTimeString()}`;
    tarjeta.appendChild(titulo);
    tarjeta.appendChild(contenido);
    // el mas reciente arriba; los anteriores se cierran para no ocupar tanto espacio
    for (const anterior of listaReportes.children) {
        anterior.open = false;
    }
    listaReportes.prepend(tarjeta);
}

function reporteGrafico(archivo) {
    const dot = Module.FS.readFile(archivo, { encoding: 'utf8' });
    const contenedor = document.createElement('div');
    contenedor.className = 'grafica';
    contenedor.textContent = 'Dibujando...';
    agregarReporte(archivo, contenedor);
    vizListo
        .then((viz) => {
            contenedor.textContent = '';
            contenedor.appendChild(viz.renderSVGElement(dot));
            contenedor.title = 'Clic para ver en grande';
            contenedor.addEventListener('click', () => abrirVisor(archivo, viz.renderSVGElement(dot)));
        })
        .catch((error) => {
            contenedor.textContent = 'No se pudo dibujar el reporte: ' + error.message;
        });
}

function reporteTexto(archivo) {
    const texto = Module.FS.readFile(archivo, { encoding: 'utf8' });
    const pre = document.createElement('pre');
    pre.className = 'texto';
    pre.textContent = texto;
    agregarReporte(archivo, pre);
}

const visor = document.getElementById('visor');

function abrirVisor(archivo, svg) {
    document.getElementById('visor-titulo').textContent = NOMBRES_REPORTES[archivo] || archivo;
    const lienzo = document.getElementById('visor-lienzo');
    lienzo.textContent = '';
    lienzo.appendChild(svg);
    visor.showModal();
}

document.getElementById('visor-cerrar').addEventListener('click', () => visor.close());

// El programa original llama a system(); aqui se resuelven esos comandos.
function ejecutarComando(comando) {
    let m;
    try {
        if ((m = comando.match(/^dot\s+-T\w+\s+(\S+)\s+-o\s+\S+$/))) {
            reporteGrafico(m[1]);
            return 0;
        }
        if ((m = comando.match(/^xdg-open\s+(\S+\.txt)$/))) {
            reporteTexto(m[1]);
            return 0;
        }
        if (/^xdg-open\s+\S+\.png$/.test(comando)) {
            return 0; // la imagen ya se dibujo con viz.js
        }
        if ((m = comando.match(/^rm -r (.+)$/))) {
            Module.FS.unlink(m[1]);
            agregarTexto(`[web] se elimino ${m[1]}\n`, 'aviso');
            return 0;
        }
        if (comando === 'sudo su') {
            agregarTexto('[web] la sincronización cargaba usuario.txt a Oracle con sqlldr; no está disponible en el navegador\n', 'aviso');
            return 0;
        }
    } catch (error) {
        agregarTexto(`[web] error al ejecutar "${comando}": ${error.message}\n`, 'aviso');
        return 1;
    }
    agregarTexto(`[web] comando no soportado: ${comando}\n`, 'aviso');
    return 127;
}

// ---------------------------------------------------------------- demo guiada

const PASOS = [
    { titulo: 'Crear un disco de 16 MB', lineas: ['1', 'disco1', RUTA_DISCOS, '16', 'm'] },
    { titulo: 'Crear una partición de 8 MB', lineas: ['4', 'disco1', RUTA_DISCOS, '1', '8', 'particion1', '0'] },
    { titulo: 'Montar la partición vdb1', lineas: ['5', 'vdb1'] },
    { titulo: 'Darle formato EXT3', lineas: ['7', 'vdb1'] },
    { titulo: 'Iniciar sesión como root', lineas: ['8', 'vdb1', 'root', '201404104', '0'] },
    { titulo: 'Crear carpetas /home, /home/andrea y /docs', lineas: ['1', '/home', '1', '/home/andrea', '1', '/docs'] },
    { titulo: 'Reporte de directorios', lineas: ['2'] },
    { titulo: 'Reporte general', lineas: ['4'] },
    { titulo: 'Reporte de inodos y de bloques', lineas: ['8', '9'] },
    { titulo: 'Bitmap de inodos y journaling', lineas: ['6', '10'] },
];

const listaPasos = document.getElementById('pasos');
const botonSiguiente = document.getElementById('siguiente');
const botonTodo = document.getElementById('todo');
let pasoActual = 0;

for (const paso of PASOS) {
    const li = document.createElement('li');
    li.textContent = paso.titulo;
    listaPasos.appendChild(li);
}

function marcarPasos() {
    [...listaPasos.children].forEach((li, i) => {
        li.className = i < pasoActual ? 'hecho' : i === pasoActual ? 'actual' : '';
    });
    const terminado = pasoActual >= PASOS.length;
    botonSiguiente.disabled = terminado;
    botonTodo.disabled = terminado;
    botonSiguiente.textContent = terminado ? 'Demo terminada' : `Paso ${pasoActual + 1}`;
}

function ejecutarPaso() {
    for (const linea of PASOS[pasoActual].lineas) {
        enviarLinea(linea);
    }
    pasoActual++;
    marcarPasos();
}

botonSiguiente.addEventListener('click', ejecutarPaso);
botonTodo.addEventListener('click', () => {
    while (pasoActual < PASOS.length) {
        ejecutarPaso();
    }
});
document.getElementById('reiniciar').addEventListener('click', () => location.reload());

// ---------------------------------------------------------------- Emscripten

var Module = {
    escribirTerminal: (bytes) => agregarTexto(decodificador.decode(bytes, { stream: true })),
    leerLinea: () => {
        if (lineasPendientes.length > 0) {
            return Promise.resolve(lineasPendientes.shift());
        }
        return new Promise((resolver) => {
            esperandoLinea = resolver;
        });
    },
    ejecutarComando,
    preRun: [
        () => {
            // la bitacora y los discos de la demo viven en la ruta que usaba el programa original
            Module.FS.mkdirTree(RUTA_DISCOS);
        },
    ],
    onRuntimeInitialized: () => {
        entrada.disabled = false;
        botonEnviar.disabled = false;
        entrada.placeholder = 'Escribe tu respuesta y presiona Enter';
        marcarPasos();
        entrada.focus();
    },
};

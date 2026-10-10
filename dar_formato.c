#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mbr.h"
#include "general.h"
#include "formato_ext3.h"
#include "dar_formato.h"
#include <math.h>
#include <sys/stat.h>
#include <time.h>

long int calcular_n(long int tamano_part) {

    float n = 0;
    n = (tamano_part - sizeof (super_bloque));
    float l = (sizeof (journaling) + 47 + sizeof (inodo) + 8 * sizeof (b_indirecto) + 46 * sizeof (b_archivos));
    float f = n / l;
    //ver si agarra el valor mas pequeño
    return f;
}

long int inicio_bm_inodos(long int inicio, long int n) {

    long int final = 0;
    final = inicio + sizeof (super_bloque) + n * sizeof (journaling);
    return final;

}

long int inicio_bm_bloques(long int incio, long int n) {
    long int final = 0;
    final = incio + sizeof (super_bloque) + n * sizeof (journaling) + n;
    return final;


}

long int inicio_inodos(long int inicio, long int n) {
    long int final = 0;
    final = inicio + sizeof (super_bloque) + n * sizeof (journaling) + n + n * 46;
    return final;

}

long int inicio_bloques(long int inicio, long int n) {

    long int final = 0;
    final = inicio + sizeof (super_bloque) + n * sizeof (journaling) + n + n * 46 + n * sizeof (inodo) + 8 * n * sizeof (b_indirecto);
    return final;
}

void inicio(FILE *archivo, long int inicio) { //colocar el punter al inicoi de cualquier particion

    fseek(archivo, 0, SEEK_SET);

    for (long int x = 0; x < inicio; x++) {
        fseek(archivo, 1, SEEK_CUR);

    }
}

void crear_carepta(char *path) {

}

void crearraiz(char *path, long int inicio_particion, long int tamano) {

    FILE *archivo = fopen(path, "r+b");
    long int n = calcular_n(tamano);
    if (archivo) {

        inicio(archivo, inicio_particion);
        // printf("%ld fin del archivo \n", ftell(archivo));
        super_bloque sb;


        fread(&sb, sizeof (super_bloque), 1, archivo);
        // printf("%s\n", sb.fecha);
        sb.primer_bloque_libre = 1;
        sb.primer_inodo_libre = 1;
        sb.indoso_free = n - 1;
        sb.bloques_free = (n * 46) - 1;
        fseek(archivo, -1 * sizeof (super_bloque), SEEK_CUR);
        fwrite(&sb, sizeof (super_bloque), 1, archivo);


        //--- saltar el journaling
        for (int x = 0; x < n; x++) {

            fseek(archivo, sizeof (journaling), SEEK_CUR);
        }

        // printf("%ld fin del archivo \n", ftell(archivo));

        char c = 'c';
        char uno = '1';

        fwrite(&uno, sizeof (uno), 1, archivo);


        for (int x = 0; x < n - 1; x++) {

            fseek(archivo, sizeof (c), SEEK_CUR);
        }

        //printf("%ld fin del archivo \n", ftell(archivo));


        fwrite(&c, sizeof (c), 1, archivo);




        for (int x = 0; x < (n * 46) - 1; x++) {

            fseek(archivo, sizeof (c), SEEK_CUR);
        }

        // printf("%ld fin del archivo \n", ftell(archivo));

        inodo tinodo;

        tinodo.ad1 = 1; //apunta a la carpeta 1
        tinodo.ad2 = -1;
        tinodo.ad3 = -1;
        tinodo.ad4 = -1;

        tinodo.ap_n1 = -1;
        tinodo.ap_n2 = -1;

        tinodo.bloues_asignados = 1;
        strcpy(tinodo.fecha, "vacio");
        strcpy(tinodo.idchar, "/");
        tinodo.llave = 1;
        tinodo.permisos = -1;

        strcpy(tinodo.propietario, "vacio");
        tinodo.tamano = 0;
        tinodo.tipo = 0; //tipo carpeta

        fwrite(&tinodo, sizeof (inodo), 1, archivo);

        for (int x = 0; x < n - 1; x++) {

            fseek(archivo, sizeof (inodo), SEEK_CUR);
        }


        for (int x = 0; x < n * 8; x++) {

            fseek(archivo, sizeof (b_indirecto), SEEK_CUR);
        }

        b_carpetas ctem;

        strcpy(ctem.nombre, "/");
        strcpy(ctem.padre, "/");

        ctem.apactual = 1; //no de carpeta 
        ctem.appadre = 1; //inodo parde

        ctem.hijo1 = -1;
        ctem.hijo2 = -1;
        ctem.hijo3 = -1;
        ctem.hijo4 = -1;
        ctem.hijo5 = -1;
        ctem.hijo6 = -1;


        fwrite(&ctem, sizeof (b_carpetas), 1, archivo);

        long int sdfasd = ftell(archivo);
        //printf("--fuck %ld",sdfasd);
        for (int x = 0; x < (n * 46) - 1; x++) {

            fseek(archivo, sizeof (b_carpetas), SEEK_CUR);
        }

        //------------crear archivo user

        fseek(archivo, inicio_particion, SEEK_SET);
        fseek(archivo, sizeof (super_bloque), SEEK_CUR);
        fseek(archivo, sizeof (journaling) * n, SEEK_CUR);
        long int acualmente = ftell(archivo);
        fseek(archivo, 1, SEEK_CUR);
        //---------------------------------------------escribir en bm de inodos
        char unorot = '1';
        fwrite(&unorot, sizeof (unorot), 1, archivo);
        fseek(archivo, acualmente, SEEK_SET);
        fseek(archivo, n, SEEK_CUR);

        acualmente = ftell(archivo);

        char charar = 'a';
        fseek(archivo, 1, SEEK_CUR);
        fwrite(&charar, sizeof (charar), 1, archivo);
        fwrite(&charar, sizeof (charar), 1, archivo);

        fseek(archivo, acualmente, SEEK_SET);
        fseek(archivo, n * 46, SEEK_CUR);

        
        acualmente = ftell(archivo); //dsps de los bitmas de archivos 
        fseek(archivo, sizeof (inodo), SEEK_CUR); //me salto el primer inodo


        inodo useres;
        useres.ad1 = 2; //apunta a bloques de archivos
        useres.ad2 = 3;
        useres.ad3 = -1;
        useres.ad4 = -1;
        useres.ap_n1 = -1;
        useres.ap_n2 = -1;
        useres.bloues_asignados = 4;
        char fe[200];
        struct tm fecha = *localtime(&(time_t){time(NULL)});
        strcpy(fe, asctime(&fecha));
        strcpy(useres.fecha,fe);
        strcpy(useres.idchar,"users.txt");
        useres.llave =2;
        useres.permisos = 777;
        strcpy(useres.propietario,"root");
        useres.tamano =36;
        useres.tipo =1;

        fwrite(&useres,sizeof(inodo),1,archivo);
        fseek(archivo,acualmente,SEEK_SET);

        fseek(archivo,n*sizeof(inodo),SEEK_CUR);
        fseek(archivo,n*sizeof(b_indirecto)*8,SEEK_CUR);
        
        acualmente =ftell(archivo);///inicio de la carpeta
        
        b_carpetas bcaarrps;
        fread(&bcaarrps,sizeof(b_carpetas),1,archivo);
        bcaarrps.hijo1 = 2; //osea el segundo inodo
        fseek(archivo,-sizeof(b_carpetas),SEEK_CUR);
        fwrite(&bcaarrps,sizeof(b_carpetas),1,archivo);
        
        //-------------------------------------------------
        b_archivos archivo1;
        archivo1.apactual=2;
        archivo1.appadre=2; //inodo padre tambien es el 2
        char primero [24]="1,G,root:1,U,root,root,2";
        memcpy(archivo1.info,primero,sizeof(archivo1.info)); // info es de 24 sin \0
        fwrite(&archivo1,sizeof(b_archivos),1,archivo);
       
        
        b_archivos archivo2;
        archivo2.apactual=3;
        archivo2.appadre=2; //inodo padre tambien es el 2
        char segundo [24]="01404104:\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
        memcpy(archivo2.info,segundo,sizeof(archivo2.info));
        fwrite(&archivo2,sizeof(b_archivos),1,archivo);
        fclose(archivo);
    }



}

void formato_ext3(idspart part, int numero) {

    if (numero < 20) {

        mbr temporal = recuperar_mbr(part[numero].ruta);
        FILE *archivo = fopen(part[numero].ruta, "r+b");
        if (archivo != NULL) {

            long int inicio;
            long int fin;

            if (part[numero].numero == 1) {
                inicio = temporal.par[0].inicio;
                fin = temporal.par[0].tamano;

            } else if (part[numero].numero == 2) {
                inicio = temporal.par[1].inicio;
                fin = temporal.par[1].tamano;

            } else if (part[numero].numero == 3) {
                inicio = temporal.par[2].inicio;
                fin = temporal.par[2].tamano;

            } else if (part[numero].numero == 4) {
                inicio = temporal.par[3].inicio;
                fin = temporal.par[3].tamano; //tamano de la particion
            }
            long int ultra = inicio + fin;
            //printf("la que deberia ser la pos final %ld: \n", ultra);

            int fsad;

            fseek(archivo, 0, SEEK_SET);
            char byte = '\0';
            for (long int x = 0; x < inicio; x++) { //me posiciono en el inicio de la particion

                fseek(archivo, sizeof (byte), SEEK_CUR);

            }

            fsad = ftell(archivo);
            //printf("inicio del archivo pos %li",ftell(archivo)); //me posicione en la particion
            long int pos_actual = ftell(archivo);
            inodo t_inodos;
            b_indirecto temp_indirectos;
            long int n = calcular_n(fin);
            journaling j[n];

            super_bloque sb;
            sb.blo_inodos = n;
            sb.blo_bloques = 46 * n;
            sb.tamano_bloque = 74;
            sb.firma = 201404104;
            sb.bloques_free = 46 * n;
            sb.indoso_free = n;
            strcpy(sb.fecha, "vacio");


            sb.bm_inodos = inicio_bm_inodos(inicio, n);
            sb.primer_inodo = inicio_inodos(inicio, n);

            sb.bm_bloques = inicio_bm_bloques(inicio, n);
            sb.primer_bloque = inicio_bloques(inicio, n);

            sb.primer_inodo_libre = 0;
            sb.primer_bloque_libre = 0;
            sb.primer_indircto_libre = 0;

            //------------------escribiendo superbloque
            fsad = ftell(archivo);

            //int tam = sizeof (super_bloque);
            fwrite(&sb, sizeof (super_bloque), 1, archivo);
            fsad = ftell(archivo);

            //------------------escribiendo journaling
            for (long int x = 0; x < n; x++) {

                strcpy(j[x].fecha, "vacio");
                strcpy(j[x].operacion, "vacio");
            }



            fwrite(&j, sizeof (j), 1, archivo);
            fsad = ftell(archivo);


            //printf("%ld jorunao \n", ftell(archivo));


            //------------------escribiendo bm indoso
            for (long int x = 0; x < n; x++) {

                fwrite(&byte, sizeof (byte), 1, archivo);

            }
            fsad = ftell(archivo);
            //------------------escribiendo bm bloques *46n

            long int n46 = n * 46;

            for (long int x = 0; x < n46; x++) {

                fwrite(&byte, sizeof (byte), 1, archivo);

            }
            fsad = ftell(archivo);


            //-------------------escribiendo inodos
            t_inodos.llave = -1;
            strcpy(t_inodos.fecha, "vacio");
            t_inodos.tamano = -1;
            t_inodos.bloues_asignados = -1;
            t_inodos.tipo = -1;


            t_inodos.ad1 = -1;
            t_inodos.ad2 = -1;
            t_inodos.ad3 = -1;
            t_inodos.ad4 = -1;
            t_inodos.ap_n1 = -1;
            t_inodos.ap_n2 = -1;

            strcpy(t_inodos.propietario, "vacio");
            t_inodos.permisos = -1;


            strcpy(t_inodos.idchar, "vacio");


            for (long int x = 0; x < n; x++) {

                fwrite(&t_inodos, sizeof (inodo), 1, archivo);
            }
            fsad = ftell(archivo);
            //-------------------------------- escribiendo blques_indirectos 8*n

            temp_indirectos.ap1 = -1;
            temp_indirectos.ap2 = -1;
            temp_indirectos.ap3 = -1;
            temp_indirectos.ap4 = -1;
            temp_indirectos.ap5 = -1;
            temp_indirectos.ap6 = -1;
            temp_indirectos.pos = -1;
            temp_indirectos.tipo = -1;

            long int n8 = n * 8;
            for (long int x = 0; x < n8; x++) {
                fwrite(&temp_indirectos, sizeof (b_indirecto), 1, archivo);
            }

            fsad = ftell(archivo);

            //---------------------------------------------bloques *46n

            for (long int x = 0; x < n46; x++) {
                fseek(archivo, 84, SEEK_CUR);

            }
            fsad = ftell(archivo);
            //printf("pos final despues de darle formato %ld \n", ftell(archivo));
            fclose(archivo);
            crearraiz(part[numero].ruta, inicio, fin);
        } else {

            printf("error al crear formato");
        }


    }




}



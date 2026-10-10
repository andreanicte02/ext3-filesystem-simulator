/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mbr.h"
#include "creacion_discos.h"
#include "general.h"

// un bloque de 1 KB en ceros para llenar los discos (antes se escribia desde un char de 1 byte)
static const char kilo_de_ceros[1024] = {0};

//-------------------------generar ids

void mostrar(idspart part) {

    for (int i = 0; i < 20; i++) {

        printf("%s", part[i].id);
    }
}

void ids(char *nombre, idspart part, int n_par, char *ruta) {

    char indice = 'a';
    int existe = 0;
    for (int i = 0; i < 20; i++) {

        if (strcmp(nombre, part[i].nombre_disco) == 0) {//osea que sie xiste
            existe = 1;
            indice = part[i].letra;
            break;
        }

    }

    if (existe == 0) { //osea que no existe



        for (int i = 0; i < 20; i++) {
            //ENCONTRAR EL INDICE Y SUMARLE 1

            if (part[i].letra > indice) {

                indice = part[i].letra;

            }


        }

        indice++;

    }

    char nuev[50];
    sprintf(nuev, "vd%c%i", indice, n_par);



    for (int i = 0; i < 20; i++) {

        if (strcmp(part[i].id, "vacio") == 0) {

            strcpy(part[i].id, nuev);
            part[i].letra = indice;
            strcpy(part[i].nombre_disco, nombre);
            part[i].numero = n_par;
            strcpy(part[i].ruta, ruta);
            part[i].montado = 0;




            break;
        }
    }



    printf("el es para la particion: ----%s\n", nuev);



}

//-----------------------------------------

int hay_espacio(long int tamano_disco, long int inicio, long int tamano_particion) {

    if (inicio + tamano_particion < tamano_disco) {
        return 1;
    }
    return 0;

}
//------------recupera mbr

void particionar(char *nombre_disco, char *path, long int tamano, idspart part) {
    // el tamano ya debe venir en bytes

    FILE *disco = fopen(path, "r+b");
    char nombre[50];
    printf("nombre de la particion:\n"); //crear el id
    scanf("%s", nombre);


    if (disco != NULL) {

        mbr temporal = recuperar_mbr(path);

        if (temporal.par[0].estado == 0) {

            if (hay_espacio(temporal.tamano, sizeof (mbr), tamano) == 1) {
                temporal.par[0].estado = 1;
                temporal.par[0].inicio = sizeof (mbr);
                temporal.par[0].no_partition = 1;
                strcpy(temporal.par[0].nombre, nombre);
                temporal.par[0].tamano = tamano;
                ids(nombre_disco, part, 1, path);

            } else {

                printf("No hay espacio en el disco \n");
            }




        } else if (temporal.par[1].estado == 0) {

            if (hay_espacio(temporal.tamano, temporal.par[0].inicio + temporal.par[0].tamano, tamano) == 1) {
                temporal.par[1].estado = 1;
                temporal.par[1].inicio = temporal.par[0].inicio + temporal.par[0].tamano;
                temporal.par[1].no_partition = 2;
                strcpy(temporal.par[1].nombre, nombre);
                temporal.par[1].tamano = tamano;
                ids(nombre_disco, part, 2, path);
            } else {
                printf("No hay espacio \n");
            }



        } else if (temporal.par[2].estado == 0) {

            if (hay_espacio(temporal.tamano, temporal.par[1].inicio + temporal.par[1].tamano, tamano) == 1) {


                temporal.par[2].estado = 1;
                temporal.par[2].inicio = temporal.par[1].inicio + temporal.par[1].tamano;
                temporal.par[2].no_partition = 3;
                strcpy(temporal.par[2].nombre, nombre);
                temporal.par[2].tamano = tamano;
                ids(nombre_disco, part, 3, path);
            } else {
                printf("no hay espacion \n");
            }

        } else if (temporal.par[3].estado == 0) {

            if (hay_espacio(temporal.tamano, temporal.par[2].inicio + temporal.par[2].tamano, tamano) == 1) {

                temporal.par[3].estado = 1;
                temporal.par[3].inicio = temporal.par[2].inicio + temporal.par[2].tamano;
                temporal.par[3].no_partition = 4;
                strcpy(temporal.par[3].nombre, nombre);
                temporal.par[3].tamano = tamano;
                ids(nombre_disco, part, 4, path);
            } else {

                printf("no hay espacion \n");
            }

        }


        rewind(disco);
        fwrite(&temporal, sizeof (mbr), 1, disco);
        fclose(disco);

        /*
        char pbtiacora[150] = "/home/nicte/Escritorio/discos/bitacora.disk";
        FILE *bit = fopen(pbtiacora, "r+b");
        if (bit) { //si  existe
            idspart otros;
            for (int x = 0; x < 20; x++) {
                otros[x].formato = part[x].formato;
                strcpy(otros[x].id, part[x].id);
                otros[x].letra = part[x].letra;
                otros[x].montado = part[x].montado;
                strcpy(otros[x].nombre_disco, part[x].nombre_disco);
                otros[x].numero = part[x].numero;
                strcpy(otros[x].ruta, part[x].ruta);
            }
            
            fseek(bit, 0, SEEK_SET);
            fwrite(&otros, sizeof (idspart), 1, bit);
            //printf("%s", part[0].id);
            fclose(bit);
        }*/

        //mbr prueba = recuperar_mbr(path);
        //printf("%s -----!",prueba.par[0].nombre);
        //escribir mbr

    } else {
        printf("error al abrir el disco \n");
    }

}

void create_mbr(char *nombre, long int tamano, char *path) {
    mbr nuevo;
    strcpy(nuevo.nombre, nombre);
    strcpy(nuevo.path, path);
    nuevo.tamano = tamano; // tamano en  
    nuevo.particiones = 0;
    nuevo.libre = tamano;
    //---------------particiones

    for (int i = 0; i < 4; i++) {
        nuevo.par[i].estado = 0;
        nuevo.par[i].inicio = -1;
        nuevo.par[i].no_partition = -1;
        strcpy(nuevo.par[i].nombre, "vacia");
        nuevo.par[i].tamano = -1;
    }

    FILE *archivo = fopen(path, "r+b"); // si no es ab

    if (archivo != NULL) {

        rewind(archivo);
        fwrite(&nuevo, sizeof (mbr), 1, archivo);
        fclose(archivo);

    }
    //mbr temporal =recuperar_mbr(path);

    //printf("%s -- \n", temporal.par[2].nombre);

}

int existe_disco(char *path) {
    FILE *archivo = fopen(path, "rb");

    if (archivo == NULL) {



        return 0;
    }

    fclose(archivo);

    return 1;



}

int es_multiplo(long int numero) {

    if (numero >= 8) {

        if (numero % 8 == 0) {
            return 1;
        } else {
            return 0;

        }


    }
    return 0;

}

void c_disco() {

    char nombre[50];
    char nombre2[50];
    char path[150];
    long int tamano;
    char medida[10];

    printf("Ingrese el nombre del disco: \n");
    scanf(" %[^\n]s", nombre);

    strcpy(nombre2, nombre);

    printf("Ingrese la ruta donde se va a almacenar disco: \n");
    scanf(" %[^\n]s", path);

    printf("Tamano del disco: \n");
    scanf("%ld", &tamano);

    printf("Mega o Kilo: \n");
    scanf("%s", medida);


    strcat(path, "/");
    strcat(path, nombre2);
    strcat(path, ".disk");


    printf("ruta: %s \n", path);

    if (existe_disco(path) == 0 && es_multiplo(tamano) == 1) {
        FILE *archivo = fopen(path, "wb+");
        long int auxtamn = 0;

        if (archivo != NULL) {
            char byte = '\0';

            if (strcasecmp(medida, "m") == 0) {
                //megas      
                auxtamn = tamano * 1024 * 1024;
                for (long int conta = 0; conta < tamano; conta++) {
                    for (int x = 0; x < 1024; x++) {

                        fwrite(kilo_de_ceros, sizeof (byte), 1024, archivo);

                    }
                }
                printf("se creo el disco medida: mb \b");

            } else if (strcasecmp(medida, "k") == 0) {
                //kilos
                auxtamn = tamano * 1024;
                for (long int conta = 0; conta < tamano; conta++) {

                    fwrite(kilo_de_ceros, sizeof (byte), 1024, archivo);
                }
                printf("se creo el disco medida: kb \n");
            }
            //crear mbr
            fclose(archivo);
            create_mbr(nombre, auxtamn, path); // va el tamaño en bytes
        }
        //crear disco

    } else {

        printf("el archivo ya existe, o la ruta no existe, o el tamano no es multiplo de 8 \n");

    }


}

void el_disco() {


    char nombre[50];
    char nombre2[50];
    char path[150];

    printf("Ingrese el nombre del disco: \n");
    scanf(" %[^\n]s", nombre);
    strcpy(nombre2, nombre);

    printf("Ingrese la ruta del disco: \n");
    scanf(" %[^\n]s", path);

    strcat(path, "/");
    strcat(path, nombre2);
    strcat(path, ".disk");
    printf("%s\n", path);

    if (existe_disco(path) == 1) {
        char nuevo[1000];
        sprintf(nuevo, "rm -r %s", path);
        system(nuevo);


    } else {

        printf("no existe el disco, o la ruta esta equivocada \n");
    }



}

void aumentar_disco() {


    char nombre[50];
    char nombre2[50];
    char path[150];

    printf("Ingrese el nombre del disco: \n");
    scanf(" %[^\n]s", nombre);
    strcpy(nombre2, nombre);

    printf("Ingrese la ruta del disco: \n");
    scanf(" %[^\n]s", path);

    strcat(path, "/");
    strcat(path, nombre2);
    strcat(path, ".disk");

    if (existe_disco(path) == 1) {

        FILE *archivo = fopen(path, "ab+");

        if (archivo != NULL) {

            char byte = '\0';
            int tamano;
            printf("Ingrese el tamaño a aumentar en MB: \n");
            scanf("%i", &tamano);

            for (long int conta = 0; conta < tamano; conta++) {
                for (int x = 0; x < 1024; x++) {

                    fwrite(kilo_de_ceros, sizeof (byte), 1024, archivo);

                }
            }
            printf("Se aumento el disco %s \n", nombre2);

            fclose(archivo);
        }




    }




}


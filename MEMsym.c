#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_LINEAS 8 // Numero de lineas de la caché
#define TAM_LINEA 16 // Numero de bytes de cada linea de la caché
#define TAM_RAM 4096 // Tamaño de la RAM
#define TAM_TEXTO 100 // Tamaño del texto que puede almacenar con los accesos_memoria.txt

// Variables globales
int globaltime = 0;
int numfallos = 0;

// Estructura del proceso
typedef struct {
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
} T_CACHE_LINE;

// Prototipo de funciones
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_LINEAS]);
void VolcarCACHE(T_CACHE_LINE *tbl);
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque);
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque);

// Prototipo de funciones propias
void imprimirEtadisticas(char* texto, int numAccesos);
// Función que guarda el volcado de la caché en un archivo binario
void guardarCache(T_CACHE_LINE *tbl);

int main(void)
{
    T_CACHE_LINE tbl_cache[NUM_LINEAS]; // Variable caché
    unsigned char Simul_RAM[TAM_RAM]; // Variable que guarda lo leido del archivo binario (RAM), tamaño 4096 dicho por el enunciado
    unsigned int direccion_mem, ETQ, palabra, linea, bloque;


    // Variables que guardan el contenido que va pidiendo los accesos a memoria
    char texto[TAM_TEXTO];
    int pos_texto=0;

    int numAccesos = 0; // Cuenta el numero de accesos a caché

    LimpiarCACHE(tbl_cache);

    // Abrimos el fichero binario
    FILE *fd_bin = fopen("CONTENTS_RAM.bin", "rb");
    if (fd_bin == NULL) {
        perror("[-] Error al abrir el archivo binario\n");
        return -1;
    }

    // Leemos el archivo binario (RAM) y la guardamos
    if (fread(Simul_RAM, sizeof(unsigned char), TAM_RAM, fd_bin) < 0) {
        printf("[-] Error con la lecutra del archivo binario\n");
        return -1;
    }
    fclose(fd_bin); // Cerramos el archivo binario

    // Abrimos el fichero de texto
    FILE *fd_txt = fopen("accesos_memoria.txt", "r");
    if (fd_txt == NULL) {
        perror("[-] Error al abrir el archivo de texto\n");
        return -1;
    }

    while (fscanf(fd_txt, "%X", &direccion_mem) == 1) {
        // Dividimos la dirección en campos
        ParsearDireccion(direccion_mem, &ETQ, &palabra, &linea, &bloque);

        // Comprobamos si coinciden las etiquetas
        if (ETQ == tbl_cache[linea].ETQ) {
            printf("\nT: %d, Acierto de CACHE, ADDR %04X Label %X linea %02X palabra %02X DATO %02X\n", globaltime, direccion_mem, ETQ, linea, palabra, tbl_cache[linea].Data[palabra]);
            globaltime += 1;
        } else {
            numfallos++;
            printf("\nT: %d, Fallo de CACHE %d, ADDR %04X Label %X linea %02X palabra %02X bloque %02X\n", globaltime, numfallos, direccion_mem, ETQ, linea, palabra, bloque);
            printf("Cargando el bloque %02X en la linea %02X\n", bloque, linea);
            TratarFallo(tbl_cache, Simul_RAM, ETQ, linea, bloque);
            globaltime += 20;
        }
        texto[pos_texto++] = tbl_cache[linea].Data[palabra];
        
        numAccesos++;
        sleep(1);
    }
    texto[pos_texto] = '\0'; // Cerramos el string

    // Imprimimos el contenido de la caché
    VolcarCACHE(tbl_cache);

    imprimirEtadisticas(texto, numAccesos);
    
    // Cerramos el archivo txt
    fclose(fd_txt);

    return 0;
}

// Inicializamos la caché
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_LINEAS]) {
    // Inicializamos ETQ de todas las líneas de la caché
    for(int i=0; i<NUM_LINEAS; i++) {
        tbl[i].ETQ = 0xFF;

        // Inicializamos todos los campos de Data
        for(int j=0; j<TAM_LINEA; j++)
            tbl[i].Data[j] = 0x23;
    }
}

void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque) {
    // Hacemos una comparación lógica AND para quedarnos con los n primeros bits y después los movemos al inicio
    *ETQ = 0b111110000000 & addr;
    *ETQ = *ETQ >> 7;
    *linea = 0b000001110000 & addr;
    *linea = *linea >> 4;
    *palabra = 0b000000001111 & addr;
    *bloque = 0b111111110000 & addr;
    *bloque = *bloque >> 4;
}

// Guardamos en la dirección de la caché la etiqueta y el bloque de la RAM
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque) {
    tbl[linea].ETQ = ETQ;
    for(int i = 0; i < TAM_LINEA; i++)
        tbl[linea].Data[i] = MRAM[(bloque * TAM_LINEA) + i];
}

// Imprime el contenido de la caché y 
void VolcarCACHE(T_CACHE_LINE *tbl) {
    // Imprimimos el banner
    printf("\n\n+====================+\n");
    printf("| Contenido de Caché |\n");
    printf("+====================+\n");

    for(int linea=0; linea < NUM_LINEAS; linea++) {
        printf("%02X: ", tbl[linea].ETQ);
        for(int pos=TAM_LINEA - 1; pos >= 0; pos--) {
            printf("%02X ", tbl[linea].Data[pos]);
        }
        printf("\n");
    }
}

// Imprime las estadísticas del programa
void imprimirEtadisticas(char* texto, int numAccesos) {
    printf("\n\n+==============+\n");
    printf("| Estadísticas |\n");
    printf("+==============+\n");
    printf("[+] Número total de accesos : %d\n", numAccesos);
    printf("[+] Número de fallos        : %d\n", numfallos);
    printf("[+] Tiempo medio de acceso  : %.2f\n", (float)globaltime / (float)numAccesos);
    printf("[+] Texto leido             : %s\n", texto);
}

void guardarCache(T_CACHE_LINE *tbl) {
    // Creamos el archivo binario
    FILE *fd_bin = fopen("CONTENTS_CACHE.bin", "wb");

    for(int linea=0; linea < NUM_LINEAS; linea++) {
        for(int pos=0; pos < TAM_LINEA; pos++) {
            fwrite(&(tbl[linea].Data[pos]), sizeof(unsigned char), 1, fd_bin); // Escribimos los datos de la línea en el fichero
        }
    }
    fclose(fd_bin);
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TAM_LINEA 16
#define NUM_FILAS 8

// Variables globales
int globaltime = 0;
int numfallos = 0;

// Estructura del proceso
typedef struct {
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
} T_CACHE_LINE;

// Prototipo de funciones
void LimpiarCACHE(T_CACHE_LINE tbl_cache[NUM_FILAS]);
void VolcarCACHE(T_CACHE_LINE *tbl_cache);
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque);
void TratarFallo(T_CACHE_LINE *tbl_cache, char *MRAM, int ETQ, int linea, int bloque);

// Prototipo funcinoes propias
void guardarRam(unsigned char Simul_RAM[], unsigned char tbl_ram[], int tam_ram);

int main(void)
{
    T_CACHE_LINE tbl_cache[NUM_FILAS]; // Variable caché
    unsigned char Simul_RAM[4096]; // Variable que guarda lo leido del archivo binario (RAM), tamaño 4096 dicho por el enunciado

    // Guarda el número de caracteres leido de cada archivo
    int tam_leido_bin = 0;
    int tam_leido_txt = 0;

    LimpiarCACHE(tbl_cache);

    // Abrimos el fichero binario
    FILE *fd_bin = fopen("CONTENTS_RAM.bin", "rb");
    if (fd_bin == NULL) {
        perror("[-] Error al abrir el archivo binario\n");
        return -1;
    }

    // Abrimos el fichero de texto
    FILE *fd_txt = fopen("accesos_memoria.txt", "r");
    if (fd_txt == NULL) {
        perror("[-] Error al abrir el archivo de texto\n");
        return -1;
    }

    do{
        // Leemos el archivo binario (RAM) y la guardamos
        tam_leido_bin = fread(Simul_RAM, sizeof(unsigned char), 4096, fd_bin);
    } while (tam_leido_bin > 0);

    do{
        unsigned char direccion_mem_leida[4]; // Variable que guarda lo leido del archivo de texto, 4B = 3B caracteres hexa + 1B salto de linea
        unsigned int direccion_mem, ETQ, palabra, linea, bloque;

        // Leemos los archivos
        tam_leido_txt = fread(direccion_mem_leida, sizeof(unsigned char), 4, fd_txt);

        // Parseamos la dirección de str a hexadecimal y comprobamos que no se repita la última dirección
        if (tam_leido_txt >= 4)
        {
            direccion_mem = (unsigned int)strtol(direccion_mem_leida, NULL, 16);
        }

        // Dividimos la dirección hexadecimal en sus campos
        ParsearDireccion(direccion_mem, &ETQ, &palabra, &linea, &bloque);

        // Comprobamos si coinciden las etiquetas
        if (ETQ == tbl_cache[linea].ETQ) {
            globaltime += 1;
            printf("\nT: %d, Acierto de CACHE, ADDR %04X Label %X linea %02X palabra %02X DATO %02X\n", globaltime, direccion_mem, ETQ, linea, palabra, bloque);

            // Volcamos la caché
            VolcarCACHE(tbl_cache);
        } else {
            TratarFallo(tbl_cache, Simul_RAM, ETQ, linea, bloque);
            numfallos++;
            globaltime += 20;
            printf("\nT: %d, Fallo de CACHE %d, ADDR %04X Label %X linea %02X palabra %02X bloque %02X\n", globaltime, numfallos, direccion_mem, ETQ, linea, palabra, bloque);
        }

        // Ponemos la ejecución en pausa por 1 segundo e incrementamos el tiempo global
        sleep(1);
    } while (tam_leido_txt > 0);

    return 0;
}

// Inicializamos la caché
void LimpiarCACHE(T_CACHE_LINE tbl_cache[NUM_FILAS]) {
    // Inicializamos ETQ de todas las líneas de la caché
    for(int i=0; i<NUM_FILAS; i++) {
        tbl_cache[i].ETQ = 0xFF;

        // Inicializamos todos los campos de Data
        for(int j=0; j<TAM_LINEA; j++)
            tbl_cache[i].Data[j] = 0x23;
    }
}

void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque) {
    int pos_addr=0;

    // Hacemos una comparación lógica and para quedarnos con los n prmeros bits y después los movemos al inicio
    *ETQ = 0b111110000000 & addr;
    *ETQ = *ETQ >> 7;
    *linea = 0b000001110000 & addr;
    *linea = *linea >> 4;
    *palabra = 0b000000001111 & addr;
    *bloque = 0b111111110000 & addr;
    *bloque = *bloque >> 4;
}

void TratarFallo(T_CACHE_LINE *tbl_cache, char *MRAM, int ETQ, int linea, int bloque) {
    tbl_cache[linea].ETQ = ETQ;
    for(int i = 0; i < TAM_LINEA; i++)
        tbl_cache[linea].Data[i] = MRAM[(bloque * TAM_LINEA) + i];
}

void VolcarCACHE(T_CACHE_LINE *tbl_cache) {
    for(int linea=0; linea < NUM_FILAS; linea++) {
        printf("%X: ", tbl_cache[linea].ETQ);
        for(int pos=0; pos < TAM_LINEA; pos++) {
            printf("%02X ", tbl_cache[linea].Data[pos]);
        }
        printf("\n");
    }
}
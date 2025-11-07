#include <stdio.h>
#define TAM_LINEA 12
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
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]);
void VolcarCACHE(T_CACHE_LINE *tbl);
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque);
void TratarFallo(T_CACHE_LINE *tbl, char *MRAM, int ETQ, int linea, int bloque);

int main(void)
{
    // Variables
    T_CACHE_LINE tbl[NUM_FILAS];
    int tam_leido_bin = 0;
    int tam_leido_txt = 0;

    LimpiarCACHE(tbl);

    // Abrimos el fichero binario
    FILE *fd_bin = fopen("CONTENTS_RAM.bin", "rb");
    if (fd_bin == NULL) {
        perror("[-] Error al abrir el archivo binario\n");
        return -1;
    }

    // Abrimos el fichero de texto
    FILE *fd_txt = fopen("CONTENTS_RAM.bin", "r");
    if (fd_txt == NULL) {
        perror("[-] Error al abrir el archivo de texto\n");
        return -1;
    }

    do
    {
        unsigned char Simul_RAM[4096]; // Variable que guarda lo leido del archivo binario
        unsigned char direccion_mem[4]; // Variable que guarda lo leido del archivo de texto, 4 = 3 caracteres hexa + 1 salto de linea

        // Leemos los archivos
        tam_leido_bin = fread(Simul_RAM, sizeof(unsigned char), 4096, fd_bin);
        tam_leido_txt = fread(direccion_mem, sizeof(unsigned char), 4, fd_txt);



    } while (tam_leido_bin > 0 && tam_leido_txt > 0);
    
    

    return 0;
}

// Inicializamos la caché
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]) {
    // Inicializamos ETQ de todas las líneas de la caché
    for(int i=0; i<NUM_FILAS; i++) {
        tbl[i].ETQ = 0xFF;

        // Inicializamos todos los campos de Data
        for(int j=0; j<TAM_LINEA; i++)
            tbl[i].Data[j] = 0x23;
    }
}

void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int *linea, int *bloque) {
    // Guardamos la ETQ
    
}


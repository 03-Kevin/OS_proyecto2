#include <stdio.h>
#include <string.h>
#include <ctype.h> // Necesario para la función isalpha()

// Función para cifrar el mensaje usando el cifrado Vigenère
void cifrarVigenere(char mensaje[], char clave[]) {
    int claveLongitud = strlen(clave);
    int i = 0;
    while (mensaje[i]) {
        char c = mensaje[i];
        if (isalpha(c)) { // Solo cifra caracteres alfabéticos
            int desplazamiento = toupper(clave[i % claveLongitud]) - 'A'; // Convertir la letra de la clave en un desplazamiento
            if (c >= 'a' && c <= 'z') {
                c = ((c - 'a' + desplazamiento) % 26) + 'a';
            } else if (c >= 'A' && c <= 'Z') {
                c = ((c - 'A' + desplazamiento) % 26) + 'A';
            }
            mensaje[i] = c;
        }
        i++;
    }
}

int main() {
    FILE *file;
    FILE *criptFile;
    char mensaje[1000];
    char clave[] = "JASON";

    // Abre el archivo en modo lectura
    file = fopen("text.txt", "r");
    if (file == NULL) {
        printf("No se pudo abrir el archivo.\n");
        return 1;
    }

    // Abre el archivo cript.txt en modo escritura
    criptFile = fopen("cripto.txt", "w");
    if (criptFile == NULL) {
        printf("No se pudo abrir el archivo cript.txt.\n");
        return 1;
    }

    // Lee el contenido del archivo línea por línea
    while (fgets(mensaje, 1000, file) != NULL) {
        // Cifra la línea
        cifrarVigenere(mensaje, clave);

        // Escribe la línea cifrada en cript.txt
        fputs(mensaje, criptFile);
    }

    // Cierra los archivos
    fclose(file);
    fclose(criptFile);

    return 0;
}

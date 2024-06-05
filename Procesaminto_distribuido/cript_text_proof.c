#include <stdio.h>
#include <string.h>
#include <ctype.h> // Necesario para la función isalpha()

// Función para descifrar el mensaje usando el cifrado Vigenère
void descifrarVigenere(char mensaje[], char clave[]) {
    int claveLongitud = strlen(clave);
    int i = 0;
    while (mensaje[i]) {
        char c = mensaje[i];
        if (isalpha(c)) { // Solo descifra caracteres alfabéticos
            int desplazamiento = toupper(clave[i % claveLongitud]) - 'A'; // Convertir la letra de la clave en un desplazamiento
            if (c >= 'a' && c <= 'z') {
                c = ((c - 'a' - desplazamiento + 26) % 26) + 'a';
            } else if (c >= 'A' && c <= 'Z') {
                c = ((c - 'A' - desplazamiento + 26) % 26) + 'A';
            }
            mensaje[i] = c;
        }
        i++;
    }
}

int main() {
    FILE *file;
    FILE *descifradoFile;
    char mensaje[1000];
    char clave[] = "JASON";

    // Abre el archivo en modo lectura
    file = fopen("cripto.txt", "r");
    if (file == NULL) {
        printf("No se pudo abrir el archivo.\n");
        return 1;
    }

    // Abre el archivo descifrado.txt en modo escritura
    descifradoFile = fopen("descifrado.txt", "w");
    if (descifradoFile == NULL) {
        printf("No se pudo abrir el archivo descifrado.txt.\n");
        return 1;
    }

    // Lee el contenido del archivo línea por línea
    while (fgets(mensaje, 1000, file) != NULL) {
        // Descifra la línea
        descifrarVigenere(mensaje, clave);

        // Escribe la línea descifrada en descifrado.txt
        fputs(mensaje, descifradoFile);
    }

    // Cierra los archivos
    fclose(file);
    fclose(descifradoFile);

    return 0;
}

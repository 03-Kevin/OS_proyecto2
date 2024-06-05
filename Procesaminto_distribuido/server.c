#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <c++/11/bits/ranges_util.h>
#include <stdbool.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SHIFT 3 // Desplazamiento para el cifrado César

#define MAX_WORD_LENGTH 100
#define MAX_WORDS 10000

typedef struct
{
    char word[MAX_WORD_LENGTH];
    int count;
} WordCount;

void toLowerCase(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = tolower(str[i]);
    }
}

void countWords(char *buffer, WordCount *wordCounts, int *numWords)
{
    char *token = strtok(buffer, " .,:;?");
    while (token != NULL)
    {
        if (!isdigit(token[0])) // Ignorar los tokens que comienzan con un número
        {
            toLowerCase(token);
            int found = 0;
            for (int i = 0; i < *numWords; i++)
            {
                if (strcmp(wordCounts[i].word, token) == 0)
                {
                    wordCounts[i].count++;
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                strcpy(wordCounts[*numWords].word, token);
                wordCounts[*numWords].count = 1;
                (*numWords)++;
            }
        }
        token = strtok(NULL, " .,:;?\n");
    }
}

void descifrarCesar(char mensaje[], int shift)
{
    for (int i = 0; mensaje[i]; i++)
    {
        if (isalpha(mensaje[i]))
        {
            char base = islower(mensaje[i]) ? 'a' : 'A';
            mensaje[i] = (mensaje[i] - base - shift + 26) % 26 + base;
        }
    }
}
void divideBuffer(char *buffer, long fileSize, char **buffer1, char **buffer2)
{
    long halfSize = fileSize / 2;
    while (buffer[halfSize] != ' ' && halfSize < fileSize)
    {
        halfSize++;
    }

    *buffer1 = malloc(halfSize + 1);
    *buffer2 = malloc(fileSize - halfSize + 1);

    strncpy(*buffer1, buffer, halfSize);
    (*buffer1)[halfSize] = '\0';

    strncpy(*buffer2, buffer + halfSize, fileSize - halfSize);
    (*buffer2)[fileSize - halfSize] = '\0';
}

void combineWordCounts(WordCount *wordCounts1, int numWords1, WordCount *wordCounts2, int numWords2, WordCount *combinedWordCounts, int *numCombinedWords)
{
    for (int i = 0; i < numWords1; i++)
    {
        combinedWordCounts[i] = wordCounts1[i];
    }
    *numCombinedWords = numWords1;

    for (int i = 0; i < numWords2; i++)
    {
        bool found = false;
        for (int j = 0; j < *numCombinedWords; j++)
        {
            if (strcmp(wordCounts2[i].word, combinedWordCounts[j].word) == 0)
            {
                combinedWordCounts[j].count += wordCounts2[i].count;
                found = true;
                break;
            }
        }
        if (!found)
        {
            combinedWordCounts[*numCombinedWords] = wordCounts2[i];
            (*numCombinedWords)++;
        }
    }
}

void findMostFrequentWord(WordCount *wordCounts, int numWords, char **mostFrequentWord, int *maxCount)
{
    *maxCount = 0;
    for (int i = 0; i < numWords; i++)
    {
        if (wordCounts[i].count > *maxCount)
        {
            *maxCount = wordCounts[i].count;
            *mostFrequentWord = wordCounts[i].word;
        }
    }
}

int main()
{
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("Error al crear el socket");
        return 1;
    }

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Error al hacer bind al socket");
        close(server_sock);
        return 1;
    }

    if (listen(server_sock, 1) < 0)
    {
        perror("Error al escuchar en el socket");
        close(server_sock);
        return 1;
    }

    printf("Servidor escuchando en el puerto %d\n", PORT);

    int client_sock = accept(server_sock, NULL, NULL);
    if (client_sock < 0)
    {
        perror("Error al aceptar la conexión");
        close(server_sock);
        return 1;
    }

    printf("Conexión aceptada\n");

    FILE *file_cifrado = fopen("cifrados.txt", "w");
    FILE *file_descifrado = fopen("descifrados.txt", "w");
    if (file_cifrado == NULL || file_descifrado == NULL)
    {
        perror("Error al abrir archivos de salida");
        close(client_sock);
        close(server_sock);
        return 1;
    }

    char buffer[BUFFER_SIZE + 1];
    ssize_t n;
    while ((n = read(client_sock, buffer, BUFFER_SIZE)) > 0)
    {
        buffer[n] = '\0'; // Asegurar la terminación nula
        fprintf(file_cifrado, "%s", buffer);

        descifrarCesar(buffer, SHIFT);
        fprintf(file_descifrado, "%s", buffer);
    }

    fclose(file_cifrado);
    fclose(file_descifrado);
    close(client_sock);
    close(server_sock);

    FILE *file = fopen("descifrados.txt", "r");
    if (file == NULL)
    {
        perror("Error al abrir el archivo descifrado");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer_total = malloc(fileSize + 1);
    fread(buffer_total, 1, fileSize, file);
    buffer_total[fileSize] = '\0';

    fclose(file);

    char *buffer1, *buffer2;
    divideBuffer(buffer_total, fileSize, &buffer1, &buffer2);

    WordCount wordCounts1[MAX_WORDS], wordCounts2[MAX_WORDS];
    int numWords1 = 0, numWords2 = 0;

    countWords(buffer1, wordCounts1, &numWords1);
    countWords(buffer2, wordCounts2, &numWords2);

    WordCount combinedWordCounts[MAX_WORDS];
    int numCombinedWords = 0;

    combineWordCounts(wordCounts1, numWords1, wordCounts2, numWords2, combinedWordCounts, &numCombinedWords);

    // ...

    char *mostFrequentWord;
    int maxCount;

    findMostFrequentWord(combinedWordCounts, numCombinedWords, &mostFrequentWord, &maxCount);

    printf("La palabra más frecuente es '%s' con una frecuencia de %d\n", mostFrequentWord, maxCount);

    // ...

    free(buffer1);
    free(buffer2);
    free(buffer_total);

    return 0;
}
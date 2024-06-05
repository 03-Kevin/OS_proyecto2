#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

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
        if (!isdigit(token[0]))
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

int main(int argc, char **argv)
{
    int rank, size;
    char buffer[BUFFER_SIZE];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2)
    {
        if (rank == 0)
        {
            printf("Este programa requiere exactamente 2 nodos (maestro y esclavo).\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0)
    {
        FILE *file_cifrado = fopen("cifrados.txt", "w");
        FILE *file_descifrado = fopen("decifrados.txt", "w");
        if (file_cifrado == NULL || file_descifrado == NULL)
        {
            perror("Error al abrir archivos de salida");
            MPI_Finalize();
            return 1;
        }

        while (fgets(buffer, BUFFER_SIZE, stdin) != NULL)
        {
            fputs(buffer, file_cifrado);

            descifrarCesar(buffer, SHIFT);
            fputs(buffer, file_descifrado);
        }

        fclose(file_cifrado);
        fclose(file_descifrado);

        FILE *file = fopen("decifrados.txt", "r");
        if (file == NULL)
        {
            perror("Error al abrir el archivo descifrado");
            MPI_Finalize();
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
        long halfSize = fileSize / 2;
        while (buffer_total[halfSize] != ' ' && halfSize < fileSize)
        {
            halfSize++;
        }

        buffer1 = malloc(halfSize + 1);
        buffer2 = malloc(fileSize - halfSize + 1);

        strncpy(buffer1, buffer_total, halfSize);
        buffer1[halfSize] = '\0';

        strncpy(buffer2, buffer_total + halfSize, fileSize - halfSize);
        buffer2[fileSize - halfSize] = '\0';

        WordCount wordCounts1[MAX_WORDS], wordCounts2[MAX_WORDS];
        int numWords1 = 0, numWords2 = 0;

        countWords(buffer1, wordCounts1, &numWords1);
        countWords(buffer2, wordCounts2, &numWords2);

        WordCount combinedWordCounts[MAX_WORDS];
        int numCombinedWords = 0;

        combineWordCounts(wordCounts1, numWords1, wordCounts2, numWords2, combinedWordCounts, &numCombinedWords);

        char *mostFrequentWord;
        int maxCount;

        findMostFrequentWord(combinedWordCounts, numCombinedWords, &mostFrequentWord, &maxCount);

        printf("La palabra más frecuente es '%s' con una frecuencia de %d\n", mostFrequentWord, maxCount);

        free(buffer1);
        free(buffer2);
        free(buffer_total);
    }
    else
    {
        if (rank == 1)
        {
            MPI_Status status;
            MPI_Recv(buffer, BUFFER_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);

            WordCount wordCounts[MAX_WORDS];
            int numWords = 0;

            countWords(buffer, wordCounts, &numWords);

            MPI_Send(wordCounts, MAX_WORDS * sizeof(WordCount), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}

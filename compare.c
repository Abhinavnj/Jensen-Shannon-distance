#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <math.h>

#include "queue.h"
#include "linkedlist.h"

int readOptionalArgs (int argc, char *argv[], int* directoryThreads, int* fileThreads, int* analysisThreads, char** fileNameSuffix);
int readRegArgs (int argc, char *argv[], char** fileNameSuffix, queue_t* fileQ, queue_t* dirQ);
int startsWith (char* str, char* prefix);
int endsWith (char* str, char* suffix);
int isDir (char* path);
int isReg (char* path);
char** getFileWords (FILE* fp, int* wordCount);
int calculateWFD(Node** head, int wordCount);
double calculateMeanFreq(Node* file1, Node* file2, char* word);
double calculateKLD(Node* calcFile, Node* suppFile);
double calculateJSD(Node* file1, Node* file2);
Node* fileWFD(char* filepath);

int main (int argc, char *argv[])
{
    // optional arguments
    int directoryThreads = 1;
    int fileThreads = 1;
    int analysisThreads = 1;
    char* fileNameSuffix = ".txt";

    queue_t fileQ, dirQ;
    init(&fileQ);
    init(&dirQ);

    int rc = EXIT_SUCCESS;

    // rc = readOptionalArgs(argc, argv, &directoryThreads, &fileThreads, &analysisThreads, &fileNameSuffix);
    // if (rc) {
    //     return rc;
    // }
    
    // readRegArgs(argc, argv, &fileNameSuffix, &fileQ, &dirQ);

    Node* head1 = fileWFD(argv[1]);

    Node* head2 = fileWFD(argv[2]);

    printf("%f\n", calculateJSD(head1, head2));

    // printList(head1);
    freeList(head1);
    // printList(head2);
    freeList(head2);

    return rc;
}

Node* fileWFD(char* filepath) {
    Node* head = NULL;
    initHead(head);

    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) {
        perror("file opening failure");
        return NULL;
    }

    int wordCount = 0;
    char** words = getFileWords(fp, &wordCount);
    for (int i = 0; i < wordCount; i++) {
        insertNode(&head, words[i]);
    }

    calculateWFD(&head, wordCount);

    return head;
}

double calculateJSD(Node* file1, Node* file2){
    return sqrt(0.5 * calculateKLD(file1, file2) + 0.5 * calculateKLD(file2, file1));
}

double calculateKLD(Node* calcFile, Node* suppFile) {
    double kldValue = 0;

    Node* ptr = calcFile;
    while (ptr != NULL) {
        double meanFrequency = calculateMeanFreq(calcFile, suppFile, ptr->word);
        kldValue += (ptr->frequency * log2(ptr->frequency / meanFrequency));
        ptr = ptr->next;
    }

    return kldValue;
}

double calculateMeanFreq(Node* file1, Node* file2, char* word) {
    return 0.5 * (frequencyByWord(file1, word) + frequencyByWord(file2, word));
}

int calculateWFD(Node** head, int wordCount) {
    double totalFreq = 0;

    Node* ptr = *head;
    while (ptr != NULL) {
        ptr->frequency = (ptr->count / (double) wordCount);
        totalFreq += ptr->frequency;
        ptr = ptr->next;
    }

    return EXIT_SUCCESS;
}

char** getFileWords(FILE* fp, int* wordCount) {
    char** words = malloc(0);

    char c;
    char* buf = malloc(5);
    strcpy(buf, "\0");
    int bufsize = 5;

    while ((c = fgetc(fp)) != EOF) {
        // generate current word
        if (!isspace(c)) {
            if (!ispunct(c)) {
                size_t len = strlen(buf);
                if (len >= (bufsize - 1)){
                    buf = realloc(buf, len * 2);
                    bufsize = len * 2;
                }
                buf[len++] = c;
                buf[len] = '\0';
            }
        }
        else { // insert word into list
            ++(*wordCount);

            int index = (*wordCount) - 1;
            words = realloc(words, (*wordCount) * sizeof(char*));
            words[index] = malloc(sizeof(buf));

            for(int i = 0; buf[i]; i++){
                buf[i] = tolower(buf[i]);
            }
            strcpy(words[index], buf);

            // clear out buffer
            buf = realloc(buf, 5);
            strcpy(buf, "\0");
            bufsize = 5;
        }
    }

    // if file does not end with a new line
    if (strcmp(buf, "\0") != 0) {
        ++(*wordCount);

        int index = (*wordCount) - 1;
        words = realloc(words, (*wordCount) * sizeof(char*));
        words[index] = malloc(sizeof(buf));

        for(int i = 0; buf[i]; i++){
            buf[i] = tolower(buf[i]);
        }
        strcpy(words[index], buf);
    }

    return words;
}

int readOptionalArgs (int argc, char *argv[], int* directoryThreads, int* fileThreads, int* analysisThreads, char** fileNameSuffix) {
    for (int i = 1; i < argc; i++) {
        if (startsWith(argv[i], "-") == 0) {
            int len = strlen(argv[i]) - 1;
            char substring[len];
            memcpy(substring, argv[i] + 2, len);
            substring[len - 1] = '\0';  

            if (startsWith(argv[i], "-s") == 0) {
                *fileNameSuffix = substring;
                printf("%s\n", *fileNameSuffix);
            }
            else if (startsWith(argv[i], "-f") == 0) {
                *fileThreads = atoi(substring);
                printf("%d\n", *fileThreads);
            }
            else if (startsWith(argv[i], "-d") == 0) {
                *directoryThreads = atoi(substring);
                printf("%d\n", *directoryThreads);
            }
            else if (startsWith(argv[i], "-a") == 0) {
                *analysisThreads = atoi(substring);
                printf("%d\n", *analysisThreads);
            }
        }
        else if (isReg(argv[i]) && isDir(argv[i])) {
            perror("incorrect arguments");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int readRegArgs (int argc, char *argv[], char** fileNameSuffix, queue_t* fileQ, queue_t* dirQ) {
    for (int i = 1; i < argc; i++) {
        if (startsWith(argv[i], "-") == 0) {
            continue;
        }
        else if (!isReg(argv[i])) {
            if (!endsWith(argv[1], *fileNameSuffix)) {
                enqueue(fileQ, argv[i]);
            }
        }
        else if (!isDir(argv[i])) {
            enqueue(dirQ, argv[i]);
        }
    }

    return EXIT_SUCCESS;
}

int startsWith (char* str, char* prefix) {
    for (int i = 0; i < strlen(prefix); i++) {
        if (str[i] != prefix[i]) {
            return 1;
        }
    }

    return 0;
}

int endsWith (char* str, char* suffix) {
    for (int i = 0; i < strlen(suffix); i++) {
        if (str[strlen(str) - strlen(suffix) + i] != suffix[i]) {
            return 1;
        }
    }

    return 0;
}

int isDir (char* path) {
    struct stat arg_data;
    if (stat(path, &arg_data) == 0) {
        if (S_ISDIR(arg_data.st_mode)) {
            return 0;
        }
    }
    else {
        return -1; // does not exist
    }

    return 1;
}

int isReg (char* path) {
    struct stat arg_data;
    if (stat(path, &arg_data) == 0) {
        if (S_ISREG(arg_data.st_mode)) {
            return 0;
        }
    }
    else {
        return -1;
    }

    return 1;
}
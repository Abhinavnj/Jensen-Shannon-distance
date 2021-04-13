#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "queue.h"
#include "linkedlist.h"

int readOptionalArgs (int argc, char *argv[], int* directoryThreads, int* fileThreads, int* analysisThreads, char** fileNameSuffix);
int readRegArgs (int argc, char *argv[], char** fileNameSuffix, queue_t* fileQ, queue_t* dirQ);
int startsWith (char* str, char* prefix);
int endsWith (char* str, char* suffix);
int isDir (char* path);
int isReg (char* path);
int calculateWFD(char* filename);

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

    // enqueue(&fileQ, "hello");
    // printf("%s\n", fileQ.data[0]);
    // printf("%s\n", dirQ.data[0]);
    // enqueue(&fileQ, "hello mom");
    // printf("%s\n", fileQ.data[1]);
    // printf("%s\n", dirQ.data[1]);
    // enqueue(&fileQ, "hello daddy");
    // printf("%s\n", fileQ.data[2]);

    // char* rv;
    // dequeue(&fileQ, &rv);

    // destroy(&fileQ);

    // Node* head = NULL;
    // initHead(head);

    // printList(head);
    // freeList(head);

    calculateWFD(argv[1]);

    return rc;
}

int calculateWFD(char* filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("file error");
        return EXIT_FAILURE;
    }

    char buf[100];
    while (fscanf(fp, "%s", buf) == 1) {
        printf("%s\n", buf);
    }

    return EXIT_SUCCESS;
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
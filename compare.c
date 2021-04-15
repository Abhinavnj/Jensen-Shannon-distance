#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <math.h>
#include <dirent.h>

#include "queueU.h"
#include "queueB.h"
#include "linkedlist.h"
#include "filenode.h"

int readRegArgs(int argc, char *argv[], char** fileNameSuffix, queueB_t* fileQ, queueU_t* dirQ);
int readOptionalArgs(int argc, char *argv[], int* directoryThreads, int* fileThreads, int* analysisThreads, char** fileNameSuffix);
int isReg(char* path);
int isDir(char* path);
int startsWith(char* str, char* prefix);
int endsWith(char* str, char* suffix);
void* fileThread(void *argptr);
void* dirThread(void *argptr);
int fileWFD(char* filepath, FileNode** WFDrepo);
char** getFileWords(FILE* fp, int* wordCount);
int calculateWFD(Node** head, int wordCount);
double calculateMeanFreq(Node* file1, Node* file2, char* word);
double calculateKLD(Node* calcFile, Node* suppFile);
double calculateJSD(Node* file1, Node* file2);

/* 
read options (determine number of threads to create)
create/initialize queues
start requested number of file and directory threads
add directories listed in arguments to directory queue
add files listed in arguments to file queue
join file and directory threads
... continue to phase 2 */

typedef struct file_arg {
    queueB_t* fileQ;
    FileNode** WFDrepo;
    int* activeThreads;
} f_arg;

typedef struct dir_arg {
    queueU_t* dirQ;
    queueB_t* fileQ;
    int* activeThreads;
} d_arg;

int main (int argc, char *argv[])
{
    // optional arguments
    int directoryThreads = 1;
    int fileThreads = 1;
    int analysisThreads = 1;

    char* defaultSuffix = ".txt";
    char* fileNameSuffix = malloc(sizeof(defaultSuffix) + 1);
    memcpy(fileNameSuffix, defaultSuffix, strlen(defaultSuffix));
    fileNameSuffix[strlen(defaultSuffix)] = '\0';

    int rc = EXIT_SUCCESS;

    rc = readOptionalArgs(argc, argv, &directoryThreads, &fileThreads, &analysisThreads, &fileNameSuffix);
    if (rc) {
        return rc;
    }

    // printf("d: %d, f: %d, a: %d, suf: %s\n", directoryThreads, fileThreads, analysisThreads, fileNameSuffix);

    queueB_t fileQ;
    queueU_t dirQ;
    initB(&fileQ);
    initU(&dirQ);

    FileNode* WFDrepo = NULL;
    initFile(WFDrepo);

    int activeThreads = 0;
    void* retval = NULL;

    readRegArgs(argc, argv, &fileNameSuffix, &fileQ, &dirQ);

    // start dir threads
    pthread_t* dir_tids = malloc(directoryThreads * sizeof(pthread_t)); // hold thread ids
    d_arg* dir_args = malloc(directoryThreads * sizeof(d_arg)); // hold arguments
    
    // initialize all arguments
    for (int i = 0; i < directoryThreads; i++) {
        dir_args[i].dirQ = &dirQ;
        dir_args[i].fileQ = &fileQ;
        dir_args[i].activeThreads = &activeThreads;
    }
    
    // start all threads
    for (int i = 0; i < directoryThreads; i++) {
        pthread_create(&dir_tids[i], NULL, dirThread, &dir_args[i]);
    }

    // start file threads
    pthread_t* file_tids = malloc(fileThreads * sizeof(pthread_t)); // hold thread ids
    f_arg* file_args = malloc(fileThreads * sizeof(f_arg)); // hold arguments
    
    // initialize all arguments
    for (int i = 0; i < fileThreads; i++) {
        file_args[i].fileQ = &fileQ;
        file_args[i].WFDrepo = &WFDrepo;
        file_args[i].activeThreads = &activeThreads;
    }
    
    // start all threads
    for (int i = 0; i < fileThreads; i++) {
        pthread_create(&file_tids[i], NULL, fileThread, &file_args[i]);
    }

    // join file and directory threads
    // wait for all threads to finish
    for (int i = 0; i < fileThreads; i++) {
        pthread_join(file_tids[i], &retval);
    }

    for (int i = 0; i < directoryThreads; i++) {
        pthread_join(dir_tids[i], &retval);
        if ((int)retval == EXIT_FAILURE) {
            rc = EXIT_FAILURE;
        }
        free(retval);
    }

    printFileList(WFDrepo);
    // printf("file count %d\n", fileQ.count);
    // printf("dir count %d\n", dirQ.count);

    destroyU(&dirQ);
    destroyB(&fileQ);

    // freeing
    free(file_tids);
    free(dir_tids);

    // TODO: analysis threads

    free(fileNameSuffix);

    return rc;
}

int readRegArgs (int argc, char *argv[], char** fileNameSuffix, queueB_t* fileQ, queueU_t* dirQ) {
    for (int i = 1; i < argc; i++) {
        if (startsWith(argv[i], "-") == 0) {
            continue;
        }
        else if (!isReg(argv[i])) {
            if (!endsWith(argv[1], *fileNameSuffix)) {
                enqueueB(fileQ, argv[i]);
            }
        }
        else if (!isDir(argv[i])) {
            enqueueU(dirQ, argv[i]);
        }
    }

    return EXIT_SUCCESS;
}

int readOptionalArgs (int argc, char *argv[], int* directoryThreads, int* fileThreads, int* analysisThreads, char** fileNameSuffix) {
    for (int i = 1; i < argc; i++) {
        if (startsWith(argv[i], "-") == 0) {
            int len = strlen(argv[i]) - 1;
            char* substring = malloc(len);
            memcpy(substring, argv[i] + 2, len);
            substring[len - 1] = '\0';

            if (startsWith(argv[i], "-s") == 0) {
                int suffixLen = strlen(substring) + 1;
                *fileNameSuffix = realloc(*fileNameSuffix, suffixLen);
                strcpy(*fileNameSuffix, substring);
            }
            else if (startsWith(argv[i], "-f") == 0) {
                *fileThreads = atoi(substring);
            }
            else if (startsWith(argv[i], "-d") == 0) {
                *directoryThreads = atoi(substring);
            }
            else if (startsWith(argv[i], "-a") == 0) {
                *analysisThreads = atoi(substring);
            }

            free(substring);
        }
        else if (isReg(argv[i]) && isDir(argv[i])) {
            perror("incorrect arguments");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
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

int startsWith (char* str, char* prefix) {
    for (int i = 0; i < strlen(prefix); i++) {
        if (str[i] != prefix[i]) {
            return 1;
        }
    }

    return 0;
}

int endsWith (char* str, char* suffix) {
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr) {
        return 0;
    }
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void* dirThread(void *argptr) {
    int* retval = malloc(sizeof(int));
    *retval = EXIT_SUCCESS;

    d_arg* args = (d_arg*) argptr;

    queueU_t* dirQ = args->dirQ;
    queueB_t* fileQ = args->fileQ;
    int* activeThreads = args->activeThreads;

    while (dirQ->count != 0 || *activeThreads > 0) {
        if (dirQ->count == 0) {
            --(*activeThreads);
            if (*activeThreads == 0) {
                pthread_cond_broadcast(&dirQ->read_ready);
                qcloseB(fileQ);
                return retval;
            }
            while (dirQ->count != 0 || *activeThreads != 0) {
                pthread_cond_wait(&dirQ->read_ready, &dirQ->lock);
            }
            if (*activeThreads == 0) {
                return retval;
            }
        }
        ++(*activeThreads);

        char* dirPath;
        dequeueU(dirQ, &dirPath);

        struct dirent* parentDirectory;
        DIR* parentDir;

        parentDir = opendir(dirPath);
        if (!parentDir) {
            perror("Failed to open directory!\n");
            *retval = EXIT_FAILURE;
            return retval;
        }

        struct stat data;
        char* subpathname;
        char* subpath = malloc(0);
        while ((parentDirectory = readdir(parentDir))) {
            subpathname = parentDirectory->d_name;
            if (subpathname[0] != '.') {
                int subpath_size = strlen(dirPath)+strlen(subpathname) + 2;
                subpath = realloc(subpath, subpath_size * sizeof(char));
                strcpy(subpath, dirPath);
            
                strcat(subpath, "/");
                strcat(subpath, subpathname);

                stat(subpath, &data);
                if (!isReg(subpath) && subpath[0] != '.') {
                    enqueueB(fileQ, subpath);
                }
                else if (!isDir(subpath) && subpath[0] != '.') {
                    enqueueU(dirQ, subpath);
                }
            }
        }
    }

    return retval;
}

void* fileThread(void *argptr) {
    int* retval = malloc(sizeof(int));
    *retval = EXIT_SUCCESS;

    f_arg* args = (f_arg*) argptr;

    queueB_t* fileQ = args->fileQ;
    FileNode** WFDrepo = args->WFDrepo;
    int* activeThreads = args->activeThreads;

    while (fileQ->count > 0 || *activeThreads > 0) { // TODO: also check all directory threads have stopped
        char* filepath = NULL;
        dequeueB(fileQ, &filepath);
        fileWFD(filepath, WFDrepo);
    }

    return retval;
}

int fileWFD(char* filepath, FileNode** WFDrepo) {
    Node* head = NULL;
    initHead(head);

    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) {
        perror("file opening failure");
        return EXIT_FAILURE;
    }

    int wordCount = 0;
    char** words = getFileWords(fp, &wordCount);
    for (int i = 0; i < wordCount; i++) {
        insertNode(&head, words[i]);
    }

    for (int i = 0; i < wordCount; i++) {
        free(words[i]);
    }
    free(words);

    calculateWFD(&head, wordCount);

    insertFileNode(WFDrepo, &head, filepath, wordCount);

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
    // if (strcmp(buf, "\0") != 0) {
    //     ++(*wordCount);

    //     int index = (*wordCount) - 1;
    //     words = realloc(words, (*wordCount) * sizeof(char*));
    //     words[index] = malloc(sizeof(buf));

    //     for(int i = 0; buf[i]; i++){
    //         buf[i] = tolower(buf[i]);
    //     }
    //     strcpy(words[index], buf);
    // }

    free(buf);

    return words;
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

double calculateMeanFreq(Node* file1, Node* file2, char* word) {
    return 0.5 * (frequencyByWord(file1, word) + frequencyByWord(file2, word));
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

double calculateJSD(Node* file1, Node* file2){
    return sqrt(0.5 * calculateKLD(file1, file2) + 0.5 * calculateKLD(file2, file1));
}
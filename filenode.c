#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filenode.h"

int initFile (FileNode* fileNodeHead) {
    fileNodeHead = NULL;

    return EXIT_SUCCESS;
}
int insertFileNode(FileNode** fileNodeHead, Node** head, char* filename, int wordCount) {
    FileNode* newNode = malloc(sizeof(FileNode));
    newNode->head = *head;
    int len = strlen(filename) + 1;
    newNode->filename = malloc(len);
    memcpy(newNode->filename, filename, len);
    newNode->next = NULL;
    newNode->wordCount = wordCount;

    if ((*fileNodeHead) == NULL) {
        *fileNodeHead = newNode;
        return 0;
    }
    else if (strcmp((*fileNodeHead)->filename, filename) > 0) {
        newNode->next = *fileNodeHead;
        *fileNodeHead = newNode;
        return 0;
    }
    else {
        FileNode* current = *fileNodeHead;
        FileNode* prev = *fileNodeHead;
        while (current != NULL){
            if (strcmp(current->filename, filename) > 0) {
                newNode->next = current;
                prev->next = newNode;
                return 0;
            }
            prev = current;
            current = current->next;
        }
        prev->next = newNode;
    }

    return 0;
}

void printFileList(FileNode* fileNodeHead) {
    FileNode* ptr = fileNodeHead;
    while (ptr != NULL) {
        printf("%s: %d\t", ptr->filename, ptr->wordCount);
        ptr = ptr->next;
    }
    printf("\n");
}

void freeFileList(FileNode* fileNodeHead) {
    FileNode* tempNode = fileNodeHead;
    while (fileNodeHead != NULL) {
        tempNode = fileNodeHead;
        fileNodeHead = fileNodeHead->next;
        free(tempNode->filename);
        freeList(tempNode->head);
        free(tempNode);
    }
}
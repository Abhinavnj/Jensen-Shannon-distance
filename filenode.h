#include "linkedlist.h"

typedef struct FileNode
{
    Node* head;
    struct FileNode* next;
    char* filename;
    int wordCount;
} FileNode;

int initFile (FileNode* fileNodeHead);
int insertFileNode(FileNode** fileNodeHead, Node** head, char* filename, int wordCount);
void printFileList(FileNode* fileNodeHead);
void freeFileList(FileNode* fileNodeHead);
int fileListLength(FileNode* fileNodeHead);
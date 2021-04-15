#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

typedef struct Node {
    char* word;
    struct Node* next;
    double frequency;
    int count;
} Node;

int initHead (Node* head);
int insertNode(Node** head, char* word);
double frequencyByWord(Node* head, char* word);
void printList(Node* head);
void freeList(Node* head);

#endif
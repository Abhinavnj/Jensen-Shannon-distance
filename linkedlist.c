#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linkedlist.h"

int initHead (Node* head) {
    head = NULL;

    return EXIT_SUCCESS;
}

int insertNode (Node** head, char* word) {
    Node* newNode = malloc(sizeof(Node));
    newNode->word = word;
    newNode->next = NULL;
    newNode->count = 1;

    if ((*head) == NULL) {
        *head = newNode;
        return 0;
    }
    else if (strcmp((*head)->word, word) > 0) {
        newNode->next = *head;
        *head = newNode;
        return 0;
    }
    else {
        Node* current = *head;
        Node* prev = *head;
        while (current != NULL){
            if (strcmp(current->word, word) == 0) {
                ++current->count;
                return 0;
            } 
            else if (strcmp(current->word, word) > 0) {
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

double frequencyByWord(Node* head, char* word) {
    Node* ptr = head;
    while (ptr != NULL) {
        if (strcmp(ptr->word, word) == 0) {
            return ptr->frequency;
        }
        ptr = ptr->next;
    }

    return 0;
}

void freeList (Node* head) {
    Node* tempNode = head;
    while (head != NULL) {
        tempNode = head;
        head = head->next;
        free(tempNode);
    }
}

void printList (Node* head) {
    Node* ptr = head;
    while (ptr != NULL) {
        printf("%s: %d\t", ptr->word, ptr->count);
        ptr = ptr->next;
    }
    printf("\n");
}
typedef struct Node {
    char* word;
    struct Node* next;
    double frequency;
    int count;
} Node;

int initHead (Node* head);
int insertNode(Node** head, char* word);
void printList(Node* head);
void freeList(Node* head);
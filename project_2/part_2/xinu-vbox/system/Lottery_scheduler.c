#include<xinu.h>
#include<stdlib.h>
struct data
{
    uint32 tickets;
    uint32 pid;
};

struct Node
{
    struct data Data;
    struct Node *Next;
};


void Create_list_of_tickets(uint32 pid, uint32 tickets, struct Node **head)
{
    struct Node* newnode = (struct Node*)getmem(sizeof(struct Node));
    newnode->Data.pid = pid;
    newnode->Data.tickets = tickets;
    newnode->Next = NULL;

    if(*head == NULL || (*head)->Data.tickets < tickets || ((*head)->Data.tickets == tickets && (*head)->Data.pid > pid))
    {
        newnode->Next = *head;
        *head = newnode;
    }
    else
    {
        struct Node* current_node = *head;
        // current_node->Next = NULL;
        while(current_node->Next != NULL && current_node->Next->Data.tickets > tickets || (current_node->Next->Data.tickets == tickets && current_node->Next->Data.pid < pid))
        {
            current_node = current_node->Next;
        }
        newnode->Next = current_node->Next;
        current_node->Next = newnode;
    }
    
}

void printList(struct Node* head) {
    printf("Printing the linked list:\n");

    struct Node* current = head;
    while (current != NULL) {
        printf("PID: %d, Tickets: %d -> ", current->Data.pid, current->Data.tickets);
        current = current->Next; // Use Next instead of next
    }
    printf("NULL\n");
}
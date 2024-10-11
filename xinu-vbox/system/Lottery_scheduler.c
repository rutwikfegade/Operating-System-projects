#include<xinu.h>
#include<stdlib.h>
#define DEBUG_CTXSW
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

struct Node *head=NULL;
pid32 new_pid;
uint32 Total_Tickets;
struct Node* current;
struct procent *ptnew;
struct procent *ptcopy;
bool8 flag;
pid32 Find_winner()
{
    // print_queue(user_list);
    Total_Tickets = 0;
    uint32 counter = 0;
    uint32 winner;
    pid32 curr;
    // kprintf("Total tickets %d\n",Total_Tickets);
    qid16 curr_1 = firstid(user_list);
    while (curr_1 != queuetail(user_list)) {
        Total_Tickets += proctab[curr_1].tickets;
        // kprintf("PID: %d, Key: %d\n", curr_1, queuetab[curr_1].qkey);
        curr_1 = queuetab[curr_1].qnext; 
    }
    // kprintf("Total tickets %d\n",Total_Tickets);
    if(Total_Tickets <= 0 )
    {
        return SYSERR;
    }
    winner = rand()%Total_Tickets;
    
    curr = firstid(user_list);
    while(TRUE)
    {
        counter += proctab[curr].tickets;
        if(counter > winner)
        {
            kprintf("winner is %d\n",curr);
            return curr;
        }
        curr = queuetab[curr].qnext;
    }
    
    // counter += proctab[curr].tickets;
    // kprintf("winner value %d and counter is %d\n",winner,counter);
    // while(!(counter > winner))
    // {
    //     // kprintf("hey\n");
    //     curr = queuetab[curr].qnext;
    //     counter += proctab[curr].tickets;
    // }
    // return curr;

}

void Lottery_scheduler(struct procent *ptold, pid32 calling_pid)
{
    
    if(ptold->prstate == PR_CURR)
    {
        ptold->prstate = PR_READY;
        if(ptold->user_process == FALSE)
        {
            insert(currpid,readylist,ptold->prprio);
        }
        else
        {
            insert_user_process(currpid,user_list,ptold->tickets);
        }

    }
    print_queue(user_list);
    new_pid = Find_winner();
    ptcopy = &proctab[new_pid];
    if(new_pid == currpid)
    {
        return;
    }
    dequeue_user_list(new_pid,user_list);
    currpid = new_pid;
    
    preempt = QUANTUM;
    ptcopy->num_ctxsw++;
    #ifdef DEBUG_CTXSW
        if(currpid != calling_pid)
        {
            kprintf("3ctxsw::%d-%d\n",calling_pid,currpid);
        }

    #endif
    ctxsw(&ptold->prstkptr, &ptcopy->prstkptr);
    
    
    return;
}


// void Create_list_of_tickets(uint32 pid, uint32 tickets)
// {
//     struct Node* current_check = head;
//     struct Node* prev = NULL;

//     while(current_check != NULL)
//     {
//         if(proctab[current_check->Data.pid].prstate == PR_FREE)
//         {
//             if(current_check == head)
//             {
//                 head = current_check->Next;
//                 Total_Tickets -= current_check->Data.tickets;
//                 freemem((char*)current_check,sizeof(struct Node));
//                 current_check = head;
//             }
//             else
//             {
//                 prev->Next = current_check->Next;
//                 Total_Tickets -= current_check->Data.tickets;
//                 freemem((char*)current_check,sizeof(struct Node));
//                 current_check = prev->Next;
//             }
//         }
//         else
//         {
//             prev = current_check;
//             current_check = current_check->Next;
//         }
//     }



//     struct Node* newnode = (struct Node*)getmem(sizeof(struct Node));
//     newnode->Data.pid = pid;
//     newnode->Data.tickets = tickets;
//     newnode->Next = NULL;

//     if(head == NULL || (head)->Data.tickets < tickets || ((head)->Data.tickets == tickets && (head)->Data.pid > pid))
//     {
//         newnode->Next = head;
//         head = newnode;
//     }
//     else
//     {
//         struct Node* current_node = head;
//         // current_node->Next = NULL;
//         while(current_node->Next != NULL && current_node->Next->Data.tickets > tickets || (current_node->Next->Data.tickets == tickets && current_node->Next->Data.pid < pid))
//         {
//             current_node = current_node->Next;
//         }
//         newnode->Next = current_node->Next;
//         current_node->Next = newnode;
//     }
    
// }

void printList(struct Node* head) {
    printf("Printing the linked list:\n");

    struct Node* current = head;
    while (current != NULL) {
        printf("PID: %d, Tickets: %d, State: %s -> ", current->Data.pid, current->Data.tickets,proctab[current->Data.pid].prstate);
        current = current->Next;
    }
    printf("NULL\n");
}
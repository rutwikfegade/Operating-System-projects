#include<xinu.h>
#include<stdlib.h>
// #define DEBUG_CTXSW
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
extern int32 Total_Tickets;
pid32 pid_is;
struct Node* current;
struct procent *ptnew;
struct procent *ptcopy;

pid32 Find_winner()
{
    uint32 counter = 0;
    uint32 winner;
    if(Total_Tickets > 0)
    {
        winner = rand()%Total_Tickets;
        // kprintf("winner is: %d and Total_tickets is:%d\n",winner,Total_Tickets);
        current = head;
        
        while(current)
        {
            counter += current->Data.tickets;
            // kprintf("counter: %d and current tickets: %d\n",counter,current->Data.tickets);
            if(counter > winner)
            {
                // kprintf("winner is: %d\n",winner);
                // proctab[current->Data.pid].tickets -= 1;
                // Total_Tickets -= 1;
                // kprintf("Winner is: %d with ticket number; %d\n",current->Data.pid,winner);
                return current->Data.pid;
            }
            current = current->Next;
        }
    }
    return SYSERR;
}

void Lottery_scheduler(struct procent *ptold)
{
    pid32 new_pid = Find_winner();
    if(new_pid == currpid || (proctab[new_pid].prstate != PR_READY && proctab[new_pid].prstate != PR_CURR) )
    {
        return;
    }
    ptcopy = &proctab[new_pid];
    if (ptold->prstate == PR_CURR) 
    { 
        // kprintf("Process inserted in userlist: %d\n",new_pid);
		ptold->prstate = PR_READY;
        // kprintf("process %d being set to %d with %d tickets in if\n",currpid,ptold->prstate,ptold->tickets);
        if(strcmp(ptold->prname, "prnull") == 0)
        {
            insert(currpid,readylist,ptold->prprio);
        }
        else
        {
            insert(currpid,user_list,ptold->tickets);
        }
		
        // print_queue(user_list);
	}
    // kprintf("winner is %d \n",new_pid);
    // print_queue(user_list);
    dequeue_user_list(new_pid,user_list);
    // ptnew = &proctab[currpid];
    // print_queue(user_list);
    
    currpid = new_pid;
    ptcopy->prstate = PR_CURR;
    preempt = QUANTUM;

    // kprintf("Context switch from PID: %d (esp: 0x%08X) to PID: %d (esp: 0x%08X)\n",
    //         currpid, ptold->prstkptr, new_pid, ptcopy->prstkptr);

    // Perform context switch
    ptold->num_ctxsw++;
    #ifdef DEBUG_CTXSW
        if(currpid != (ptold-proctab))
        {
            kprintf("3ctxsw::%d-%d\n",(ptold-proctab),currpid);
        }

    #endif
    ctxsw(&ptold->prstkptr, &ptcopy->prstkptr);
    
    
    return;
}




void Create_list_of_tickets(uint32 pid, uint32 tickets)
{
    struct Node* current_check = head;
    struct Node* prev = NULL;

    while(current_check != NULL)
    {
        if(proctab[current_check->Data.pid].prstate == PR_FREE)
        {
            if(current_check == head)
            {
                head = current_check->Next;
                Total_Tickets -= current_check->Data.tickets;
                freemem((char*)current_check,sizeof(struct Node));
                current_check = head;
            }
            else
            {
                prev->Next = current_check->Next;
                Total_Tickets -= current_check->Data.tickets;
                freemem((char*)current_check,sizeof(struct Node));
                current_check = prev->Next;
            }
        }
        else
        {
            prev = current_check;
            current_check = current_check->Next;
        }
    }



    struct Node* newnode = (struct Node*)getmem(sizeof(struct Node));
    newnode->Data.pid = pid;
    newnode->Data.tickets = tickets;
    newnode->Next = NULL;

    if(head == NULL || (head)->Data.tickets < tickets || ((head)->Data.tickets == tickets && (head)->Data.pid > pid))
    {
        newnode->Next = head;
        head = newnode;
    }
    else
    {
        struct Node* current_node = head;
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
        printf("PID: %d, Tickets: %d, State: %s -> ", current->Data.pid, current->Data.tickets,proctab[current->Data.pid].prstate);
        current = current->Next;
    }
    printf("NULL\n");
}
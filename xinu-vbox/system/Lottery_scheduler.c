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

struct Node *head=NULL;
extern int32 Total_Tickets;
pid32 pid_is;
struct Node* current;
struct procent *ptnew;
struct procent *ptcopy;

pid32 Find_winner()
{
    srand(1);
    uint32 counter = 0;
    if(Total_Tickets > 0)
    {
        uint32 winner = rand()%Total_Tickets;
        current = head;
        
        while(current)
        {
            counter += current->Data.tickets;
            if(counter > winner)
            {
                proctab[current->Data.pid].tickets -= 1;
                Total_Tickets -= 1;
                kprintf("Winner is: %d with ticket number; %d\n",current->Data.pid,winner);
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
    ptcopy = &proctab[new_pid];
    if(new_pid == currpid)
    {
        return;
    }
    if (ptold->prstate == PR_CURR) 
    { 
        // kprintf("Process inserted in userlist: %d\n",new_pid);
		ptold->prstate = PR_READY;
        kprintf("process %d being set to %d with %d tickets in if\n",currpid,ptold->prstate,ptold->tickets);
		insert(currpid,user_list,ptold->tickets);
        print_queue(user_list);
	}
    else
    {
        kprintf("process %d being set to %d with %d tickets in if\n",new_pid,ptcopy->prstate,ptcopy->tickets);
        insert(new_pid,user_list,ptcopy->tickets);
        print_queue(user_list);
    }
    // print_ready_list();
    while (nonempty(user_list))
    {
        pid32 next_pid = firstid(user_list);
        kprintf("Next PID to check: %d\n", next_pid);

        if(next_pid == new_pid)
        {
            currpid = dequeue(user_list);
            break;
        }
        else
        {
            dequeue(user_list);
            enqueue(next_pid,user_list);
        }
    }
    kprintf("process Dequeued is: %d\n",currpid);
    ptnew = &proctab[currpid];
    
    // kprintf("Switching to PID: %d (Process Name: %s)\n", pid_is, ptnew->prname);
    ptnew->prstate = PR_CURR;
    preempt = QUANTUM;

    // Log stack pointer info before context switch
    // kprintf("Context switch from PID: %d (esp: 0x%08X) to PID: %d (esp: 0x%08X)\n",
    //         currpid, ptold->prstkptr, pid_is, ptnew->prstkptr);

    // Perform context switch
    ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
    
    
    return;
}




void Create_list_of_tickets(uint32 pid, uint32 tickets)
{
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
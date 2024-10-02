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
void Lottery_scheduler(struct procent *ptold)
{
    struct procent *ptnew = NULL;
    struct Node* current;
    if(strcmp(ptold->prname, "prnull") == 0 || ptold->user_process == TRUE)                    // Filter the prnull and user ( this if is unncessary though )
    {
        // kprintf("1st if and Total_Tickets: %d\n",Total_Tickets);
        if(Total_Tickets > 0)                                                      // Will only take user process and not the null pprocess until all the tickets are utilized
        {
            // kprintf("2nd if\n");
            // kprintf("user_process: %d, prname != prnull: %d\n", ptold->user_process, strcmp(ptold->prname, "prnull") != 0);
            if(ptold->user_process == TRUE && strcmp(ptold->prname, "prnull") != 0)                // Filter only user process
            {
                // kprintf("3rd if\n");
                // do
                // {
                    // print_ready_list();
                    // printList(head);
                    while(Total_Tickets > 0)
                    {
                        srand(1);
                        uint32 winner = rand()%(Total_Tickets);
                        kprintf("winner is: %d\n",winner);
                        current = head;
                        uint32 counter = 0;
                        while(current)
                        {
                        
                            counter = counter + current->Data.tickets;
                            kprintf("counter is: %d, pid is: %d, tickets of pid is: %d, Total_tickets are: %d\n",counter,current->Data.pid,current->Data.tickets,Total_Tickets);
                            if(counter > winner)
                            {
                                if (proctab[current->Data.pid].prstate == PR_CURR) {  /* Process remains eligible */
                            

                                /* Old process will no longer remain current */
                                // kprintf("here\n");
                                proctab[current->Data.pid].prstate = PR_READY;
                                // kprintf("1\n");
                                insert(currpid, readylist, current->Data.tickets);
                                // kprintf("2\n");
                                }
                                // kprintf("4th if\n");
                                if(proctab[current->Data.pid].prstate == PR_READY)              // schedule process that are ready
                                {
                                    // kprintf("process that is ready to schedule pid: %d\n",current->Data.pid);
                                    // ptnew = &proctab[current->Data.pid];
                                   
                                    current->Data.tickets -= 1;
                                    Total_Tickets -= 1;
                                    break;
                                    
                                }
                                // kprintf("Exiting 4th if\n");
                            }
                            current = current->Next;
                        } 
                        // kprintf("Exiting while\n");
                        currpid = dequeue(readylist);
                        ptnew = &proctab[currpid];
                        ptnew->prstate = PR_CURR;
                        preempt = QUANTUM;
                        ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
                       
                    }
                    // else
                    // {
                    //     kprintf("Else\n");
                    //     break;
                    // }
                    
                // } while (ptnew != NULL);               // Get random numbers until winner is ready
               
                
                kprintf("1\n");

            }
            else                            // if the user process are still present and we get a null process then just skip it dont schedule it
            {
                kprintf("2\n");
                return;
            }
        }
        else                            // if user process are done so we can schedule null process
        {
            kprintf("3\n");
           return;
        }
        
    }

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
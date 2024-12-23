
#define NLOCKS 20
#define NALOCKS 20
#define NOPROC  -1
#define NOLOCK  -1
#define NPILOCKS 20

sid32  DeadLock_array[NPROC];
uint32 counter_array[NPROC];
pri16  priority_holder;

typedef struct  
{
    uint32 flag;
}sl_lock_t;


typedef struct
{
    uint32 flag;
    uint32 guard;
    qid16  lock_queue;
    uint32 set_park_flag;
}lock_t;


typedef struct
{
    uint32 flag;
    uint32 guard;
    qid16  lock_queue;
    uint32 set_park_flag;
    uint32  lock_id;
    pid32   lock_owner;
}al_lock_t;


typedef struct
{
    uint32 flag;
    uint32 guard;
    qid16  lock_queue;
    uint32 set_park_flag;
    uint32  lock_id;
    pid32   lock_owner;
    pri16   acquired_priority;
}pi_lock_t;



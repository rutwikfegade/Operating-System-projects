#include <xinu.h>

syscall sl_initlock(sl_lock_t *mutex)
{
    mutex->flag = 0;
}

syscall sl_lock(sl_lock_t *mutex)
{
    while(test_and_set(&mutex->flag,1));
}

syscall sl_unlock(sl_lock_t *mutex)
{
    mutex->flag = 0;
}
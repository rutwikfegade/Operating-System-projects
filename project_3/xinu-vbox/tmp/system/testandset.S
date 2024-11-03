

        .text
        .global test_and_set

test_and_set:
        pushl	%ebp            /* Save old base pointer */
        movl	%esp,%ebp       /* Set up new base pointer */
        movl	8(%ebp),%eax    /* Move first parameter (*ptr) in eax register */
        movl	12(%ebp),%ebx   /* Move second parameter (new_value) in ebx register */
        XCHG    %ebx,(%eax)     /* Exchange the values of the both the regsiter i.e *ptr = new_value */
        movl    %ebx, %eax      /* Move the old value to eax so that it can be returned */
        popl    %ebp            /* Restore the base pointer */
        ret
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * ldelete  --  delete a semaphore by releasing its table entry
 * ------------------------------------------------------------------------
 */
SYSCALL ldelete(int lk)
{
	STATWORD ps;    
	int	pid,i;
	struct	lentry	*lptr =&lockarray[lk];

	disable(ps);
	if (lptr->lstate==LFREE || isbadlock(lk)) 
	{
		restore(ps);
		return(SYSERR);
	}
	
	for(i=0;i<NPROC;i++)
	{
		if(lptr->lacquired[i]==1) /*processes stop using the lock before it is deleted  */
		{
			proctab[i].plocksheld[lk] = DELETED;
			lptr->lacquired[i] = 0;
			 
		}
	}

	lptr->lstate = LFREE;
	lptr->ldel=1;/*placeholder to store if lock has been deleted once or not. 1 means the lock has been deleted.*/
	if (nonempty(lptr->lqhead)) {
		pid=getfirst(lptr->lqhead);
		while( pid != EMPTY)
		  {
		    proctab[pid].pwaitret = DELETED;
		    ready(pid,RESCHNO); // no need to call resched each time a proc unblocked; can call when all have been unblocked
		  }
		resched();
	}
	restore(ps);
	return(OK);
}

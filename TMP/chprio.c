/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include<lock.h>
/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;
	int i;
	struct lentry *lptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;
	if(pptr->pinh != 0)
		pptr->pinh = newprio;
	

	if(pptr->pstate == PRWAIT)
	{
		lptr = &lockarray[pptr->plockid];
		if(pptr->pstate==PRWAIT)
		{
                lptr->lprio = maxprocprio_waitlk(lptr);
		
        i=0;
        while(i<NPROC)        
		{
				if(lptr->lacquired[i])
					priority_inheritance(&proctab[i]);
			i++;
		}
		}
	}
	restore(ps);
	return(newprio);
}

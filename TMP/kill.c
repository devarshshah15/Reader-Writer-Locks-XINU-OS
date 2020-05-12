/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include<lock.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev,i;
	struct 	lentry	*lptr;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;
					lptr = &lockarray[pptr->plockid];
		
			/* increment the lock count if it is a writer or last reader */
			if((lptr->ltype == READ) && (lptr->lreadcnt == 0))
			{
                                lptr->ltemp++;
				
			}
			if(lptr->ltype == WRITE)
			{
				 lptr->ltemp++;
			}
			if(changeprio(pptr) == lptr->lprio){
				dequeue(pid);
				lptr->lprio = maxprocprio_waitlk(lptr);
				i=0;
		
				while(i<NPROC){
					if(lptr->lacquired[i])
					{
						
						priority_inheritance(&proctab[i]);
				}
				i++;
				}
			}

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}

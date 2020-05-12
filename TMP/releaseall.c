#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*-----------------------------------------------------------
 * releaseall  --  release lock(s) for the calling  process
 *-----------------------------------------------------------
 */
SYSCALL releaseall (int numlocks, int ldes1, ...)
{
	
	int i, lk,flag=0;
	unsigned long *lockptr=(unsigned long *)(&ldes1);
	for(i=0;i<numlocks;i++)
	{
		lk=*lockptr++;
		if (SYSERR == release(lk)) 
		{
			flag = 1;
		}
	}
	if(flag==1)
	{
		return SYSERR;
	}
	else
	{
		return OK;
	}
}

int release(int lk)
{
	STATWORD ps;    
	struct lentry *lptr=&lockarray[lk];
	struct pentry *pptr=&proctab[currpid];
	disable(ps);
	if (isbadlock(lk) || (lptr= &lockarray[lk])->lstate==LFREE || !(lptr->lacquired[currpid]) ) 
	{
		restore(ps);
		return SYSERR;
	}
	if(nonempty(lptr->lqhead))
	{	
		int best_r = highest_lockpriority(lk,READ);
		int best_w = highest_lockpriority(lk,WRITE);
		if(lptr->ltype == READ)
		{
			lptr->lreadcnt--;
		}
		lptr->lacquired[currpid] = 0;
		proctab[currpid].plocksheld[lk] = 0;
		
	if(lptr->ltype == WRITE || (lptr->ltype == READ && lptr->lreadcnt == 0))
	{
		lptr->ltemp++;
		if(q[best_r].qkey < q[best_w].qkey)
		{
            lptr->ltype = WRITE;
            lptr->lacquired[best_w] = 1;  
			proctab[best_w].plocksheld[lk] = 1;
            
			ready(dequeue(best_w), RESCHNO);         
			lptr->lprio = maxprocprio_waitlk(lptr);
			proctab[best_w].plockid = -1;           
		}

		else if(q[best_r].qkey > q[best_w].qkey)
		{
		while(q[best_r].qkey > q[best_w].qkey)
		{
			if(best_r > -1)
			{
				lptr->ltype = READ;
				lptr->lacquired[best_r] = 1;
				proctab[best_r].plocksheld[lk] = 1;
				lptr->lreadcnt++;
				
				ready(dequeue(best_r), RESCHNO);
				lptr->lprio = maxprocprio_waitlk(lptr);
				proctab[best_r].plockid = -1; 		
				best_r = highest_lockpriority(lk,READ);
			}
		}			
		}
		

		/*below cases are when the priorities for both the readers and writers are equal*/
		/* (ctr1000 - wait start time of reader) - (ctr1000 - wait start time of writer) <= 400 milliseconds */
		/*reader process will be chosen as it can have grace period of 0.4s*/
		else if((q[best_w].qlwstime >= q[best_r].qlwstime) && (q[best_w].qlwstime - q[best_r].qlwstime <= 400 )) 
		{
			lptr->ltype = READ;
			lptr->lacquired[best_r] = 1;
			proctab[best_r].plocksheld[lk] = 1;
			lptr->lreadcnt++;
            
			ready(dequeue(best_r), RESCHNO);
			lptr->lprio = maxprocprio_waitlk(lptr);
			proctab[best_r].plockid = -1;            
		}	
		/*write process will be choosen*/
		else
		{											
			lptr->ltype = WRITE;
			lptr->lacquired[best_w] = 1;
			proctab[best_w].plocksheld[lk] = 1;
            
			ready(dequeue(best_w), RESCHNO);
            lptr->lprio = maxprocprio_waitlk(lptr);
			proctab[best_w].plockid = -1;     /* best_w is no longer waiting in a lock queue */       

		}
					
			resched();
		}

		}

	
	priority_inheritance(pptr);
	restore(ps);
	return OK;
}



void priority_inheritance(struct pentry * pptr)
{
	
	
	struct lentry *highestlptr = NULL,*lptr;
	int i=0,numlocks = 0;
	// find the highest priority lock among all the locks
	while(i<NLOCKS)
	{           
		lptr = &lockarray[i];            
		if(pptr->plocksheld[i])
		{
			numlocks++;
			if(numlocks == 1 || (highestlptr != NULL && changeprio(pptr) > highestlptr->lprio ) )
			{
			
				highestlptr = lptr;
			}

		}
		i++;	
	}
	/*condition to check when all the locks will be released*/

	if(numlocks==0)
	{		
		pptr->pinh = 0;	
	}
	else if(numlocks!=0)
	{
		pptr->pinh = highestlptr->lprio;
		
	}
	else if(numlocks!=0)
	{
		if(pptr->pinh < pptr->pprio)
			pptr->pinh = 0;
	}
	if(pptr->plockid != -1)
	{
		lptr = &lockarray[pptr->plockid];
		lptr->lprio = maxprocprio_waitlk(lptr);
		i=0;
		while(i<NPROC)
		{
			if(lptr->lacquired[i]==1)
			{
				priority_inheritance(&proctab[i]);
			}
			i++;
		}
	}

}

/*Process with the maximum priority holding the lock in the lock queue can be found.*/


int maxprocprio_waitlk(struct lentry *lptr)
{
	
	if(lptr == NULL)
		return -1;
	struct pentry *pptr=&proctab[currpid];
	int max_priority = 0;
	int prev = q[lptr->lqtail].qprev;
	if(nonempty(lptr->lqhead))
	{
		while(prev != lptr->lqhead)
		{
			pptr = &proctab[prev];
			if(changeprio(pptr) > max_priority)
				max_priority = changeprio(pptr);
			prev = q[prev].qprev;
		}
	}
	return max_priority;

}

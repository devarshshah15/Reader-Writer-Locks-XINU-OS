#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

extern unsigned long ctr1000;
/*------------------------------------------------------------------------
 * lock  -- current process waits on a lock
 * ------------------------------------------------------------------------
 */

SYSCALL	lock(int lk, int type, int priority)
{
	STATWORD ps;    
	

	disable(ps);
	struct	lentry	*lptr=&lockarray[lk];
	struct	pentry	*pptr=&proctab[currpid];
	/*If the lock has been held or acquired by the process before, deleting it will raise an error*/
	if (isbadlock(lk) || lptr->lstate==LFREE || ((proctab[currpid].plocksused[lk] == 1) && lptr->ldel == 1) ) 
	{
		restore(ps);
		return(SYSERR);
	}
	
	/* both waiting and acquiring are considered as using the lock */
	proctab[currpid].plocksused[lk] = 1;
	if (lptr->ltemp <= 0) 
	{
		if(type == READ)//reader process
		{
			reader_request(lk,priority);
			restore(ps);
			return proctab[currpid].pwaitret;
		}
		
		if(type == WRITE) //writer process
		{
			writer_request(lk, priority);
			restore(ps);
			return proctab[currpid].pwaitret;
		}
	}
	
	/* first reader or writer acquires the lock as no process is in the lock queue*/
	lptr->ltype=type;
	lptr->ltemp--;
	if(type == READ)
		lptr->lreadcnt = 1;	/* first reader */
	lptr->lacquired[currpid] = 1;
	proctab[currpid].plocksheld[lk] = 1;
	restore(ps);
	return(OK);
}


	
int higherpriority_writer_waiting(int lk, int priority)
{
	struct lentry* lptr=&lockarray[lk];

	if(isempty(lptr->lqhead))
		return 0;

	
	/* traversing from tail as the queue is sorted on lock priority */
	int prev = q[lptr->lqtail].qprev;
	while(prev != lptr->lqhead)
	{
		if(q[prev].qltype == WRITE && q[prev].qkey >= priority)
		 	return 1;
		prev = q[prev].qprev;
	}
	return 0;
	
}

int reader_request(int lk, int priority)
{
	struct lentry *lptr = &lockarray[lk];
	struct pentry *pptr=&proctab[currpid];
	int i;
	int flag=0;
	/* to check if writer has lock or not*/

	if(lockarray[lk].ltype==WRITE) 
	{
		i=0;
		while(i<NPROC)
		{
			if(lockarray[lk].lacquired[i]==1)
			{
				flag=1;
			}
			i++;
		}
	}



	/* if a writer doesn't have the lock and no writer with priority at least as large as the reader's is waiting - reader gets the lock */
	if(!flag && !(higherpriority_writer_waiting(lk, priority)))
	{
		(lptr->lacquired)[currpid] = 1;
		(lptr->lreadcnt)++;
		proctab[currpid].plocksheld[lk] = 1;
		
	}
	else{	
		proctab[currpid].pstate = PRWAIT; 
		proctab[currpid].plockid = lk;
		insert_lockque(currpid, lptr->lqhead, priority, READ);
		if(changeprio(pptr) > lptr->lprio)
	                lptr->lprio = changeprio(pptr);
		
	    i=0;
	    while(i<NPROC)
	    {
	    	if(lptr->lacquired[i]==1)
	    	{
	    		priority_inheritance(&proctab[i]);
	    	}
	    	i++;
	    }
	    

		proctab[currpid].pwaitret = OK;
		resched();
	
	}

	return 1;
}


int writer_request(int lk, int priority)
{
	struct lentry* lptr = &lockarray[lk];
	struct pentry *pptr=&proctab[currpid];
	int i;
	proctab[currpid].pstate = PRWAIT; //process waits
        proctab[currpid].plockid = lk;
        insert_lockque(currpid, lptr->lqhead, priority, WRITE);
	if(changeprio(pptr) > lptr->lprio)
		lptr->lprio = changeprio(pptr);
        
	/* waiting triggers priority elevations */
	i=0;
	while(i<NPROC)
	{
        if(lptr->lacquired[i]==1)
		{
                	priority_inheritance(&proctab[i]);
		}
		i++;                      
        }
        proctab[currpid].pwaitret = OK;
        resched();	
	return 1;
}



int insert_lockque(int proc, int head, int wprio, int ltype)
{
	int	next,prev;			/* runs through list		*/

	next = q[head].qnext;
	while (q[next].qkey < wprio)	/* tail has maxint as key	*/
		next = q[next].qnext;
	q[proc].qnext = next;
	q[proc].qprev = prev = q[next].qprev;
	q[proc].qkey  = wprio;
	q[proc].qltype = ltype;
	q[proc].qlwstime = ctr1000;
	q[prev].qnext = proc;
	q[next].qprev = proc;
	
	return(OK);
}

int changeprio(struct pentry *pptr)
{
	if(pptr->pinh!=0)
	{
		return pptr->pinh;
	}
	else
	{
		return pptr->pprio;
	}
}


int highest_lockpriority(int lk, int ltype)
{
	struct lentry *lptr = &lockarray[lk];
	
	if(isempty(lptr->lqhead))
		return -1;
	
	int prev = q[lptr->lqtail].qprev;
	while(prev != lptr->lqhead)
	{
		if(q[prev].qltype == ltype)
			return prev;
		prev = q[prev].qprev;
	}
	
return -1;	// no process with the currpid exists
}



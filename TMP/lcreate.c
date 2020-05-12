#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int newlock();

/*------------------------------------------------------------------------
 * lcreate  --  create and initialize a lock, returning its id
 *------------------------------------------------------------------------
 */

SYSCALL lcreate(void)
{
	STATWORD ps;
	int lk,i; /*lock*/
	disable(ps);
	if ( (lk=newlock())==SYSERR)
	{
		restore(ps);
		return(SYSERR);
	}
	lockarray[lk].lreadcnt = 0;
	lockarray[lk].ltemp=1;
	lockarray[lk].lprio = 0;


	i=0;
	while(i<NPROC)
	{
		lockarray[lk].lacquired[i]=0; /* currently no process will be holding the lock*/
		i++;
	}

	/* lqhead and lqtail were initialized at system startup */
	restore(ps);
	return(lk);
}

LOCAL int newlock()
{
	int	lk;
	int	i;
	for (i=0 ; i<NLOCKS ; i++) {
		lk=nextlock--;
		if (nextlock < 0)
			nextlock = NLOCKS-1;
		if (lockarray[lk].lstate==LFREE) {
			if(lockarray[lk].ldel != 1)
				lockarray[lk].ldel = 0;
			lockarray[lk].lstate = LUSED;
			return(lk);
		}
	}
	return(SYSERR);
}

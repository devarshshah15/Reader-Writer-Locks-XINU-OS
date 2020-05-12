#ifndef _LOCK_H_ //ifndef and endif to compile the code
#define _LOCK_H

#ifndef NLOCKS
#define NLOCKS     50 /* number of locks if not defined*/
#endif

#define READ 0
#define WRITE 1
#define LFREE '\01' /* This lock is free */
#define LUSED '\02' /* This lock is used */

struct lentry
{
	int lstate;/*whether the lock is free or used*/
	int lqhead; /*head of the waiting queue*/
	int lqtail; /*tail of the waiting queue*/
	int lprio; /*maximum priority among the processes waiting for a particular lock*/
	int lacquired[NPROC]; /*processes currently holding this lock*/
	int ltype; /*whether the lock is acquired by reader or writer*/
	int lreadcnt; /*number of active readers associated with the lock*/
	int ldel; /*placeholder to store if lock is deleted once or not*/	
	int ltemp; /*count of this lock initialized to 1*/
};

extern struct lentry lockarray[];
extern int nextlock;

void linit (void);

#define	isbadlock(l)	(l<0 || l>=NLOCKS)
#endif

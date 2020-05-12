#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include<q.h>
#include<stdio.h>


void linit()
{
	int i;
	
	for(i=0;i<NLOCKS;i++)
	{
		struct lentry *lptr=&lockarray[i];
		lptr->lstate=LFREE;
		lptr->lqhead=newqueue();
		lptr->lqtail=1+lptr->lqhead; /*tailID=1+HeadID*/

	}
}

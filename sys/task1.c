#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include<lock.h>



#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }


void lprocess1(int lck)
{
      struct pentry *pptr=&proctab[currpid];
    kprintf("%s(priority = %d) is requesting to enter critical section\n", pptr->pname, getprio(currpid));    
    lock(lck, WRITE, DEFAULT_LOCK_PRIO);
    kprintf("%s(priority = %d) has entered critical section\n", pptr->pname, getprio(currpid));
    sleep(1);
        kprintf("----------------------Lowest-------------------------------");
    
        kprintf("\n%s has completed critical section(ramped up priority = %d)\n", pptr->pname, getprio(currpid));
        releaseall(1, lck);
        kprintf("%s Original priority = %d\n", pptr->pname, getprio(currpid));
    
}

void lprocess2(int lck)
{
    struct pentry *pptr=&proctab[currpid];
    
    kprintf("%s(priority = %d) has started\n", pptr->pname, getprio(currpid));
    sleep(1);
	kprintf("----------------------Medium-------------------------------");
    kprintf("\n%s has completed its execution\n");
}

void lprocess3(int lck, int pr1)
{
    struct pentry *pptr=&proctab[currpid];
    kprintf("%s(priority = %d) is requesting to enter critical section\n", pptr->pname, getprio(currpid));
    kprintf("Ramping up the priority of %s\n", pptr->pname, getprio(pr1));
    lock(lck, WRITE, DEFAULT_LOCK_PRIO);
    kprintf("%s(priority = %d) has entered critical section\n", pptr->pname, getprio(currpid));
    kprintf("----------------------Highest-------------------------------");
    kprintf("\n%s has completed critical section\n", pptr->pname);
    releaseall(1, lck);
}

void testlock()
{
    int lck,pr1,pr2,pr3;
    lck = lcreate();

    pr1 = create(lprocess1, 2000, 10, "p1", 1, lck);
    pr2 = create(lprocess2, 2000, 20, "p2", 1, lck);
    pr3 = create(lprocess3, 2000, 30, "p3", 2, lck, pr1);
    
    resume(pr1);
 
    resume(pr2);
    sleep(1);
    resume(pr3);
    sleep(5);

    ldelete(lck);
}

/* to test semaphore */
void process1(int sem)
{
    struct pentry *pptr=&proctab[currpid];

        kprintf("%s(priority = %d) is requesting to enter critical section\n", pptr->pname, getprio(currpid));
  
	wait(sem);

        kprintf("%s(priority = %d) has entered critical section\n", pptr->pname, getprio(currpid));
  	sleep(1);              
        kprintf("----------------------Lowest-------------------------------");
        kprintf("\n%s has completed critical section(priority = %d)\n", pptr->pname, getprio(currpid));
        signal(sem);
        
}

void process2(int sem)
{
     struct pentry *pptr=&proctab[currpid];
        kprintf("%s(priority = %d) has started\n", pptr->pname, getprio(currpid));
        
          kprintf("----------------------Medium-------------------------------");
        kprintf("\n%s has completed its execution\n");
}

void process3(int sem){
    struct pentry *pptr=&proctab[currpid];
    kprintf("%s(priority = %d) is requesting to enter critical section\n", pptr->pname, getprio(currpid));
    wait(sem);
    kprintf("%s(priority = %d) has entered critical section\n", pptr->pname, getprio(currpid));
    kprintf("----------------------Highest-------------------------------");
            kprintf("\n%s has completed critical section\n", pptr->pname);
            signal(sem);
    }


void testsem()
{
        int sem,pr1,pr2,pr3;
        sem = screate(1);

        pr1 = create(process1, 2000, 10, "p1", 1, sem);
        pr2 = create(process2, 2000, 20, "p2", 1, sem);
        pr3 = create(process3, 2000, 30, "p3", 1, sem);

        resume(pr1);
        sleep(1);
        resume(pr2);
        sleep(1);
        resume(pr3);
        sleep(5);

}

int main( )
{
        kprintf("\n--------------- Testing with Semaphores ----------------- \n");
        testsem();
        kprintf("\n--------------- Testing with Locks ---------------------- \n");
        testlock();
        sleep(5);
        
        shutdown();
}


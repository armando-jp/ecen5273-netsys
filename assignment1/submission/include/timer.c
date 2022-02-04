#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include "timer.h"

pid_t gettid(void);
static void handler(int sig, siginfo_t *si, void *uc);
/*******************************************************************************
* Variable declerations
*******************************************************************************/
// timer variables
int res = 0;
timer_t timerid;

struct itimerspec its_start = 
{   /* specify start delay and interval */
    .it_value.tv_sec     = 0,
    .it_value.tv_nsec    = 0,
    .it_interval.tv_sec  = 5,
    .it_interval.tv_nsec = 0
};

struct itimerspec its_stop = 
{   /* specify start delay and interval */
    .it_value.tv_sec     = 0,
    .it_value.tv_nsec    = 0,
    .it_interval.tv_sec  = 0,
    .it_interval.tv_nsec = 0
};

// handler data struct
struct t_eventData eventData = { .myData = false };

// signal handler variables
/* specify signal and handler */
/* specifies the action when receiving a signal */
struct sigaction sa;

/* Initialize signal */
struct sigevent sev; 




/*******************************************************************************
* Timer Handler Method
*******************************************************************************/
static void handler(int sig, siginfo_t *si, void *uc)
{
    UNUSED(sig);
    UNUSED(uc);
    struct t_eventData *data = (struct t_eventData *) si->_sifields._rt.si_sigval.sival_ptr;
    // printf("Timer fired %d - thread-id: %d\n", ++data->myData, gettid());
    data->myData = true;
}

/*******************************************************************************
* Timer Methods
*******************************************************************************/
/* create timer */
void timer_init_create()
{
    res = timer_create(CLOCK_REALTIME, &sev, &timerid);
    if ( res != 0)
    {
        fprintf(stderr, "Error timer_create: %s\n", strerror(errno));
    }
}

void timer_init_signal()
{
    sigemptyset(&sa.sa_mask);
}

void timer_register_sig_handle()
{
    if (sigaction(SIGRTMIN, &sa, NULL) == -1){
        fprintf(stderr, "Error sigaction: %s\n", strerror(errno));
    }    
}


void timer_start()
{
    res = timer_settime(timerid, 0, &its_start, NULL);

    if ( res != 0)
    {
        fprintf(stderr, "Error timer_settime: %s\n", strerror(errno));
    }
}

void timer_stop()
{
    res = timer_settime(timerid, 0, &its_stop, NULL);

    if ( res != 0)
    {
        fprintf(stderr, "Error timer_settime: %s\n", strerror(errno));
    }
}

// one big function to initialize and configure our timer
void timer_init()
{
    timer_init_sigevent();
    timer_init_create();
    timer_init_sigaction();
    timer_init_signal();
    timer_register_sig_handle();
}

/*******************************************************************************
* Variable Setup Functions
*******************************************************************************/
void timer_init_sigaction()
{
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
}

void timer_init_sigevent()
{
    sev.sigev_notify = SIGEV_SIGNAL; // Linux-specific
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = &eventData;
}

/*******************************************************************************
* Timer GET/SET methods
*******************************************************************************/
bool timer_get_flag()
{
    return eventData.myData;
}

void timer_clear_flag()
{
    eventData.myData = false;
}
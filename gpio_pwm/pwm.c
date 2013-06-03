/**
 * @brief Userspace application for PWM output 
 		  Copyright (C) 2013 Andrei Danaila, All Rights Reserved
		  				<mailto: adanaila >at< ctrlinux[.]com>

	This file is part of pwm application.

    pwm application is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 2.

    pwm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with pwm application.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>
#include <pthread.h>

#define DEBUG_ON 0
#define DEBUG if(DEBUG_ON)printf

//#define PERIOD 1000000000.0 //Period is 1s = 1*10^9ns
#define PERIOD 1000000.0 	  //Period is 1ms = 1*10^6ns
#define BUF_SIZE 80

static FILE *gpio = NULL;
static int fd_gpio = 0;
static int gpio_val = 0; 
static struct itimerspec old_time  = {0}, new_time  = {0};
static unsigned long time_on = PERIOD/2, time_off = PERIOD/2;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static timer_t on_timer  = {0};


void timer_on_handler (int signum)
{
    struct timespec tp;
    char buffer [BUF_SIZE];
    char gpio_out_val = 0;
    int ret_val = 0;

    ret_val = pthread_mutex_lock(&mtx);
    if (0 != ret_val)
        perror("pthread_mutex_lock");

    if (clock_gettime (CLOCK_MONOTONIC, &tp) == -1)
        perror ("clock_gettime");
    gpio_out_val = gpio_val + '0';

    sprintf (buffer, "TIMER_ON  %ld s %ld ns \n", tp.tv_sec, tp.tv_nsec );
    DEBUG("%s", buffer); 
    ret_val = write(fd_gpio,&gpio_out_val,1);
    if (ret_val == -1)
        perror("write");

    if (gpio_val)
    {
        DEBUG("ON Cycle setting off interval to %lu\n",time_off);
        new_time.it_interval.tv_nsec = time_on;
        new_time.it_value.tv_nsec = time_on;
    }
    else
    {
        DEBUG("OFF Cycle setting on interval to %lu\n",time_on);
        new_time.it_interval.tv_nsec = time_off;
        new_time.it_value.tv_nsec = time_off;
    }

    if (-1 == timer_settime(on_timer, 0, &new_time,&old_time))
        perror("timer_setttime");
    gpio_val ^= 1;

    ret_val = pthread_mutex_unlock(&mtx);
    if (0 != ret_val)
        perror("pthread_mutex_unlock");
}

int main ()
{
    char user_input[10]= {0};
    int  user_val = 0, ret_val = 0;
    struct timespec res;
    struct sigevent on_event = {0};
    struct sigaction on_action  = {0};

    fd_gpio = open("/sys/class/gpio/gpio38/value",O_RDWR | O_SYNC);
    if ( fd_gpio < 0)
       perror("open"); 

    /* Set signal to send on timer expiration */
    on_event.sigev_signo = SIGUSR1;
    on_event.sigev_notify = SIGEV_SIGNAL;

    /* Set timer callback */
    on_action.sa_handler = timer_on_handler;

    if(-1 == sigaction(SIGUSR1,&on_action,NULL))
        perror("sigaction");

    /* Create timer */
    if (-1 == timer_create(CLOCK_REALTIME ,&on_event, &on_timer))
        perror("timer_create");

    /* 50% duty cycle */
    new_time.it_interval.tv_nsec = time_on;
    new_time.it_value.tv_nsec = time_on;

    if (-1 == timer_settime(on_timer, 0, &new_time,&old_time))
        perror("timer_setttime");

    printf("Finished setting timer, starting PWM at 50 percent. Enter new val 1-100 for",
			"new modulation value\n");

	/* Wait for user to input a new PWM modulation value */
    while (1)
    {
        scanf("%d",&user_val);
        if (0 < user_val && user_val <= 100)
        {
			ret_val = pthread_mutex_lock(&mtx);
			gpio_val = 1;
			if (0 != ret_val)
				perror("pthread_mutex_lock");
            DEBUG("User val is %d\n",user_val);
            time_on  = (unsigned int)(PERIOD *     (user_val/100.0));
            time_off = (unsigned int)(PERIOD * (1 - user_val/100.0));
            DEBUG("Calculated time periods time_on: %lu, time_off %lu\n",time_on,time_off);
            if (-1 == timer_settime(on_timer, 0, &new_time,&old_time))
                perror("timer_setttime");
			ret_val = pthread_mutex_unlock(&mtx);
			if (0 != ret_val)
				perror("pthread_mutex_unlock");
        }
        user_val = 0;
    };
    return 0;
}

/*
 * Simple threadpool monitor usage example.
 * Created by christopherdec on 24/11/24.
 */

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "threadpool.h"

#define POOL_SIZE   8
#define BUFFER_SIZE   100


/*
 * Simple function to test if a number is prime or not
 */   
static int is_prime(int x) {
    if (x <= 1 ||
        (x != 2  &&  x % 2 == 0)    ||
        (x != 3  &&  x % 3 == 0)    ||
        (x != 5  &&  x % 5 == 0)) {
            return 0;
    }

    int d = 3;
    while(d <= x/2) {
        if(x % d == 0) {
            return 0;
        }
        d = d + 2;
    }
    return 1;
}


/*
 * Tests if x is prime or not, then prints the result
 */
static void print_prime(int x) {
    if (is_prime(x))
        printf("print_prime: %d is prime\n", x);
    else
        printf("print_prime: %d is not prime\n", x);
}


/*
 *  x is a twin prime number if (x - 2) or (x + 2) is also a prime number
 */
static void print_twin_prime(int x) {
    if (is_prime(x) == 0 )
        printf("print_twin_prime: %d is not prime\n", x);
    else if (is_prime(x-2) == 1 )
        printf("print_twin_prime: %d is prime twin of %d\n", x, x-2);
    else if (is_prime(x+2) == 1 )
        printf("print_twin_prime: %d is prime twin of %d\n", x, x+2);
    else
        printf("print_twin_prime: %d is not prime twin\n", x);
}


static void submit_jobs(int n_jobs) {
    for (int i=0; i < n_jobs; i++) {
        int x = rand() % 1000000;
        int ret;

        printf("main: Submitting job with x=%d\n", x);

        if((rand() % 2) == 0)
            ret = threadpool_submit(print_prime, x);
        else
            ret = threadpool_submit(print_twin_prime, x);

        if (ret == -1)
            printf("main: Failed to submit job for %d\n", x);
    }
}

/*
 * Initializes the pool;
 * Submits ten times the buffer size in jobs;
 * Sleeps for one sec then shutdowns the pool;
 * Reinitializes the pool;
 * Submits more jobs;
 * Waits for all jobs to finish;
 * Shutdowns the pool a final time.
*/
int main(void) {
    printf("main: Start\n");
    srand(time(NULL));

    if (threadpool_init(POOL_SIZE, BUFFER_SIZE) == -1) {
        printf("main: Failed to initialize threadpool\n");
        exit(EXIT_FAILURE);
    }

    submit_jobs(10 * BUFFER_SIZE);

    sleep(1);

    printf("main: Attempting to shutdown threadpool. Current queue size: %d\n", threadpool_queue_size());

    threadpool_shutdown();

    printf("main: Threadpool shutdown complete... Sleeping for 5 seconds\n");

    sleep(5);

    printf("main: Attempting to reinitialize threadpool\n");

    if( threadpool_init( POOL_SIZE, BUFFER_SIZE) == -1 ) {
        printf("main: Failed to reinitialize threadpool\n");
        exit(EXIT_FAILURE);
    }

    printf("main: Threadpool reinitialized successfully. Attempting to submit jobs\n");

    submit_jobs(50);

    printf("main: Waiting for all jobs to finish\n");

    while(threadpool_queue_size() > 0)
        sleep(1);

    printf("main: All jobs have finished. Attempting to shutdown threadpool. Current queue size: %d\n", threadpool_queue_size());

    threadpool_shutdown();

    printf("main: Finish\n");
    return(0);
}
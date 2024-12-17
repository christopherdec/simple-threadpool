/*
 * Simple threadpool monitor implementation.
 * Created by christopherdec on 24/11/24.
 */


#ifndef THREADPOOL_H
#define THREADPOOL_H

extern int threadpool_init(int pool_size, int buffer_size);

extern int threadpool_submit(void(* func)(int), int arg);

extern int threadpool_queue_size(void);

extern int threadpool_shutdown(void);

#endif
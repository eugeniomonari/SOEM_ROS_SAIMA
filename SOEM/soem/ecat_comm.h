#pragma once
#include "ethercat.h"
#include <iostream>

namespace ecat_comm 
{
    template <typename T>
    class ecatComm
    {
    public:
        static int dorun;
        static int64 toff,gl_delta;
        static volatile int wkc;
        static uint8 *digout;
        static boolean inOP;
        static int expectedWKC;
        static uint8 currentgroup;
        static boolean needlf;
        char IOmap[4096];
        pthread_t thread1, thread2;
        
        ~ecatComm();
        void ecatinit(char *ifname, int ctime);
        void ecatsetup(char *ifname);
        static OSAL_THREAD_FUNC_RT ecatthread(void *ptr);
        static OSAL_THREAD_FUNC ecatcheck(void *ptr);
        static void add_timespec(struct timespec *ts, int64 addtime);
        static void ec_sync(int64 reftime, int64 cycletime , int64 *offsettime);
    };
}

#include "ecat_comm.tpp"

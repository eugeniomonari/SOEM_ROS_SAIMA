#pragma once
#include "ethercat.h"
#include <iostream>
#include <vector>

namespace ecat_comm 
{
    template <typename input_structs, typename output_structs>
    class ecatComm
    {
    public:
        int dorun;
        int64 toff,gl_delta;
        volatile int wkc;
        uint8 *digout;
        boolean inOP;
        int expectedWKC;
        uint8 currentgroup;
        boolean needlf;
        char IOmap[4096];
        pthread_t thread1, thread2;
        std::vector<int> m_slaves;
        int slaves_number;
        input_structs inputs;
        output_structs outputs;
        int m_ctime;
        boolean error = FALSE;
        
        ~ecatComm();
        void ecatinit(int slaves[], char *ifname, int ctime);
        void ecatsetup(char *ifname);
        OSAL_THREAD_FUNC_RT ecatthread();
        OSAL_THREAD_FUNC ecatcheck();
        void add_timespec(struct timespec *ts, int64 addtime);
        void ec_sync(int64 reftime, int64 cycletime , int64 *offsettime);
        template<size_t I = 0>
        void read_write_PDO();
    };
}

#include "ecat_comm.tpp"

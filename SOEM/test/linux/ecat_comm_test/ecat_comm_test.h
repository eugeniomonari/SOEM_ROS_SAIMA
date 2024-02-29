#pragma once
#include "ecat_comm.h"






// #include "ethercat.h"
// 
// namespace ecat_comm 
// {
//     template <typename T>
//     class ecatComm
//     {
//     public:
//         static int dorun;
//         static int64 toff,gl_delta;
//         static volatile int wkc;
//         static uint8 *digout;
//         static boolean inOP;
//         static int expectedWKC;
//         static uint8 currentgroup;
//         static boolean needlf;
//         char IOmap[4096];
//         pthread_t thread1, thread2;
//         
//         ecatComm();
//         ecatComm(char *ifname, int ctime);
//         ~ecatComm();
//         void ecatsetup(char *ifname);
//         static OSAL_THREAD_FUNC_RT ecatthread(void *ptr);
//         static OSAL_THREAD_FUNC ecatcheck(void *ptr);
//         static void add_timespec(struct timespec *ts, int64 addtime);
//         static void ec_sync(int64 reftime, int64 cycletime , int64 *offsettime);
//     };
// }






class ecatCommATIAxiaFTSensor : public ecat_comm::ecatComm<ecatCommATIAxiaFTSensor>
{
    public:
        static int set_bias;
        static int filter;
        
        ecatCommATIAxiaFTSensor(char *ifname, int ctime);
        static void ecat_read_write_PDO();
};

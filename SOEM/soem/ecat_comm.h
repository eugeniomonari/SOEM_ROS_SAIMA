#pragma once
#include "ethercat.h"

namespace ecat_comm 
{
    extern bool operational_state_reached;
    
    OSAL_THREAD_FUNC_RT ecatthread(void *ptr);
    
    class ecatComm
    {
    public:
        char IOmap[4096];
        int expectedWKC;
        boolean inOP;
        bool all_good = true;
        volatile int wkc;
        uint8 currentgroup = 0;
        boolean needlf;
        
        ecatComm(char *ifname);
        void ecat_setup(char *ifname);
        void ecat_update();
        virtual void ecat_read_write_PDO();
        void ecat_check();
    };
}

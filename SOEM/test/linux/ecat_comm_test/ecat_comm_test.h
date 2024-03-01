#pragma once
#include "ecat_comm.h"

class ecatCommATIAxiaFTSensor : public ecat_comm::ecatComm<ecatCommATIAxiaFTSensor>
{
    public:
        static int set_bias;
        static int filter;
        
//         ecatCommATIAxiaFTSensor(char *ifname, int ctime);
        static void ecat_read_write_PDO();
};

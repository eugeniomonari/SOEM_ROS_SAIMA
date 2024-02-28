#pragma once
#include "ecat_comm.h"
#include "ecat_comm_devices.h"

class ecatCommATIAxiaFTSensor : public ecat_comm::ecatComm
{
public:
    int set_bias=0;
    int filter=0;
    
    ecatCommATIAxiaFTSensor(char *ifname);
    void ecat_read_write_PDO() override;
};

#include <iostream>
#include "ecat_comm_test.h"
#include "ecat_comm.h"

ecatCommATIAxiaFTSensor::ecatCommATIAxiaFTSensor(char* ifname)
    : ecat_comm::ecatComm(ifname)
{
}


void ecatCommATIAxiaFTSensor::ecat_read_write_PDO()
{
    std::cout << (((double)ecat_comm::read_ATIAxiaFTSensor(5).Fz)/1000000)/9.80665 << std::endl;
    ecat_comm::write_ATIAxiaFTSensor(5,set_bias,0,filter,0,1);
}

int main(int argc, char** argv){
	ecatCommATIAxiaFTSensor ecat_comm(argv[1]);
    ecat_comm.set_bias = 0;
    ecat_comm.filter = 0;
    while(1)
    {
        ecat_comm.ecat_update();
        ecat_comm.set_bias = 0;
        osal_usleep(10000);
    }
	return 0;
}

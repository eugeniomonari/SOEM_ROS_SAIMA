#include <iostream>
#include "ecat_comm_test.h"
#include "ecat_comm_devices.h"

int ecatCommATIAxiaFTSensor::set_bias = 0;
int ecatCommATIAxiaFTSensor::filter = 0;

void ecatCommATIAxiaFTSensor::ecat_read_write_PDO()
{
    std::cout << (((double)ecat_comm::read_ATIAxiaFTSensor(2).Fz)/1000000)/9.80665 << std::endl;
    ecat_comm::write_ATIAxiaFTSensor(2,set_bias,0,filter,0,1);
}

int main(int argc, char** argv){
	ecatCommATIAxiaFTSensor ecat_comm;
    ecat_comm.set_bias = 1;
    ecat_comm.filter = 0;
    ecat_comm.ecatinit(argv[1],atoi(argv[2]));
    while(1)
    {
        osal_usleep(10000);
        ecat_comm.set_bias = 0;
    }
	return 0;
}

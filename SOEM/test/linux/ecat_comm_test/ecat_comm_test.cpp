#include <iostream>
#include "ecat_comm.h"
#include "ecat_comm_devices.h"

int main(int argc, char** argv){
    ecat_comm::ecatComm<std::tuple<ecat_comm::in_ATIAxiaFTSensort>,std::tuple<ecat_comm::out_ATIAxiaFTSensort>> ecat_comm;
    int slaves[1] = {2};
    ecat_comm.ecatinit(slaves,argv[1],atoi(argv[2]));
    std::get<0>(ecat_comm.outputs).Control1 = 1;
    while(1)
    {
        std::cout << (((double)std::get<0>(ecat_comm.inputs).Fz)/1000000)/9.80665 << std::endl;
        osal_usleep(1000);
        std::get<0>(ecat_comm.outputs).Control1 = 0;
    }
	return 0;
}

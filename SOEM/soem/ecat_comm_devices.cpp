#include "ecat_comm_devices.h"
#include "ethercat.h"
#include <iostream>
#include <cmath>

namespace ecat_comm
{
    in_ATIAxiaFTSensort read_ATIAxiaFTSensor(int slave_number)
    {
        in_ATIAxiaFTSensort *in_ATIAxiaFTSensor;
        in_ATIAxiaFTSensor = (in_ATIAxiaFTSensort*) ec_slave[slave_number].inputs;
        return *in_ATIAxiaFTSensor;
    }
    
    void write_ATIAxiaFTSensor(int slave_number, int set_bias, int clear_bias, int filter, int calibration, int sample_rate)
    {
        out_ATIAxiaFTSensort *out_ATIAxiaFTSensor;
        out_ATIAxiaFTSensor = (out_ATIAxiaFTSensort*) ec_slave[slave_number].outputs;
        uint32_t control_code = 0;
        if (set_bias <= 1)
        {
            control_code = control_code + set_bias*std::pow(2,0);
        }
        else
        {
            std::cout << "out_ATIAxiaFTSensor: invalid control code for set_bias." << std::endl;
        }
        if (clear_bias <= 1)
        {
            control_code = control_code + clear_bias*std::pow(2,2);
        }
        else
        {
            std::cout << "out_ATIAxiaFTSensor: invalid control code for clear_bias." << std::endl;
        }
        if (filter <= 8)
        {
            control_code = control_code + filter*std::pow(2,4);
        }
        else
        {
            std::cout << "out_ATIAxiaFTSensor: invalid control code for filter." << std::endl;
        }
        if (calibration <= 1)
        {
            control_code = control_code + calibration*std::pow(2,8);
        }
        else
        {
            std::cout << "out_ATIAxiaFTSensor: invalid control code for calibration." << std::endl;
        }
        if (sample_rate <= 3)
        {
            control_code = control_code + sample_rate*std::pow(2,12);
        }
        else
        {
            std::cout << "out_ATIAxiaFTSensor: invalid control code for sample_rate." << std::endl;
        }
        out_ATIAxiaFTSensor->Control1 = control_code;
        out_ATIAxiaFTSensor->Control2 = 0;
    }
}


#pragma once
#include "ethercat.h"

// For info as to how to define the struct of new devices, see https://github.com/OpenEtherCATsociety/SOEM/issues/492 for memory address alignment based on data type of inputs and outputs

namespace ecat_comm 
{
    typedef struct PACKED
    {
        int32_t Fx;
        int32_t Fy;
        int32_t Fz;
        int32_t Tx;
        int32_t Ty;
        int32_t Tz;
        uint32_t Status;
        uint32_t SampleNumber;
    } in_ATIAxiaFTSensort;
    
    typedef struct PACKED
    {
        uint32_t Control1;
        uint32_t Control2;
    } out_ATIAxiaFTSensort;
}


#include <iostream>
#include "ecat_comm_test.h"
#include "ecat_comm_devices.h"







// #define stack64k (64 * 1024)
// #define EC_TIMEOUTMON 500
// #define NSEC_PER_SEC 1000000000
// 
// namespace ecat_comm
// {
//     template<typename T>
//     int ecatComm<T>::dorun = 0;
//     template<typename T>
//     uint8 *ecatComm<T>::digout = 0;
//     template<typename T>
//     uint8 ecatComm<T>::currentgroup = 0;
//     template<typename T>
//     int64 ecatComm<T>::toff;
//     template<typename T>
//     int64 ecatComm<T>::gl_delta;
//     template<typename T>
//     volatile int ecatComm<T>::wkc;
//     template<typename T>
//     boolean ecatComm<T>::inOP;
//     template<typename T>
//     int ecatComm<T>::expectedWKC;
//     template<typename T>
//     boolean ecatComm<T>::needlf;
//     template<typename T>
//     OSAL_THREAD_FUNC_RT ecatComm<T>::ecatthread(void *ptr)
//     {
//         struct timespec   ts, tleft;
//         int ht;
//         int64 cycletime;
// 
//         clock_gettime(CLOCK_MONOTONIC, &ts);
//         ht = (ts.tv_nsec / 1000000) + 1; /* round to nearest ms */
//         ts.tv_nsec = ht * 1000000;
//         cycletime = *(int*)ptr * 1000; /* cycletime in ns */
//         toff = 0;
//         dorun = 0;
//         ec_send_processdata();
//         while(1)
//         {
//             /* calculate next cycle start */
//             add_timespec(&ts, cycletime + toff);
//             /* wait to cycle start */
//             clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, &tleft);
//             if (dorun>0)
//             {
//                 wkc = ec_receive_processdata(EC_TIMEOUTRET);
// 
//                 dorun++;
//                 
//                 T::ecat_read_write_PDO();
//                 
//                 /* if we have some digital output, cycle */
//                 if( digout ) *digout = (uint8) ((dorun / 16) & 0xff);
// 
//                 if (ec_slave[0].hasdc)
//                 {
//                     /* calulate toff to get linux time and DC synced */
//                     ec_sync(ec_DCtime, cycletime, &toff);
//                 }
//                 ec_send_processdata();
//             }
//         }
//     }
//     
//     template<typename T>
//     void ecatComm<T>::add_timespec(struct timespec *ts, int64 addtime)
//     {
//         int64 sec, nsec;
//         nsec = addtime % NSEC_PER_SEC;
//         sec = (addtime - nsec) / NSEC_PER_SEC;
//         ts->tv_sec += sec;
//         ts->tv_nsec += nsec;
//         if ( ts->tv_nsec > NSEC_PER_SEC )
//         {
//             nsec = ts->tv_nsec % NSEC_PER_SEC;
//             ts->tv_sec += (ts->tv_nsec - nsec) / NSEC_PER_SEC;
//             ts->tv_nsec = nsec;
//         }
//     }
//     
//     template<typename T>
//     void ecatComm<T>::ec_sync(int64 reftime, int64 cycletime , int64 *offsettime)
//     {
//         static int64 integral = 0;
//         int64 delta;
//         /* set linux sync point 50us later than DC sync, just as example */
//         delta = (reftime - 50000) % cycletime;
//         if(delta> (cycletime / 2)) { delta= delta - cycletime; }
//         if(delta>0){ integral++; }
//         if(delta<0){ integral--; }
//         *offsettime = -(delta / 100) - (integral / 20);
//         gl_delta = delta;
//     }
//     
//     template<typename T>
//     OSAL_THREAD_FUNC ecatComm<T>::ecatcheck(void *ptr)
//         {
//             int slave;
// 
//         while(1)
//         {
//             if( inOP && ((wkc < expectedWKC) || ec_group[currentgroup].docheckstate))
//             {
//                 if (needlf)
//                 {
//                 needlf = FALSE;
//                 printf("\n");
//                 }
//                 /* one ore more slaves are not responding */
//                 ec_group[currentgroup].docheckstate = FALSE;
//                 ec_readstate();
//                 for (slave = 1; slave <= ec_slavecount; slave++)
//                 {
//                 if ((ec_slave[slave].group == currentgroup) && (ec_slave[slave].state != EC_STATE_OPERATIONAL))
//                 {
//                     ec_group[currentgroup].docheckstate = TRUE;
//                     if (ec_slave[slave].state == (EC_STATE_SAFE_OP + EC_STATE_ERROR))
//                     {
//                         printf("ERROR : slave %d is in SAFE_OP + ERROR, attempting ack.\n", slave);
//                         ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);
//                         ec_writestate(slave);
//                     }
//                     else if(ec_slave[slave].state == EC_STATE_SAFE_OP)
//                     {
//                         printf("WARNING : slave %d is in SAFE_OP, change to OPERATIONAL.\n", slave);
//                         ec_slave[slave].state = EC_STATE_OPERATIONAL;
//                         ec_writestate(slave);
//                     }
//                     else if(ec_slave[slave].state > EC_STATE_NONE)
//                     {
//                         if (ec_reconfig_slave(slave, EC_TIMEOUTMON))
//                         {
//                             ec_slave[slave].islost = FALSE;
//                             printf("MESSAGE : slave %d reconfigured\n",slave);
//                         }
//                     }
//                     else if(!ec_slave[slave].islost)
//                     {
//                         /* re-check state */
//                         ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
//                         if (ec_slave[slave].state == EC_STATE_NONE)
//                         {
//                             ec_slave[slave].islost = TRUE;
//                             printf("ERROR : slave %d lost\n",slave);
//                         }
//                     }
//                 }
//                 if (ec_slave[slave].islost)
//                 {
//                     if(ec_slave[slave].state == EC_STATE_NONE)
//                     {
//                         if (ec_recover_slave(slave, EC_TIMEOUTMON))
//                         {
//                             ec_slave[slave].islost = FALSE;
//                             printf("MESSAGE : slave %d recovered\n",slave);
//                         }
//                     }
//                     else
//                     {
//                         ec_slave[slave].islost = FALSE;
//                         printf("MESSAGE : slave %d found\n",slave);
//                     }
//                 }
//                 }
//                 if(!ec_group[currentgroup].docheckstate)
//                 printf("OK : all slaves resumed OPERATIONAL.\n");
//             }
//             osal_usleep(10000);
//         }
//     }
//     
//     template<typename T>
//     void ecatComm<T>::ecatsetup(char *ifname)
//     {
//         int cnt, i, j, oloop, iloop;
// 
//         printf("Starting Redundant test\n");
// 
//         /* initialise SOEM, bind socket to ifname */
//         //   if (ec_init_redundant(ifname, ifname2))
//         if (ec_init(ifname))
//         {
//             printf("ec_init on %s succeeded.\n",ifname);
//             /* find and auto-config slaves */
//             if ( ec_config(FALSE, &IOmap) > 0 )
//             {
//                 printf("%d slaves found and configured.\n",ec_slavecount);
//                 /* wait for all slaves to reach SAFE_OP state */
//                 ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE);
// 
//                 /* configure DC options for every DC capable slave found in the list */
//                 ec_configdc();
// 
//                 /* read indevidual slave state and store in ec_slave[] */
//                 ec_readstate();
//                 for(cnt = 1; cnt <= ec_slavecount ; cnt++)
//                 {
//                     /* check for EL2004 or EL2008 */
//                     if( !digout && ((ec_slave[cnt].eep_id == 0x0af83052) || (ec_slave[cnt].eep_id == 0x07d83052)))
//                     {
//                     digout = ec_slave[cnt].outputs;
//                     }
//                 }
//                 expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
//                 printf("Calculated workcounter %d\n", expectedWKC);
// 
//                 printf("Request operational state for all slaves\n");
//                 ec_slave[0].state = EC_STATE_OPERATIONAL;
//                 /* request OP state for all slaves */
//                 ec_writestate(0);
//                 /* activate cyclic process data */
//                 dorun = 1;
//                 /* wait for all slaves to reach OP state */
//                 ec_statecheck(0, EC_STATE_OPERATIONAL,  5 * EC_TIMEOUTSTATE);
//                 oloop = ec_slave[0].Obytes;
//                 if ((oloop == 0) && (ec_slave[0].Obits > 0)) oloop = 1;
//                 if (oloop > 8) oloop = 8;
//                 iloop = ec_slave[0].Ibytes;
//                 if ((iloop == 0) && (ec_slave[0].Ibits > 0)) iloop = 1;
//                 if (iloop > 8) iloop = 8;
//                 if (ec_slave[0].state == EC_STATE_OPERATIONAL )
//                 {
//                     printf("Operational state reached for all slaves.\n");
//                     inOP = TRUE;
//                 }
//                 else
//                 {
//                     printf("Not all slaves reached operational state.\n");
//                     ec_readstate();
//                     for(i = 1; i<=ec_slavecount ; i++)
//                     {
//                         if(ec_slave[i].state != EC_STATE_OPERATIONAL)
//                         {
//                             printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",
//                                 i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
//                         }
//                     }
//                 }
//             }
//             else
//             {
//                 printf("No slaves found!\n");
//             }
//         }
//         else
//         {
//             printf("No socket connection on %s\nExcecute as root\n",ifname);
//         }
//     }
//     
//     template<typename T>
//     ecatComm<T>::ecatComm()
//     {
//     }
//     
//     template<typename T>
//     ecatComm<T>::ecatComm(char* ifname, int ctime)
//     {
//         dorun = 0;
//         osal_thread_create_rt(&thread1, stack64k * 2, (void*) &ecatComm::ecatthread, (void*) &ctime);
//         osal_thread_create(&thread2, stack64k * 4, (void*) &ecatComm::ecatcheck, NULL);
//         ecatsetup(ifname);
//     }
//     
//     template<typename T>
//     ecatComm<T>::~ecatComm()
//     {
//         ec_close();
//     }
// 
// 
// }







int ecatCommATIAxiaFTSensor::set_bias = 0;
int ecatCommATIAxiaFTSensor::filter = 0;

ecatCommATIAxiaFTSensor::ecatCommATIAxiaFTSensor(char* ifname, int ctime)
    : ecat_comm::ecatComm<ecatCommATIAxiaFTSensor>(ifname, ctime)
{
}

void ecatCommATIAxiaFTSensor::ecat_read_write_PDO()
{
    std::cout << (((double)ecat_comm::read_ATIAxiaFTSensor(5).Fz)/1000000)/9.80665 << std::endl;
    ecat_comm::write_ATIAxiaFTSensor(5,set_bias,0,filter,0,1);
}

int main(int argc, char** argv){
	ecatCommATIAxiaFTSensor ecat_comm(argv[1],atoi(argv[2]));
    ecat_comm.set_bias = 1;
    ecat_comm.filter = 0;
    while(1)
    {
        osal_usleep(10000);
        ecat_comm.set_bias = 0;
    }
	return 0;
}

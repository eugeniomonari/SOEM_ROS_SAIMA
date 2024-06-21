// #include "ecat_comm.h"
#include "ethercat.h"
#include <iostream>
#include <tuple>

#define stack64k (64 * 1024)
#define EC_TIMEOUTMON 500
#define NSEC_PER_SEC 1000000000

namespace ecat_comm
{
    template<typename input_structs, typename output_structs>
    OSAL_THREAD_FUNC call_ecatthread(void* ptr)
    {
        auto foo = static_cast<ecatComm<input_structs,output_structs>*>(ptr);
        foo->ecatthread();
    }
    
    template<typename input_structs, typename output_structs>
    void call_ecatcheck(void* ptr)
    {
        auto foo = static_cast<ecatComm<input_structs,output_structs>*>(ptr);
        foo->ecatcheck();
    }
    
    template<typename input_structs, typename output_structs>
    OSAL_THREAD_FUNC_RT ecatComm<input_structs,output_structs>::ecatthread()
    {
        struct timespec   ts, tleft;
        int ht;
        int64 cycletime;

        clock_gettime(CLOCK_MONOTONIC, &ts);
        ht = (ts.tv_nsec / 1000000) + 1; /* round to nearest ms */
        ts.tv_nsec = ht * 1000000;
        cycletime = m_ctime * 1000; /* cycletime in ns */
        toff = 0;
        dorun = 0;
        
        ec_send_processdata();
        while(1)
        {
            add_timespec(&ts, cycletime + toff);
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, &tleft);
            if (dorun>0)
            {
                wkc = ec_receive_processdata(EC_TIMEOUTRET);

                dorun++;
                
                for (size_t i = 0; i < slaves_number; i++)
                {
                    read_write_PDO();
                }
                
                /* if we have some digital output, cycle */
                if( digout ) *digout = (uint8) ((dorun / 16) & 0xff);

                if (ec_slave[0].hasdc)
                {
                    /* calulate toff to get linux time and DC synced */
                    ec_sync(ec_DCtime, cycletime, &toff);
                }
                ec_send_processdata();
            }
        }
    }
    
    template<typename input_structs, typename output_structs>
    template<size_t I>
    void ecatComm<input_structs,output_structs>::read_write_PDO()
    {
        if constexpr (I < std::tuple_size<input_structs>::value)
        {
            typename std::tuple_element<I,input_structs>::type* ith_input;
            ith_input = (typename std::tuple_element<I,input_structs>::type*) ec_slave[m_slaves[I]].inputs;
            std::get<I>(inputs) = *ith_input;
            typename std::tuple_element<I,output_structs>::type* ith_output;
            ith_output = (typename std::tuple_element<I,output_structs>::type*) ec_slave[m_slaves[I]].outputs;
            *ith_output = std::get<I>(outputs);
            read_write_PDO<I + 1>();
        }
    }
    
    template<typename input_structs, typename output_structs>
    void ecatComm<input_structs,output_structs>::add_timespec(struct timespec *ts, int64 addtime)
    {
        int64 sec, nsec;
        nsec = addtime % NSEC_PER_SEC;
        sec = (addtime - nsec) / NSEC_PER_SEC;
        ts->tv_sec += sec;
        ts->tv_nsec += nsec;
        if ( ts->tv_nsec > NSEC_PER_SEC )
        {
            nsec = ts->tv_nsec % NSEC_PER_SEC;
            ts->tv_sec += (ts->tv_nsec - nsec) / NSEC_PER_SEC;
            ts->tv_nsec = nsec;
        }
    }
    
    template<typename input_structs, typename output_structs>
    void ecatComm<input_structs,output_structs>::ec_sync(int64 reftime, int64 cycletime , int64 *offsettime)
    {
        static int64 integral = 0;
        int64 delta;
        /* set linux sync point 50us later than DC sync, just as example */
        delta = (reftime - 50000) % cycletime;
        if(delta> (cycletime / 2)) { delta= delta - cycletime; }
        if(delta>0){ integral++; }
        if(delta<0){ integral--; }
        *offsettime = -(delta / 100) - (integral / 20);
        gl_delta = delta;
    }
    
    template<typename input_structs, typename output_structs>
    OSAL_THREAD_FUNC ecatComm<input_structs,output_structs>::ecatcheck()
        {
            int slave;

        while(1)
        {
            if( inOP && ((wkc < expectedWKC) || ec_group[currentgroup].docheckstate))
            {
                if (needlf)
                {
                needlf = FALSE;
                printf("\n");
                }
                error = TRUE;
                std::cout << "EtherCAT communication in error" << std::endl;
                /* one ore more slaves are not responding */
                ec_group[currentgroup].docheckstate = FALSE;
                ec_readstate();
                for (slave = 1; slave <= ec_slavecount; slave++)
                {
                if ((ec_slave[slave].group == currentgroup) && (ec_slave[slave].state != EC_STATE_OPERATIONAL))
                {
                    ec_group[currentgroup].docheckstate = TRUE;
                    if (ec_slave[slave].state == (EC_STATE_SAFE_OP + EC_STATE_ERROR))
                    {
                        printf("ERROR : slave %d is in SAFE_OP + ERROR, attempting ack.\n", slave);
                        ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);
                        ec_writestate(slave);
                    }
                    else if(ec_slave[slave].state == EC_STATE_SAFE_OP)
                    {
                        printf("WARNING : slave %d is in SAFE_OP, change to OPERATIONAL.\n", slave);
                        ec_slave[slave].state = EC_STATE_OPERATIONAL;
                        ec_writestate(slave);
                    }
                    else if(ec_slave[slave].state > EC_STATE_NONE)
                    {
                        if (ec_reconfig_slave(slave, EC_TIMEOUTMON))
                        {
                            ec_slave[slave].islost = FALSE;
                            printf("MESSAGE : slave %d reconfigured\n",slave);
                        }
                    }
                    else if(!ec_slave[slave].islost)
                    {
                        /* re-check state */
                        ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
                        if (ec_slave[slave].state == EC_STATE_NONE)
                        {
                            ec_slave[slave].islost = TRUE;
                            printf("ERROR : slave %d lost\n",slave);
                        }
                    }
                }
                if (ec_slave[slave].islost)
                {
                    if(ec_slave[slave].state == EC_STATE_NONE)
                    {
                        if (ec_recover_slave(slave, EC_TIMEOUTMON))
                        {
                            ec_slave[slave].islost = FALSE;
                            printf("MESSAGE : slave %d recovered\n",slave);
                        }
                    }
                    else
                    {
                        ec_slave[slave].islost = FALSE;
                        printf("MESSAGE : slave %d found\n",slave);
                    }
                }
                }
                if(!ec_group[currentgroup].docheckstate)
                printf("OK : all slaves resumed OPERATIONAL.\n");
            }
            osal_usleep(10000);
        }
    }
    
    template<typename input_structs, typename output_structs>
    void ecatComm<input_structs,output_structs>::ecatsetup(char *ifname)
    {
        int cnt, i, j, oloop, iloop;

        printf("Starting EtherCAT communication\n");

        /* initialise SOEM, bind socket to ifname */
        if (ec_init(ifname))
        {
            printf("ec_init on %s succeeded.\n",ifname);
            /* find and auto-config slaves */
            if ( ec_config(FALSE, &IOmap) > 0 )
            {
                printf("%d slaves found and configured.\n",ec_slavecount);
                /* wait for all slaves to reach SAFE_OP state */
                ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE);

                /* configure DC options for every DC capable slave found in the list */
                ec_configdc();

                /* read indevidual slave state and store in ec_slave[] */
                ec_readstate();
                for(cnt = 1; cnt <= ec_slavecount ; cnt++)
                {
                    /* check for EL2004 or EL2008 */
                    if( !digout && ((ec_slave[cnt].eep_id == 0x0af83052) || (ec_slave[cnt].eep_id == 0x07d83052)))
                    {
                    digout = ec_slave[cnt].outputs;
                    }
                }
                expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
                printf("Calculated workcounter %d\n", expectedWKC);

                printf("Request operational state for all slaves\n");
                ec_slave[0].state = EC_STATE_OPERATIONAL;
                /* request OP state for all slaves */
                ec_writestate(0);
                /* activate cyclic process data */
                dorun = 1;
                /* wait for all slaves to reach OP state */
                ec_statecheck(0, EC_STATE_OPERATIONAL,  5 * EC_TIMEOUTSTATE);
                oloop = ec_slave[0].Obytes;
                if ((oloop == 0) && (ec_slave[0].Obits > 0)) oloop = 1;
                if (oloop > 8) oloop = 8;
                iloop = ec_slave[0].Ibytes;
                if ((iloop == 0) && (ec_slave[0].Ibits > 0)) iloop = 1;
                if (iloop > 8) iloop = 8;
                if (ec_slave[0].state == EC_STATE_OPERATIONAL )
                {
                    printf("Operational state reached for all slaves.\n");
                    inOP = TRUE;
                }
                else
                {
                    error = TRUE;
                    std::cout << "EtherCAT communication in error" << std::endl;
                    printf("Not all slaves reached operational state.\n");
                    ec_readstate();
                    for(i = 1; i<=ec_slavecount ; i++)
                    {
                        if(ec_slave[i].state != EC_STATE_OPERATIONAL)
                        {
                            printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",
                                i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
                        }
                    }
                }
            }
            else
            {
                printf("No slaves found!\n");
            }
        }
        else
        {
            printf("No socket connection on %s\nExcecute as root\n",ifname);
        }
    }

    template<typename input_structs, typename output_structs>
    void ecatComm<input_structs,output_structs>::ecatinit(int slaves[], char *ifname, int ctime)
    {
        dorun = 0;
        slaves_number = sizeof(slaves)/sizeof(slaves[0]);
        for (int i = 0; i < slaves_number; i++)
        {
            m_slaves.push_back(slaves[i]);
        }
        m_ctime = ctime;
        osal_thread_create_rt(&thread1, stack64k * 2, (void*) &call_ecatthread<input_structs,output_structs>, this);
        osal_thread_create(&thread2, stack64k * 4, (void*) &call_ecatcheck<input_structs,output_structs>, this);
        ecatsetup(ifname);
    }
    
    template<typename input_structs, typename output_structs>
    ecatComm<input_structs,output_structs>::~ecatComm()
    {
        ec_close();
    }


}


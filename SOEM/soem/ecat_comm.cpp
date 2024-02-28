#include "ecat_comm.h"
#include <iostream>

#define stack64k (64 * 1024)
#define EC_TIMEOUTMON 500

namespace ecat_comm

{
    bool operational_state_reached = false;
    
    void ecatthread(void* ptr)
    {
        ec_send_processdata();
        int64 foo = *(int*)ptr * 1000;
        while(foo > 0 && !operational_state_reached)
        {
            ec_receive_processdata(EC_TIMEOUTRET);
            ec_send_processdata();
            osal_usleep(20000);
        }
    }
    
    ecatComm::ecatComm(char* ifname)
    {
        pthread_t waiting_thread;
        int ctime = 500;
        osal_thread_create_rt(&waiting_thread, stack64k * 2, (void*) &ecatthread, (void*) &ctime);
        ecat_setup(ifname);
    }
    
    void ecatComm::ecat_setup(char* ifname)
    {
        int cnt, i, oloop, iloop;

        printf("Starting Redundant test\n");

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
                    printf("Slave:%d Name:%s Output size:%3dbits Input size:%3dbits State:%2d delay:%d.%d\n",
                        cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits,
                        ec_slave[cnt].state, (int)ec_slave[cnt].pdelay, ec_slave[cnt].hasdc);
                    printf("         Out:%p,%4d In:%p,%4d\n",
                        ec_slave[cnt].outputs, ec_slave[cnt].Obytes, ec_slave[cnt].inputs, ec_slave[cnt].Ibytes);
//                     /* check for EL2004 or EL2008 */
//                     if( !digout && ((ec_slave[cnt].eep_id == 0x0af83052) || (ec_slave[cnt].eep_id == 0x07d83052)))
//                     {
//                         digout = ec_slave[cnt].outputs;
//                     }
                }
                expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
                printf("Calculated workcounter %d\n", expectedWKC);

                printf("Request operational state for all slaves\n");
                ec_slave[0].state = EC_STATE_OPERATIONAL;
                /* request OP state for all slaves */
                ec_writestate(0);
//                 /* activate cyclic process data */
//                 dorun = 1;
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
                    operational_state_reached = true;
//                     /* acyclic loop 5000 x 20ms = 10s */
//                     for(i = 1; i <= 5000; i++)
//                     {
//                     printf("Processdata cycle %5d , Wck %3d, DCtime %12"PRId64", dt %12"PRId64", O:",
//                         dorun, wkc , ec_DCtime, gl_delta);
//                     for(j = 0 ; j < oloop; j++)
//                     {
//                         printf(" %2.2x", *(ec_slave[0].outputs + j));
//                     }
//                     printf(" I:");
//                     for(j = 0 ; j < iloop; j++)
//                     {
//                         printf(" %2.2x", *(ec_slave[0].inputs + j));
//                     }
//                     printf("\r");
//                     fflush(stdout);
//                     osal_usleep(20000);
//                     }
//                     dorun = 0;
//                     inOP = FALSE;
                }
                else
                {
                    printf("Not all slaves reached operational state.\n");
                    all_good = false;
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
//                 printf("Request safe operational state for all slaves\n");
//                 ec_slave[0].state = EC_STATE_SAFE_OP;
//                 /* request SAFE_OP state for all slaves */
//                 ec_writestate(0);
            }
            else
            {
                printf("No slaves found!\n");
                all_good = false;
            }
//             printf("End redundant test, close socket\n");
//             /* stop SOEM, close socket */
//             ec_close();
        }
        else
        {
            printf("No socket connection on %s\nExcecute as root\n",ifname);
            all_good = false;
        }
    }
    
    void ecatComm::ecat_update()
    {
        wkc = ec_receive_processdata(EC_TIMEOUTRET);
        ecat_read_write_PDO();
        ec_send_processdata();
        ecat_check();
    }
    
    void ecatComm::ecat_check()
    {
        int slave;
        if( inOP && ((wkc < expectedWKC) || ec_group[currentgroup].docheckstate))
        {
            all_good = false;
            if (needlf)
            {
            needlf = FALSE;
            printf("\n");
            }
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
            {
                printf("OK : all slaves resumed OPERATIONAL.\n");
            }
        }
    }
    
    void ecatComm::ecat_read_write_PDO()
    {
        
    }
}


/*
 * File:
 *    tiLibTest.c
 *
 * Description:
 *    Test Vme TI interrupts with GEFANUC Linux Driver
 *    and TI library
 *
 *
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "jvme.h"
#include "tiLib.h"
#include "hcalLib.h"
/* #include "remexLib.h" */

DMA_MEM_ID vmeIN,vmeOUT;
extern DMANODE *the_event;
extern unsigned int *dma_dabufp;

extern int tiA32Base;

#define BLOCKLEVEL 30

#define DO_READOUT


int 
main(int argc, char *argv[]) {

  int stat;
  int clk_val = 6;
  if(argc>1) {
    clk_val = atoi(argv[1]);
  }


  printf("\nHCAL Lib Test\n");
  printf("----------------------------\n");

/*   remexSetCmsgServer("dafarm28"); */
/*   remexInit(NULL,1); */

  printf("Size of DMANODE    = %d (0x%x)\n",sizeof(DMANODE),sizeof(DMANODE));
  printf("Size of DMA_MEM_ID = %d (0x%x)\n",sizeof(DMA_MEM_ID),sizeof(DMA_MEM_ID));

  vmeOpenDefaultWindows();

  /* Setup Address and data modes for DMA transfers
   *   
   *  vmeDmaConfig(addrType, dataType, sstMode);
   *
   *  addrType = 0 (A16)    1 (A24)    2 (A32)
   *  dataType = 0 (D16)    1 (D32)    2 (BLK32) 3 (MBLK) 4 (2eVME) 5 (2eSST)
   *  sstMode  = 0 (SST160) 1 (SST267) 2 (SST320)
   */
  vmeDmaConfig(2,5,1);

  /* INIT dmaPList */

  dmaPFreeAll();
  vmeIN  = dmaPCreate("vmeIN",10240,50,0);
  vmeOUT = dmaPCreate("vmeOUT",0,0,0);
    
  dmaPStatsAll();

  dmaPReInitAll();

  /*     gefVmeSetDebugFlags(vmeHdl,0x0); */
  /* Set the TI structure pointer */
  /*     tiInit((2<<19),TI_READOUT_EXT_POLL,0); */
  tiA32Base=0x09000000;
  tiSetFiberLatencyOffset_preInit(0x20);
  tiInit(0x00a80000,TI_READOUT_EXT_POLL,TI_INIT_SKIP_FIRMWARE_CHECK);
  tiCheckAddresses();

  printf("Clocking in HCAL: %d\n",clk_val);
  hcalClkIn(clk_val);
  printf("Clocked in HCAL: %d.\n",clk_val);

/*
  tiDefinePulserEventType(0xAA,0xCD);

  tiSetSyncEventInterval(10);

  tiSetEventFormat(3);

  char mySN[20];
  printf("0x%08x\n",tiGetSerialNumber((char **)&mySN));
  printf("mySN = %s\n",mySN);

#ifndef DO_READOUT
  tiDisableDataReadout();
  tiDisableA32();
#endif

  tiLoadTriggerTable(0);
    
  tiSetTriggerHoldoff(1,4,0);
  tiSetTriggerHoldoff(2,4,0);

  tiSetPrescale(0);
  tiSetBlockLevel(BLOCKLEVEL);

  stat = tiIntConnect(TI_INT_VEC, mytiISR, 0);
  if (stat != OK) 
    {
      printf("ERROR: tiIntConnect failed \n");
      goto CLOSE;
    } 
  else 
    {
      printf("INFO: Attached TI Interrupt\n");
    }

  */
  /*     tiSetTriggerSource(TI_TRIGGER_TSINPUTS); */
  /*
  tiSetTriggerSource(TI_TRIGGER_PULSER);
  tiEnableTSInput(TI_TS_INPUT_2);
*/

  /*     tiSetFPInput(0x0); */
  /*     tiSetGenInput(0xffff); */
  /*     tiSetGTPInput(0x0); */

/*
  tiSetBusySource(TI_BUSY_LOOPBACK,1);

  tiSetBlockBufferLevel(1);
*/
/*   tiSetFiberDelay(1,2); */
/*   tiSetSyncDelayWidth(1,0x3f,1); */
    
/*
  tiSetBlockLimit(100);

  printf("Hit enter to reset stuff\n");
  getchar();

  tiClockReset();
  taskDelay(1);
  tiTrigLinkReset();
  taskDelay(1);
  tiEnableVXSSignals();

  int again=0;
 AGAIN:
  taskDelay(1);
  tiSyncReset(1);

  taskDelay(1);
    
  tiStatus(1);

  printf("Hit enter to start triggers\n");
  getchar();

  tiIntEnable(0);
  tiStatus(1);
*/
/* #define SOFTTRIG */
#ifdef SOFTTRIG
  tiSetRandomTrigger(1,0x7);
/*   taskDelay(10); */
/*   tiSoftTrig(1,0x1000,0x700,0); */
#endif

/*
  printf("Hit any key to Disable TID and exit.\n");
  getchar();
  tiStatus(1);
*/
#ifdef SOFTTRIG
  /* No more soft triggers */
  /*     tidSoftTrig(0x0,0x8888,0); */
  tiSoftTrig(1,0,0x700,0);
  tiDisableRandomTrigger();
#endif

  tiIntDisable();

  tiIntDisconnect();

  /*
  if(again==1)
    {
      again=0;
      goto AGAIN;
    }
*/


/*
 CLOSE:
*/
  dmaPFreeAll();
  vmeCloseDefaultWindows();

  exit(0);
}


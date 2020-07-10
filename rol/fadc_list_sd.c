/*************************************************************************
 *
 *  fadc_list.c - Library of routines for readout and buffering of 
 *                events using a JLAB Trigger Interface and
 *                Distribution Module (TID) AND one or more FADC250 
 *                with a Linux VME controller.
 *
 */

/* Event Buffer definitions */
#define MAX_EVENT_POOL     100
#define MAX_EVENT_LENGTH   (75000 << 2)      /* Size in Bytes */

/* Define Interrupt source and address */
#define TI_MASTER   /* Master accepts triggers and distributes them, if needed */
#define TI_READOUT TI_READOUT_EXT_POLL  /* Poll for available data, external triggers */
#define TI_ADDR    (21<<19)              /* GEO slot 21 */

/* Decision on whether or not to readout the TI for each block 
   - Comment out to disable readout 
*/
#define TI_DATA_READOUT

/* Decision on whether or not to use F1
   - Comment out to disable readout 
*/
/* #define ENABLE_F1  */
#ifdef ENABLE_F1
#define F1_WINDOW 2900
#define F1_LATENCY 2900
#endif


/* Decision on whether or not to enable the HCAL pulser
   - Comment out to disable HCAL pulser
*/
#define ENABLE_HCAL_PULSER

/*
#ifdef TI_SLAVE
int tsCrate=0;
#else
#ifdef TI_MASTER
int tsCrate=1;
#endif
#endif
*/

#define BANK_FADC        3
#define BANK_TI          4
#define BANK_TEST        5
#define BANK_HCAL_PULSER 6
#define BANK_F1          7

#define FIBER_LATENCY_OFFSET 0x40  /* measured longest fiber length */

#include "dmaBankTools.h"
#include "tiprimary_list.c" /* source required for CODA */
#include "fadcLib.h"         /* Header for FADC250 library */
#include "sdLib.h"  
#include "remexLib.h"
#include "hcal_usrstrutils.c" /* So we can read from a flags file */

#ifdef ENABLE_HCAL_PULSER
#include "hcalLib.h"
#endif

/* FADC250 Global Definitions */
#define FADC_START_SLOT       14    /* First ADC SLOT */
int faMode=1;
int flag_FADC_WINDOW_LAT = 115; /* Should now be read from flags file! */
int flag_FADC_WINDOW_WIDTH  = 20; /* Note, now read from flags file! */
//#define FADC_WINDOW_LAT       200  /* Trigger Window Latency */
//#define FADC_WINDOW_WIDTH     200  /* Trigger Window Width */
/* New values for LED pulser 2019-06-01 */
/* Settings for cosmic tests 10/7/2019 */
#ifdef ENABLE_HCAL_PULSER
/* 2020-03-04 Pulser system*/
//#define FADC_WINDOW_LAT       160 //160 for Pulser
//#define FADC_WINDOW_WIDTH      60 //60 for Pulser
#else /* Use cosmic trigger setup */
//#define FADC_WINDOW_LAT       118 //133 for half detector. /* Trigger Window Latency */
//#define FADC_WINDOW_WIDTH      20 //20 for half detector.  /* Trigger Window Width */
#endif
/* 2019-06-18 */
//#define FADC_WINDOW_LAT       330  /* Trigger Window Latency */
//#define FADC_WINDOW_WIDTH      80  /* Trigger Window Width */

#define FADC_DAC_LEVEL       3300 /* Internal DAC Level */
#define FADC_THRESHOLD       0x50 /* Threshold for data readout */
#define FADC_THRESHOLD       0x00 /* Threshold for data readout, i.e. no threshold */

/* Change latency for cosmic tests (Cornejo 20181207) */
/* #define FADC_WINDOW_LAT       225  */  /* Trigger Window Latency */
/* #define FADC_WINDOW_WIDTH     150  */  /* Trigger Window Width */

/* CTP */
#define CTP_THRESHOLD    4000

/* FLAGS */
#define NINPUTS 6 /* 6 trigger inputs on the new TI */
int flag_prescale[NINPUTS];
/* Flags for HCal pulser */
int flag_pulserTriggerInput;
int flag_LED_NSTEPS; /* Number of steps in sequence */
int flag_LED_STEP[50]; /* Step LED configuration */
int flag_LED_NSTEP[50]; /* Number of triggers in this step */

/* TEMPORARY */
/* EXPERIMENT IMPLEMENTING THE F1 TDC IN THE READOUT */
#ifdef ENABLE_F1
extern int f1tdcA32Base;fadc_f1tdc_347.root
int F1_SLOT;
extern int f1ID[20];
#define F1_ADDR 0x900000
#define ENABLE_F1_READOUT
#endif



unsigned int fadcSlotMask   = 0;    /* bit=slot (starting from 0) */
extern   int fadcA32Base;           /* This will need to be reset from it's default
                                     * so that it does not overlap with the TID */
extern   int nfadc;                 /* Number of FADC250s verified with the library */
extern   int fadcID[FA_MAX_BOARDS]; /* Array of slot numbers, discovered by the library */
extern   int fadcAddrList[FA_MAX_BOARDS]; /* Array of slot numbers, discovered by the library */
int NFADC;                          /* The Maximum number of tries the library will
                                     * use before giving up finding FADC250s */
int FA_SLOT;                        /* We'll use this over and over again to provide
				     * us access to the current FADC slot number */ 



/* for the calculation of maximum data words in the block transfer */
unsigned int MAXFADCWORDS = 0;
unsigned int MAXTIWORDS  = 0;

/* Default Global Blocklevel (Number of events per block) */
unsigned int BLOCKLEVEL=1;
#define BUFFERLEVEL 1


#ifdef ENABLE_HCAL_PULSER
#define HCAL_LED_NLIST 3 /* Number of steps in sequence */
int flag_PULSER_ENABLED; /* Determined based on ps and nsteps */
unsigned int HCAL_LED_COUNT = 0;
unsigned int HCAL_LED_ITER=0;
unsigned int HCAL_LED_C_STEP;
unsigned int HCAL_LED_C_NSTEP;
#endif

/* function prototype */
void rocTrigger(int arg);
void readUserFlags();


/****************************************
 *  DOWNLOAD
 ****************************************/
void
rocDownload()
{
  int islot;

  remexSetCmsgServer("sbs1");
  remexSetRedirect(1);
  remexInit(NULL,1);
  
  /* Read our flags files */
  readUserFlags();


  /* Setup Address and data modes for DMA transfers
   *   
   *  vmeDmaConfig(addrType, dataType, sstMode);
   *
   *  addrType = 0 (A16)    1 (A24)    2 (A32)
   *  dataType = 0 (D16)    1 (D32)    2 (BLK32) 3 (MBLK) 4 (2eVME) 5 (2eSST)
   *  sstMode  = 0 (SST160) 1 (SST267) 2 (SST320)
   */
  vmeDmaConfig(2,5,1); 

  /***************************************
   * TI Setup 
   ***************************************/
#ifndef TI_DATA_READOUT
  /* Disable data readout */
  tiDisableDataReadout();
  /* Disable A32... where that data would have been stored on the TI */
  tiDisableA32();
#endif

  /* Set crate ID */
  tiSetCrateID(0x15); /* ROC 1 */

  tiSetTriggerSource(TI_TRIGGER_TSINPUTS); 

  /* Load the trigger table that associates 
     pins 21/22 | 23/24 | 25/26 : trigger1
     pins 29/30 | 31/32 | 33/34 : trigger2
  */
  tiLoadTriggerTable(0);

  tiSetTriggerHoldoff(1,10,0);
  tiSetTriggerHoldoff(2,10,0);

  /* Set the sync delay width to 0x40*32 = 2.048us */
  tiSetSyncDelayWidth(0x54, 0x40, 1);

  /* Set the SyncReset type to fixed 4 ms width */
  tiSetSyncResetType(1);

  /* Set the busy source to non-default value (no Switch Slot B busy) */
/*   tiSetBusySource(TI_BUSY_LOOPBACK,1); */
/*   tiSetBusySource(0,1); */

  tiSetFiberDelay(0x40,FIBER_LATENCY_OFFSET);

#ifdef TI_MASTER
  /* Set number of events per block (if Master, will be broadcasted at end of Prestart)*/
  tiSetBlockLevel(BLOCKLEVEL);
#endif

  tiSetEventFormat(1);

  tiSetBlockBufferLevel(BUFFERLEVEL);

  tiStatus(0);
  
#ifdef ENABLE_F1
/* TEST: EXPERIMENT WITH IMPLEMENTING F1 TDC INTO READOUT */
// F1
//f1GStatus(0);
  iFlag = 0x0; /* no SDC */
  //iFlag |= 4;  /* read from file */
  //iFlag |= 2;  /* Normal Resolution, Trigger matching */
  iFlag |= 3;  /* Normal Resolution, Start Stop */
  //iFlag |= 1;

  f1Init(F1_ADDR,0x0,1,iFlag);
  F1_SLOT = f1ID[0];

  /* Setup F1TDC */
  /*   f1Clear(F1_SLOT); */
  /*   f1SetConfig(F1_SLOT,2,0xff); */
  f1EnableData(F1_SLOT,0xff);
  f1SetBlockLevel(F1_SLOT,1);
  /*   f1DisableBusError(F1_SLOT); */
  f1EnableBusError(F1_SLOT);
  printf("F1 TDC Configuration: \n");
  f1ConfigShow(0, 0);
  printf("F1 TDC Status\n");
  f1Status(F1_SLOT,0);
  f1Clear(F1_SLOT);
  f1SetWindow(0,F1_WINDOW,F1_LATENCY,0);
  f1ConfigShow(0, 0);

//MARCO: Sync reset to F1 - connect output of TI to input SYNC of F1 (for V1 only?)
tiSetOutputPort(0,0,0,0);
usleep(50000);
tiSetOutputPort(1,1,1,1);
usleep(50000);
tiSetOutputPort(0,0,0,0);
usleep(50000);
  printf("\n\n\nF1 Status (again, after clearing and stuff...\n\n\n\n");
  f1Status(F1_SLOT,0);
  printf("\n\n\nEnd status of F1\n\n\n");
#endif

#ifdef ENABLE_HCAL_PULSER
  hcalClkIn(0); /* Turn off LEDs at end of run */
#endif


  printf("rocDownload: User Download Executed\n");

}


/****************************************
 *  PRESTART
 ****************************************/
void
rocPrestart()
{
  unsigned short iflag;
  int stat,islot;
  int i,ti_input_triggers;

  /* Read in the user flag files */
  readUserFlags();

  vmeDmaConfig(2,5,1); 
  /* Set needed TS input bits */
  ti_input_triggers = 0;
  printf("prescales: ");
  for(i = 0; i < NINPUTS; i++) {
    ti_input_triggers |= (flag_prescale[i]>0)<<i;
    printf("ps%d=%d ",i+1,flag_prescale[i]);
  }
  printf("\n");
  tiEnableTSInput( ti_input_triggers ); 

  /* Setup the prescales */
  for(i = 0; i < NINPUTS; i++) {
    if(flag_prescale[i]>0) {
      tiSetInputPrescale(1<<i,flag_prescale[i]);
    }
  }

  tiLoadTriggerTable(0);

  tiSetTriggerHoldoff(1,10,0);
  tiSetTriggerHoldoff(2,10,0);

  /* Set the sync delay width to 0x40*32 = 2.048us */
  tiSetSyncDelayWidth(0x54, 0x40, 1);

  /* Set the SyncReset type to fixed 4 ms width */
  tiSetSyncResetType(1);

  /* Set the busy source to non-default value (no Switch Slot B busy) */
/*   tiSetBusySource(TI_BUSY_LOOPBACK,1); */
/*   tiSetBusySource(0,1); */

  tiSetFiberDelay(0x40,FIBER_LATENCY_OFFSET);


/*   tiSetup(21); */

// 03Apr2013, moved this into ctpInit()
/*   ctpFiberReset(); */

  /***************************************
   * FADC Setup 
   ***************************************/
  /* Here, we assume that the addresses of each board were set according to their
   * geographical address (slot number):
   * Slot  3:  (3<<19) = 0x180000
   * Slot  4:  (4<<19) = 0x200000
   * ...
   * Slot 20: (20<<19) = 0xA00000
   */

  NFADC = (16+2-4)*0+7;   /* 16 slots + 2 (for the switch slots) */
  FA_SLOT=FADC_START_SLOT;
  if(FA_SLOT==12) { // skip slot 12! (Slot 11 will be skipped in the logic of the for looop
    FA_SLOT++;
  }
  for(islot = 0; islot < NFADC; islot++) {
    if(FA_SLOT==11) {
     FA_SLOT+=2; // Skip slots 11+12 on VXS crates
    }
    fadcAddrList[islot] = (FA_SLOT++)<<19;
  }
  fadcA32Base=0x09000000;

  /* Setup the iFlag.. flags for FADC initialization */
  int iFlag=0;
  /* Sync Source */
  iFlag |= (1<<0);    /* VXS */
  /* Trigger Source */
  iFlag |= (1<<2);    /* VXS */
  /* Clock Source */
  iFlag |= (0<<5);    /* Self */
  iFlag |= FA_INIT_SKIP_FIRMWARE_CHECK; /* no check of FADC firmware for HPS version */
  iFlag |= FA_INIT_USE_ADDRLIST; /* Use our address list */
  vmeSetQuietFlag(1); /* skip the errors associated with BUS Errors */
  faInit((unsigned int)(3<<19),(1<<19),NFADC,iFlag);
  /*faInit((unsigned int)0x600000,(1<<19),2,iFlag);*/
  /*faInit((unsigned int)0x600000,(1<<19),3,iFlag); */
  /*NFADC=2;*/        /* Redefine our NFADC with what was found from the driver */
  /*NFADC=3;*/        /* Redefine our NFADC with what was found from the driver */
 
  vmeSetQuietFlag(0); /* Turn the error statements back on */
  
  /* Calculate the maximum number of words per block transfer (assuming Pulse mode)
   *   MAX = NFADC * BLOCKLEVEL * (EvHeader + TrigTime*2 + Pulse*2*chan) 
   *         + 2*32 (words for byte alignment) 
   */
  if(faMode == 1) /* Raw window Mode */
    //MAXFADCWORDS = NFADC * BLOCKLEVEL * (1+2+FADC_WINDOW_WIDTH*16) + 3;
    MAXFADCWORDS = NFADC * BLOCKLEVEL * (1+1+2+(flag_FADC_WINDOW_WIDTH*16)) + 3;
  else /* Pulse mode */
    MAXFADCWORDS = NFADC * BLOCKLEVEL * (1+2+32) + 2*32;
  /* Maximum TID words is easier to calculate, but we can be conservative, since
   * it's first in the readout
   */
/*   MAXTIDWORDS = 8+(3*BLOCKLEVEL); */
  
  printf("**************************************************\n");
  printf("* Calculated MAX FADC words per block = %d\n",MAXFADCWORDS);
/*   printf("* Calculated MAX TID  words per block = %d\n",MAXTIDWORDS); */
  printf("**************************************************\n");
  /* Check these numbers, compared to our buffer size.. */
/*   if( (MAXFADCWORDS+MAXTIDWORDS)*4 > MAX_EVENT_LENGTH ) */
/*     { */
/*       printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"); */
/*       printf(" WARNING.  Event buffer size is smaller than the expected data size\n"); */
/*       printf("     Increase the size of MAX_EVENT_LENGTH and recompile!\n"); */
/*       printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"); */
/*     } */

  
  if(NFADC>1)
    faEnableMultiBlock(1);

  /* Additional Configuration for each module */
  fadcSlotMask=0;
  for(islot=0;islot<NFADC;islot++) 
    {
      FA_SLOT = fadcID[islot];      /* Grab the current module's slot number */
      printf("This is the slot number I got: FA_SLOT = %d\n",FA_SLOT);
      fadcSlotMask |= (1<<FA_SLOT); /* Add it to the mask */

      /* Set the internal DAC level */
      faSetDAC(FA_SLOT,FADC_DAC_LEVEL,0);
      /* Set the threshold for data readout */
      faSetThreshold(FA_SLOT,FADC_THRESHOLD,0);
      faPrintThreshold(FA_SLOT);
      /*  Setup option 1 processing - RAW Window Data     <-- */
      /*        option 2            - RAW Pulse Data */
      /*        option 3            - Integral Pulse Data */
      /*  Setup 200 nsec latency (PL  = 50)  */
      /*  Setup  80 nsec Window  (PTW = 20) */
      /*  Setup Pulse widths of 36ns (NSB(3)+NSA(6) = 9)  */
      /*  Setup up to 1 pulse processed */
      /*  Setup for both ADC banks(0 - all channels 0-15) */
      /* Integral Pulse Data */
      faSetProcMode(FA_SLOT,faMode,flag_FADC_WINDOW_LAT,flag_FADC_WINDOW_WIDTH,3,6,1,0);
	
      /* Bus errors to terminate block transfers (preferred) */
      faEnableBusError(FA_SLOT);
      /* Set the Block level */
      faSetBlockLevel(FA_SLOT,BLOCKLEVEL);

      /* Set the individual channel pedestals for the data that is sent
       * to the CTP
       */
      int ichan;
      for(ichan=0; ichan<16; ichan++)
	{
	  faSetChannelPedestal(FA_SLOT,ichan,0);
	}

   }

  /***************************************
   *   SD SETUP
   ***************************************/
  sdInit(0);   /* Initialize the SD library */
  sdSetActiveVmeSlots(fadcSlotMask); /* Use the fadcSlotMask to configure the SD */
  sdStatus();



  /* FADC Perform some resets, status */
  for(islot=0;islot<NFADC;islot++) 
    {
      FA_SLOT = fadcID[islot];
      faSetClockSource(FA_SLOT,2);
      faClear(FA_SLOT);
      faResetToken(FA_SLOT);
      faResetTriggerCount(FA_SLOT);
      faStatus(FA_SLOT,0);
    }

  /* TI Status */
  tiStatus(0);

  /*  Enable FADC */
  for(islot=0;islot<NFADC;islot++) 
    {
      FA_SLOT = fadcID[islot];
      faChanDisable(FA_SLOT,0xffff);
      faSetMGTTestMode(FA_SLOT,0);
      faEnable(FA_SLOT,0,0);
    }

  sdStatus();

#ifdef ENABLE_HCAL_PULSER
  if(flag_PULSER_ENABLED) {
    HCAL_LED_ITER=0;
    HCAL_LED_COUNT=0;
    printf("HCAL Pulser sequence: ");
    for(i = 0 ; i < flag_LED_NSTEPS; i++) {
      printf(" %d/%d",flag_LED_STEP[i],flag_LED_NSTEP[i]);
    }
    printf("\n");
    /* Clock in the first setting */
    HCAL_LED_C_STEP = flag_LED_STEP[0];
    HCAL_LED_C_NSTEP = flag_LED_NSTEP[0];
    hcalClkIn(HCAL_LED_C_STEP);
  } else {
    hcalClkIn(0);
  }

#endif

  printf("rocPrestart: User Prestart Executed\n");
  /* SYNC is issued after this routine is executed */

#ifdef ENABLE_F1
  /* Clear F1 TDCs */
  F1_SLOT = f1ID[0];
  f1Clear(F1_SLOT);

//MARCO: Sync reset to F1 - connect output of TI to input SYNC of F1 (for V1 only?)
tiSetOutputPort(0,0,0,0);
usleep(50000);
tiSetOutputPort(1,1,1,1);
usleep(50000);
tiSetOutputPort(0,0,0,0);
usleep(50000);
#endif

}

/****************************************
 *  GO
 ****************************************/
void
rocGo()
{
  /* Enable modules, if needed, here */
  int iwait=0;
  int islot, allchanup=0;

  /* Get the current block level */
  BLOCKLEVEL = tiGetCurrentBlockLevel();
  printf("%s: Current Block Level = %d\n",
	 __FUNCTION__,BLOCKLEVEL);


  //ctpGetErrorLatchFS(1);

  for(islot=0;islot<NFADC;islot++)
    {
      FA_SLOT = fadcID[islot];
      faChanDisable(FA_SLOT,0x0);
      faSetMGTTestMode(FA_SLOT,1);
      faSetBlockLevel(FA_SLOT,BLOCKLEVEL);
    }


  /* Enable modules, if needed, here */

#ifdef ENABLE_F1
  F1_SLOT = f1ID[0];
  f1EnableData(F1_SLOT,0xff);
#endif


  tiSetOutputPort(0,0,0,0);
  /* Interrupts/Polling enabled after conclusion of rocGo() */
}

/****************************************
 *  END
 ****************************************/
void
rocEnd()
{
  int islot;

  /* FADC Disable */
  for(islot=0;islot<NFADC;islot++) 
    {
      FA_SLOT = fadcID[islot];
      faDisable(FA_SLOT,0);
      faStatus(FA_SLOT,0);
    }

  tiStatus(0);
  sdStatus();
#ifdef ENABLE_HCAL_PULSER
  hcalClkIn(0); /* Turn off LEDs at the end */
#endif
  /* Turn off all output ports */
  tiSetOutputPort(0,0,0,0);

#ifdef ENABLE_F1
  F1_SLOT = f1ID[0];
  f1Reset(F1_SLOT,0); 
#endif

  printf("rocEnd: Ended after %d blocks\n",tiGetIntCount());
  
}

/****************************************
 *  READOUT TRIGGER
 ****************************************/
void
rocTrigger(int arg)
{
  int islot,ii;
  int dCnt, len=0, idata;
  int stat, itime, gbready;
  int roflag=1;
  int syncFlag=0;
  int datascan=0;
  int nwords;
  static unsigned int roEventNumber=0;


  roEventNumber++;
  syncFlag = tiGetSyncEventFlag();

  if(tiGetSyncEventReceived())
    {
      printf("%s: Sync Event received at readout event %d\n",
	     __FUNCTION__,roEventNumber);
    }

  if(syncFlag)
    {
      printf("%s: Sync Flag Received at readout event %d\n",
	     __FUNCTION__,roEventNumber);
/*       printf("  Sleeping for 10 seconds... \n"); */
/*       sleep(10); */
/*       printf("  ... Done\n"); */
    }

  /* Set high, the first output port */
  tiSetOutputPort(1,0,0,0);

  BANKOPEN(BANK_TEST,BT_UI4,0);
  *dma_dabufp++ = LSWAP(tiGetIntCount());
  *dma_dabufp++ = LSWAP(0xdead);
  *dma_dabufp++ = LSWAP(0xcebaf111);
  BANKCLOSE;

#ifdef TI_DATA_READOUT
  BANKOPEN(BANK_TI,BT_UI4,0);

  vmeDmaConfig(2,5,1); 
  dCnt = tiReadBlock(dma_dabufp,8+(3*BLOCKLEVEL),1);
  if(dCnt<=0)
    {
      printf("No data or error.  dCnt = %d\n",dCnt);
    }
  else
    {
      dma_dabufp += dCnt;
    }

  BANKCLOSE;
#endif

  /* Readout FADC */
  if(NFADC!=0)
    {
      FA_SLOT = fadcID[0];
      for(itime=0;itime<100;itime++) 
	{
	  gbready = faGBready();
	  stat = (gbready == fadcSlotMask);
	  if (stat>0) 
	    {
	      break;
	    }
	}
      if(stat>0) 
	{
	  if(NFADC>1) roflag=2; /* Use token passing scheme to readout all modules */
	  BANKOPEN(BANK_FADC,BT_UI4,0);
	  dCnt = faReadBlock(FA_SLOT,dma_dabufp,MAXFADCWORDS,roflag);
	  if(dCnt<=0)
	    {
	      printf("FADC%d: No data or error.  dCnt = %d\n",FA_SLOT,dCnt);
	    }
	  else
	    {
	      if(dCnt>=MAXFADCWORDS)
		{
		  printf("%s: WARNING.. faReadBlock returned dCnt >= MAXFADCWORDS (%d >= %d)\n",
			 __FUNCTION__,dCnt, MAXFADCWORDS);
		}
	      else 
		dma_dabufp += dCnt;
	    }
	  BANKCLOSE;
	} 
      else 
	{
	  printf ("FADC%d: no events   stat=%d  intcount = %d   gbready = 0x%08x  fadcSlotMask = 0x%08x\n",
		  FA_SLOT,stat,tiGetIntCount(),gbready,fadcSlotMask);
	}

      /* Reset the Token */
      if(roflag==2)
	{
	  for(islot=0; islot<NFADC; islot++)
	    {
	      FA_SLOT = fadcID[islot];
	      faResetToken(FA_SLOT);
	    }
	}
    }


#ifdef ENABLE_F1
#ifdef ENABLE_F1_READOUT
  vmeDmaConfig(2,3,0);
  BANKOPEN(BANK_F1,BT_UI4,0);

    /* Insert trigger count  - Make sure bytes are ordered little-endian (LSWAP)*/
  // Marco:
  *dma_dabufp++ = LSWAP(tiGetIntCount());

  F1_SLOT = f1ID[0];
  /* Check for valid data here */
  for(ii=0;ii<100;ii++)
    {
      datascan = f1Dready(F1_SLOT);
      if (datascan>0)
        {
          break;
        }
    }
//printf("F1_datascan: %d\n",datascan);
  if(datascan>0)
    {
      nwords = f1ReadEvent(F1_SLOT,dma_dabufp,500,1);
//printf("  -> nwords = %d\n",nwords);

      if(nwords < 0)
        {
          //printf("ERROR: in transfer (event = %d), status = 0x%x\n", tirGetIntCount(),nwords);
          // Marco:
          printf("ERROR: in transfer (event = %d), status = 0x%x\n", tiGetIntCount(),nwords);
          *dma_dabufp++ = LSWAP(0xda000bad);
        }
      else
        {
          dma_dabufp += nwords;
        }
    }
  else
    {
      //printf("ERROR: Data not ready in event %d\n",tirGetIntCount());
      //Marco:
      printf("ERROR: Data not ready in event %d\n",tiGetIntCount());
      *dma_dabufp++ = LSWAP(0xda000bad);
    }
  *dma_dabufp++ = LSWAP(0xda0000ff); /* Event EOB */

  BANKCLOSE;
  vmeDmaConfig(2,5,1);

#endif
#endif

#ifdef ENABLE_HCAL_PULSER
  if(flag_PULSER_ENABLED) {
    HCAL_LED_COUNT++;
    BANKOPEN(BANK_HCAL_PULSER,BT_UI4,0);
    /**dma_dabufp++ = LSWAP(tiGetIntCount());*/
    *dma_dabufp++ = LSWAP(HCAL_LED_ITER);
    *dma_dabufp++ = LSWAP(HCAL_LED_C_STEP);
    *dma_dabufp++ = LSWAP(HCAL_LED_COUNT);
    *dma_dabufp++ = LSWAP((HCAL_LED_ITER<<22)|(HCAL_LED_C_STEP<<16)|HCAL_LED_COUNT);
    BANKCLOSE;
    /* Run the HCAL pulser clock in code */
    if(HCAL_LED_COUNT>=HCAL_LED_C_NSTEP) {
      HCAL_LED_COUNT=0;
      HCAL_LED_ITER++;
      if(HCAL_LED_ITER>=flag_LED_NSTEPS) {
        HCAL_LED_ITER=0;
      }
      HCAL_LED_C_STEP=flag_LED_STEP[HCAL_LED_ITER];
      HCAL_LED_C_NSTEP=flag_LED_NSTEP[HCAL_LED_ITER];
      printf("Clocking in HCAL LED: %2d, %2d (tircount:%d)\n",
        HCAL_LED_ITER,HCAL_LED_C_STEP,tiGetIntCount());
      hcalClkIn(HCAL_LED_C_STEP);
    }
  }
#endif

  /* Turn off all output ports */
  tiSetOutputPort(0,0,0,0);
}

/*
 * rocCleanup
 *  - Routine called just before the library is unloaded.
 */

void
rocCleanup()
{
  int islot=0;

  /*
   * Perform a RESET on all FADC250s.
   *   - Turn off all A32 (block transfer) addresses
   */
/*   printf("%s: Reset all FADCs\n",__FUNCTION__); */
  
  for(islot=0; islot<NFADC; islot++)
    {
      FA_SLOT = fadcID[islot];
      faReset(FA_SLOT,1); /* Reset, and DO NOT restore A32 settings (1) */
    }

  remexClose();

}

/*  Read the user flags/configuration file. */
void readUserFlags()
{
  int i;
  char pstext[10];

  printf("Reading user flags file.");
  init_strings();
  flag_pulserTriggerInput=getint("pulserTrigger");
  /* Read the FADC configuration */
  flag_FADC_WINDOW_LAT=getint("fadc_window_lat");
  flag_FADC_WINDOW_WIDTH=getint("fadc_window_width");

  /* Read prescales */
  printf("prescales: ");
  for(i = 0; i < NINPUTS; i++) {
    sprintf(pstext,"ps%d",i);
    flag_prescale[i] = getint(pstext);
    printf("%s=%d, ",pstext,flag_prescale[i]);
  }
  printf("\n");

#ifdef ENABLE_HCAL_PULSER
  /* Read the hcal pulser step info */
  if(flag_pulserTriggerInput >= 0 && flag_pulserTriggerInput <6) {
    flag_PULSER_ENABLED = flag_prescale[flag_pulserTriggerInput];
    if(flag_PULSER_ENABLED) {
      flag_LED_NSTEPS=getint("pulser_nsteps");
      char ptext[50];
      for(i = 0; i < flag_LED_NSTEPS; i++) {
        sprintf(ptext,"pulser_step%d",i);
        flag_LED_STEP[i]  = getint(ptext);
        sprintf(ptext,"pulser_nstep%d",i);
        flag_LED_NSTEP[i] = getint(ptext);
      }
    }
  } else {
    flag_PULSER_ENABLED=0;
  }
#endif
}



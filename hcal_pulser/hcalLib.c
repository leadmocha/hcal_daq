
#define _GNU_SOURCE

#define DEVEL

#ifdef VXWORKS
#include <vxWorks.h>
#include <sysLib.h>
#include <logLib.h>
#include <taskLib.h>
#include <intLib.h>
#include <iv.h>
#include <semLib.h>
#include <vxLib.h>
#include "vxCompat.h"
#include "../jvme/jvme.h"
#else
#include <sys/prctl.h>
#include <unistd.h>
#include "jvme.h"
#endif
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "tiLib.h"
#define _GNU_SOURCE

#define DEVEL

#ifdef VXWORKS
#include <vxWorks.h>
#include <sysLib.h>
#include <logLib.h>
#include <taskLib.h>
#include <intLib.h>
#include <iv.h>
#include <semLib.h>
#include <vxLib.h>
#include "vxCompat.h"
#include "../jvme/jvme.h"
#else
#include <sys/prctl.h>
#include <unistd.h>
#include "jvme.h"
#endif
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "tiLib.h"
#include "hcalLib.h"

const int HCAL_CBIT = 1;
const int HCAL_DBIT = 2;

unsigned int opbits[4];

/*
void hcalConfigBits(unsigned int cbit, unsigned int dbit)
{
}
*/

void hcalSetOutputPort(unsigned int cbit, unsigned int dbit)
{
  opbits[HCAL_CBIT]=cbit;
  opbits[HCAL_DBIT]=dbit;
  tiSetOutputPort(opbits[0],opbits[1],opbits[2],opbits[3]);
}

int hcalClkIn(unsigned int iset)
{
  unsigned int dbit;
  int i,j;
  if(iset<0||iset>63) {
    return -1;
  }
  /* Clear the output bits */
  for(i = 0; i < 4; i++) {
    opbits[i] = 0;
  }
  opbits[0] = 1; /* Since I think it's on by default */


  /* Do this twice! */
  for(j = 0; j < 2; j++) {
    /* Loop over lowest 6bits in iset to set the dbit */
    for(i = 0; i < 6; i++) {
      dbit = 0;
      if((iset>>i)&0x1) {
        dbit = 1;
      }
      hcalSetOutputPort(0,dbit);
      hcalSetOutputPort(1,dbit); /* by making the clock in in the middle, we ensure the dbit is properly set before shifting bits on the bit-shifter */
      hcalSetOutputPort(0,dbit);
    }
  }

  taskDelay(2);
  return 0;
}

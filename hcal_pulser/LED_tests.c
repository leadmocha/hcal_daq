
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <vxWorks.h>
//#include "./../vxwrks/ppc/target/h/vxWorks.h"

#include "v1495.h"

/*sergey*/
#define EIEIO    __asm__ volatile ("eieio")
#define SYNC     __asm__ volatile ("sync")

/* Parameters for the access to the Flash Memory */
#define VME_FIRST_PAGE_STD    768    /* first page of the image STD */
#define VME_FIRST_PAGE_BCK    1408   /* first page of the image BCK */
#define USR_FIRST_PAGE_STD    48     /* first page of the image STD */
#define USR_FIRST_PAGE_BCK    1048   /* first page of the image BCK */
#define PAGE_SIZE             264    /* Number of bytes per page in the target flash */

/* flash memory Opcodes */
#define MAIN_MEM_PAGE_READ          0x00D2
#define MAIN_MEM_PAGE_PROG_TH_BUF1  0x0082



int clk_loop(unsigned int iwait,unsigned int count)
{ int i,j,k,res,dbit,cbit,nprint;
  int bits[6];

  unsigned short  xread     ;
  unsigned short  xwrote    ;
  unsigned short* x         ;
  unsigned int localAddress,fullAddress,v1495Address ;
  int iset;

  for(;;1>0){
    printf("Clock in what? >");
    scanf("%d/n",&iset);

  if(iset<0||iset>63){printf("6 bit value to set. Must choose iset in range: 0-63\n
Arguments are: iset,iwait,count     Defaults:0, 20, 100\n");}

  v1495Address=0x950000;
  res = sysBusToLocalAdrs(0x39 , v1495Address , &localAddress);

  fullAddress = localAddress+0x1024;
  //printf("D_Control at %x\n",fullAddress);
  x      = (unsigned short*)fullAddress;
  /*set Dcontrol to 0 (Out, TTL)      1 is (Out, NIM)*/
  *x     = 0;          

  /*Point x to D_Data_Low*/
  fullAddress = localAddress+0x1028;
  //  printf("D_Data_low at %x\n",fullAddress);
  x      = (unsigned short*)fullAddress;
  *x     = 0;          


  if(iwait==0){iwait=1;}
  if(count==0){count=1000;}

  printf("Clock in %d then flash LEDs %d times with wait of %d ms. ",iset,count,iwait*10);

  k=1;
  for(i=0;i<6;i++){
    bits[i]=k;
    k=2*k;
  }



  printf("Set bits:");
  for(i=0;i<6;i++){
    dbit=0;
    if(bits[i]&iset){dbit=1;printf("%d ",i);}

  /* clock it in*/
  cbit=2;
  *x=dbit;
  //  taskDelay(1); /* 1 tick = 10ms */
  *x=cbit+dbit;
  // taskDelay(1); /* 1 tick = 10ms */
  *x=dbit;
  *x=0;
  //  taskDelay(1); /* 1 tick = 10ms */
  }

  printf("\nFlashing");
  nprint=2./(0.01*iwait);/* print one dot per two seconds*/
  for(i=0;i<count;i++){
    *x=4;
    *x=0;
  taskDelay(iwait); /* 1 tick = 10ms */
    if(i%nprint==0){printf(".");}
  }
  printf("\n");


  }
  return(iset);  //never reach this line
}





int count(int nloop,unsigned int iwait,unsigned int count)
{int i,j,k;
  if(nloop==0){nloop=1;}
  for(i=0;i<nloop;i++){
    printf("\n\n\n Loop %d of %d \n",i+1,nloop);
    for(j=0;j<64;j++){
      clk(j,iwait,count);}
}
  return(nloop);
}

int clk(int iset,unsigned int iwait,unsigned int count)
{ int i,j,k,res,dbit,cbit,nprint;
  int bits[6];

  unsigned short  xread     ;
  unsigned short  xwrote    ;
  unsigned short* x         ;
  unsigned int localAddress,fullAddress,v1495Address ;


  if(iset<0||iset>63){printf("6 bit value to set. Must choose iset in range: 0-63\n
Arguments are: iset,iwait,count     Defaults:0, 20, 100\n");return(iset);}

  v1495Address=0x950000;
  res = sysBusToLocalAdrs(0x39 , v1495Address , &localAddress);

  fullAddress = localAddress+0x1024;
  //printf("D_Control at %x\n",fullAddress);
  x      = (unsigned short*)fullAddress;
  /*set Dcontrol to 0 (Out, TTL)      1 is (Out, NIM)*/
  *x     = 0;          

  /*Point x to D_Data_Low*/
  fullAddress = localAddress+0x1028;
  //  printf("D_Data_low at %x\n",fullAddress);
  x      = (unsigned short*)fullAddress;
  *x     = 0;          


  if(iwait==0){iwait=20;}
  if(count==0){count=100;}

  printf("Clock in %d then flash LEDs %d times with wait of %d ms. ",iset,count,iwait*10);

  k=1;
  for(i=0;i<6;i++){
    bits[i]=k;
    k=2*k;
  }

 

  printf("Set bits:");
  for(i=0;i<6;i++){
    dbit=0;
    if(bits[i]&iset){dbit=1;printf("%d %c",i,7);
      //taskDelay(50);
}


  /* clock it in*/
  cbit=2;
  *x=dbit;
  //  taskDelay(1); /* 1 tick = 10ms */
  *x=cbit+dbit;
  //  taskDelay(1); /* 1 tick = 10ms */
  *x=dbit;
  *x=0;
  //  taskDelay(1); /* 1 tick = 10ms */
  }

  printf("\nFlashing");
  nprint=2./(0.01*iwait);/* print one dot per two seconds*/
  for(i=0;i<count;i++){
    *x=4;
    *x=0;
  taskDelay(iwait); /* 1 tick = 10ms */
    if(i%nprint==0){printf(".");}
  }
  printf("\n");
  return(iset);
}

int clk_fix(int iset)
{ int i,j,k,res,dbit,cbit,nprint,iwait;
  int bits[6];

  unsigned short  xread     ;
  unsigned short  xwrote    ;
  unsigned short* x         ;
  unsigned int localAddress,fullAddress,v1495Address ;
  // load bit pattern then flash at 100 Hz
  // reload iset bit pattern every 1000 flashes

  if(iset<0||iset>63){printf("6 bit value to set. Must choose iset in range: 0-63\n
Arguments are: iset,iwait,count     Defaults:0, 20, 100\n");return(iset);}

  v1495Address=0x950000;
  res = sysBusToLocalAdrs(0x39 , v1495Address , &localAddress);

  fullAddress = localAddress+0x1024;
  //printf("D_Control at %x\n",fullAddress);
  x      = (unsigned short*)fullAddress;
  /*set Dcontrol to 0 (Out, TTL)      1 is (Out, NIM)*/
  *x     = 0;          

  /*Point x to D_Data_Low*/
  fullAddress = localAddress+0x1028;
  //  printf("D_Data_low at %x\n",fullAddress);
  x      = (unsigned short*)fullAddress;
  *x     = 0;          

  iwait=1;

  printf("Clock in %d then flash LEDs infinite times, reloading every 1000 flashes with wait of %d ms. ",iset,iwait*10);

  k=1;
  for(i=0;i<6;i++){
    bits[i]=k;
    k=2*k;
  }

 
  while(0<1)
    {
  printf("Set bits:");
  for(i=0;i<6;i++){
    dbit=0;
    if(bits[i]&iset){dbit=1;printf("%d %c",i,7);
      //taskDelay(50);
}


  /* clock it in*/
  cbit=2;
  *x=dbit;
  //  taskDelay(1); /* 1 tick = 10ms */
  *x=cbit+dbit;
  //  taskDelay(1); /* 1 tick = 10ms */
  *x=dbit;
  *x=0;
  //  taskDelay(1); /* 1 tick = 10ms */
  }

  printf("\nFlashing");
  nprint=2./(0.01*iwait);/* print one dot per two seconds*/
  for(i=0;i<1000;i++){
    *x=4;
    *x=0;
  taskDelay(iwait); /* 1 tick = 10ms */
    if(i%nprint==0){printf(".");}
  }
  //  printf("\n");
  //  return(iset);
    }
}


int LED1(unsigned int iwait,unsigned int count)
{ int i,res;

  unsigned short  xread     ;
  unsigned short  xwrote    ;
  unsigned short* x         ;
  unsigned int localAddress ;

  //set Dcontrol to 1 (Nim, out)
  unsigned int fullAddress = 0x951024;
  res = sysBusToLocalAdrs(0x39 , fullAddress , &localAddress);
  x      = (unsigned short*)localAddress;
  *x     = 1;          

  //Point x to D_Data_Low
  fullAddress = 0x951028;
  res = sysBusToLocalAdrs(0x39 , fullAddress , &localAddress);
  x      = (unsigned short*)localAddress;
  *x     = 0;          

  for(i=1;i<count;i++){
    *x=4;
    *x=2;
  taskDelay(iwait); /* 1 tick = 10ms */
   
  }
  return(0);
}








int bq_wo(unsigned int address , unsigned short word)
{
  unsigned short *x;
  unsigned short xread;
  unsigned int localAddress;
  int res;
  int i;
  
  //95 for v1495 logic and trig unit    E1 for V1742 digitizer
  
  unsigned int fullAddress = 0x00950000 + address;
  
  res = sysBusToLocalAdrs (0x39,fullAddress,&localAddress);
  
  printf("\nAddress=0x%x\nlocalAddress=0x%x\nres=%d\n\n" , fullAddress , localAddress , res);
  
  x = (unsigned short*)localAddress;
  
  //xread=*x;
  //printf("Read 0x%x        ",xread);
  
  *x = word;
  
  //xread=*x;
  //printf("Now 0x%x\n",xread);
  
  printf("Wrote contents without reading\n");
  
  return(res);
  
}



int il_tw(unsigned int address , unsigned short word)
{
  //this function takes a word and writes it to an appropriate address of the ROC
  //VxWorks recognizes this appropriate address, and sees that we REALLY want to 
  //write the the VME crate's address space. So it handles that part. The addresses
  //in question are handled by the sysBusToLocalAdrs function.

  unsigned short  xread     ;
  unsigned short  xwrote    ;
  unsigned short* x         ;
  unsigned int localAddress ;

  unsigned int fullAddress = 0x00950000 + address;//We tack on the 95 in front, that's how the v1495 knows we're talking to it.

  int res;
  
  //We tack on the 95 in front, that's how the address space
  // of the V1495 is defined
  
  //95 for v1495 logic and trig unit, E1 for V1742 digitizer
  
  //but the ROC we're using doesn't know what these addresses
  //are actually about. the ROC sees a different set of addresses
  //that are mapped to the V1945 addresses. We have to 
  //convert the fullAddress to something the ROC knows about
  //this new address is referred to as local address

  //sysBusToLocalAdrs(0x39, address you want to convert, where the converted address is put)
  //pretty much gives me back fullAddress + 0x90000000
  res = sysBusToLocalAdrs(0x39 , fullAddress , &localAddress);

  //res is just something sysBusToLocalAdrs returns in the event of a successful address conversion
  
  printf("\nAddress = 0x%x\nlocalAddress = 0x%x\nres=%d\n\n" , fullAddress , localAddress,res);
  
  x      = (unsigned short*)localAddress;//point x to where local address points
  xread  = *x;                       //put contents which x points to in x read
  *x     = word;                     //put contents of word into where x is pointing
  xwrote = *x;                       //take the contents of x, the word, and put them in xwrote

  printf("Read 0x%x....... Now 0x%x\n" , xread , xwrote);

  return(res);

}


//dumps words by the lines
int bq_dmp(unsigned int addr,int nline)
{
  unsigned short xread;
  unsigned short *x;

  unsigned int full_addr;
  unsigned int laddr;
  
  int i;
  int j;
  int res;
  
  //95 for v1495 logic and trig unit    E1 for V1742 digitizer
  unsigned int start_addr = 0x00950000 + addr;
  
  for(i=0;i<nline;i++)
    {
      printf("%8.8x ** ",start_addr+i*0x20);
      
      for(j=0;j<0x20;j+=2)
	{
	  full_addr = start_addr + j + i*0x20;
	  
	  res=sysBusToLocalAdrs (0x39,full_addr,&laddr);
	  
	  //  printf(" addr=0x%x\n laddr=0x%x   res=%d\n",full_addr,laddr,res);
	  x=(unsigned short*)laddr;
	  
	  xread=*x;
	  
	  printf("%4x ",xread);
	}
      printf("\n");
    }
 
  return(res);
  
}

int bq_dmp_rpt(unsigned int addr,int nline)
{
  unsigned short xread;
  unsigned short *x;

  unsigned int full_addr;
  unsigned int laddr;
  
  int i;
  int j;
  int res;
  
  //95 for v1495 logic and trig unit    E1 for V1742 digitizer
  unsigned int start_addr = 0x00950000 + addr;
 do
    {
      printf("\r");
      for(i=0;i<nline;i++)
	{
	  printf("%8.8x ** ",start_addr+i*0x20);
	  
	  for(j=0;j<0x20;j+=2)
	    {
	      full_addr = start_addr + j + i*0x20;
	      
	      res=sysBusToLocalAdrs (0x39,full_addr,&laddr);
	      
	      //  printf(" addr=0x%x\n laddr=0x%x   res=%d\n",full_addr,laddr,res);
	      x=(unsigned short*)laddr;
	      
	      xread=*x;
	      
	      printf("%4x ",xread);
	    }
	  //printf("\n");
	}
    }
 while (1>0);
  return(res);
  
}




//dumps words by the number specified i the routine
int il_dmp(unsigned int addr,int nwords)

{
  unsigned short xread;
  unsigned short *x;

  unsigned int full_addr;
  unsigned int laddr;
  int j;
  int res;
  
  //95 for v1495 logic and trig unit    E1 for V1742 digitizer
  unsigned int start_addr = 0x00950000 + addr;
  
  printf("%8.8x ** ", start_addr );
  
  for( j = 0 ; j < nwords*2 ; j += 2 )
    {
      if ((j%(0x20) == 0) && (j != 0)) 
	{
	  printf("\n");
	  printf("%8.8x ** ", start_addr + j);
	}

      full_addr = start_addr + j;
      
      res=sysBusToLocalAdrs (0x39,full_addr,&laddr);
      
      //printf(" addr=0x%x\n laddr=0x%x   res=%d\n",full_addr,laddr,res);

      x=(unsigned short*)laddr;
      
      xread=*x;
      
      printf("%4x ",xread);
    }

  printf("\n");
 
  return(res);
  
}

//repeatedly dumps a range of addresses, ntimes
int il_dmp_rpt(unsigned int addr,int nwords,int ntimes)

{
  unsigned short xread;
  unsigned short *x;

  unsigned int full_addr;
  unsigned int laddr;
  int j;
  int i;
  int res;
  
  //95 for v1495 logic and trig unit    E1 for V1742 digitizer
  unsigned int start_addr = 0x00950000 + addr;
 
  for (i = 0; i < ntimes; i++)
    {
      //printf("iteration number %d\n",i + 1);
      printf("%8.8x ** ", start_addr );

      for( j = 0 ; j < nwords*2 ; j += 2 )
	{
	  if ((j%(0x20) == 0) && (j != 0)) 
	    {
	      printf("\n");
	      printf("%8.8x ** ", start_addr + j);
	    }
	  
	  full_addr = start_addr + j;
	  
	  res=sysBusToLocalAdrs (0x39,full_addr,&laddr);
	  
	  //printf(" addr=0x%x\n laddr=0x%x   res=%d\n",full_addr,laddr,res);
	  
	  x=(unsigned short*)laddr;
	  
	  xread=*x;
	  
	  printf("%4x ",xread);
	}
      printf("\n");
    }
 
 
  return(res);
  
}

////dump words indefinitely. diagnostic.
//int il_dmp_frvr(unsigned int addr,int nwords)
//
//{
//  unsigned short xread;
//  unsigned short *x;
//
//  unsigned int full_addr;
//  unsigned int laddr;
//  int j;
//  int i;
//  int res;
//  
//  //95 for v1495 logic and trig unit    E1 for V1742 digitizer
//  unsigned int start_addr = 0x00950000 + addr;
//  
//  printf("%8.8x ** ", start_addr );
//  
//do
//  {
//  for( j = 0 ; j < nwords*2 ; j += 2 )
//    {
//      if ((j%(0x20) == 0) && (j != 0)) 
//	{
//	  printf("\n");
//	  printf("%8.8x ** ", start_addr + j);
//	}
//      
//      full_addr = start_addr + j;
//      
//      res=sysBusToLocalAdrs (0x39,full_addr,&laddr);
//      
//      //printf(" addr=0x%x\n laddr=0x%x   res=%d\n",full_addr,laddr,res);
//      
//      x=(unsigned short*)laddr;
//      
//      xread=*x;
//      
//      printf("%4x ",xread);
//    }
//  printf("\n");
//  while (true)
// 
// 
//  return(res);
//  
//}



//find which addresses are in-use or MT
//MT stands for EMPTY
int bq_mt()
{
  unsigned int laddr;
  int i;
  int res;
  unsigned int *x;
  unsigned int xread;
  unsigned int full_addr;

 for(i=0x800;i<0x2000;i++)
   {
     full_addr = i*0x10000;
     res = sysBusToLocalAdrs (0x09,full_addr,&laddr);
     
     x = (unsigned int*)laddr;
     xread = *x;
     
     // printf("Add 0x%x -> 0x%x  Read 0x%x\n",full_addr,laddr,xread);
     
     if(xread != 0xffffffff)
       {
	 printf("Address in use:0x%x  ->0x%x   ",full_addr,x);
	 printf("Read value of 0x%x\n",xread);
	 //return(res);
       }
   }
 
 return(res);

}




//find which addresses are in-use or MT
int bq_mt24()
{
  unsigned int full_addr;
  unsigned int xread;	 
  unsigned int laddr;
  unsigned int *x;	 

  int i;
  int res;
  
  for(i=0x0 ; i<0x100 ; i++)
    {
      int good=0;
      int j;
      //      printf("%x\n",i);
      for(j=0x0 ; j<0x100 ; j++)
	{
	  full_addr = i*0x10000 + j*0x100;
	  res = sysBusToLocalAdrs (0x39,full_addr,&laddr);
	  x = (unsigned int*)laddr;
	  xread = *x;
	  if(xread != 0xffffffff)
	    {
	      good = 1;
	      //printf("i=0x%x j=0x%x\n",i,j);
	    }
	}
      // printf("Add 0x%x -> 0x%x  Read 0x%x\n",full_addr,laddr,xread);
      if(good == 1)
	{
	  full_addr = i*0x10000;
	  
	  res=sysBusToLocalAdrs (0x39,full_addr,&laddr);
	  
	  printf("Address in use:0x%x  ->0x%x   ",full_addr,x);
	  
	  x=(unsigned int*)laddr;
	  
	  xread=*x;
	  
	  printf("Read value of 0x%x\n",xread);
	  
	}
    }
  
  return(res);
  
}

//peek into user flash on v1495
int il_peek()
{
  unsigned int startingAddress = 0x00950000;  
  unsigned char page[PAGE_SIZE + 1];
  unsigned char c, c_rev;	   
  unsigned int address;               
  
  FILE *cf , *cf_rev , *ctxt ;
  char filename[100]         ;

  int i , j , k ,ipage, res  ;   
  int flag    = 0            ; /* flag=0 is for user FPGA */
  int maxPage = 2000         ; 
  int numff   = 0            ;

  res = sysBusToLocalAdrs( 0x39 , startingAddress , &address );

  //initialize filename, page
  filename[0] = '\0';
  page[0]     = '\0';
 
  //fill it with integers, get rid of junk
  for(i = 0 ; i < PAGE_SIZE ; i++)
    {
      page[j] = j;
      
      if(j == PAGE_SIZE - 1 ) 
	{
	  page[PAGE_SIZE] = '\0';
	}
    }
  
  // open the configuration files 
  strcpy(filename,"v1495_user_fpga.out");
  cf = fopen(filename,"wb");

  strcpy(filename , "v1495_user_fpga_reversed.out");
  cf_rev = fopen(filename,"wb");
  
  //check to make sure the config files are actually open
  if(cf == NULL || cf_rev == NULL)
    {
      printf("\n\nCan't open v1495 firmware file >%s< - exit\n",filename);
      return(0);
    }           

  //begin the reading process
  for( i =0 ; i < maxPage ; i++)
    {
      
      ipage = i + USR_FIRST_PAGE_STD;//the page 0 is not the first page, for some reason
                                     //so its shifted by this constant, USR_FIRST_PAGE_STD

      //fill in the page with meaningful v1495 data
      read_flash_page1( address , page, ipage , flag);

      //write page i normally to binary
      for( j = 0 ; j < PAGE_SIZE ; j++)
	{
	  c = page[j];
	  
	  if( c == 0xff)  numff++;
	  else            numff = 0;
	  
	  fputc(c,cf);
	}
            //write page i backwards to binary
      for( j = 0 ; j < PAGE_SIZE ; j++)
	{

	  c = page[j];
	  c_rev = 0;
	  
	  for( k = 0 ;  k < 8 ; k++)
	    {
	      if(c & ( 1 << k ) ) c_rev = c_rev | ( 0x80 >> k ) ;
	    }

	  fputc(c_rev,cf_rev);
	}

      //this kills the for loop after it sees a page if all ff's
      //it signifies that there is no more meaningful programming 
      //in the memory. In addition, it also closes the file on
      //a page boundary.
      if(numff >= PAGE_SIZE )  break;
      
    }
  
  printf("closing reversed file\n");
  fclose(cf_rev);
  printf("Done... now closing un-reversed file\n");
  fclose(cf);
  printf("closed file....  bye now\n");
  printf("Done\n");

  return(res);
  

}




//IT WORKS!
int kill_me()
{  
  int i, j ;
  int maxPage = 2000;
 
  unsigned char* integers;
  
  integers =  malloc((maxPage*PAGE_SIZE + 1)*sizeof(unsigned char));
  
  integers[0] = '\0';
  
  for(i = 0; i < maxPage ; i++)
    {
      printf("%d  ",i);

      if(i%20 == 0) printf("\n");

      for(j = 0; j < PAGE_SIZE; j++)
	{
	  integers[maxPage*i + j] = maxPage*i + j;
	  
	  if( ( i == ( maxPage - 1 ) ) && ( j == (PAGE_SIZE - 1) ) ) integers[maxPage*PAGE_SIZE] = '\0';
	}
    }

  free(integers);
  
  return(0);
}








/****************************************************************************
 write_flash_page
    flag=0 for USER flash (default)
        =1 for VME flash
****************************************************************************/
int write_flash_page1(unsigned int addr, unsigned char *page, int pagenum, int flag)
{
  volatile V1495 *v1495 = (V1495 *) addr;
  int i, flash_addr, data;
  unsigned char addr0, addr1, addr2;
  int res = 0;
  unsigned short *Sel_Flash; /* VME Address of the FLASH SELECTION REGISTER */
  unsigned short *RW_Flash;  /* VME Address of the FLASH Read/Write REGISTER */

  if(flag==1)
  {
    Sel_Flash = (short *)&(v1495->selflashVME);
    RW_Flash = (short *)&(v1495->flashVME);
  }
  else
  {
    Sel_Flash = (short *)&(v1495->selflashUSER);
    RW_Flash = (short *)&(v1495->flashUSER);
  }

  EIEIO;
  SYNC;
  flash_addr = pagenum << 9;
  addr0 = (unsigned char)flash_addr;
  addr1 = (unsigned char)(flash_addr>>8);
  addr2 = (unsigned char)(flash_addr>>16);

  EIEIO;
  SYNC;
  /* enable flash (NCS = 0) */
  data = 0;
  *Sel_Flash = data;

  EIEIO;
  SYNC;
  /* write opcode */
  //  printf("about to send opcode %x \n",MAIN_MEM_PAGE_PROG_TH_BUF1);
  data = MAIN_MEM_PAGE_PROG_TH_BUF1;
  *RW_Flash = data;

  EIEIO;
  SYNC;
  /* write address */
  //  printf("about to send addr2 %x \n",addr2);
  data = addr2;
  *RW_Flash = data;
  //printf("about to send addr1 %x \n",addr1);
  data = addr1;
  *RW_Flash = data;
  //printf("about to send addr0 %x \n",addr0);
  data = addr0;
  *RW_Flash = data;

  EIEIO;
  SYNC;
  /* write flash page */
  for(i=0; i<PAGE_SIZE; i++)
  {
    data = page[i];
    //printf("about to send data %x \n",data);
    *RW_Flash = data;
  }

  EIEIO;
  SYNC;
  /* wait 20ms (max time required by the flash to complete the writing) */
  taskDelay(5); /* 1 tick = 10ms */

  EIEIO;
  SYNC;
  /* disable flash (NCS = 1) */
  data = 1;
  *Sel_Flash = data;

  EIEIO;
  SYNC;
  /* wait 20ms (max time required by the flash to complete the writing) */
  taskDelay(5);
  EIEIO;
  SYNC;

  return(res);
}






/****************************************************************************
 read_flash_page
****************************************************************************/
int read_flash_page1(unsigned int addr, unsigned char *page, int pagenum, int flag)
{
  volatile V1495 *v1495 = (V1495 *) addr;
  int i, flash_addr, data;
  /*volatile*/ unsigned short data16;
  unsigned char addr0,addr1,addr2;
  int res = 0;
  unsigned short *Sel_Flash; /* VME Address of the FLASH SELECTION REGISTER */
  unsigned short *RW_Flash;  /* VME Address of the FLASH Read/Write REGISTER */

  

  if(flag==1)
  {
    Sel_Flash = (short *)&(v1495->selflashVME);
    RW_Flash  = (short *)&(v1495->flashVME);
  }
  else
  {
    Sel_Flash = (short *)&(v1495->selflashUSER);
    RW_Flash =  (short *)&(v1495->flashUSER);
  }

  EIEIO;
  SYNC;
  flash_addr = pagenum << 9;
  addr0 = (unsigned char)flash_addr;
  addr1 = (unsigned char)(flash_addr>>8);
  addr2 = (unsigned char)(flash_addr>>16);

  EIEIO;
  SYNC;
  /* enable flash (NCS = 0) */
  data = 0;
  *Sel_Flash = data;


  EIEIO;
  SYNC;
  /* write opcode */
  data = MAIN_MEM_PAGE_READ;
  *RW_Flash = data;



  EIEIO;
  SYNC;
  /* write address */
  data = addr2;
  *RW_Flash = data;
  data = addr1;
  *RW_Flash = data;
  data = addr0;
  *RW_Flash = data;

  //printf("read_page1 wrote address: %x %x %x\n",addr2,addr1,addr0);
  EIEIO;
  SYNC;
  /* additional don't care bytes */
  data = 0;
  for(i=0; i<4; i++)
  {
    *RW_Flash = data;
  }

  EIEIO;
  SYNC;
  /* read flash page */
  for(i=0; i<PAGE_SIZE; i++)
  {
    data16 = *RW_Flash;
    page[i] = (unsigned char)data16;
    
  }

  EIEIO;
  SYNC;

  /* disable flash (NCS = 1) */
  data = 1;
  *Sel_Flash = data;
  EIEIO;
  SYNC;
  
  
  return(res);
}


int v1495test1()
{
  volatile V1495 *v1495 = (V1495 *) 0xf0410000;
  unsigned short *data16 = (unsigned short *)&(v1495->control);

  printf("Control      [0x%08x] = 0x%04x\n",&(v1495->control),v1495->control);
  printf("firmwareRev  [0x%08x] = 0x%04x\n",&(v1495->firmwareRev),v1495->firmwareRev);
  printf("selflashVME  [0x%08x] = 0x%04x\n",&(v1495->selflashVME),v1495->selflashVME);
  printf("flashVME     [0x%08x] = 0x%04x\n",&(v1495->flashVME),v1495->flashVME);
  printf("selflashUSER [0x%08x] = 0x%04x\n",&(v1495->selflashUSER),v1495->selflashUSER);
  printf("flashUSER    [0x%08x] = 0x%04x\n",&(v1495->flashUSER),v1495->flashUSER);
  printf("configROM    [0x%08x] = 0x%04x\n",&(v1495->configROM[0]),v1495->configROM[0]);

  return(0);
}



int v1495reload(unsigned int addr)
{
  volatile V1495 *v1495 = (V1495 *) addr;

  unsigned short *Conf_Flash;
  
  Conf_Flash = (short *)&(v1495->configUSER);

  printf("Reloading user FPGA firmware...");

  EIEIO;
  SYNC;
  *Conf_Flash = 1;
  EIEIO;
  SYNC;

  printf("done!\n");

  return 0;
}






/*****************************************************************************
   MAIN

     baseaddr: full board address (for example 0x80510000)
     filename: RBF file name
     page: =0 for standard, =1 for backup
     user_vme: Firmware to update selector = 0 => USER, 1 => VME

*****************************************************************************/
int v1495firmware(unsigned int baseaddr, char *filename, int page, int user_vme)
{
  int finish,i;
  int bp, bcnt, pa;
  char c;
  unsigned char pdw[PAGE_SIZE], pdr[PAGE_SIZE];
  unsigned long vboard_base_address;
  FILE *cf;

  /*page = 0;     ONLY STD !!!!!!!!!!!!! */
  /*user_vme = 0; ONLY USER !!!!!!!!!!!! */

  printf("\n");
  printf("********************************************************\n");
  printf("* CAEN SpA - Front-End Division                        *\n");
  printf("* ---------------------------------------------------- *\n");
  printf("* Firmware Upgrade of the V1495                        *\n");
  printf("* Version 1.1 (27/07/06)                               *\n");
  printf("*   Sergey Boyarinov: CLAS version 23-Apr-2007         *\n");
  printf("********************************************************\n\n");

  /* open the configuration file */
  cf = fopen(filename,"rb");
  if(cf==NULL)
  {
    printf("\n\nCan't open v1495 firmware file >%s< - exit\n",filename);
    exit(0);
  }                                                                          

  if(user_vme == 0) /* FPGA "User" */
  {
    if(page == 0)
    {
      printf("Writing STD page of the USER FPGA\n");
      pa = USR_FIRST_PAGE_STD;
    }
    else if(page == 1)
    {
      // printf("Backup image not supported for USER FPGA\n");
      // exit(0);
        printf("Writing BCK page of the USER FPGA\n");
      pa = USR_FIRST_PAGE_BCK;
	}
    else
    {
      printf("Bad Image.\n");
	  exit(0);
    }

    printf("Updating firmware of the FPGA USER with the file %s\n",filename);
  }
  //
  //  else if(user_vme == 1) /* FPGA "VME_Interface" */
  //{
  //  if(page == 0)
  //    {
  //	printf("Writing STD page of the VME FPGA\n");
  //	pa = VME_FIRST_PAGE_STD;
  //    }
  //  else if(page == 1)
  //    {
  //	printf("Writing BCK page of the VME FPGA\n");
  //	pa = VME_FIRST_PAGE_BCK;
  //	}
  //  else
  //  {
  //    printf("Bad Image.\n");
  //    exit(0);
  //	}
  //
  //  printf("Updating firmware of the FPGA VME with the file %s\n", filename);
  //}
  //
  else
  {
    printf("Bad FPGA Target.\n");
	exit(0);
  }






  bcnt = 0; /* byte counter */
  bp = 0;   /* byte pointer in the page */
  finish = 0;

  /* while loop */
  while(!finish)
  {
    c = (unsigned char) fgetc(cf); /* read one byte from file */

    /* mirror byte (lsb becomes msb) */
    pdw[bp] = 0;
    for(i=0; i<8; i++)
    {
      if(c & (1<<i))
	  {
        pdw[bp] = pdw[bp] | (0x80>>i);
	  }
	}

    bp++;
    bcnt++;
    if(feof(cf))
    {
      printf("End of file: bp=%d bcnt=%d\n",bp,bcnt);
      finish = 1;
    }

    /* write and verify a page */
    if((bp == PAGE_SIZE) || finish)
    {
      write_flash_page1(baseaddr, pdw, pa, user_vme);
      if (pa%50 == 0) printf("Writing page %d\n",pa);
      //printf("not really writing page %d\n",pa);
      //      printf("about to read flash page \n");
      read_flash_page1(baseaddr, pdr, pa, user_vme);
      for(i=0; i<PAGE_SIZE; i++)
      {
	//	printf("check %d read: %x  wrote: %x\n",i,pdr[i],pdw[i]);
        if(pdr[i] != pdw[i])
        {
          printf("[%3d] written 0x%02x, read back 0x%02x",i,pdw[i],pdr[i]);
          printf(" -> Flash writing failure (byte %d of page %d)!\n",i,pa);
          printf("\nFirmware not loaded !\n");
 
	  printf("Did you remember to use the full LOCAL address?  eg 0x90950000\n");
         exit(0);
        }
      }
      bp=0;
      pa++;
    }
  } /* end of while loop */

  fclose(cf);
  printf("\nFirmware loaded successfully. Written %d bytes\n", bcnt);
//  printf("Write 1 at address 0x8016 to reload updated version of the User FPGA\n");

  printf("\n");
  v1495reload(baseaddr);

  exit(0);
}

int ld_file(char *filename)
{
  int res;
  res = v1495firmware(0x90950000,filename,0,0);
  return res;
}

int rst()
{
  il_tw(0x800a,1);
  exit(0);
}

unsigned short get_value(unsigned int addr)
{
  unsigned short xread;
  unsigned short *x;

  unsigned int full_addr;
  unsigned int laddr;
  int j;
  int res;
  
  full_addr = 0x00950000 + addr;
      
  res = sysBusToLocalAdrs (0x39,full_addr,&laddr);

  x=(unsigned short*)laddr;
  
  xread=*x;
     
  //printf("value is %x",xread);
  //printf("\n");
  //exit(0);
  return(xread);
}

//look_for_fuckups(0x1db000, 0x1dd000), 50ms pulse

//look_for_fuckups(0x141800,0x141b00), 33 ms pulse (too fast for the ROC to keep up with)

int look_for_fuckups(int lowThreshold,int highThreshold)
//clockTickThreshold is the maximum number of clock ticks we expect between times put in the FIFO
{

  //address 0x105c is last_event_time_FIFO(31 downto 16), most significant bits
  //address 0x105e is last_event_time_FIFO(15 downto 0), least significant bits
  
  unsigned long firstTime = 0;
  unsigned long secondTime = 0;
  unsigned int realDifference = 0;
  signed int difference = 0;
  int count = 0;
   
  if (highThreshold == 0)
    {
      highThreshold = lowThreshold + 0x100;
    }

  //wait until we get a valid number, all F's means there's nothing in the FIFO yet, and thus not a valid time
  do
    {
      firstTime = (get_value(0x105c))*0x10000 ;
      firstTime += get_value(0x105e);      
    }
  while(firstTime == 0xFFFFFFFF);

  do
    {   
      secondTime = (get_value(0x105c))*0x10000 ;
      secondTime += get_value(0x105e);
    }
  while(secondTime == 0xFFFFFFFF || secondTime == firstTime);

    
  //infinite loop will break if a screw up is found
  do
    {
      count = count + 1;
      difference = ((signed int)secondTime) - ((signed int)firstTime);

      if (count == 1000)
      {
	  printf("\r f= %8x s=%8x  d=%8x",firstTime,secondTime,difference);
	  count = 0;
	  }
      
      
      //if (difference > highThreshold || difference < lowThreshold) break;
      if (difference > highThreshold )
      	{
      	  printf("\nfirstTime = %x \n",firstTime);			  
      	  printf("secondTime = %x \n",secondTime);			  
      	  printf("highThreshold violated: difference = %x but highThreshold = %x \n",difference, highThreshold);
      	}
      if (difference < lowThreshold ) 
      	{
      	  printf("\nfirstTime = %x \n",firstTime);			  
      	  printf("secondTime = %x \n",secondTime);			  
      	  printf("lowThreshold violated: difference = %x but lowThreshold = %x \n", difference, lowThreshold);
      	}

      //if we haven't broken yet, get some new numbers
      firstTime = secondTime;
      
      do
	{
	  //printf("here7\n");
	  secondTime = get_value(0x105c)*0x10000 ;
	  secondTime+= get_value(0x105e);
	}
      while (secondTime == 0xFFFFFFFF || secondTime == firstTime);
      
      //printf("here8\n");
      
    }
  while (1 > 0);
    
  //if we're out of this loop, we have an error
  
  printf("\nWe have an error!!!!\n");
  printf("firstTime  = %x\n" ,firstTime);
  printf("secondTime = %x\n",secondTime);
  printf("difference = %x\n", difference);
  printf("\n");
  
  exit(0);
}

int unsignedcheck()
{
  unsigned int num1;
  unsigned int num2;

  num1 = 10;
  num2 = 20;
  printf("\n");
  printf("num2 - num1 = %x", num2 - num1); //10
  printf("\n");
  printf("num1 - num2 = %x", num1 - num2); //?
  printf("\n");
  printf("signed, num1 - num2 = %x", (signed long int)(num1 - num2)); //should be -10
  printf("\n");
  exit(0);
}

int csucks()
{
  unsigned int big;
  unsigned int small;
  
  big = 0xFFFFFFF0;
  small = 0x1;
  if (big < small) printf("csucks");
  else
    {
      printf("\n");
      printf("c's alright");
    }

  exit(0);
}



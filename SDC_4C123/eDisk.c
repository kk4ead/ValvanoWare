// eDisk.c
// Runs on LM4F120/TM4C123
// Use SSI0, PD7 to communicate with the SDC.
// Valvano
// March 17, 2014

/* This example accompanies the books
   Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers, Volume 3,  
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2014

   Volume 3, Program 6.3, section 6.6   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) connected to PA4 (SSI0Rx)
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss) <- GPIO high to disable TFT
// CARD_CS (pin 5) connected to PD7 GPIO output 
// Data/Command (pin 4) connected to PA6 (GPIO)<- GPIO low not using TFT
// RESET (pin 3) connected to PA7 (GPIO)<- GPIO high to disable TFT
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "edisk.h"

// CS is PD7  
// to change CS to another GPIO, change SDC_CS and CS_Init
#define SDC_CS   (*((volatile unsigned long *)0x40007200))  
void CS_Init(void){ unsigned long volatile delay;
  SYSCTL_RCGCGPIO_R |= 0x08; // activate port D
  delay = SYSCTL_RCGCGPIO_R;
  delay = SYSCTL_RCGCGPIO_R;
  GPIO_PORTD_LOCK_R = 0x4C4F434B;   // 2) unlock PortD PD7  
  GPIO_PORTD_CR_R |= 0xFF;          // allow changes to PD7-0       
  GPIO_PORTD_PUR_R |= 0x80;         // enable weak pullup on PD7
  GPIO_PORTD_DIR_R |= 0x80;         // make PD7 output 
  GPIO_PORTD_DR4R_R |= 0x80;        // 4mA output on outputs
  SDC_CS = 0xFF;
  GPIO_PORTD_PCTL_R &= ~0xF0000000;
  GPIO_PORTD_AMSEL_R &= ~0x80; // disable analog functionality on PD7
  GPIO_PORTD_DEN_R |= 0x80;    // enable digital I/O on PD7
}
//********SSI0_Init*****************
// Initialize SSI0 interface to SDC
// inputs:  clock divider to set clock frequency
// outputs: none
// assumes: system clock rate is 80 MHz, SCR=0
// SSIClk = SysClk / (CPSDVSR * (1 + SCR)) = 80 MHz/CPSDVSR
// 200 for    400,000 bps slow mode, used during initialization
// 8   for 10,000,000 bps fast mode, used during disk I/O
void Timer5_Init(void);
void SSI0_Init(unsigned long CPSDVSR){
  SYSCTL_RCGCGPIO_R |= 0x01;   // activate port A
  SYSCTL_RCGCSSI_R |= 0x01;    // activate SSI0
  CS_Init();                            // activate CS, make it high (deselect)
  Timer5_Init();
  GPIO_PORTA_AFSEL_R |= 0x3C;           // enable alt funct on PA2,4,5
  GPIO_PORTA_PUR_R |= 0x3C;             // enable weak pullup on PA2,3,4,5
  GPIO_PORTA_DEN_R |= 0x3C;             // enable digital I/O on PA2,3,4,5
                                        // configure PA2,3,4, 5 as SSI
  GPIO_PORTA_DIR_R |= 0xC8;             // PA7,PA6,PA3 output (CS to LCD)
  GPIO_PORTA_DATA_R |= 0x88;            // PA7,PA3 high (disable LCD)
  
  GPIO_PORTA_DR4R_R |= 0xEC;            // 4mA output on outputs
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFF0000FF)+0x00222200;
  GPIO_PORTA_AMSEL_R = 0;               // disable analog functionality on PA
  SSI0_CR1_R &= ~SSI_CR1_SSE;           // disable SSI
  SSI0_CR1_R &= ~SSI_CR1_MS;            // master mode
                                        // clock divider for 8 MHz SSIClk (assumes 16 MHz PIOSC)
  SSI0_CPSR_R = (SSI0_CPSR_R&~SSI_CPSR_CPSDVSR_M)+CPSDVSR; 
  // CPSDVSR must be even from 2 to 254
  
  SSI0_CR0_R &= ~(SSI_CR0_SCR_M |       // SCR = 0 (80 Mbps base clock)
                  SSI_CR0_SPH |         // SPH = 0
                  SSI_CR0_SPO);         // SPO = 0
                                        // FRF = Freescale format
  SSI0_CR0_R = (SSI0_CR0_R&~SSI_CR0_FRF_M)+SSI_CR0_FRF_MOTO;
                                        // DSS = 8-bit data
  SSI0_CR0_R = (SSI0_CR0_R&~SSI_CR0_DSS_M)+SSI_CR0_DSS_8;
  SSI0_CR1_R |= SSI_CR1_SSE;            // enable SSI
}
void MakeTxhigh(void){
  GPIO_PORTA_AFSEL_R &= ~0x20;           // disable alt funct on PA5
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFF0FFFFF);
  GPIO_PORTA_DATA_R |= 0x20;            // PA5 high 
}
void MakeTxSSI(void){
  GPIO_PORTA_AFSEL_R |= 0x20;           // enable alt funct on PA5
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFF0FFFFF)+0x00200000;
}
//********SSI0_Out*****************
// Send data to SDC
// inputs:  data out to TxD
// outputs: data in from RxD
unsigned short SSI0_Out(unsigned short code){
  while((SSI0_SR_R&SSI_SR_TNF)==0){};// wait until room in FIFO
  SSI0_DR_R = code;                  // data out
  while((SSI0_SR_R&SSI_SR_RNE)==0){};// wait until response
  return SSI0_DR_R;                  // acknowledge response
}
/*-----------------------------------------------------------------------*/
/* MMC/SDC (in SPI mode) control module  (C)ChaN, 2007                   */
/*-----------------------------------------------------------------------*/
/* Only rcvr_spi(), xmit_spi(), disk_timerproc() and some macros         */
/* are platform dependent.                                               */
/*-----------------------------------------------------------------------*/

/*
 * This file was modified from a sample available from the FatFs
 * web site. It was modified to work with a Tiva TM4C123 LaunchPad
 * evaluation board.
 */



/* Definitions for MMC/SDC command */
#define CMD0    (0x40+0)    /* GO_IDLE_STATE */
#define CMD1    (0x40+1)    /* SEND_OP_COND */
#define CMD8    (0x40+8)    /* SEND_IF_COND */
#define CMD9    (0x40+9)    /* SEND_CSD */
#define CMD10    (0x40+10)    /* SEND_CID */
#define CMD12    (0x40+12)    /* STOP_TRANSMISSION */
#define CMD16    (0x40+16)    /* SET_BLOCKLEN */
#define CMD17    (0x40+17)    /* READ_SINGLE_BLOCK */
#define CMD18    (0x40+18)    /* READ_MULTIPLE_BLOCK */
#define CMD23    (0x40+23)    /* SET_BLOCK_COUNT */
#define CMD24    (0x40+24)    /* WRITE_BLOCK */
#define CMD25    (0x40+25)    /* WRITE_MULTIPLE_BLOCK */
#define CMD41    (0x40+41)    /* SEND_OP_COND (ACMD) */
#define CMD55    (0x40+55)    /* APP_CMD */
#define CMD58    (0x40+58)    /* READ_OCR */

// asserts the CS pin to the card
static void SELECT(void){
  SDC_CS = 0;
}

// de-asserts the CS pin to the card
static void DESELECT(void){
  SDC_CS = 0xFF;
}


/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

static volatile DSTATUS Stat = STA_NOINIT;    /* Disk status */

static volatile BYTE Timer1, Timer2;    /* 100Hz decrement timer */

static BYTE CardType;            /* b0:MMC, b1:SDC, b2:Block addressing */

static BYTE PowerFlag = 0;     /* indicates if "power" is on */

/*-----------------------------------------------------------------------*/
/* Transmit a byte to MMC via SPI  (Platform dependent)                  */
/*-----------------------------------------------------------------------*/
static void xmit_spi(BYTE dat){DWORD volatile rcvdat;
  while((SSI0_SR_R&SSI_SR_TNF)==0){};    // wait until room in FIFO
  SSI0_DR_R = dat;                       // data out
  while((SSI0_SR_R&SSI_SR_RNE)==0){};    // wait until response
  rcvdat = SSI0_DR_R;                    // acknowledge response
}


/*-----------------------------------------------------------------------*/
/* Receive a byte from MMC via SPI  (Platform dependent)                 */
/*-----------------------------------------------------------------------*/
static BYTE rcvr_spi(void){ 
  while((SSI0_SR_R&SSI_SR_TNF)==0){};    // wait until room in FIFO
  SSI0_DR_R = 0xFF;                      // data out, garbage
  while((SSI0_SR_R&SSI_SR_RNE)==0){};    // wait until response
  return (BYTE)SSI0_DR_R;                // read received data
}

static void rcvr_spi_m(BYTE *dst){
  *dst = rcvr_spi();
}

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/
static BYTE wait_ready(void){
  BYTE res;
  Timer2 = 50;    /* Wait for ready in timeout of 500ms */
  rcvr_spi();
  do
    res = rcvr_spi();
  while ((res != 0xFF) && Timer2);
  return res;
}


/*-----------------------------------------------------------------------*/
/* Send 80 or so clock transitions with CS and DI held high. This is     */
/* required after card power up to get it into SPI mode                  */
/*-----------------------------------------------------------------------*/
static void send_initial_clock_train(void){unsigned int i;
  DESELECT();/* Ensure CS is held high. */
/* Switch the SSI TX line to a GPIO and drive it high too. */
  MakeTxhigh();
/* Send 10 bytes over the SSI. This causes the clock to wiggle the */
/* required number of times. */
  for(i = 0 ; i < 10 ; i++)    {
    SSI0_Out(0xFF); // write dummy data, flush return data
  }
/* Revert to hardware control of the SSI TX line. */
  MakeTxSSI();
}


/*-----------------------------------------------------------------------*/
/* Power Control  (Platform dependent)                                   */
/*-----------------------------------------------------------------------*/
/* When the target system does not support socket power control, there   */
/* is nothing to do in these functions and chk_power always returns 1.   */

static void power_on(void){
/*
 * This doesnt really turn the power on, but initializes the
 * SSI port and pins needed to talk to the card.
 */
  SSI0_Init(200); // 400,000 bps, slow mode
/* Set DI and CS high and apply more than 74 pulses to SCLK for the card */
/* to be able to accept a native command. */
  send_initial_clock_train();
  PowerFlag = 1;
}

// set the SSI speed to the max setting
static void set_max_speed(void){
  SSI0_Init(8); // 10,000,000 bps (could run at 12 MHz)
}

static void power_off (void){
  PowerFlag = 0;
}

static int chk_power(void){        /* Socket power state: 0=off, 1=on */
  return PowerFlag;
}



/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/

static BOOL rcvr_datablock (
    BYTE *buff,         /* Data buffer to store received data */
    UINT btr){          /* Byte count (must be even number) */
  BYTE token;
  Timer1 = 10;
  do {                            /* Wait for data packet in timeout of 100ms */
    token = rcvr_spi();
  } while ((token == 0xFF) && Timer1);
  if(token != 0xFE) return FALSE;    /* If not valid data token, retutn with error */

  do {                            /* Receive the data block into buffer */
    rcvr_spi_m(buff++);
    rcvr_spi_m(buff++);
  } while (btr -= 2);
  rcvr_spi();                        /* Discard CRC */
  rcvr_spi();

  return TRUE;                    /* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
static BOOL xmit_datablock(
    const BYTE *buff,    /* 512 byte data block to be transmitted */
    BYTE token){          /* Data/Stop token */
  BYTE resp, wc;


  if(wait_ready() != 0xFF) return FALSE;

  xmit_spi(token);                    /* Xmit data token */
  if(token != 0xFD) {    /* Is data token */
    wc = 0;
    do{                            /* Xmit the 512 byte data block to MMC */
      xmit_spi(*buff++);
      xmit_spi(*buff++);
    } while (--wc);
    xmit_spi(0xFF);                    /* CRC (Dummy) */
    xmit_spi(0xFF);
    resp = rcvr_spi();                /* Reveive data response */
    if((resp & 0x1F) != 0x05)        /* If not accepted, return with error */
      return FALSE;
  }

  return TRUE;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static BYTE send_cmd (
    BYTE cmd,        /* Command byte */
    DWORD arg){      /* Argument */

  BYTE n, res;


  if (wait_ready() != 0xFF) return 0xFF;

    /* Send command packet */
  xmit_spi(cmd);                        /* Command */
  xmit_spi((BYTE)(arg >> 24));        /* Argument[31..24] */
  xmit_spi((BYTE)(arg >> 16));        /* Argument[23..16] */
  xmit_spi((BYTE)(arg >> 8));            /* Argument[15..8] */
  xmit_spi((BYTE)arg);                /* Argument[7..0] */
  n = 0;
  if (cmd == CMD0) n = 0x95;            /* CRC for CMD0(0) */
  if (cmd == CMD8) n = 0x87;            /* CRC for CMD8(0x1AA) */
  xmit_spi(n);

    /* Receive command response */
  if (cmd == CMD12) rcvr_spi();        /* Skip a stuff byte when stop reading */
  n = 10;                                /* Wait for a valid response in timeout of 10 attempts */
  do
    res = rcvr_spi();
  while ((res & 0x80) && --n);

  return res;            /* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/
// since this program initializes the disk, it must run with 
//    the disk periodic task operating
DSTATUS eDisk_Init(BYTE drv){        /* Physical drive nmuber (0) */

  BYTE n, ty, ocr[4];


  if (drv) return STA_NOINIT;            /* Supports only single drive */
  if (Stat & STA_NODISK) return Stat;    /* No card in the socket */

  power_on();                            /* Force socket power on */
  send_initial_clock_train();

  SELECT();                /* CS = L */
  ty = 0;
  if(send_cmd(CMD0, 0) == 1){            /* Enter Idle state */
    Timer1 = 100;                        /* Initialization timeout of 1000 msec */
    if(send_cmd(CMD8, 0x1AA) == 1){    /* SDC Ver2+ */
      for(n = 0; n < 4; n++) ocr[n] = rcvr_spi();
      if(ocr[2] == 0x01 && ocr[3] == 0xAA) {    /* The card can work at vdd range of 2.7-3.6V */
        do{
          if (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 1UL << 30) == 0)    break;    /* ACMD41 with HCS bit */
        } while (Timer1);
        if(Timer1 && send_cmd(CMD58, 0) == 0) {    /* Check CCS bit */
          for(n = 0; n < 4; n++) ocr[n] = rcvr_spi();
          ty = (ocr[0] & 0x40) ? 6 : 2;
        }
      }
    } else {                            /* SDC Ver1 or MMC */
      ty = (send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 0) <= 1) ? 2 : 1;    /* SDC : MMC */
      do{
        if(ty == 2){
          if(send_cmd(CMD55, 0) <= 1 && send_cmd(CMD41, 0) == 0) break;    /* ACMD41 */
        } else {
          if (send_cmd(CMD1, 0) == 0) break;                                /* CMD1 */
        }
      } while (Timer1);
      if(!Timer1 || send_cmd(CMD16, 512) != 0)    /* Select R/W block length */
        ty = 0;
    }
  }
  CardType = ty;
  DESELECT();            /* CS = H */
  rcvr_spi();            /* Idle (Release DO) */

  if(ty){            /* Initialization succeded */
    Stat &= ~STA_NOINIT;        /* Clear STA_NOINIT */
    set_max_speed();
  } else {            /* Initialization failed */
    power_off();
  }

  return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS eDisk_Status (BYTE drv){ /* Physical drive nmuber (0) */
  if (drv) return STA_NOINIT;        /* Supports only single drive */
  return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT eDisk_Read (
    BYTE drv,            /* Physical drive nmuber (0) */
    BYTE *buff,          /* Pointer to the data buffer to store read data */
    DWORD sector,        /* Start sector number (LBA) */
    BYTE count){         /* Sector count (1..255) */

  if (drv || !count) return RES_PARERR;
  if (Stat & STA_NOINIT) return RES_NOTRDY;

  if (!(CardType & 4)) sector *= 512;    /* Convert to byte address if needed */

  SELECT();            /* CS = L */

  if (count == 1) {    /* Single block read */
    if ((send_cmd(CMD17, sector) == 0)    /* READ_SINGLE_BLOCK */
            && rcvr_datablock(buff, 512))
      count = 0;
  }
  else {                /* Multiple block read */
    if (send_cmd(CMD18, sector) == 0) {    /* READ_MULTIPLE_BLOCK */
      do {
        if (!rcvr_datablock(buff, 512)) break;
        buff += 512;
      } while (--count);
      send_cmd(CMD12, 0);                /* STOP_TRANSMISSION */
    }
  }

  DESELECT();            /* CS = H */
  rcvr_spi();            /* Idle (Release DO) */

  return count ? RES_ERROR : RES_OK;
}


//*************** eDisk_ReadBlock ***********
// Read 1 block of 512 bytes from the SD card  (write to RAM)
// Inputs: pointer to an empty RAM buffer
//         sector number of SD card to read: 0,1,2,...
// Outputs: result
//  RES_OK        0: Successful 
//  RES_ERROR     1: R/W Error 
//  RES_WRPRT     2: Write Protected 
//  RES_NOTRDY    3: Not Ready 
//  RES_PARERR    4: Invalid Parameter 
DRESULT eDisk_ReadBlock(
    BYTE *buff,         /* Pointer to the data buffer to store read data */
    DWORD sector){      /* Start sector number (LBA) */
  return eDisk_Read(0,buff,sector,1);
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT eDisk_Write (
    BYTE drv,            /* Physical drive nmuber (0) */
    const BYTE *buff,    /* Pointer to the data to be written */
    DWORD sector,        /* Start sector number (LBA) */
    BYTE count){         /* Sector count (1..255) */

  if (drv || !count) return RES_PARERR;
  if (Stat & STA_NOINIT) return RES_NOTRDY;
  if (Stat & STA_PROTECT) return RES_WRPRT;

  if (!(CardType & 4)) sector *= 512;    /* Convert to byte address if needed */

  SELECT();            /* CS = L */

  if (count == 1) {    /* Single block write */
    if ((send_cmd(CMD24, sector) == 0)    /* WRITE_BLOCK */
          && xmit_datablock(buff, 0xFE))
      count = 0;
  }
  else {                /* Multiple block write */
    if (CardType & 2) {
      send_cmd(CMD55, 0); send_cmd(CMD23, count);    /* ACMD23 */
    }
    if (send_cmd(CMD25, sector) == 0) {    /* WRITE_MULTIPLE_BLOCK */
      do {
        if (!xmit_datablock(buff, 0xFC)) break;
        buff += 512;
      } while (--count);
      if (!xmit_datablock(0, 0xFD))    /* STOP_TRAN token */
        count = 1;
    }
  }

  DESELECT();            /* CS = H */
  rcvr_spi();            /* Idle (Release DO) */

  return count ? RES_ERROR : RES_OK;
}


//*************** eDisk_WriteBlock ***********
// Write 1 block of 512 bytes of data to the SD card
// Inputs: pointer to RAM buffer with information
//         sector number of SD card to write: 0,1,2,...
// Outputs: result
//  RES_OK        0: Successful 
//  RES_ERROR     1: R/W Error 
//  RES_WRPRT     2: Write Protected 
//  RES_NOTRDY    3: Not Ready 
//  RES_PARERR    4: Invalid Parameter 
DRESULT eDisk_WriteBlock (
    const BYTE *buff,   /* Pointer to the data to be written */
    DWORD sector){      /* Start sector number (LBA) */
  return eDisk_Write(0,buff,sector,1);  // 1 block
}

#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE drv,        /* Physical drive nmuber (0) */
    BYTE ctrl,       /* Control code */
    void *buff){     /* Buffer to send/receive control data */

  DRESULT res;
  BYTE n, csd[16], *ptr = buff;
  WORD csize;


  if (drv) return RES_PARERR;

  res = RES_ERROR;

  if (ctrl == CTRL_POWER) {
    switch (*ptr) {
      case 0:        /* Sub control code == 0 (POWER_OFF) */
        if (chk_power())
          power_off();        /* Power off */
        res = RES_OK;
        break;
      case 1:        /* Sub control code == 1 (POWER_ON) */
        power_on();                /* Power on */
        res = RES_OK;
        break;
      case 2:        /* Sub control code == 2 (POWER_GET) */
        *(ptr+1) = (BYTE)chk_power();
        res = RES_OK;
        break;
      default :
        res = RES_PARERR;
    }
  }
  else {
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    SELECT();        /* CS = L */

    switch (ctrl) {
      case GET_SECTOR_COUNT :    /* Get number of sectors on the disk (DWORD) */
        if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
          if ((csd[0] >> 6) == 1) {    /* SDC ver 2.00 */
            csize = csd[9] + ((WORD)csd[8] << 8) + 1;
            *(DWORD*)buff = (DWORD)csize << 10;
          } else {                    /* MMC or SDC ver 1.XX */
            n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
            csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
            *(DWORD*)buff = (DWORD)csize << (n - 9);
          }
          res = RES_OK;
        }
        break;

      case GET_SECTOR_SIZE :    /* Get sectors on the disk (WORD) */
        *(WORD*)buff = 512;
        res = RES_OK;
        break;

      case CTRL_SYNC :    /* Make sure that data has been written */
        if (wait_ready() == 0xFF)
          res = RES_OK;
        break;

      case MMC_GET_CSD :    /* Receive CSD as a data block (16 bytes) */
        if (send_cmd(CMD9, 0) == 0        /* READ_CSD */
              && rcvr_datablock(ptr, 16))
          res = RES_OK;
        break;

      case MMC_GET_CID :    /* Receive CID as a data block (16 bytes) */
        if (send_cmd(CMD10, 0) == 0        /* READ_CID */
              && rcvr_datablock(ptr, 16))
          res = RES_OK;
        break;

      case MMC_GET_OCR :    /* Receive OCR as an R3 resp (4 bytes) */
        if (send_cmd(CMD58, 0) == 0) {    /* READ_OCR */
          for (n = 0; n < 4; n++)
            *ptr++ = rcvr_spi();
          res = RES_OK;
        }

//        case MMC_GET_TYPE :    /* Get card type flags (1 byte) */
//            *ptr = CardType;
//            res = RES_OK;
//            break;

      default:
        res = RES_PARERR;
      }

    DESELECT();            /* CS = H */
    rcvr_spi();            /* Idle (Release DO) */
  }

  return res;
}



/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms                        */

void disk_timerproc(void){   /* 100Hz decrement timer */
  if(Timer1) Timer1--;
  if(Timer2) Timer2--;
}

/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support a real time clock.          */

DWORD get_fattime (void){

    return    ((2014UL-1980) << 25) // Year = 2014
            | (3UL << 21)           // Month = March
            | (17UL << 16)          // Day = 17
            | (11U << 11)           // Hour = 11
            | (38U << 5)            // Min = 38
            | (0U >> 1)             // Sec = 0
            ;

}
void Timer5_Init(void){volatile unsigned short delay;
  SYSCTL_RCGCTIMER_R |= 0x20;
  delay = SYSCTL_SCGCTIMER_R;
  delay = SYSCTL_SCGCTIMER_R;
  TIMER5_CTL_R = 0x00000000;       // 1) disable timer5A during setup
  TIMER5_CFG_R = 0x00000000;       // 2) configure for 32-bit mode
  TIMER5_TAMR_R = 0x00000002;      // 3) configure for periodic mode, default down-count settings
  TIMER5_TAILR_R = 799999;         // 4) reload value, 10 ms, 80 MHz clock
  TIMER5_TAPR_R = 0;               // 5) bus clock resolution
  TIMER5_ICR_R = 0x00000001;       // 6) clear timer5A timeout flag
  TIMER5_IMR_R = 0x00000001;       // 7) arm timeout interrupt
  NVIC_PRI23_R = (NVIC_PRI23_R&0xFFFFFF00)|0x00000040; // 8) priority 2
// interrupts enabled in the main program after all devices initialized
// vector number 108, interrupt number 92
  NVIC_EN2_R = 0x10000000;         // 9) enable interrupt 92 in NVIC
  TIMER5_CTL_R = 0x00000001;       // 10) enable timer5A
}
// Executed every 10 ms
void Timer5A_Handler(void){ 
  TIMER5_ICR_R = 0x00000001;       // acknowledge timer5A timeout
  disk_timerproc();
}


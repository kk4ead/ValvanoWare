/*-----------------------------------------------------------------------
/  Low level disk interface modlue include file  R0.04a   (C)ChaN, 2007
/-----------------------------------------------------------------------
 * Modified by Jonathan Valvano to simplify usage in Lab 5
 * March 17, 2014
 */
#ifndef _DISKIO

#define _READONLY	0	/* 1: Read-only mode */

typedef signed int		INT;
typedef unsigned int	UINT;

/* These types are assumed as 8-bit integer */
typedef signed char		CHAR;
typedef unsigned char	UCHAR;
typedef unsigned char	BYTE;

/* These types are assumed as 16-bit integer */
typedef signed short	SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;

/* These types are assumed as 32-bit integer */
typedef signed long		LONG;
typedef unsigned long	ULONG;
typedef unsigned long	DWORD;

/* Boolean type */
typedef enum { FALSE = 0, TRUE } BOOL;

/* Status of Disk Functions */
typedef BYTE	DSTATUS;

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;


/*---------------------------------------*/
/* Prototypes for disk control functions */

//*************** eDisk_Init ***********
// Initialize the interface between microcontroller and the SD card
// Inputs: drive number (only drive 0 is supported)
// Outputs: status
//  STA_NOINIT   0x01   Drive not initialized
//  STA_NODISK   0x02   No medium in the drive
//  STA_PROTECT  0x04   Write protected
// since this program initializes the disk, it must run with 
//    the disk periodic task operating
DSTATUS eDisk_Init(BYTE drive);
//*************** eDisk_Status ***********
// Check the status of the SD card
// Inputs: drive number (only drive 0 is supported)
// Outputs: status
//  STA_NOINIT   0x01   Drive not initialized
//  STA_NODISK   0x02   No medium in the drive
//  STA_PROTECT  0x04   Write protected
DSTATUS eDisk_Status (BYTE drive);


//*************** eDisk_Read ***********
// Read data from the SD card  (write to RAM)
// Inputs: drive number (only drive 0 is supported)
//         pointer to an empty RAM buffer
//         sector number of SD card to read: 0,1,2,...
//         number of sectors to read
// Outputs: result
//  RES_OK        0: Successful 
//  RES_ERROR     1: R/W Error 
//  RES_WRPRT     2: Write Protected 
//  RES_NOTRDY    3: Not Ready 
//  RES_PARERR    4: Invalid Parameter 
DRESULT eDisk_Read (
  BYTE drv,     // Physical drive number (0)
  BYTE *buff,   // Pointer to buffer to read data
  DWORD sector, // Start sector number (LBA)
  BYTE count);  // Sector count (1..255)

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
DRESULT eDisk_ReadBlock (
    BYTE *buff,         /* Pointer to the data buffer to store read data */
    DWORD sector);      /* Start sector number (LBA) */

#if	_READONLY == 0

//*************** eDisk_Write ***********
// (read from RAM) Write data to the SD card
// Inputs: drive number (only drive 0 is supported)
//         pointer to RAM buffer with information
//         sector number of SD card to write: 0,1,2,...
//         number of sectors to read
// Outputs: result
//  RES_OK        0: Successful 
//  RES_ERROR     1: R/W Error 
//  RES_WRPRT     2: Write Protected 
//  RES_NOTRDY    3: Not Ready 
//  RES_PARERR    4: Invalid Parameter 
DRESULT eDisk_Write (
  BYTE drv,         // Physical drive number (0)
  const BYTE *buff, // Pointer to the data to be written
  DWORD sector,     // Start sector number (LBA)
  BYTE count);      // Sector count (1..255)

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
    DWORD sector);      /* Start sector number (LBA) */


#endif
DRESULT disk_ioctl (BYTE, BYTE, void*);

/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */


/* Command code for disk_ioctrl() */

#define GET_SECTOR_COUNT	1
#define GET_SECTOR_SIZE		2
#define CTRL_SYNC			3
#define CTRL_POWER			4
#define CTRL_LOCK			5
#define CTRL_EJECT			6
#define MMC_GET_CSD			10
#define MMC_GET_CID			11
#define MMC_GET_OCR			12
#define ATA_GET_REV			20
#define ATA_GET_MODEL		21
#define ATA_GET_SN			22


#define _DISKIO
#endif

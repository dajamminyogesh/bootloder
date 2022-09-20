#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H
#include "stm32f4xx_hal.h"
#include "usb_host.h"
#include "fatfs.h"
#include "usart.h"

#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define RAM_BUFFER_SIZE               ((uint32_t)64*1024)       /*KBytes*/
#define filename "app.bin"
#define filenamebatch "firmwarE.bin"
#define filenamenew "xuqingyu.bin"
#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_4                 //用户代码起始地址ַ
#define FLASH_USER_END_ADDR      ((uint32_t)0x0807ffff)             //用户代码结束地址

extern ApplicationTypeDef Appli_state; //定义ApplicationTypeDef枚举  Appli_state申请状态ַ
uint32_t FLASH_Erase_Write(void);
static uint32_t GetSector(uint32_t Address);
extern uint32_t bootloaderTimeout;
void jumpToApp(void);
void UP_Data(void);

#endif 


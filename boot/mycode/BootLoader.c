#include "BootLoader.h"                                       //编译之后大约占flash23k״̬
#include "usb_host.h"

extern USBH_HandleTypeDef hUsbHostFS;
FRESULT res;                                                  //FRESULT 枚举 表示对文件系统 执行之后返回的状态ص�״̬
static FLASH_EraseInitTypeDef EraseInitStruct;                //flash扇区结构体
uint8_t RAM_Buffer[RAM_BUFFER_SIZE];                          //读取bin文件缓存区
uint32_t APP_Size;                                            //用户程序字长  读取的字节
uint32_t Address = 0;                                         //分配用户程序起始地址ַ
uint32_t SectorError = 0;
__IO uint32_t data32 = 0, MemoryProgramStatus = 0;            //MemoryProgramStatus 内存程序状态
uint8_t errorcode;                                            //擦除失败的扇区
uint32_t *p;
uint8_t lseekFlag = 0;                                        //读指针偏移几次标志
uint8_t SystemUpdateFlag = 0;                                 // SystemUpdateFlag系统更新标志־
uint16_t t = 0;
uint8_t batchFlag = 1;
typedef void (*pFunction)(void);
pFunction Jump_To_Application;                                //函数指针
uint32_t JumpAddress;                                         //跳转地址   为用户起始地址加4
uint32_t bootloaderTimeout = 0;
uint32_t FLASH_Erase_Write(void)		                      //FLASH擦写
{
	uint32_t i = 0;
	HAL_FLASH_Unlock();		                                  //解锁
	if (lseekFlag == 1) {
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;               //擦除类型，扇区擦除
		EraseInitStruct.Sector = FLASH_SECTOR_4;                           //要擦除的扇区
		EraseInitStruct.NbSectors = 4;                                     //一次只擦除4个扇区
		EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;              //电压
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)   //擦除所定义的结构体扇区
				{
			errorcode = HAL_FLASH_GetError();		                       //擦除失败的扇区
			printf("errorcode %d", errorcode);
			Error_Handler();
		}
	}
	__HAL_FLASH_DATA_CACHE_DISABLE();                                 //闪存数据缓存禁用
	__HAL_FLASH_INSTRUCTION_CACHE_DISABLE();                          //闪存指令高速缓存禁用
	__HAL_FLASH_DATA_CACHE_RESET();                                   //闪存数据缓存重置
	__HAL_FLASH_INSTRUCTION_CACHE_RESET();                            //闪存指令高速缓存重置
	__HAL_FLASH_INSTRUCTION_CACHE_ENABLE();                           //闪存指令高速缓存启用
	__HAL_FLASH_DATA_CACHE_ENABLE();                                  //闪存数据缓存启用
	printf("HAL_OK\r\n");
	Address = FLASH_USER_START_ADDR + (lseekFlag - 1) * RAM_BUFFER_SIZE;         //分配用户程序起始地址ַ
	while (Address
			< FLASH_USER_START_ADDR + (lseekFlag - 1) * RAM_BUFFER_SIZE
					+ APP_Size) {
		p = (uint32_t*) &RAM_Buffer[i];                                          //P指向 读取bin文件缓存区
		uint32_t pData = *p;

		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, pData) == HAL_OK) //HAL_FLASH_ProgramFLASH写入数据函数   Address写入的地址  *p写入的数据
				{
			Address = Address + 4;                                               //因为一次写入32位  所以地址加4
			i += 4;
			if (i >= RAM_BUFFER_SIZE)
				break;
		} else {
			printf("Address-error\r\n");
			Error_Handler();
		}
	}
	HAL_FLASH_Lock();                                                       //上锁
	Address = FLASH_USER_START_ADDR;		                                //校验
	MemoryProgramStatus = 0x0;
	printf("CHEAK\r\n");
	while (Address < FLASH_USER_END_ADDR) {
		data32 = *(__IO uint32_t*) Address;
		if (data32 != *(uint32_t*) RAM_Buffer) {
			MemoryProgramStatus++;
		}
		Address = Address + 4;
	}
	printf("CHEAK-finish\r\n");
	return HAL_OK;
}
void jumpToApp() {
	USBH_LL_DeInit(&hUsbHostFS);
	printf("FLASH_USER_START_ADDR %X \r\n",
			(*(__IO uint32_t*) FLASH_USER_START_ADDR));
	printf("FLASH_USER_RESET_ADDR %X \r\n",
			(*(__IO uint32_t*) (FLASH_USER_START_ADDR + 4)));
	JumpAddress = *(__IO uint32_t*) (FLASH_USER_START_ADDR + 4);	//取复位中断向量的值
	Jump_To_Application = (pFunction) JumpAddress;
	Jump_To_Application();
	printf("shoud not entry!! \n");
}
void UP_Data(void) {
	if (SystemUpdateFlag == 0) {
		printf("state\n");
		if (lseekFlag || (int32_t) (bootloaderTimeout - HAL_GetTick()) > 0) {
			if (SystemUpdateFlag == 0 && Appli_state == APPLICATION_READY) //满足  系统更新标志为0，申请状态是准备状态
					{
				bootloaderTimeout = HAL_GetTick() + 2000;                 //喂狗
				res = f_mount(&USBHFatFS, (TCHAR const*) USBHPath, 0);    //挂载文件系统
				printf("xxxx %d\r\n", res);
				if (res != FR_OK) {
					printf("mount no %d\r\n", res);
					Error_Handler();
				} else {
					printf("mount yes\r\n");
				}
				res = f_stat(filenamebatch, NULL);                        //查找文件名
				if (res != FR_OK) {
					res = f_stat(filename, NULL);
					if (res != FR_OK)
						jumpToApp();                                      //两个都查不到跳转app程序
					else
						batchFlag = 0;
				} else {
					batchFlag = 1;
				}
				if (batchFlag == 1)
					res = f_open(&USBHFile, filenamebatch, FA_READ);       //filename外部定义好的  FA_READ可以从文件中读取数据
				else
					res = f_open(&USBHFile, filename, FA_READ);
				if (res != FR_OK) {
					printf("open no  %d\r\n", res);
					Error_Handler();
				} else {
					printf("open yes \r\n");
				}
				APP_Size = f_size(&USBHFile);                              //读取所打开的文件多少个字节长度
				res = f_lseek(&USBHFile, lseekFlag * RAM_BUFFER_SIZE);     //移动打开的文件读指针
				if (res != FR_OK) {
					printf("lseek no %d\r\n", res);
					Error_Handler();
				} else {
					lseekFlag++;                                           //偏移下一次的指针
					printf("lseek yes\r\n");
				}
				res = f_read(&USBHFile, RAM_Buffer, sizeof(RAM_Buffer),
						(void*) &APP_Size); // RAM_Buffer用于存储读取数据的缓冲区 sizeof(RAM_Buffer)要读取的字节数  APP_Size读取的字节
				if (res != FR_OK) {
					printf("read no %d\r\n", res);
					Error_Handler();
				} else {
					printf("read yes\r\n");
					if (APP_Size < RAM_BUFFER_SIZE)                         //判断是否读取完文件
						SystemUpdateFlag = 1;
					else
						SystemUpdateFlag = 0;
				}
				if ((0 < APP_Size) && (APP_Size < FLASH_USER_END_ADDR)) {
					FLASH_Erase_Write();                                    //擦写
					if (SystemUpdateFlag == 1 && batchFlag == 0) {
						f_close(&USBHFile);                                 //有open就有关闭文件
						if (f_rename(filename, filenamenew) == 0)           //更新用户代码后修改文件名
							printf("shibai  \r\n");
					}
					if (SystemUpdateFlag == 1) {
						printf("jumpToApp\r\n");
						f_close(&USBHFile);                                 //有open就有关闭文件
						jumpToApp();                                        //跳转到用户程序
					}
				} else {
					printf("APP_Size \r\n");
				}
				f_close(&USBHFile);                                         //有open就有关闭文件
			}
		} else {
			jumpToApp();
		}
	}
}


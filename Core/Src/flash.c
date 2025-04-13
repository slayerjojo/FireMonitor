#include "flash.h"
#include "main.h"

#define W25X_WriteEnable        0x06
#define W25X_WriteDisable		0x04
#define W25X_ReadStatusReg		0x05
#define W25X_WriteStatusReg		0x01
#define W25X_ReadData			0x03
#define W25X_FastReadData		0x0B
#define W25X_FastReadDual		0x3B
#define W25X_PageProgram		0x02
#define W25X_BlockErase			0xD8
#define W25X_SectorErase		0x20
#define W25X_ChipErase			0xC7
#define W25X_PowerDown			0xB9
#define W25X_ReleasePowerDown	0xAB
#define W25X_DeviceID			0xAB
#define W25X_ManufactDeviceID   0x90
#define W25X_JedecDeviceID		0x9F

#define Dummy_Byte              0xFF

#define SPI_FLASH_PageSize              256
#define SPI_FLASH_SectorSize            4096

extern SPI_HandleTypeDef hspi1;

static uint8_t SPI_FLASH_ReadWrite(uint8_t txdata)
{
    uint8_t rxdata;
    HAL_SPI_TransmitReceive(&hspi1, &txdata, &rxdata, 1, 1000);
    return rxdata;
}

static uint8_t SPI_FLASH_ReadStatus1(void)
{
    uint8_t status;
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    SPI_FLASH_ReadWrite(W25X_ReadStatusReg);
    status = SPI_FLASH_ReadWrite(0xff);
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
    return status;
}

static void SPI_FLASH_WriteStatus1(uint8_t status)
{
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    SPI_FLASH_ReadWrite(W25X_WriteStatusReg);
    SPI_FLASH_ReadWrite(status);
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}

static void SPI_FLASH_WaitForWriteEnd(void)
{
    while(SPI_FLASH_ReadStatus1() & 0x01);
}

static void SPI_FLASH_WriteEnable(void)
{
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    SPI_FLASH_ReadWrite(W25X_WriteEnable);
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}

void SPI_FLASH_PageWrite(uint32_t addr, const uint8_t *buffer, uint16_t size)
{
    SPI_FLASH_WriteEnable();
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    SPI_FLASH_ReadWrite(W25X_PageProgram);
    SPI_FLASH_ReadWrite((addr & 0xFF0000) >> 16);
    SPI_FLASH_ReadWrite((addr & 0xFF00) >> 8);
    SPI_FLASH_ReadWrite(addr & 0xFF);
    while (size--)
    {
        SPI_FLASH_ReadWrite(*buffer++);
    }
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);

    SPI_FLASH_WaitForWriteEnd();
}

void flash_init(void)
{
    uint8_t uid[8] = {0};

    SEGGER_RTT_printf(0, "flash device:%x\n", flash_device_id());
    flash_uid(uid);
    SEGGER_RTT_printf(0, "flash uid:%02x%02x%02x%02x%02x%02x%02x%02x\n", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6], uid[7]);
}

uint16_t flash_device_id(void)
{
    uint16_t deviceid;

    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    SPI_FLASH_ReadWrite(W25X_ManufactDeviceID);
    SPI_FLASH_ReadWrite(0);
    SPI_FLASH_ReadWrite(0);
    SPI_FLASH_ReadWrite(0);
    deviceid = SPI_FLASH_ReadWrite(0xFF) << 8;
    deviceid |= SPI_FLASH_ReadWrite(0xFF);
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);

    return deviceid;
}

void flash_uid(uint8_t *uid)
{
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    SPI_FLASH_ReadWrite(0x4b);
    SPI_FLASH_ReadWrite(0);
    SPI_FLASH_ReadWrite(0);
    SPI_FLASH_ReadWrite(0);
    SPI_FLASH_ReadWrite(0);
    uid[0] = SPI_FLASH_ReadWrite(0xFF);
    uid[1] = SPI_FLASH_ReadWrite(0xFF);
    uid[2] = SPI_FLASH_ReadWrite(0xFF);
    uid[3] = SPI_FLASH_ReadWrite(0xFF);
    uid[4] = SPI_FLASH_ReadWrite(0xFF);
    uid[5] = SPI_FLASH_ReadWrite(0xFF);
    uid[6] = SPI_FLASH_ReadWrite(0xFF);
    uid[7] = SPI_FLASH_ReadWrite(0xFF);
    uid[8] = SPI_FLASH_ReadWrite(0xFF);
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}

void flash_write(uint32_t addr, const uint8_t *buffer, uint16_t size)
{
    uint16_t pos = 0;
    while (size)
    {
        uint16_t length = size;
        if (length > SPI_FLASH_PageSize - pos % SPI_FLASH_PageSize)
            length = SPI_FLASH_PageSize - pos % SPI_FLASH_PageSize;
        SPI_FLASH_PageWrite(addr + pos, buffer + pos, length);
        pos += length;
        size -= length;
    }
}

void flash_sector_erase(uint32_t addr)
{
    addr /= SPI_FLASH_SectorSize;

    SPI_FLASH_WriteEnable();
    SPI_FLASH_WaitForWriteEnd();

    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    SPI_FLASH_ReadWrite(W25X_SectorErase);
    SPI_FLASH_ReadWrite((addr & 0xFF0000) >> 16);
    SPI_FLASH_ReadWrite((addr & 0xFF00) >> 8);
    SPI_FLASH_ReadWrite(addr & 0xFF);
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);

    SPI_FLASH_WaitForWriteEnd();
}

void flash_read(uint32_t addr, uint8_t *buffer, uint16_t size)
{
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    SPI_FLASH_ReadWrite(W25X_ReadData);

    SPI_FLASH_ReadWrite((addr & 0xFF0000) >> 16);
    SPI_FLASH_ReadWrite((addr & 0xFF00) >> 8);
    SPI_FLASH_ReadWrite(addr & 0xFF);

    while (size--)
    {
        *buffer++ = SPI_FLASH_ReadWrite(Dummy_Byte);
    }
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}

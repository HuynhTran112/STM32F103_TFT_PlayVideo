#include "sd_card.h"
#include "delay.h"

static uint8_t sd_type = SD_TYPE_UNKNOWN;

static uint8_t SPI2_Transfer(uint8_t data)
{
    while (!(SPI2->SR & SPI_SR_TXE));
    SPI2->DR = data;
    while (!(SPI2->SR & SPI_SR_RXNE));
    return (uint8_t)SPI2->DR;
}

void SD_GPIO_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN; //36 MHz
    
    GPIOB->CRH &= ~(0xFFFFU << 16);
    GPIOB->CRH |= (0x3U << 16);   /* PB12: CS output */
    GPIOB->CRH |= (0xBU << 20);   /* PB13: SCK alternate function */
    GPIOB->CRH |= (0x4U << 24);   /* PB14: MISO input */
    GPIOB->CRH |= (0xBU << 28);   /* PB15: MOSI alternate function */
    
    SD_CS_HIGH();
}

void SD_SPI_Init(void)
{
    /*
    SPI_CR1_MSTR  : Master mode
    SPI_CR1_SSM   : Software slave management (GPIO)
    SPI_CR1_SSI   : Set when SSM = 1
    SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2 : SPI clock = fPCLK / 256 = 140 kHz
    SPI_CR1_SPE   : Enable SPI peripheral
    */
    SPI2->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI |
                SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2;
    SPI2->CR1 |= SPI_CR1_SPE;
}

void SD_SPI_SetSpeed(uint8_t fast)
{
    /*SPI disable*/
    SPI2->CR1 &= ~SPI_CR1_SPE;
    /*Clear BR*/
    SPI2->CR1 &= ~SPI_CR1_BR;
    if (fast) {
        SPI2->CR1 |= SPI_CR1_BR_0; // fCLK / 4 = 9 MHz
    } else {
        SPI2->CR1 |= SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2; // fCLK / 256 = 140 kHz
    }
    /*SPI enable*/
    SPI2->CR1 |= SPI_CR1_SPE;
}

static uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg)
{
    uint8_t res;
    uint16_t retry;
    
    retry = 100;
    while (SPI2_Transfer(0xFF) != 0xFF && retry--);
    /*Send Command Frame*/
    SPI2_Transfer(0x40 | cmd);
    SPI2_Transfer((uint8_t)(arg >> 24));
    SPI2_Transfer((uint8_t)(arg >> 16));
    SPI2_Transfer((uint8_t)(arg >> 8));
    SPI2_Transfer((uint8_t)arg);
    /*Send CRC*/
    if (cmd == CMD0) SPI2_Transfer(0x95);
    else if (cmd == CMD8) SPI2_Transfer(0x87);
    else SPI2_Transfer(0x01);
    /*Wait for response*/
    retry = 200;
    do {
        res = SPI2_Transfer(0xFF);
    } while ((res & 0x80) && retry--); //Wait for response 0x00–0x7F
    
    return res;
}

uint8_t SD_Init(void)
{
    uint8_t res;
    uint16_t retry;
    uint8_t ocr[4];
    
    SD_GPIO_Init();
    SD_SPI_Init();
    delay(20);
    
    /* Send at least 74 clock cycles with CS high to initialize card */
    SD_CS_HIGH();
    for (uint8_t i = 0; i < 10; i++) {
        SPI2_Transfer(0xFF);
    }
    
    /* Enter idle state */
    SD_CS_LOW();
    retry = 100;
    do {
        res = SD_SendCmd(CMD0, 0);
    } while (res != 0x01 && retry--); //if res = 0x01, end loop  
    
    if (res != 0x01) {
        SD_CS_HIGH();
        return 1;
    }
    
    /* Check for SDv2 card */
    res = SD_SendCmd(CMD8, 0x1AA);
    if (res == 0x01) {
        for (uint8_t i = 0; i < 4; i++) {
            ocr[i] = SPI2_Transfer(0xFF);
        }
        /*Check if 2.7-3.6V voltage is supported*/
        if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
            retry = 1000;
            do {
                SD_SendCmd(CMD55, 0);
                res = SD_SendCmd(ACMD41, 0x40000000);
            } while (res && retry--);
            
            if (res == 0) {
                res = SD_SendCmd(CMD58, 0);
                for (uint8_t i = 0; i < 4; i++) {
                    ocr[i] = SPI2_Transfer(0xFF);
                }
                sd_type = (ocr[0] & 0x40) ? SD_TYPE_SDHC : SD_TYPE_SDv2;
            }
        }
    } else {
        /* SDv1 card initialization */
        retry = 1000;
        do {
            SD_SendCmd(CMD55, 0);
            res = SD_SendCmd(ACMD41, 0);
        } while (res && retry--);
        
        if (res == 0) {
            sd_type = SD_TYPE_SDv1;
            SD_SendCmd(CMD16, 512);
        }
    }
    
    SD_CS_HIGH();
    SPI2_Transfer(0xFF);
    
    if (sd_type == SD_TYPE_UNKNOWN) {
        return 1;
    }
    
    SD_SPI_SetSpeed(1);
    return 0;
}

uint8_t SD_ReadSector(uint8_t *buffer, uint32_t sector)
{
    uint8_t res;
    uint16_t retry;
    
    if (sd_type != SD_TYPE_SDHC) {
        sector <<= 9;
    }
    
    SD_CS_LOW();
    res = SD_SendCmd(CMD17, sector);
    if (res != 0) {
        SD_CS_HIGH();
        return 1;
    }
    
    retry = 10000;
    do {
        res = SPI2_Transfer(0xFF);
    } while (res == 0xFF && retry--);
    
    if (res != 0xFE) {
        SD_CS_HIGH();
        return 2;
    }
    
    for (uint16_t i = 0; i < 512; i++) {
        buffer[i] = SPI2_Transfer(0xFF);
    }
    
    SPI2_Transfer(0xFF);
    SPI2_Transfer(0xFF);
    
    SD_CS_HIGH();
    SPI2_Transfer(0xFF);
    
    return 0;
}

uint8_t SD_ReadMultiSector(uint8_t *buffer, uint32_t sector, uint32_t count)
{
    uint8_t res;
    uint16_t retry;
    
    if (sd_type != SD_TYPE_SDHC) {
        sector <<= 9;
    }
    
    SD_CS_LOW();
    res = SD_SendCmd(CMD18, sector);
    if (res != 0) {
        SD_CS_HIGH();
        return 1;
    }
    
    while (count--) {
        retry = 10000;
        do {
            res = SPI2_Transfer(0xFF);
        } while (res == 0xFF && retry--);
        
        if (res != 0xFE) {
            SD_CS_HIGH();
            return 2;
        }
        
        for (uint16_t i = 0; i < 512; i++) {
            *buffer++ = SPI2_Transfer(0xFF);
        }
        
        SPI2_Transfer(0xFF);
        SPI2_Transfer(0xFF);
    }
    
    SD_SendCmd(CMD12, 0);
    SD_CS_HIGH();
    SPI2_Transfer(0xFF);
    
    return 0;
}

uint8_t SD_GetType(void)
{
    return sd_type;
}

#include "GLCD_ST7735.h"
#include "delay.h"

/* 5x7 pixel font covering ASCII 32-90 (space through 'Z') */
static const uint8_t Font5x7[] = {
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x5F,0x00,0x00,
    0x00,0x07,0x00,0x07,0x00,
    0x14,0x7F,0x14,0x7F,0x14,
    0x24,0x2A,0x7F,0x2A,0x12,
    0x23,0x13,0x08,0x64,0x62,
    0x36,0x49,0x55,0x22,0x50,
    0x00,0x05,0x03,0x00,0x00,
    0x00,0x1C,0x22,0x41,0x00,
    0x00,0x41,0x22,0x1C,0x00,
    0x08,0x2A,0x1C,0x2A,0x08,
    0x08,0x08,0x3E,0x08,0x08,
    0x00,0x50,0x30,0x00,0x00,
    0x08,0x08,0x08,0x08,0x08,
    0x00,0x60,0x60,0x00,0x00,
    0x20,0x10,0x08,0x04,0x02,
    0x3E,0x51,0x49,0x45,0x3E,
    0x00,0x42,0x7F,0x40,0x00,
    0x42,0x61,0x51,0x49,0x46,
    0x21,0x41,0x45,0x4B,0x31,
    0x18,0x14,0x12,0x7F,0x10,
    0x27,0x45,0x45,0x45,0x39,
    0x3C,0x4A,0x49,0x49,0x30,
    0x01,0x71,0x09,0x05,0x03,
    0x36,0x49,0x49,0x49,0x36,
    0x06,0x49,0x49,0x29,0x1E,
    0x00,0x36,0x36,0x00,0x00,
    0x00,0x56,0x36,0x00,0x00,
    0x00,0x08,0x14,0x22,0x41,
    0x14,0x14,0x14,0x14,0x14,
    0x41,0x22,0x14,0x08,0x00,
    0x02,0x01,0x51,0x09,0x06,
    0x32,0x49,0x79,0x41,0x3E,
    0x7E,0x11,0x11,0x11,0x7E,
    0x7F,0x49,0x49,0x49,0x36,
    0x3E,0x41,0x41,0x41,0x22,
    0x7F,0x41,0x41,0x22,0x1C,
    0x7F,0x49,0x49,0x49,0x41,
    0x7F,0x09,0x09,0x01,0x01,
    0x3E,0x41,0x41,0x51,0x32,
    0x7F,0x08,0x08,0x08,0x7F,
    0x00,0x41,0x7F,0x41,0x00,
    0x20,0x40,0x41,0x3F,0x01,
    0x7F,0x08,0x14,0x22,0x41,
    0x7F,0x40,0x40,0x40,0x40,
    0x7F,0x02,0x04,0x02,0x7F,
    0x7F,0x04,0x08,0x10,0x7F,
    0x3E,0x41,0x41,0x41,0x3E,
    0x7F,0x09,0x09,0x09,0x06,
    0x3E,0x41,0x51,0x21,0x5E,
    0x7F,0x09,0x19,0x29,0x46,
    0x46,0x49,0x49,0x49,0x31,
    0x01,0x01,0x7F,0x01,0x01,
    0x3F,0x40,0x40,0x40,0x3F,
    0x1F,0x20,0x40,0x20,0x1F,
    0x7F,0x20,0x18,0x20,0x7F,
    0x63,0x14,0x08,0x14,0x63,
    0x03,0x04,0x78,0x04,0x03,
    0x61,0x51,0x49,0x45,0x43,
};
    /*
    TXE: Transmit buffer (r)
    0: not empty
    1: empty
    RXNE: Receive buffer (r)
    0:  empty
    1:  not empty
    */
uint8_t LCD_SPI_Transfer(uint8_t data)
{
    /*Wait TX buffer empty -> DR Write*/
    while (!(SPI1->SR & SPI_SR_TXE));
    SPI1->DR = data;
    /*Wait RX buffer not empty -> DR Read*/
    while (!(SPI1->SR & SPI_SR_RXNE));
    return (uint8_t)SPI1->DR;
}

void LCD_WriteCmd(uint8_t cmd)
{
    LCD_DC_CMD();
    LCD_SPI_Transfer(cmd);
}

void LCD_WriteData(uint8_t data)
{
    LCD_DC_DATA();
    LCD_SPI_Transfer(data);
}

void LCD_GPIO_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_SPI1EN;
    /*
    PA0: Output Push-pull (RST) -> 3
    PA2: Output Push-pull (CS) -> 3
    PA5: AF Push-pull (SCK) -> B
    PA7: AF Push-pull (MOSI) -> B
    */
    GPIOA->CRL &= ~(0xF0F00F0FU);
    GPIOA->CRL |= 0xB0B00303;
    /*
    PA8: Output Push-pull (DC) -> 3
    */
    GPIOA->CRH &= ~(0xFU);
    GPIOA->CRH |= 0x3U;
    LCD_CS_HIGH();
    LCD_RST_HIGH();
}

void LCD_SPI_Init(void)
{
    /*
    SPI_CR1_SSM   Software slave management (GPIO)
    SPI_CR1_SSI   Set when SSM = 1
    SPI_CR1_MSTR  Master mode
    SPI_CR1_BR_1  Baudrate = fPCLK/8 = 9MHz
    SPI_CR1_SPE   Enable SPI
    */
    SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR | SPI_CR1_BR_1;
    SPI1->CR1 |= SPI_CR1_SPE;
}

static void LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    /*
    0x2A: Set column address
    */
    LCD_WriteCmd(CASET);
    LCD_WriteData(0); LCD_WriteData((uint8_t)x1);
    LCD_WriteData(0); LCD_WriteData((uint8_t)x2);
    /*
    0x2B: Set row address
    */
    LCD_WriteCmd(RASET);
    LCD_WriteData(0); LCD_WriteData((uint8_t)y1);
    LCD_WriteData(0); LCD_WriteData((uint8_t)y2);
    /*
    0x2C: Memory write
    */
    LCD_WriteCmd(RAMWR);
}

void LCD_Init(void)
{
    LCD_GPIO_Init();
    LCD_SPI_Init();
    
    LCD_CS_LOW();
    
    LCD_RST_HIGH(); delay(10);
    LCD_RST_LOW();  delay(10);
    /*RST high must be >10ms*/
    LCD_RST_HIGH(); delay(150);
    /* 0x01: Software reset */
    LCD_WriteCmd(SWRESET); delay(150);
    /* 0x11: Sleep out */
    LCD_WriteCmd(SLPOUT); delay(200);
    /* 
    0x3A: Interface pixel format 
    IFPF [2:0]
    011: 18-bit
    101: 16-bit
    110: 12-bit
    111: 16-bit*/
    LCD_WriteCmd(COLMOD); LCD_WriteData(IFPF_16BIT);
    /* 0x36: Memory data access control 
    MY [7] 
    MX [6] 
    MV [5] 
    ML [4] 
    0: LCD refresh Top to Bottom
    1: LCD refresh Bottom to Top
    RGB [3] 
    0: RGB
    1: BGR
    MH [2] 
    0: LCD refresh Left to Right
    1: LCD refresh Right to Left
    */
    LCD_WriteCmd(MADCTL); LCD_WriteData(MADCTL_MX | MADCTL_BGR);
    /* 0x29: Display on */
    LCD_WriteCmd(DISPON); delay(100);
    
    LCD_CS_HIGH();
}

void LCD_FillScreen(uint16_t color)
{
    LCD_FillRect(0, 0, SCREEN_W, SCREEN_H, color);
}

void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    LCD_CS_LOW();
    LCD_SetWindow(x, y, x + w - 1, y + h - 1);
    LCD_DC_DATA();
    
    uint8_t hi = (uint8_t)(color >> 8);
    uint8_t lo = (uint8_t)color;
    
    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        LCD_SPI_Transfer(hi);
        LCD_SPI_Transfer(lo);
    }
    LCD_CS_HIGH();
}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= SCREEN_W || y >= SCREEN_H) return;
    
    LCD_CS_LOW();
    LCD_SetWindow(x, y, x, y);
    LCD_DC_DATA();
    LCD_SPI_Transfer((uint8_t)(color >> 8));
    LCD_SPI_Transfer((uint8_t)color);
    LCD_CS_HIGH();
}

void LCD_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size)
{
    if (c < 32 || c > 'Z') return;
    
    const uint8_t *bmp = &Font5x7[(c - 32) * 5];
    
    for (uint8_t i = 0; i < 5; i++) {
        uint8_t line = bmp[i];
        for (uint8_t j = 0; j < 7; j++, line >>= 1) {
            uint16_t px = x + i * size;
            uint16_t py = y + j * size;
            LCD_FillRect(px, py, size, size, (line & 1) ? color : bg);
        }
    }
}

void LCD_DrawString(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size)
{
    while (*str) {
        LCD_DrawChar(x, y, *str++, color, bg, size);
        /*
        5 pixel + padding= 6 pixels
        */
        x += 6 * size;
    }
}

void LCD_SetFullscreen(void)
{
    LCD_CS_LOW();
    LCD_SetWindow(0, 0, SCREEN_W - 1, SCREEN_H - 1);
    LCD_DC_DATA();
}

void LCD_SendBuffer(uint8_t *buffer, uint16_t len)
{
    /*
    Data = [High Low]
    */
    for (uint16_t i = 0; i < len; i += 2) {
        while (!(SPI1->SR & SPI_SR_TXE));
        SPI1->DR = buffer[i + 1];
        while (!(SPI1->SR & SPI_SR_TXE));
        SPI1->DR = buffer[i];
    }
    /*
    Wait for SPI transfer to complete
    */
    while (SPI1->SR & SPI_SR_BSY); //BSY 0: not busy, 1: busy
}

void LCD_EndTransfer(void)
{
    LCD_CS_HIGH();
}

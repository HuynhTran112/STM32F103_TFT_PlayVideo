#ifndef __GLCD_ST7735__h
#define __GLCD_ST7735__h

#include "stm32f10x.h"
#include <stdint.h>

/* ST7735 Commands */
#define SWRESET  0x01
#define SLPOUT   0x11
#define DISPON   0x29
#define CASET    0x2A
#define RASET    0x2B
#define RAMWR    0x2C
#define MADCTL   0x36
#define COLMOD   0x3A

/* Interface Pixel Format (0x3A) */
#define IFPF_12BIT 0x03
#define IFPF_16BIT 0x05
#define IFPF_18BIT 0x06

/* Memory Data Access Control (0x36) */
#define MADCTL_MH  0x04
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_ML  0x10
#define MADCTL_MV  0x20
#define MADCTL_MX  0x40
#define MADCTL_MY  0x80

/* GPIO control macros for display interface signals */
#define LCD_RST_HIGH()  (GPIOA->BSRR = (1U << 0))
#define LCD_RST_LOW()   (GPIOA->BRR  = (1U << 0))
#define LCD_CS_HIGH()   (GPIOA->BSRR = (1U << 2))
#define LCD_CS_LOW()    (GPIOA->BRR  = (1U << 2))
#define LCD_DC_DATA()   (GPIOA->BSRR = (1U << 8)) //S [15:0] R [31:16]
#define LCD_DC_CMD()    (GPIOA->BRR  = (1U << 8)) 

/* Display geometry after 90-degree rotation */
#define SCREEN_W  160
#define SCREEN_H  128

/* Standard RGB565 color definitions */
#define BLACK       0x0000
#define WHITE       0xFFFF
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define YELLOW      0xFFE0
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define ORANGE      0xFD20

/* Initialization and configuration */
void LCD_GPIO_Init(void);
void LCD_SPI_Init(void);
void LCD_Init(void);

/* Drawing primitives */
void LCD_FillScreen(uint16_t color);
void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
void LCD_DrawString(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size);

/* Video streaming interface */
void LCD_SetFullscreen(void);
void LCD_SendBuffer(uint8_t *buffer, uint16_t len);
void LCD_EndTransfer(void);

/* Low-level SPI access */
uint8_t LCD_SPI_Transfer(uint8_t data);
void LCD_WriteCmd(uint8_t cmd);
void LCD_WriteData(uint8_t data);

#endif

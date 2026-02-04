#ifndef SD_CARD_H
#define SD_CARD_H

#include "stm32f10x.h"
#include <stdint.h>

/* SD command definitions */
#define CMD0    0   /* GO_IDLE_STATE */
#define CMD8    8   /* SEND_IF_COND */
#define CMD12   12  /* STOP_TRANSMISSION */
#define CMD16   16  /* SET_BLOCKLEN */
#define CMD17   17  /* READ_SINGLE_BLOCK */
#define CMD18   18  /* READ_MULTIPLE_BLOCK */
#define CMD55   55  /* APP_CMD prefix */
#define CMD58   58  /* READ_OCR */
#define ACMD41  41  /* SD_SEND_OP_COND */

/* SD card type identifiers */
#define SD_TYPE_UNKNOWN  0
#define SD_TYPE_SDv1     1
#define SD_TYPE_SDv2     2
#define SD_TYPE_SDHC     3

/* Chip select control */
#define SD_CS_LOW()   (GPIOB->BRR  = (1U << 12))
#define SD_CS_HIGH()  (GPIOB->BSRR = (1U << 12))

/* Public interface */
void     SD_GPIO_Init(void);
void     SD_SPI_Init(void);
void     SD_SPI_SetSpeed(uint8_t fast);
uint8_t  SD_Init(void);
uint8_t  SD_ReadSector(uint8_t *buffer, uint32_t sector);
uint8_t  SD_ReadMultiSector(uint8_t *buffer, uint32_t sector, uint32_t count);
uint8_t  SD_GetType(void);

#endif

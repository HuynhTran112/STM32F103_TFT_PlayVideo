#include "SysClockConfig.h"
#include "delay.h"
#include "GLCD_ST7735.h"
#include "sd_card.h"

#define SECTORS_PER_FRAME  80      /* 160 x 128 x 2 bytes = 40960 / 512 */
#define TOTAL_FRAMES       341     /* Total frames in video file */

static void ShowError(void)
{
    LCD_FillScreen(RED);
    while (1);
}

int main(void)
{
    uint8_t buffer[512];
    uint32_t sector = 0;
    uint32_t frame = 0;
    
    SysClockConfig();
    LCD_Init();
    
    if (SD_Init() != 0) {
        ShowError();
    }
    
    /* Continuous video playback with automatic looping */
    while (1) {
        LCD_SetFullscreen();
        
        for (uint8_t s = 0; s < SECTORS_PER_FRAME; s++) {
            if (SD_ReadSector(buffer, sector++) != 0) {
                ShowError();
            }
            LCD_SendBuffer(buffer, 512);
        }
        
        LCD_EndTransfer();
        
        if (++frame >= TOTAL_FRAMES) {
            frame = 0;
            sector = 0;
        }
    }
}

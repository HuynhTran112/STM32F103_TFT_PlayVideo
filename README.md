# STM32F103 TFT Video Player (SD Card + ST7735)

## Description
This project is a simple video player for **STM32F103C8T6** (Blue Pill) that reads raw RGB565 video data from an **SD Card** and streams it to an **ST7735 TFT LCD** at 160x128 resolution. The system uses bare-metal register-level programming (no HAL) for maximum performance and minimal code size.

## Requirements
*   **IDE**: Keil MDK-ARM v5
*   **Programmer**: ST-Link V2
*   **Hardware Components**:
    *   STM32F103C8T6 (Blue Pill)
    *   ST7735 TFT LCD (1.8" 160x128, SPI)
    *   MicroSD Card Module (SPI)
    *   SD Card (FAT32 formatted, raw video data)

## Features
*   Raw RGB565 video playback at 160x128 resolution
*   Automatic video looping
*   High-speed SPI communication (18 MHz for LCD, 9 MHz for SD)
*   Bare-metal register-level code (no HAL library)
*   Low memory footprint (512-byte sector buffer)

## Project Structure
```text
├── main.c             # System init & video playback loop
├── SysClockConfig.c   # System clock configuration (72 MHz)
├── SysClockConfig.h
├── delay.c            # SysTick delay functions
├── delay.h
├── GLCD_ST7735.c      # ST7735 TFT LCD driver (SPI1)
├── GLCD_ST7735.h
├── sd_card.c          # SD Card driver (SPI2)
└── sd_card.h
```

## Hardware Connections
### ST7735 TFT LCD (SPI1)
| LCD Pin | STM32 Pin | Function |
| :--- | :--- | :--- |
| VCC | 3.3V | Power |
| GND | GND | Ground |
| CS | PA2 | Chip Select |
| RST | PA0 | Reset |
| DC (A0) | PA8 | Data/Command |
| SDA (MOSI) | PA7 | SPI1 MOSI |
| SCK | PA5 | SPI1 Clock |
| LED | 3.3V | Backlight |

### SD Card Module (SPI2)
| SD Pin | STM32 Pin | Function |
| :--- | :--- | :--- |
| VCC | 3.3V | Power |
| GND | GND | Ground |
| CS | PB12 | Chip Select |
| MOSI | PB15 | SPI2 MOSI |
| MISO | PB14 | SPI2 MISO |
| SCK | PB13 | SPI2 Clock |

## How to Use
### 1. Prepare the Video File
1.  Use **FFmpeg** to convert your video to raw RGB565 format:
    ```bash
    ffmpeg -i input.mp4 -vf "scale=160:128,fps=20" -pix_fmt rgb565be -f rawvideo video.rgb
    ```
2.  Rename the output file from `video.rgb` to `video.img`.
3.  Use **Win32 Disk Imager** to write the `.img` file to the SD Card:
    *   Open Win32 Disk Imager.
    *   Select the `video.img` file.
    *   Choose the correct SD Card drive letter.
    *   Click **Write**.
    > ⚠️ **Warning**: This will overwrite the SD card. Make sure to select the correct drive!

4.  Update `TOTAL_FRAMES` in `main.c`:
    ```c
    #define TOTAL_FRAMES  341   /* (File size in bytes) / (160 * 128 * 2) */
    ```

### 2. Build and Flash
1.  Download or clone this repository.
2.  Open **Keil MDK-ARM** and create a new project:
    *   **Project → New µVision Project**
    *   Select device: `STM32F103C8`
3.  Add source files to the project:
    *   Right-click **Source Group 1 → Add Existing Files**
    *   Add all `.c` files: `main.c`, `SysClockConfig.c`, `delay.c`, `GLCD_ST7735.c`, `sd_card.c`
4.  Configure Include Paths:
    *   **Options for Target → C/C++ → Include Paths**
    *   Add the project folder path (where `.h` files are located)
5.  Configure Debugger:
    *   **Options for Target → Debug**
    *   Select **ST-Link Debugger**
6.  Build the project: **Project → Build Target** (or press `F7`).
7.  Connect **ST-Link V2** to the Blue Pill.
8.  Flash the firmware: **Flash → Download** (or press `F8`).

## System Workflow
1.  **Startup**: System clock is configured to 72 MHz using HSE and PLL.
2.  **Initialization**: LCD and SD Card are initialized via SPI.
3.  **Playback Loop**:
    *   Set LCD to fullscreen mode (160x128 window).
    *   Read 80 sectors (40 KB) per frame from SD Card.
    *   Stream pixel data to LCD via SPI DMA-like transfer.
    *   Loop back to frame 0 when video ends.
4.  **Error Handling**: If SD Card fails, LCD turns red and halts.

## Preview
<p align="center">
  <img src="img/demo.jpg" alt="Demo" width="50%">
</p>

## Video Demo
[![Watch the video](img/thumbnail.jpg)](https://www.youtube.com/watch?v=YOUR_VIDEO_ID)

## Result Summary
*   Smooth video playback at ~20 FPS.
*   Stable SPI communication with SD Card and LCD.
*   Low memory usage (only 512-byte buffer).
*   No external libraries required.

## Author
**Trần Huỳnh**  
Major: Computer Engineering Technology  
Faculty of Electrical-Electronics, HCMUTE  
Email: huynhtran30112004@gmail.com

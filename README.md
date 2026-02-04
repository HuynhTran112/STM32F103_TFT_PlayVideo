# STM32F103 TFT Video Player (SD Card + ST7735)

## Description
This project is a simple video player for **STM32F103C8T6** (Blue Pill) that reads raw RGB565 video data from an **SD Card** and display it to an **ST7735 TFT LCD** at 160x128 resolution.

## Requirements
*   **IDE**: [Keil MDK-ARM v5](https://www.keil.com/demo/eval/arm.htm)
*   **Programmer**: ST-Link V2
*   **Software Tools**:
    *   [FFmpeg](https://ffmpeg.org/download.html) – Convert video to RGB
    *   [Win32 Disk Imager](https://sourceforge.net/projects/win32diskimager/) – Write video to SD Card
*   **Hardware Components**:
    *   STM32F103C8T6 (Blue Pill)
    *   ST7735 TFT LCD (1.8" 160x128, SPI)
    *   MicroSD Card Module (SPI)
    *   SD Card

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

4.  Calculate `TOTAL_FRAMES` using PowerShell:
    ```powershell
    (Get-Item video.img).Length / 40960
    ```
5.  Update `TOTAL_FRAMES` in `main.c` with the result (eg: 3250):
    ```c
    #define TOTAL_FRAMES  3250
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

## How It Works
### ST7735 TFT LCD
**Concept**: The ST7735 receives commands and pixel data via SPI. To display an image, you set a drawing window then stream RGB565 pixel data.

**Important Notes**:
*   **SPI Speed**: Max 15 MHz for write. This project uses 9 MHz (72 MHz ÷ 8) for stability.
*   **Pixel Format**: RGB565 (16-bit) — each pixel = 2 bytes. One frame (160×128) = 40,960 bytes.
*   **DC Pin**: Low = command, High = data. Must toggle correctly or display won't work.
*   **Reset**: Hardware reset (RST low >10ms) required before sending commands.

**Display Flow**:
1.  **Initialize**: Reset → Sleep out → Set color format (16-bit) → Set orientation → Display ON.
2.  **Set Window**: `CASET` (column 0–159) + `RASET` (row 0–127) defines drawing area.
3.  **Send Data**: After `RAMWR`, stream all pixels continuously without pausing.
4.  **End**: Pull CS high after all data sent.

**Troubleshooting**:
*   White screen → RST timing wrong (wait >120ms after reset).
*   Wrong colors → Check RGB/BGR order in `MADCTL` command.
*   Partial display → `CASET`/`RASET` offset may be wrong for your LCD model.

### SD Card (SPI Mode)
**Concept**: SD Card supports SPI mode. Data is stored in 512-byte sectors. Video is written as raw data from sector 0.

**Important Notes**:
*   **Init Speed**: Must use ≤400 kHz. This project uses 140 kHz (36 MHz ÷ 256).
*   **Fast Speed**: After init, switch to 9 MHz (36 MHz ÷ 4) for video streaming.
*   **Card Types**: SDHC uses sector address directly. SDv1/v2 need sector × 512.
*   **Sector Size**: Always 512 bytes. One frame = 80 sectors.

**Initialization Flow**:
1.  Send 74+ clocks with CS high (wake up card).
2.  `CMD0` → expect `0x01` (idle state).
3.  `CMD8` with `0x1AA` → if `0x01`, it's SDv2/SDHC.
4.  `ACMD41` repeatedly → wait until `0x00` (ready).
5.  `CMD58` → check bit 30 for SDHC.
6.  Switch SPI to fast speed.

**Reading Flow**:
1.  `CMD17` + sector number → response `0x00`.
2.  Wait for token `0xFE`.
3.  Read 512 bytes.
4.  Read 2 CRC bytes (ignored).

**Troubleshooting**:
*   Init fails → Check wiring, use slower speed, ensure 3.3V.
*   Read timeout → Card not init properly, or sector address wrong.
*   Garbage data → SDHC uses sector number, SDv1/v2 uses byte address.

### Video Playback Logic
**Data Flow**:
```
SD Card (sector 0, 1, 2...) → 512-byte buffer → SPI1 → LCD → Display
```

**Performance**:
*   1 frame = 80 sectors = 40,960 bytes.
*   At 9 MHz, read 512 bytes ≈ 0.5 ms → 1 frame ≈ 40 ms → ~25 FPS max.
*   Actual FPS depends on SPI overhead and LCD write speed.

## System Workflow
1.  **Startup**: System clock configured to 72 MHz (HSE + PLL).
2.  **Init**: LCD and SD Card initialized via SPI.
3.  **Playback**:
    *   Set LCD fullscreen window (160×128).
    *   Read 80 sectors per frame from SD.
    *   Stream to LCD.
    *   Loop when video ends.
4.  **Error**: If SD fails, LCD fills red and halts.

## Preview
<p align="center">
  <img src="img/demo.jpg" alt="Demo" width="50%">
</p>

## Video Demo
[![Watch the video](img/thumbnail.jpg)](https://www.youtube.com/watch?v=YOUR_VIDEO_ID)

## Result Summary
*   Video playback runs smoothly at approximately 20 FPS.
*   SD Card and LCD communicate stably via SPI without data loss.

## Author
**Trần Huỳnh**  
Major: Computer Engineering Technology  
Faculty of Electrical-Electronics, HCMUTE  
Email: huynhtran30112004@gmail.com

/**
 *  @filename   :   epd2in9.h
 *  @brief      :   Header file for e-paper display library epd2in9.cpp
 *  @author     :   Yehui from Waveshare
 *  
 *  Copyright (C) Waveshare     September 5 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef EPD2IN9_BADGE_H
#define EPD2IN9_BADGE_H

#include "epdif-badge.h"

// Display resolution
#define EPD_WIDTH       128
#define EPD_HEIGHT      296

// DEPG0290B01 commands
#define DRIVER_OUTPUT_CONTROL                       0x01
#define DEEP_SLEEP_MODE                             0x10
#define SW_RESET                                    0x12
#define MASTER_ACTIVATION                           0x20
#define DISPLAY_UPDATE_CONTROL_2                    0x22
#define WRITE_RAM                                   0x24
#define WRITE_VCOM_REGISTER                         0x2C
#define WRITE_LUT_REGISTER                          0x32
#define SET_RAM_X_ADDRESS_START_END_POSITION        0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION        0x45
#define SET_RAM_X_ADDRESS_COUNTER                   0x4E
#define SET_RAM_Y_ADDRESS_COUNTER                   0x4F


extern const unsigned char lut_full_update[];
extern const unsigned char lut_partial_update[];

class Epd : EpdIf {
public:
    unsigned long width;
    unsigned long height;

    Epd();
    ~Epd();
    int  Init();
    void SendCommand(unsigned char command);
    void SendData(unsigned char data);
    void WaitUntilIdle(void);
    void Reset(void);
    void SetFrameMemory(
        const unsigned char* image_buffer,
        int x,
        int y,
        int image_width,
        int image_height
    );
    void SetFrameMemory(const unsigned char* image_buffer);
    void ClearFrameMemory(unsigned char color);
    void DisplayFrame(void);
    void Sleep(void);

private:
    unsigned int reset_pin;
    unsigned int dc_pin;
    unsigned int cs_pin;
    unsigned int busy_pin;
    const unsigned char* lut;

    void SetLut(const unsigned char* lut);
    void SetMemoryArea(int x_start, int y_start, int x_end, int y_end);
    void SetMemoryPointer(int x, int y);
};

#endif /* EPD2IN9_BADFE_H */

/* END OF FILE */

/**
 *  @filename   :   epd2in9-badge.cpp
 *  @brief      :   Implements for e-paper library
 *  @author     :   Yehui from Waveshare
 *              :   krzychb updated for DEPG0290B01
 *
 *  Copyright (C) Waveshare     September 9 2017
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

#include <stdlib.h>
#include "epd2in9-badge.h"

Epd::~Epd() {
};

Epd::Epd() {
    reset_pin = RST_PIN;
    dc_pin = DC_PIN;
    cs_pin = CS_PIN;
    busy_pin = BUSY_PIN;
    width = EPD_WIDTH;
    height = EPD_HEIGHT;
};

int Epd::Init() {

    // SPI interface init
    if (IfInit() != 0) {
        return -1;
    }

    // DEPG0290B01 init
    
    Reset();                // hardware reset
    SendCommand(SW_RESET);  // Software reset

    SendCommand(0x74);      // Set analog block control
    SendData(0x54);
    
    SendCommand(0x7E);      // Set digital block control
    SendData(0x3B);

    SendCommand(0x11);      // RAM data entry mode
    SendData(0x03);         // Address counter is updated in Y direction, Y increment, X increment

    SendCommand(0x3C);      // Set border waveform for VBD (see datasheet)
    SendData(0x01);

    SendCommand(0x2C);      // Set VCOM value
    SendData(0x26);

    SendCommand(0x03);      // Gate voltage setting (17h = 20 Volt, ranges from 10v to 21v)
    SendData(0x17);
    
    SendCommand(0x04);      // Source voltage setting (15volt, 0 volt and -15 volt)
    SendData(0x41);
    SendData(0x00);
    SendData(0x32);

    return 0;
}

/**
 *  @brief: basic function for sending commands
 */
void Epd::SendCommand(unsigned char command) {
    DigitalWrite(dc_pin, LOW);
    SpiTransfer(command);
}

/**
 *  @brief: basic function for sending data
 */
void Epd::SendData(unsigned char data) {
    DigitalWrite(dc_pin, HIGH);
    SpiTransfer(data);
}

/**
 *  @brief: Wait until the busy_pin goes LOW
 */
void Epd::WaitUntilIdle(void) {
    while(DigitalRead(busy_pin) == HIGH) {      //LOW: idle, HIGH: busy
        DelayMs(100);
        Serial.print(".");
    }      
}

/**
 *  @brief: module reset.
 *          often used to awaken the module in deep sleep,
 *          see Epd::Sleep();
 */
void Epd::Reset(void) {
    Serial.print("Hardware reset");
    DigitalWrite(reset_pin, LOW);
    DelayMs(200);
    DigitalWrite(reset_pin, HIGH);
    DelayMs(200);
    Serial.println(" done.");
}

/**
 *  @brief: set the look-up table register
 */
void Epd::SetLut(const unsigned char* lut) {
    Serial.print("Writing LUT");
    this->lut = lut;
    SendCommand(WRITE_LUT_REGISTER);
    for (int i = 0; i < 70; i++) {
        SendData(this->lut[i]);
    }
    Serial.println(" done.");
}

/**
 *  @brief: put an image buffer (RAM) to the frame memory.
 *          this won't update the display.
 */
void Epd::SetFrameMemory(
    const unsigned char* image_buffer,
    int x,
    int y,
    int image_width,
    int image_height
) {
    int x_end;
    int y_end;

    if (
        image_buffer == NULL ||
        x < 0 || image_width < 0 ||
        y < 0 || image_height < 0
    ) {
        return;
    }
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    x &= 0xF8;
    image_width &= 0xF8;
    if (x + image_width >= this->width) {
        x_end = this->width - 1;
    } else {
        x_end = x + image_width - 1;
    }
    if (y + image_height >= this->height) {
        y_end = this->height - 1;
    } else {
        y_end = y + image_height - 1;
    }
    SetMemoryArea(x, y, x_end, y_end);
    SetMemoryPointer(x, y);
    SendCommand(WRITE_RAM);
    /* send the image data */
    for (int j = 0; j < y_end - y + 1; j++) {
        for (int i = 0; i < (x_end - x + 1) / 8; i++) {
            SendData(image_buffer[i + j * (image_width / 8)]);
        }
    }
}

/**
 *  @brief: put an image buffer (IRAM) to the frame memory.
 *          this won't update the display.
 */
void Epd::SetFrameMemory(const unsigned char* image_buffer) {
    SetLut(lut_full_update);
    Serial.print("Writing frame memory");
    SetMemoryArea(0, 0, this->width - 1, this->height - 1);
    SetMemoryPointer(0, 0);
    SendCommand(WRITE_RAM);
    /* send the image data */
    for (int i = 0; i < this->width / 8 * this->height; i++) {
        SendData(pgm_read_byte(&image_buffer[i]));
    }
    Serial.println(" done.");
}

/**
 *  @brief: clear the frame memory with the specified color.
 *          this won't update the display.
 */
void Epd::ClearFrameMemory(unsigned char color) {
    SetLut(lut_full_update);
    Serial.print("Clearing frame memory");
    SetMemoryArea(0, 0, this->width - 1, this->height - 1);
    SetMemoryPointer(0, 0);
    SendCommand(WRITE_RAM);
    /* send the color data */
    for (int i = 0; i < this->width / 8 * this->height; i++) {
        SendData(color);
    }
    Serial.println(" done.");
}

/**
 *  @brief: update the display
 *          there are 2 memory areas embedded in the e-paper display
 *          but once this function is called,
 *          the the next action of SetFrameMemory or ClearFrame will 
 *          set the other memory area.
 */
void Epd::DisplayFrame(void) {

    Serial.print("Displaying frame");

    SendCommand(0x3A); // write number of overscan lines
    SendData(26);      // 26 dummy lines per gate
    SendCommand(0x3B); // write time to write every line
    SendData(0x08);    // 62us per line

    SendCommand(0x01);  // configure length of update
    SendData(0x27);     // y_len & 0xff
    SendData(0x01);     // y_len >> 8
    SendData(0x00);
    
    SendCommand(0x0f);  // configure starting-line of update
    SendData(0x00);     // y_start & 0xff
    SendData(0x00);     // y_start >> 8

    SendCommand(0x22);
                        // bitmapped enabled phases of the update: (in this order)
                        //   80 - enable clock signal
                        //   40 - enable CP
                        //   20 - load temperature value
                        //   10 - load LUT
                        //   08 - initial display
                        //   04 - pattern display
                        //   02 - disable CP
                        //   01 - disable clock signal
    SendData(0xC7);
    SendCommand(0x20);  // start update
    WaitUntilIdle();
    Serial.println(" done.");
}

/**
 *  @brief: private function to specify the memory area for data R/W
 */
void Epd::SetMemoryArea(int x_start, int y_start, int x_end, int y_end) {
    SendCommand(SET_RAM_X_ADDRESS_START_END_POSITION);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    SendData((x_start >> 3) & 0xFF);
    SendData((x_end >> 3) & 0xFF);
    SendCommand(SET_RAM_Y_ADDRESS_START_END_POSITION);
    SendData(y_start & 0xFF);
    SendData((y_start >> 8) & 0xFF);
    SendData(y_end & 0xFF);
    SendData((y_end >> 8) & 0xFF);
}

/**
 *  @brief: private function to specify the start point for data R/W
 */
void Epd::SetMemoryPointer(int x, int y) {
    SendCommand(SET_RAM_X_ADDRESS_COUNTER);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    SendData((x >> 3) & 0xFF);
    SendCommand(SET_RAM_Y_ADDRESS_COUNTER);
    SendData(y & 0xFF);
    SendData((y >> 8) & 0xFF);
}

/**
 *  @brief: After this command is transmitted, the chip would enter the 
 *          deep-sleep mode to save power. 
 *          The deep sleep mode would return to standby by hardware reset. 
 *          You can use Epd::Init() to awaken
 */
void Epd::Sleep() {
    SendCommand(DEEP_SLEEP_MODE);
    WaitUntilIdle();
}

const unsigned char lut_full_update[] =
{
    0x90, 0x50, 0xa0, 0x50, 0x50, 0x00, 0x00,
    0x00, 0x00, 0x10, 0xa0, 0xa0, 0x80, 0x00,
    0x90, 0x50, 0xa0, 0x50, 0x50, 0x00, 0x00,
    0x00, 0x00, 0x10, 0xa0, 0xa0, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x17, 0x04, 0x00, 0x00, 0x00,
    0x0b, 0x04, 0x00, 0x00, 0x00,
    0x06, 0x05, 0x00, 0x00, 0x00,
    0x04, 0x05, 0x00, 0x00, 0x00,
    0x01, 0x0e, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char lut_partial_update[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x20, 0xa0, 0x80, 0x00, 0x00, 0x00, 0x00,
    0x50, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x05, 0x00, 0x00, 0x00,
    0x01, 0x08, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00
};


/* END OF FILE */

/**
 *  @filename   :   epd2in9-demo.ino
 *  @brief      :   2.9inch e-paper display demo
 *  @author     :   Yehui from Waveshare
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

#include <SPI.h>
#include <epd2in9-badge.h>
#include <epdpaint.h>
#include "imagedata.h"

#define COLORED     0
#define UNCOLORED   1

/**
  * Due to RAM not enough in Arduino UNO, a frame buffer is not allowed.
  * In this case, a smaller image buffer is allocated and you have to 
  * update a partial display several times.
  * 1 byte = 8 pixels, therefore you have to set 8*N pixels at a time.
  */
unsigned char image[4736];
Paint paint(image, 0, 0);    // width should be the multiple of 8 
Epd epd;
unsigned long time_start_ms;
unsigned long time_now_s;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Application 'epd2in9-badge.ino' started");
  if (epd.Init() != 0) {
      Serial.println("Init of DEPG0290B01 failed");
      return;
  }
  paint.SetRotate(ROTATE_270);
  paint.SetWidth(128);
  paint.SetHeight(296);
  paint.Clear(UNCOLORED);
  epd.ClearFrameMemory(UNCOLORED);
}

void loop() {

  
  // White welcome screen
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(60, 40, "Hello world!", &Font24, COLORED);
  paint.DrawStringAt(90, 64, "MER 2018", &Font24, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
  epd.DisplayFrame();
  delay(3000);

  // Sample image
  epd.SetFrameMemory(IMAGE_DATA);
  epd.DisplayFrame();
  delay(3000);

    // Black screen
  paint.Clear(COLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
  epd.DisplayFrame();
  delay(3000);
}


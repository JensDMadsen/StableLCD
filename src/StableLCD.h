//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                      //
//  StableLCD - Arduino HD44780 LCD Library with focus on robustness and stability.                                                     //
//  Copyright (C) 2026 Jens Dyekjær Madsen                                                                                              //
//                                                                                                                                      //
//  Filename: StableLCD.h                                                                                                               //
//                                                                                                                                      //
//  Description:                                                                                                                        //
//    Arduino StableLCD wrapper implementation based on LiquidCrystalBase and the HD44780PIN GPIO backend.                              //
//    Combines the LiquidCrystalBase API layer with the HD44780PIN physical backend into a simple single-class HD44780 LCD interface    //
//    supporting both 4-bit and 8-bit parallel interfaces.                                                                              //
//                                                                                                                                      //
//  This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License    //
//  as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.             //
//                                                                                                                                      //
//  This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of      //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.                   //
//  You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the              //
//  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA                                            //
//                                                                                                                                      //
//  It is not allowed to change any license or copyright statements, but feel free to modify, change, and add your own copyrights       //
//  below this line only !                                                                                                              //
//  ----------------------------------------------------------------------------------------------------------------------------------  //
//                                                                                                                                      //
//  Tested on AVR architecture (Arduino Nano) but should work with any architecture.                                                    //
//                                                                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef StableLCD_h
#define StableLCD_h

#include "HD44780PIN.h"
#include "LiquidCrystalBase.h"

// ------------------------------------------------------------------------
// StableLCD top layer
// ------------------------------------------------------------------------

class StableLCD : public LiquidCrystalBase {
public:                                             // Constructors, configuration only:
  StableLCD(uint8_t rs,uint8_t rw,uint8_t ena,uint8_t d0,uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4,uint8_t d5,uint8_t d6,uint8_t d7,uint8_t pwr=0)    //   Setup 8 bit display, R/W
    :LiquidCrystalBase(false),lcd(rs,rw,ena,d0,d1,d2,d3,d4,d5,d6,d7,pwr) {}
  StableLCD(uint8_t rs,uint8_t rw,uint8_t ena,uint8_t d0,uint8_t d1,uint8_t d2,uint8_t d3,uint8_t pwr=0)                                                //   Setup 4 bit display, R/W
    :LiquidCrystalBase(true),lcd(rs,rw,ena,d0,d1,d2,d3,pwr) {}

  virtual uint8_t getAddress() override             { return lcd.readAC();            } // Read raw address counter (AC). Returns 0xff at timeout or if not initialized.
  virtual bool command(uint8_t value ) override     { return lcd.command(value);      } // Send command byte to LCD controller. Returns false on timeout.
  virtual bool writeData(uint8_t value) override    { return lcd.writeData(value);    } // Write data byte to DDRAM or CGRAM. Returns false on timeout.
  virtual uint8_t readData() override               { return lcd.readData();          } // Read data byte from DDRAM or CGRAM. Returns 0xff on error.

private:
  virtual bool enableLCD() override                 { return lcd.enable();            } // Enables LCD display and turns power on.
  virtual bool disableLCD() override                { return lcd.disable();           } // Disables LCD display and turns power off.
  virtual bool initLCD() override                   { return lcd.init();              } // Initialize LCD bus/controller. Returns false on timeout or if not enabled.

  HD44780PIN lcd;                                                                       // Include HD44780PIN with arduino GPIO interface.
};

#endif

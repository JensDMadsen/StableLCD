//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                      //
//  StableLCD - Arduino HD44780 LCD Library with focus on robustness and stability.                                                     //
//  Copyright (C) 2026 Jens Dyekjær Madsen                                                                                              //
//                                                                                                                                      //
//  Filename: HD44780PIN.h                                                                                                              //
//                                                                                                                                      //
//  Description:                                                                                                                        //
//    Arduino digital pin backend for HD44780PHY.                                                                                       //
//    Maps HD44780 RS, RW, Enable, and data bus signals to Arduino GPIO pins.                                                           //
//    Provides the physical pin interface used by HD44780PHY and supports both 4-bit and 8-bit parallel interfaces.                     //
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


#ifndef HD44780PIN_h
#define HD44780PIN_h

#include "HD44780PHY.h"

// ------------------------------------------------------------------------
// HD44780PIN physical GPIO backend
// ------------------------------------------------------------------------

class HD44780PIN : public HD44780PHY {
public:                                                                                                                                               // Constructors:
  HD44780PIN(uint8_t rs,uint8_t rw,uint8_t ena,uint8_t d0,uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4,uint8_t d5,uint8_t d6,uint8_t d7,uint8_t pwr=0) //   Setup 8 bit display, R/W
    :mode4bit(false),rs_pin(rs),rw_pin(rw), e_pin(ena),db_pin{d0,d1,d2,d3,d4,d5,d6,d7},pwr_pin(pwr) {}
  HD44780PIN(uint8_t rs,uint8_t rw,uint8_t ena,uint8_t d4,uint8_t d5,uint8_t d6,uint8_t d7,uint8_t pwr=0)                                             //   Setup 4 bit display, R/W
    :mode4bit(true),rs_pin(rs),rw_pin(rw), e_pin(ena),db_pin{255,255,255,255,d4,d5,d6,d7},pwr_pin(pwr) {}
protected:
  virtual void initPins() override;             // Initialize physical LCD pins.
  virtual bool is4bitMode() override;           // Return true for 4-bit interface, false for 8-bit.
  virtual void setEN(bool en) override;         // Control LCD Enable signal.
  virtual void setRW(bool rw) override;         // Control LCD Read/Write signal.
  virtual void setRS(bool rs) override;         // Control LCD Register Select signal.
  virtual void writeState() override;           // Set data bus direction to write mode.
  virtual void readState() override;            // Set data bus direction to read mode.
  virtual void writeBus(uint8_t val) override;  // Write value to LCD data bus. In 4-bit mode only DB7..DB4 are transferred.
  virtual uint8_t readBus() override;           // Read current value from LCD data bus. In 4-bit mode only DB7..DB4 are used and DB3..DB0 must return as zero.
  virtual bool readDB7() override;              // Optional optimized DB7 read. If not implemented readBus() bit 7 is used.
  virtual bool power(bool on) override;         // Optional display power control.
private:
  // Modes
  bool mode4bit;                                // Set true for 4-bit interface.
  // Pins
  uint8_t rs_pin;                               // RS-pin for HD44780. Low for command, and high for data/characters.
  uint8_t rw_pin;                               // RW-pin for HD44780. Low for write to LCD, and high for read from LCD.
  uint8_t e_pin;                                // E-pin for HD44780. Enable/Strobe pin, activated by high signal.
  uint8_t db_pin[8];                            // Data pins DB0..DB7 to HD44780. In 4-bit mode only DB7..DB4 are used.
  uint8_t pwr_pin;                              // Pin used for power, is 0 if not used.
};

#endif

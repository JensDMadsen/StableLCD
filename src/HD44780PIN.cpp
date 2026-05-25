//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                      //
//  StableLCD - Arduino HD44780 LCD Library with focus on robustness and stability.                                                     //
//  Copyright (C) 2026 Jens Dyekjær Madsen                                                                                              //
//                                                                                                                                      //
//  Filename: HD44780PIN.cpp                                                                                                            //
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


#include "HD44780PIN.h"

// ------------------------------------------------------------------------
// HD44780PIN physical GPIO backend
// ------------------------------------------------------------------------

void HD44780PIN::initPins() {                                   // Initialize physical LCD pins.
  setEN(false);
  setRW(false);
  setRS(false);
  pinMode(e_pin,  OUTPUT);
  pinMode(rw_pin, OUTPUT);
  pinMode(rs_pin, OUTPUT);
}

bool HD44780PIN::is4bitMode()   { return mode4bit;            } // Return true for 4-bit interface, false for 8-bit.
void HD44780PIN::setEN(bool en) { digitalWrite(e_pin, en);    } // Control LCD Enable signal.
void HD44780PIN::setRW(bool rw) { digitalWrite(rw_pin, rw);   } // Control LCD Read/Write signal.
void HD44780PIN::setRS(bool rs) { digitalWrite(rs_pin, rs);   } // Control LCD Register Select signal.

void HD44780PIN::writeState() {                                 // Set data bus direction to write mode.
  if (!mode4bit) {                                              // DB3..DB0 direction is only set in 8-bit mode:
    pinMode(db_pin[0], OUTPUT);                                 //   Set DB3..DB0 to OUTPUT in 8-bit mode.
    pinMode(db_pin[1], OUTPUT);
    pinMode(db_pin[2], OUTPUT);
    pinMode(db_pin[3], OUTPUT);
  }
  pinMode(db_pin[4], OUTPUT);                                   // Set DB7..DB4 to OUTPUT in all modes.
  pinMode(db_pin[5], OUTPUT);
  pinMode(db_pin[6], OUTPUT);
  pinMode(db_pin[7], OUTPUT);
}

void HD44780PIN::readState()  {                                 // Set data bus direction to read mode. In 4-bit mode only DB7..DB4 is set.
  if (!mode4bit) {                                              // DB3..DB0 direction is only set in 8-bit mode:
    pinMode(db_pin[0], INPUT_PULLUP);                           //   Set DB3..DB0 to INPUT_PULLUP in 8-bit mode.
    pinMode(db_pin[1], INPUT_PULLUP);
    pinMode(db_pin[2], INPUT_PULLUP);
    pinMode(db_pin[3], INPUT_PULLUP);
  }
  pinMode(db_pin[4], INPUT_PULLUP);                             // Set DB7..DB4 to INPUT_PULLUP in all modes.
  pinMode(db_pin[5], INPUT_PULLUP);
  pinMode(db_pin[6], INPUT_PULLUP);
  pinMode(db_pin[7], INPUT_PULLUP);
}

void HD44780PIN::writeBus(uint8_t val)  {                       // Write value to LCD data bus. In 4-bit mode only DB7..DB4 are transferred.
  if (!mode4bit) {                                              // DB3..DB0 is only transferred in 8-bit mode:
    digitalWrite(db_pin[0],((val&0x01)!=0));                    //   Send out DB3..DB0 in 8-bit mode.
    digitalWrite(db_pin[1],((val&0x02)!=0));
    digitalWrite(db_pin[2],((val&0x04)!=0));
    digitalWrite(db_pin[3],((val&0x08)!=0));
  }
  digitalWrite(db_pin[4],((val&0x10)!=0));                      // Send out DB7..DB4 in all modes.
  digitalWrite(db_pin[5],((val&0x20)!=0));
  digitalWrite(db_pin[6],((val&0x40)!=0));
  digitalWrite(db_pin[7],((val&0x80)!=0));
}

uint8_t HD44780PIN::readBus()  {                                // Read current value from LCD data bus. In 4-bit mode only DB7..DB4 are used.
  uint8_t val = 0;
  if (!mode4bit) {                                              // DB3..DB0 is only read in 8-bit mode:
    val = digitalRead(db_pin[0]) | (digitalRead(db_pin[1])<<1) | (digitalRead(db_pin[2])<<2) | (digitalRead(db_pin[3])<<3);
  }                                                             // DB7..DB4 is read in all modes:
  val|= (digitalRead(db_pin[4])<<4) | (digitalRead(db_pin[5])<<5) | (digitalRead(db_pin[6])<<6) | (digitalRead(db_pin[7])<<7);
  return val;
}

bool HD44780PIN::readDB7() {                                    // Optional optimized DB7 read. If not implemented readBus() bit 7 is used.
  return digitalRead(db_pin[7]);
}
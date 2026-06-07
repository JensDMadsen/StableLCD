//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                      //
//  StableLCD - Arduino HD44780 LCD Library with focus on robustness and stability.                                                     //
//  Copyright (C) 2026 Jens Dyekjær Madsen                                                                                              //
//                                                                                                                                      //
//  Filename: HD44780PHY.h                                                                                                              //
//                                                                                                                                      //
//  Description:                                                                                                                        //
//    Low-level HD44780 physical transport and protocol layer.                                                                          //
//    Handles bus direction, raw byte/nibble transfers, busy flag polling, timeouts, enable/disable state, optional power control,      //
//    and controller initialization/synchronization.                                                                                    //
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


#ifndef HD44780PHY_h
#define HD44780PHY_h

#include <Arduino.h>

// ------------------------------------------------------------------------
// Physical interface layer
//
// These functions must be implemented by the hardware specific backend.
// They provide the low level electrical interface used by the HD44780
// transport layer.
//
// The implementation may use individual GPIO pins, direct port access,
// external bus hardware, shift registers, FastIO libraries, etc.
// ------------------------------------------------------------------------

class HD44780PHY {
protected:
  static constexpr uint16_t TIMEOUT_MS = 200;                   // Set timeout in ms.
  virtual void busDelay() { delayMicroseconds(1); }             // Small bus timing delay.

  virtual void initPins() = 0;                                  // Initialize physical LCD pins.
  virtual bool is4bitMode() = 0;                                // Return true for 4-bit interface, false for 8-bit.
  virtual void setEN(bool high) = 0;                            // Control LCD Enable signal.
  virtual void setRW(bool high) = 0;                            // Control LCD Read/Write signal.
  virtual void setRS(bool high) = 0;                            // Control LCD Register Select signal.
  virtual void writeState() = 0;                                // Set data bus direction to write mode.
  virtual void readState() = 0;                                 // Set data bus direction to read mode.
  virtual void writeBus(uint8_t val) = 0;                       // Write value to LCD data bus. In 4-bit mode only DB7..DB4 are transferred.
  virtual uint8_t readBus() = 0;                                // Read current value from LCD data bus. In 4-bit mode only DB7..DB4 are used and DB3..DB0 must return as zero.
  virtual bool readDB7() { return (readBus() & 0x80) != 0; }    // Read DB7 signal state. Backends may override for faster busy polling.
  virtual void power(bool) { }                               // Called by enable() and disable().

public:
  // ------------------------------------------------------------------------
  // HD44780 transport / protocol layer
  // ------------------------------------------------------------------------

  bool enable();                                                // Enables display and turns power on.
  void disable();                                               // Disables display and turns power off.
  bool init();                                                  // Initialize LCD bus/controller. Returns false on timeout or if not enabled.
  bool readBusy();                                              // Read busy flag from LCD controller.
  bool waitBusy();                                              // Wait until busy flag clears. Returns false on timeout or if not initialized.
  bool command(uint8_t cmd);                                    // Send command byte to LCD controller. Returns false on timeout or if not initialized.
  bool writeData(uint8_t data);                                 // Write data byte to DDRAM or CGRAM. Returns false on timeout or if not initialized.
  uint8_t readStatus();                                         // Read status byte from LCD controller.
  uint8_t readAC();                                             // Read address counter (AC). Returns 0xff at timeout or if not initialized.
  uint8_t readData();                                           // Read data byte from DDRAM or CGRAM. Returns 0xff at timeout or if not initialized.

private:
  void enterWriteMode();                                        // Set up data bus as output for LCD write operations.
  void enterReadMode();                                         // Set up data bus as input for LCD read operations.
  void writeRaw(uint8_t v);                                     // Transfer raw byte or nibble to LCD controller.
  uint8_t readRaw();                                            // Read raw byte or nibble from LCD controller.
  bool readRawDB7();                                            // Read raw DB7 from LCD controller.
  bool writeByte(uint8_t rs, uint8_t v);                        // Transfer complete byte to LCD controller. rs=status/data. Returns false on timeout.
  uint8_t readByte(uint8_t rs, bool wait);                      // Read complete byte from LCD controller. rs=status/data. If wait: returns 0xff at timeout.
  bool busIsReadMode = false;                                   // Remember current bus direction.
  bool enabled = false;                                         // True when display is enabled.
  bool initialized = false;                                     // True when display is initialized and ready for use.
};

#endif

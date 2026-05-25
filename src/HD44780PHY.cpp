//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                      //
//  StableLCD - Arduino HD44780 LCD Library with focus on robustness and stability.                                                     //
//  Copyright (C) 2026 Jens Dyekjær Madsen                                                                                              //
//                                                                                                                                      //
//  Filename: HD44780PHY.cpp                                                                                                            //
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


#include "HD44780PHY.h"

// ------------------------------------------------------------------------
// Physical interface layer
// ------------------------------------------------------------------------

bool HD44780PHY::enable() {                             // Enables display and turns power on.
  power(true);                                          // Turn power on.
  enabled = true;                                       // Display is now enabled and ready to be initialized.
  initialized = false;                                  // Display is not yet initialized.
  return true;                                          // Return always true.
}

void HD44780PHY::disable() {                            // Disables display and turns power off.
  enabled = false;                                      // Display is disabled.
  initialized = false;                                  // Display not initialized.
  power(false);                                         // Turn power off.
}

bool HD44780PHY::init() {                               // Initialize LCD bus/controller. Returns false on timeout or if not enabled.
  if (!enabled) return false;                           // Display needs to be enabled to be initialized.
  const uint32_t start = millis();                      // Store start time for timeout.
  initPins();                                           // Make sure pins are in correct state.
  busIsReadMode = false;                                // Force enterReadMode() to set up read mode.
  enterReadMode();                                      // Make sure pins are in read mode.
  setRS(false);                                         // Select status/command register.
  bool second;                                          // Stores second DB7 candidate.
  do {                                                  // Wait until not busy.
    do {                                                //   Loop until busy candidate is low.
      if ((millis()-start) > TIMEOUT_MS) return false;  //     Return false and exit on timeout.
    } while (readRawDB7());                             //   Wait until DB7 candidate is low.
    second = readRawDB7();                              //   Read second DB7 candidate.
    writeRaw(0x30); writeRaw(0x30); writeRaw(0x30);     //   Force known 8-bit state: Sends raw 3 x 0x3 when 4-bit interface, or 3 x 0x30 at 8-bit interface.
  } while (second);                                     // Retry on possible 0,1 nibble phase mismatch. Leave on stable 0,0.
  if (is4bitMode()) {                                   // If 4-bit interface:
    writeRaw(0x20);                                     //   Set display to 4-bit interface.
  }
  initialized = true;                                   // Display is initialized.
  return true;                                          // init() finished, return true.
}

bool HD44780PHY::readBusy() {                           // Read busy flag from LCD controller.
  if (!initialized) return false;                       // Return false if not initialized.
  setRS(false);                                         // Select status/command.
  if (is4bitMode()) {                                   // If 4 bit:
    bool busy = readRawDB7();                           //   Read hi nibble DB7 (busy).
    readRawDB7();                                       //   Read lo nibble DB7 and ignore.
    return busy;                                        //   Return busy flag.
  }                                                     // If 8 bit:
  return readRawDB7();                                  //   Return busy flag.
}

bool HD44780PHY::waitBusy() {                           // Wait until busy flag clears. Returns false on timeout.
  if (!initialized) return false;                       // Return false if not initialized.
  const uint32_t start = millis();                      // Store start time for timeout.
  while (readBusy()) {                                  // Loop on busy:
    if ((millis()-start) > TIMEOUT_MS) return false;    //   Return false and exit on timeout.
  }
  return true;                                          // Return true if no busy and no timeout.
}

bool HD44780PHY::command(uint8_t cmd)   { return writeByte(0, cmd);  }      // Send command byte to LCD controller. Returns false on timeout.
bool HD44780PHY::writeData(uint8_t d)   { return writeByte(1, d);    }      // Write data byte to DDRAM or CGRAM. Returns false on timeout.
uint8_t HD44780PHY::readStatus()        { return readByte(0, false); }      // Read status byte from LCD controller.
uint8_t HD44780PHY::readAC()            { return readByte(0, true);  }      // Read address counter (AC). Returns 0xff at timeout.
uint8_t HD44780PHY::readData()          { return readByte(1, true);  }      // Read data byte from DDRAM or CGRAM. Returns 0xff at timeout.

// Helper functions

bool HD44780PHY::writeByte(uint8_t rs, uint8_t v) {     // Transfer complete byte to LCD controller. rs=status/data. Returns false on timeout.
  if (!initialized) return false;                       // Return false if not initialized.
  if (!waitBusy()) return false;                        // Wait on no busy, return false if timeout.
  setRS(rs);                                            // Select command/data.
  if (is4bitMode()) {                                   // If 4 bit:
    writeRaw(v);                                        //   Output hi nibble (DB7..DB4).
    writeRaw(v << 4);                                   //   Output lo nibble (DB3..DB0).
  } else {                                              // If 8 bit:
    writeRaw(v);                                        //   Output byte (DB7..DB0).
  }
  return true;                                          // Data transfer complete.
}

uint8_t HD44780PHY::readByte(uint8_t rs, bool wait) {   // Read complete byte from LCD controller. rs=status/data. If wait: returns 0xff at timeout.
  if (!initialized) return 0xff;                        // Return 0xff if not initialized.
  if (wait) {                                           // If wait is set:
    if (!waitBusy()) return 0xff;                       //   Wait on not busy, and return 0xff if timeout.
  }
  setRS(rs);                                            // Select status/data.
  if (is4bitMode()) {                                   // If 4 bit:
    uint8_t hi = readRaw();                             //   Read hi nibble (DB7..DB4).
    uint8_t lo = readRaw();                             //   Read lo nibble (DB3..DB0).
    return hi | (lo >> 4);                              //   Return full byte.
  }                                                     // If 8 bit:
  return readRaw();                                     //   Return byte.
}

void HD44780PHY::enterWriteMode() {                     // Set up data bus as output for LCD write operations.
  if (!busIsReadMode) return;                           // Return if already write mode.
  setRW(false);                                         // LCD releases data bus.
  busDelay();                                           // Wait before enabling MCU outputs.
  writeState();                                         // MCU may now drive data bus.
  busIsReadMode = false;                                // Remember current bus direction (write).
}

void HD44780PHY::enterReadMode() {                      // Set up data bus as input for LCD read operations.
  if (busIsReadMode) return;                            // Return if already read mode.
  readState();                                          // Release MCU data bus outputs and set to input.
  setRW(true);                                          // LCD may drive data bus now.
  busIsReadMode = true;                                 // Remember current bus direction (read).
}

void HD44780PHY::writeRaw(uint8_t v) {                  // Transfer raw byte or nibble to LCD controller.
  enterWriteMode();                                     // Set up data bus as output for LCD write operations.
  writeBus(v);                                          // Put data on LCD data bus.
  busDelay();                                           // Setup time before strobe.
  setEN(true);                                          // Enable LCD data strobe to latch data into LCD.
  busDelay();                                           // Strobe pulse width.
  setEN(false);                                         // Release LCD data strobe.
  busDelay();                                           // Hold time after write cycle.
}

uint8_t HD44780PHY::readRaw() {                         // Read raw byte or nibble from LCD controller.
  enterReadMode();                                      // Set up data bus as input for LCD read operations.
  busDelay();                                           // Setup time before strobe.
  setEN(true);                                          // Enable LCD data strobe.
  busDelay();                                           // Wait for data to settle.
  uint8_t val = readBus();                              // Sample LCD data bus.
  setEN(false);                                         // Release LCD data strobe.
  busDelay();                                           // Hold time after read cycle.
  return val;                                           // Return value.
}

bool HD44780PHY::readRawDB7() {                         // Read raw DB7 from LCD controller.
  enterReadMode();                                      // Set up data bus as input for LCD read operations.
  busDelay();                                           // Setup time before strobe.
  setEN(true);                                          // Enable LCD data strobe.
  busDelay();                                           // Wait for data to settle.
  bool b = readDB7();                                   // Sample DB7 from LCD data bus.
  setEN(false);                                         // Release LCD data strobe.
  busDelay();                                           // Hold time after read cycle.
  return b;                                             // Return boolean value.
}
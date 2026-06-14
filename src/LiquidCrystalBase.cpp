//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                      //
//  StableLCD - Arduino HD44780 LCD Library with focus on robustness and stability.                                                     //
//  Copyright (C) 2026 Jens Dyekjær Madsen                                                                                              //
//                                                                                                                                      //
//  Filename: LiquidCrystalBase.cpp                                                                                                     //
//                                                                                                                                      //
//  Description:                                                                                                                        //
//    Arduino LiquidCrystal compatible base class for HD44780 displays.                                                                 //
//    Provides the user API, Print integration, cursor/display control, state restore, clear/initClear handling, and display            //
//    verification support. Hardware access is provided by derived classes.                                                             //
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


#include "LiquidCrystalBase.h"

// ------------------------------------------------------------------------
// HD44780 LiquidCrystal interface layer
// ------------------------------------------------------------------------

bool LiquidCrystalBase::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {           // Actual initialization: Configure, initialize and clear display.
  if (!enableLCD()) return false;                                                       // Enable LCD, return false on error.
                                                                                        // Enter normal write mode:
  verifyClr();                                                                          //   Clear verify errors.
  verifyEnd();                                                                          //   Leave verify mode.

  numlines = lines;                                                                     // Store number of lines.

  lcd_functionset &= ~(LCD_2LINE | LCD_5x10DOTS);                                       // Default to LCD_1LINE and LCD_5x8DOTS.
  if ((dotsize != LCD_5x8DOTS) && (lines == 1)) lcd_functionset |= LCD_5x10DOTS;        // Setup font.
  if (lines > 1) lcd_functionset |= LCD_2LINE;                                          // Setup 2 lines if more than one line.

  setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);                                  // Set offsets.

  lcd_displaycontrol = LCD_DISPLAYCONTROL_INIT;                                         // Setup lcd_displaycontrol init value.
  lcd_entrymodeset   = LCD_ENTRYMODESET_INIT;                                           // Setup lcd_entrymodeset init value.

  if (initClear()) return true;                                                         // Initialize and clear the display.
                                                                                        // If error:
  disableLCD();                                                                         //   Disable LCD
  return false;                                                                         //   Return error
}

uint8_t LiquidCrystalBase::getLine() {                                                  // Return logical row from current DDRAM address.
  uint8_t ac = getAddress();                                                            // Read current DDRAM address.
  if (ac == 0xff) return 0xff;                                                          // Return error if address read failed.
  for (uint8_t row=0; row<numlines; row++) {                                            // Search configured logical rows.
    if ((ac >= row_offsets[row]) &&                                                     // AC inside logical row:
        (ac < (row_offsets[row] + getCols()))) {
      return row;                                                                       // Return matching logical row.
    }
  }
  return 0xff;                                                                          // AC outside configured logical rows.
}

bool LiquidCrystalBase::setCursor(uint8_t col, uint8_t row) {                           // Set cursor position.
  const size_t max_lines = sizeof(row_offsets) / sizeof(*row_offsets);                  // Get lines from array size.
  if ( row >= max_lines ) row = max_lines - 1;                                          // Handle limits.
  if ( row >= numlines )  row = numlines - 1;                                           // Handle limits.
  return command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

bool LiquidCrystalBase::createChar(uint8_t location, uint8_t charmap[]) {               // Create CGRAM Custom characters.
  location &= 0x7;                                                                      // We only have 8 locations 0-7.
  if (!command(LCD_SETCGRAMADDR | (location << 3))) return false;                       // Setup CGRAM address.
  for (int i=0; i<8; i++) {
    if (!write(charmap[i])) return false;                                               // Write/verify charmap to CGRAM.
  }
  return true;                                                                          // Return true if ok, false on error.
}

size_t LiquidCrystalBase::write(uint8_t value) {                                        // Write or verify one data byte.
  if (!verifyMode) {                                                                    // If normal write mode:
    return writeData(value) ? 1 : 0;                                                    //   Write data and return bytes written.
  }                                                                                     // If verify mode:
  if (readData() != value) {                                                            //   If mismatch:
    verifyOkFlag = false;                                                               //     Store verify error.
    return 0;                                                                           //     Return 0 if mismatch.
  }
  return 1;                                                                             //   Return 1 if verified.
}

static inline bool isTerminator(char c, const char *t) {        						// Return true if c matches one of the terminators.
  while (*t) {                                                  						// Loop through zero-terminated terminator string.
    if (c == *t++) return true;                                 						// Return true on match.
  }						
  return false;                                                 						// No terminator matched.
}

size_t LiquidCrystalBase::readUntil(char *buf, size_t size, const char *terminators, bool trimLeft, bool trimRight) {
  if (size-- == 0) return 0;                                    						// No room for terminator.
  size_t n = 0;                                                 						// Number of chars stored.
  uint8_t spaces = 0;                                           						// Pending spaces.
  buf[0] = 0;                                                   						// Always return valid string.
						
  uint8_t pos = getPos();                                       						// Read current logical column.
  uint8_t cols = getCols();                                     						// Read logical line width.
  if (pos == 255) return 0;                                     						// Return empty string if position cannot be read.
						
  for (uint8_t i = pos; i < cols; i++) {                        						// Read until logical end of line.
    uint8_t value = readData();                                 						// Read next LCD character.
    if (value == 0xff) break;                                   						// Stop on read error.
						
    char c = (char)value;                                       						// Convert value to character.
    if (trimLeft && c == ' ') continue;                         						// Skip leading spaces before terminator check.
    trimLeft = false;                                           						// Leading trim is done.
						
    if (isTerminator(c, terminators)) break;                    						// Stop on terminator.
						
    if (c == ' ') {                                             						// Space:
      spaces++;                                                 						//   Store as pending.
      continue;                                                 						//   Do not write yet.
    }						
						
    while (spaces && (n < size)) { spaces--; buf[n++] = ' '; }  						// Flush internal spaces.
    spaces = 0;                                                 						// Clear pending spaces.
    if (n < size) buf[n++] = c;                                 						// Store character if room.
  }						
						
  if (!trimRight) {                                             						// If trailing spaces must be kept:
    while (spaces && (n < size)) { spaces--; buf[n++] = ' '; }  						//   Flush pending trailing spaces.
  }						
						
  buf[n] = 0;                                                   						// Zero terminate string.
  return n;                                                     						// Return number of stored chars.
}						
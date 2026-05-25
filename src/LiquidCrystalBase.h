//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                      //
//  StableLCD - Arduino HD44780 LCD Library with focus on robustness and stability.                                                     //
//  Copyright (C) 2026 Jens Dyekjær Madsen                                                                                              //
//                                                                                                                                      //
//  Filename: LiquidCrystalBase.h                                                                                                       //
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


#ifndef LiquidCrystalBase_h
#define LiquidCrystalBase_h

#include <Arduino.h>
#include <Print.h>

// ------------------------------------------------------------------------
// HD44780 LiquidCrystal interface layer
// ------------------------------------------------------------------------

class LiquidCrystalBase : public Print {
protected:
  // commands
  static constexpr uint8_t LCD_CLEARDISPLAY     = 0x01;
  static constexpr uint8_t LCD_RETURNHOME       = 0x02;
  static constexpr uint8_t LCD_ENTRYMODESET     = 0x04;
  static constexpr uint8_t LCD_DISPLAYCONTROL   = 0x08;
  static constexpr uint8_t LCD_CURSORSHIFT      = 0x10;
  static constexpr uint8_t LCD_FUNCTIONSET      = 0x20;
  static constexpr uint8_t LCD_SETCGRAMADDR     = 0x40;
  static constexpr uint8_t LCD_SETDDRAMADDR     = 0x80;

  // flags for display entry mode
  static constexpr uint8_t LCD_ENTRYRIGHT       = 0x00;
  static constexpr uint8_t LCD_ENTRYLEFT        = 0x02;
  static constexpr uint8_t LCD_ENTRYSHIFTINCREMENT = 0x01;
  static constexpr uint8_t LCD_ENTRYSHIFTDECREMENT = 0x00;

  // flags for display on/off control
  static constexpr uint8_t LCD_DISPLAYON        = 0x04;
  static constexpr uint8_t LCD_DISPLAYOFF       = 0x00;
  static constexpr uint8_t LCD_CURSORON         = 0x02;
  static constexpr uint8_t LCD_CURSOROFF        = 0x00;
  static constexpr uint8_t LCD_BLINKON          = 0x01;
  static constexpr uint8_t LCD_BLINKOFF         = 0x00;

  // flags for display/cursor shift
  static constexpr uint8_t LCD_DISPLAYMOVE      = 0x08;
  static constexpr uint8_t LCD_CURSORMOVE       = 0x00;
  static constexpr uint8_t LCD_MOVERIGHT        = 0x04;
  static constexpr uint8_t LCD_MOVELEFT         = 0x00;

  // flags for function set
  static constexpr uint8_t LCD_8BITMODE         = 0x10;
  static constexpr uint8_t LCD_4BITMODE         = 0x00;
  static constexpr uint8_t LCD_2LINE            = 0x08;
  static constexpr uint8_t LCD_1LINE            = 0x00;
  static constexpr uint8_t LCD_5x10DOTS         = 0x04;
  static constexpr uint8_t LCD_5x8DOTS          = 0x00;

  // Setup init values
  static constexpr uint8_t LCD_4BIT_INIT                = LCD_FUNCTIONSET | LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;             // lcd_functionset for 4 bit mode
  static constexpr uint8_t LCD_8BIT_INIT                = LCD_FUNCTIONSET | LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;             // lcd_functionset for 8 bit mode
  static constexpr uint8_t LCD_DISPLAYCONTROL_INIT      = LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;    // lcd_displaycontrol init value
  static constexpr uint8_t LCD_ENTRYMODESET_INIT        = LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;           // lcd_entrymodeset init value

public:
  LiquidCrystalBase(bool mode4bit) : lcd_functionset( mode4bit? LCD_4BIT_INIT : LCD_8BIT_INIT ) { }             // Constructor, configuration only. Sets lcd_functionset to interface width.

  bool begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);                                       // Actual initialization: Configure, initialize and clear display.
  void end()                    { disableLCD();                                                               } // Stop display.

  bool noDisplay()              { return command( lcd_displaycontrol &= ~LCD_DISPLAYON );                     } // Turn the display off (quickly)
  bool display()                { return command( lcd_displaycontrol |= LCD_DISPLAYON );                      } // Turn the display on (quickly)
  bool noBlink()                { return command( lcd_displaycontrol &= ~LCD_BLINKON );                       } // Turn off the blinking cursor
  bool blink()                  { return command( lcd_displaycontrol |= LCD_BLINKON );                        } // Turn on the blinking cursor
  bool noCursor()               { return command( lcd_displaycontrol &= ~LCD_CURSORON );                      } // Turns the underline cursor off
  bool cursor()                 { return command( lcd_displaycontrol |= LCD_CURSORON );                       } // Turns the underline cursor on
  bool leftToRight()            { return command( lcd_entrymodeset   |= LCD_ENTRYLEFT );                      } // This is for text that flows Left to Right
  bool rightToLeft()            { return command( lcd_entrymodeset   &= ~LCD_ENTRYLEFT );                     } // This is for text that flows Right to Left
  bool autoscroll()             { return command( lcd_entrymodeset   |= LCD_ENTRYSHIFTINCREMENT );            } // This will 'right justify' text from the cursor
  bool noAutoscroll()           { return command( lcd_entrymodeset   &= ~LCD_ENTRYSHIFTINCREMENT );           } // This will 'left justify' text from the cursor
  bool scrollDisplayLeft()      { return command( LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT );         } // These commands scroll the display without changing the RAM
  bool scrollDisplayRight()     { return command( LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT );        } // Scroll move right
  bool home()                   { return command( LCD_RETURNHOME );                                           } // Home
  bool clear()      { return command(LCD_CLEARDISPLAY) && command(lcd_entrymodeset);                          } // Clear
  bool initClear()  { return initLCD() && command(lcd_functionset) && command(lcd_displaycontrol) && clear(); } // Init, restore LCD state, and clear display. Not intended for fast refresh loops.

  void setRowOffsets(int row0, int row1, int row2, int row3) { row_offsets[0]=row0; row_offsets[1]=row1; row_offsets[2]=row2; row_offsets[3]=row3;    } // Set rows
  bool setCursor(uint8_t, uint8_t);                                                                             // Set cursor position
  bool createChar(uint8_t, uint8_t[]);                                                                          // Create CGRAM Custom characters

  void verifyBegin()            { verifyMode = true; verifyClr();                                             } // Enable verify mode and clear verify status.
  void verifyEnd()              { verifyMode = false;                                                         } // Disable verify mode.
  void verifyClr()              { verifyOkFlag = true;                                                        } // Clear verify error status.
  bool verifyOk()               { return verifyOkFlag;                                                        } // Return true if verify is still ok.

  virtual bool command(uint8_t value ) = 0;                     // Send command byte to LCD controller. Returns false on timeout.
  virtual bool writeData(uint8_t value) = 0;                    // Write data byte to DDRAM or CGRAM. Returns false on timeout.
  virtual uint8_t readData() = 0;                               // Read data byte from DDRAM or CGRAM. Returns 0xff on error.
  virtual bool enableLCD() = 0;                                 // Enables LCD display and turns power on.
  virtual void disableLCD() = 0;                                // Disables LCD display and turns power off.
  virtual bool initLCD() = 0;                                   // Initialize LCD bus/controller. Returns false on timeout or if not enabled.
  virtual size_t write(uint8_t value) override;                 // Setup Print::write to send to writeData().
  using Print::write;

private:
// Modes
  uint8_t lcd_displaycontrol = LCD_DISPLAYCONTROL_INIT;         // Stores displaycontrol commands, init to LCD_DISPLAYCONTROL
  uint8_t lcd_entrymodeset   = LCD_ENTRYMODESET_INIT;           // Stores entrymodeset commands, init to LCD_ENTRYMODESET
  uint8_t lcd_functionset    = LCD_4BIT_INIT;                   // Stores functionset commands, init to LCD_4BIT_INIT
// Lines
  uint8_t numlines;                                             // Stores number of lines on display
  uint8_t row_offsets[4];                                       // Stores offsets for display
// Verify
  bool verifyMode = false;                                      // True if write() verifies instead of writing.
  bool verifyOkFlag = true;                                     // True while all verified bytes match.
};

#endif

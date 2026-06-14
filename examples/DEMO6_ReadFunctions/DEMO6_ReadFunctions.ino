// This example is for use with StableLCD arduino library by Jens D. Madsen

// This example shows use of position, address, and read functions with StableLCD.

#include <StableLCD.h>                                                              // Include the library code

// Initialize pins
const int rs = 6, rw = 7, en = 8;                                                   // Control pins
const int d4 = 9, d5 = 10, d6 = 11, d7 = 12;                                        // Data bus pins
const int pwr = 5;                                                                  // Power is connected to pin 5

StableLCD lcd(rs, rw, en, d4, d5, d6, d7, pwr);                                     // Use StableLCD with power control

char buffer[81];                                                                    // Space for 80 chars + zero terminator.

void printState() {                                                                 // Print current LCD address and logical position.
  Serial.print(F("getAddress()= 0x")); Serial.println(lcd.getAddress(),HEX);        // Report address
  Serial.print(F("getLine()   = ")); Serial.println(lcd.getLine());                 // Report logical line
  Serial.print(F("getPos()    = ")); Serial.println(lcd.getPos());                  // Report logical position
}

void printBuffer() {                                                                // Print read buffer with quotes.
  Serial.print(F("buffer      = \"")); Serial.print(buffer); Serial.println("\"");
}

void setup() {                                                                      // Setup
  Serial.begin(115200);                                                             // Usart to 115200 baud
  while (!Serial) { }                                                               // Wait on Serial
  Serial.println();                                                                 // Extra space
  Serial.println(F("Test of StableLCD read functions"));                               // Text to Serial
  Serial.println();                                                                 // Extra space

  Serial.println(F("LCD begin/print:"));
  Serial.println(F("line 0:       \"StableLCD tests\""));
  Serial.println(F("line 1:       \"read functions \""));

  Serial.println();                                                                 // Extra space
  Serial.println(F("LCD setup:    lcd.begin(16,2)"));
  lcd.begin(16,2);                                                                  // Set up columns & rows and turn LCD on
  Serial.print(F("getRows()   = ")); Serial.println(lcd.getRows());                 // Report Rows
  Serial.print(F("getCols()   = ")); Serial.println(lcd.getCols());                 // Report Cols
  printState();                                                                     // Report address and logical cursor position

  Serial.println();                                                                 // Extra Space
  Serial.println(F("LCD print:    lcd.print(\"StableLCD tests\")"));
  lcd.print(F("StableLCD tests"));                                                  // Output text on LCD display
  printState();                                                                     // Report address and logical cursor position

  Serial.println();                                                                 // Extra Space
  Serial.println(F("LCD setup:    lcd.setCursor(0,1)"));
  Serial.println(F("LCD print:    lcd.print(\"read functions\")"));
  lcd.setCursor(0,1);                                                               // Set cursor on first position, line 2
  lcd.print(F("read functions"));                                                   // Output text on LCD display
  printState();                                                                     // Report address and logical cursor position
  uint8_t saveCursor = lcd.getAddress();                                            // Save cursor
  Serial.print(F("saveCursor  = 0x")); Serial.println(saveCursor,HEX);              // Report cursor saved address

  Serial.println();                                                                 // Extra Space
  Serial.println(F("LCD setup:    lcd.setCursor(0,9)"));                            // This should not be possible (line 9)
  lcd.setCursor(0,9);                                                               // Set cursor, row is clamped to last line
  printState();                                                                     // Report address and logical cursor position

  // setCursor() does not clamp columns.
  // Invalid DDRAM positions therefore return 255.
  Serial.println();                                                                 // Extra Space
  Serial.println(F("LCD setup:    lcd.setCursor(99,0)"));                           // This should not be possible (col 99)
  lcd.setCursor(99,0);                                                              // Set cursor outside logical display width
  printState();                                                                     // Report address and logical cursor position

  Serial.println();                                                                 // Extra Space
  Serial.println(F("Restore addr: lcd.setAddress(saveCursor)"));                    // Restore cursor test
  Serial.print(F("saveAddress = 0x")); Serial.println(saveCursor,HEX);              // Report saved cursor address
  lcd.setAddress(saveCursor);                                                       // Restore cursor address
  printState();                                                                     // Report address and logical cursor position

  // Read functions on first LCD screen:
  //            "StableLCD tests "
  //            "read functions  "

  Serial.println();                                                                 // Extra Space
  Serial.println(F("- readLine() -"));                                              // Text
  Serial.println(F("Function:     Read line, trailing spaces are removed"));        // Text
  Serial.println(F("LCD setup:    lcd.setCursor(0,1)"));                            // Text
  lcd.setCursor(0,1);                                                               // Set cursor on first position, line 2
  Serial.println(F("LCD read:     lcd.readLine(buffer,sizeof(buffer))"));           // Text
  lcd.readLine(buffer,sizeof(buffer));                                              // Read from first position line 2
  printBuffer();                                                                    // Report buffer

  Serial.println();                                                                 // Extra Space
  Serial.println(F("- readUntil() -"));                                             // Text
  Serial.println(F("Function:     Read complete line including spaces."));          // Text
  Serial.println(F("LCD setup:    lcd.setCursor(0,0)"));                            // Text
  lcd.setCursor(0,0);                                                               // Start of line 0
  Serial.println(F("LCD read:     lcd.readUntil(buffer,sizeof(buffer))"));
  lcd.readUntil(buffer,sizeof(buffer));                                             // Read to EOL, no terminator
  printBuffer();                                                                    // Report buffer

  Serial.println();                                                                 // Extra Space
  Serial.println(F("- read() -"));                                                  // Text
  Serial.println(F("Function:     Read tokens, leading and trailing spaces are removed."));
  Serial.println(F("LCD setup:    lcd.setCursor(0,1)"));                            // Text
  lcd.setCursor(0,1);                                                               // Set cursor on first position, line 2
  Serial.println(F("LCD read:     lcd.read(buffer,sizeof(buffer))"));               // Text
  lcd.read(buffer,sizeof(buffer));                                                  // Read first token
  printBuffer();                                                                    // Report buffer
  Serial.println(F("LCD read:     lcd.read(buffer,sizeof(buffer))"));               // Text
  lcd.read(buffer,sizeof(buffer));                                                  // Read next token, continues from current AC
  printBuffer();                                                                    // Report buffer

  delay(3000);                                                                      // Wait 2 seconds before showing new screen

  // New LCD screen for separator and terminator tests.
  Serial.println();                                                                 // Extra Space
  Serial.println(F("LCD clear/print:"));
  Serial.println(F("line 0:       \" TEMP := 24.5\""));
  Serial.println(F("line 1:       \"   KEY:=VALUE\""));

  lcd.clear();                                                                      // Clear LCD for parser test text
  lcd.print(F(" TEMP := 24.5"));                                                    // Text with space and punctuation separators
  lcd.setCursor(0,1);                                                               // Second line
  lcd.print(F("   KEY:=VALUE"));                                                    // Text with consecutive separators

  Serial.println();                                                                 // Extra Space
  Serial.println(F("- readUntil('=') -"));                                          // Text
  Serial.println(F("Function:     Read until terminator, no trim."));               // Text
  Serial.println(F("LCD setup:    lcd.setCursor(0,0)"));                            // Text
  lcd.setCursor(0,0);                                                               // Start of line 0
  Serial.println(F("LCD read:     lcd.readUntil(buffer,sizeof(buffer),'=')"));
  lcd.readUntil(buffer,sizeof(buffer),'=');                                         // Read key before =
  printBuffer();                                                                    // Report buffer

  Serial.println();                                                                 // Extra Space
  Serial.println(F("- read(':') -"));                                               // Text
  Serial.println(F("Function:     Read token using ':' as separator."));            // Text
  Serial.println(F("LCD setup:    lcd.setCursor(0,1)"));                            // Text
  lcd.setCursor(0,1);                                                               // Start of line 1
  Serial.println(F("LCD read:     lcd.read(buffer,sizeof(buffer),':')"));           // Text
  lcd.read(buffer,sizeof(buffer),':');                                              // Read token before :
  printBuffer();                                                                    // Report buffer
  Serial.println(F("LCD read:     lcd.read(buffer,sizeof(buffer),'=')"));           // Text
  lcd.read(buffer,sizeof(buffer),'=');                                              // Read next token before =
  printBuffer();                                                                    // Report buffer
  Serial.println(F("LCD read:     lcd.read(buffer,sizeof(buffer))"));               // Text
  lcd.read(buffer,sizeof(buffer));                                                  // Read next token using default space separator
  printBuffer();                                                                    // Report buffer

  Serial.println();                                                                 // Extra Space
  Serial.println(F("- read(\":=\") -"));                                            // Text
  Serial.println(F("Function:     Multiple separators are individual characters."));
  Serial.println(F("LCD setup:    lcd.setCursor(0,1)"));                            // Text
  lcd.setCursor(0,1);                                                               // Start of line 1
  Serial.println(F("LCD read:     lcd.read(buffer,sizeof(buffer),\":=\")"));        // Text
  lcd.read(buffer,sizeof(buffer),":=");                                             // Read token before :
  printBuffer();                                                                    // Report buffer
  Serial.println(F("LCD read:     lcd.read(buffer,sizeof(buffer),\":=\")"));        // Text
  lcd.read(buffer,sizeof(buffer),":=");                                             // Next char is =, therefore empty token
  printBuffer();                                                                    // Report buffer
  Serial.println(F("LCD read:     lcd.read(buffer,sizeof(buffer),\":=\")"));        // Text
  lcd.read(buffer,sizeof(buffer),":=");                                             // Read value after =
  printBuffer();                                                                    // Report buffer

  Serial.println();                                                                 // Extra Space
  Serial.println(F("- read(\" :=\") -"));                                           // Text
  Serial.println(F("Function:     Multiple separators with space."));
  Serial.println(F("LCD setup:    lcd.setCursor(0,1)"));                            // Text
  lcd.setCursor(0,0);                                                               // Start of line 0
  Serial.println(F("LCD read:     lcd.read(buffer,sizeof(buffer),\" :=\")"));       // Text
  lcd.read(buffer,sizeof(buffer)," :=");                                            // Read token before :
  printBuffer();                                                                    // Report buffer
  Serial.println(F("LCD read:     lcd.read(buffer,sizeof(buffer),\" :=\")"));       // Text
  lcd.read(buffer,sizeof(buffer)," :=");                                            // Next char is =, therefore empty token
  printBuffer();                                                                    // Report buffer
  Serial.println(F("LCD read:     lcd.read(buffer,sizeof(buffer),\" :=\")"));       // Text
  lcd.read(buffer,sizeof(buffer)," :=");                                            // Read value after =
  printBuffer();                                                                    // Report buffer
  Serial.println(F("LCD read:     lcd.read(buffer,sizeof(buffer),\" :=\")"));       // Text
  lcd.read(buffer,sizeof(buffer)," :=");                                            // Next char is =, therefore empty token
  printBuffer();                                                                    // Report buffer
  Serial.println(F("LCD read:     lcd.read(buffer,sizeof(buffer),\" :=\")"));       // Text
  lcd.read(buffer,sizeof(buffer)," :=");                                            // Read value after =
  printBuffer();                                                                    // Report buffer

  Serial.println();                                                                 // Extra Space
  Serial.println(F("Test finished"));                                               // Text
  Serial.println();                                                                 // Extra Space
}

void loop() {
}
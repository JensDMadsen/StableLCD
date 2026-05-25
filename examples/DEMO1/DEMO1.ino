
// This example is for use with StableLCD arduino library by Jens D. Madsen

#include <StableLCD.h>                          // Include the library code

// Initialize pins
const int rs = 6, rw = 7, en = 8;               // Control pins
const int d4 = 9, d5 = 10, d6 = 11, d7 = 12;    // Data bus pins

StableLCD lcd(rs, rw, en, d4, d5, d6, d7);

void setup() {
  lcd.begin(16, 2);                             // Set up the LCD's number of columns and rows
  lcd.print("StableLCD");                       // Output text
}

void loop() {
}

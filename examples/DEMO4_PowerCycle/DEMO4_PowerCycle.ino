
// This example is for use with StableLCD arduino library by Jens D. Madsen

// This example tests power cycles with StableLCD.
// It has testet more than 2.000.000 cycles, with no errors reported.

#include <StableLCD.h>                          // Include the library code

// Initialize pins
const int rs = 6, rw = 7, en = 8;               // Control pins
const int d4 = 9, d5 = 10, d6 = 11, d7 = 12;    // Data bus pins
const int pwr = 5;                              // Power is connected to pin 5

const unsigned long displayOnDelayMs  = 250;    // Time that LCD display is on to allow text to be read
const unsigned long powerOffDelayMs   = 250;    // Time that LCD display is off between cycles

const bool printEveryCycle = true;              // Set this to off, if you only wants outputs if errors

StableLCD lcd(rs, rw, en, d4, d5, d6, d7, pwr); // Use StableLCD with power controle

uint32_t counter = 0;                           // Count all loops
uint32_t okCount = 0;                           // Count ok (no errors)
uint32_t errCount = 0;                          // Count errors

void setup() {                                  // Setup
  Serial.begin(115200);                         //   Usart to 115200 baud
  while (!Serial) { }                           //   Wait on Serial
  Serial.println();                             //   Extra space
  Serial.println("Power on/off test program");  //   Text
}

void loop() {                                   // Test loop

  counter++;                                    // Loop counter

  uint32_t t0 = micros();                       // t0 = start time
  bool beginOk = lcd.begin(16, 2);              // Set up the LCD's number of columns and rows and turn LCD on
  uint32_t t1 = micros();                       // t1 = time to turn LCD display on (power up, initClear LCD)

  bool writeOk = false;                         // Flag, indicates if text is written to LCD display
  bool verifyOk = false;                        // Flag, indicates that text is verified ok on LCD display

  if (beginOk) {                                // Do only execute if begin succeed ok
    lcd.print("n=");                            // Output text n=counter at first line
    lcd.print(counter);                         //   counter

    lcd.home();                                 // Place cursor on text to verify
    lcd.verifyBegin();                          // Verify mode begins
    lcd.print("n=");                            // Output text n=counter at first line
    lcd.print(counter);                         //   counter
    lcd.print("   ");                           // Verify that clear() left spaces after the text.
    lcd.verifyEnd();                            // Verify mode ends

    verifyOk = lcd.verifyOk();                  // Store true if verify is ok

    lcd.setCursor(0, 1);                        // Set cursor on first position, line 2
    if (verifyOk) {                             // Output result of verification LCD display, second line
      lcd.print("Verify ok");
    } else {
      lcd.print("Verify error");
    }

    writeOk = true;                             // Indicates that text is written to LCD display
  }

  uint32_t t2 = micros();                       // t2 = time that text has been written on LCD display

  if (beginOk && writeOk && verifyOk) {         // Count ok and errors
    okCount++;
  } else {
    errCount++;
  }

  if (printEveryCycle || !verifyOk || !beginOk || !writeOk) {   // Output statistics to USART
    Serial.print("Inittime = ");
    Serial.print(t1 - t0);
    Serial.print(". Totaltime = ");
    Serial.print(t2 - t0);
    Serial.print(". Verified OK = ");
    Serial.print(okCount);
    Serial.print(". Errors  = ");
    Serial.println(errCount);
  }

  delay(displayOnDelayMs);                      // Delay to see what display is showing.
  lcd.end();                                    // Turn LCD off
  delay(powerOffDelayMs);                       // Delay while LCD is off
}
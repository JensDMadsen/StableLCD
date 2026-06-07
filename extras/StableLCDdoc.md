
# StableLCD

## Table of Contents

- [Introduction](#introduction)
- [Background and Motivation](#background-and-motivation)
- [Design Goals](#design-goals)
- [Architecture](#architecture)
- [Robust Initialization Algorithm](#robust-initialization-algorithm)
- [Class Responsibilities and Implementation Binding](#class-responsibilities-and-implementation-binding)
- [Porting and Custom Backends](#porting-and-custom-backends)
- [Runtime Synchronization and Recovery](#runtime-synchronization-and-recovery)
- [Performance Considerations](#performance-considerations)
- [Hardware Considerations](#hardware-considerations)
- [Example Usage Patterns](#example-usage-patterns)
- [Conclusion](#conclusion)

---
# Introduction

StableLCD is an Arduino HD44780 LCD library focused on:

- robustness,
- synchronization,
- recovery,
- and verification.

Traditional HD44780 LCD libraries normally assume that communication with the display always works correctly. In many real embedded systems this is not always true.

Noise, brownouts, timing problems, cable issues, unexpected resets, or partial controller initialization may leave the LCD controller and software driver out of synchronization.

StableLCD was designed to detect, handle, and recover from such situations.

The library includes:

- busy flag synchronization,
- timeout handling,
- display verification,
- communication recovery,
- optional power management,
- and a robust initialization sequence capable of synchronizing with the LCD controller even when startup timing, controller state, or nibble alignment is unknown.

Unlike traditional LCD libraries that blindly assume writes succeed, StableLCD can verify that expected data actually exists in display memory by reading data back from the LCD controller. **This library requires the RW (Read/Write) pin to be connected**, as it uses the pin for busy flag polling, display verification, and robust synchronization.

This makes it possible to continuously validate LCD operation during runtime, for example before clearing a watchdog timer in safety-oriented or long-running embedded systems.

StableLCD is compatible with standard HD44780 character displays and supports both 4-bit and 8-bit interfaces.

The library is organized in layers:

- `HD44780PHY`
  - Physical transport and protocol layer.
  - Handles bus communication, busy polling, synchronization, timeout handling, and optional power control.

- `LiquidCrystalBase`
  - LCD logic and Arduino-compatible API layer.
  - Implements cursor handling, display state management, verification support, and `Print` integration.

- `HD44780PIN`
  - Arduino GPIO backend implementation.
  - Connects the generic PHY layer to Arduino digital I/O.

- `StableLCD`
  - User-facing wrapper layer.
  - Binds `LiquidCrystalBase` to the `HD44780PIN` backend.

The layered architecture allows alternative hardware backends to be implemented without changing the LCD logic layer.

Possible future backends could include:

- direct GPIO,
- FastIO,
- shift registers,
- I2C expanders,
- SPI interfaces,
- memory mapped buses,
- or runtime selectable PHY drivers.

---

## Basic Example

```cpp
#include <StableLCD.h>

const int rs = 6, rw = 7, en = 8;
const int d4 = 9, d5 = 10, d6 = 11, d7 = 12;

StableLCD lcd(rs, rw, en, d4, d5, d6, d7);

void setup() {
  lcd.begin(16, 2);
  lcd.print("StableLCD");
}

void loop() {
}
```

---

## Verify Example

StableLCD can verify display contents by reading data back from the LCD controller instead of writing new data.

```cpp
lcd.home();

lcd.verifyBegin();
lcd.print("StableLCD");
lcd.verifyEnd();

lcd.setCursor(0,1);

if (lcd.verifyOk()) {
  lcd.print("Verify ok");
} else {
  lcd.print("Verify error");
}
```

In verify mode, normal write operations are transparently replaced by LCD read-and-compare operations.

This allows existing `Print`-based code to be reused for display verification without introducing separate verify-specific APIs.

---

## Recovery Example

StableLCD supports communication recovery by reinitializing the LCD controller and restoring the software display state.

```cpp
if (!lcd.verifyOk()) {
  lcd.initClear();
  lcd.print("StableLCD");
}
```

For more severe communication failures, a full power-cycle recovery strategy may also be used:

```cpp
lcd.end();
delay(100);
lcd.begin(16,2);
```

StableLCD has currently been tested primarily on AVR architecture using Arduino Nano boards, but the architecture is intended to remain portable across platforms supporting Arduino-compatible GPIO and timing functions.

---
# Background and Motivation
The HD44780 controller was introduced in 1984 - over 40 years ago. For four decades, engineers have copied the same initialization code from datasheets, accepting fixed delays and startup assumptions as "just how it's done".

This library started as a simple observation: Why wait 50ms when the display might be ready in 2ms? Why assume the controller is in 8-bit mode when it might not be?

The synchronization algorithm in this library is, to my knowledge, the first published method that can reliably synchronize with an HD44780 display from *any* unknown state - without a single fixed delay. No assumptions about power-up timing, no assumptions about mode, no assumptions about nibble alignment. 

The name "StableLCD" reflects the goal: a display driver that doesn't just work most of the time, but works *reliably* - even when things go wrong. Runtime verification, automatic recovery, and active synchronization aren't just features; they're the foundation.

I hope this library saves you hours of debugging flaky displays, and perhaps inspires a more robust approach to embedded communication in general.

---
# Design Goals

StableLCD was designed with focus on:

- robustness,
- consistency,
- recovery,
- verification,
- and layered driver architecture.

The goal of the library is not only to write text to an LCD display, but also to provide mechanisms for:

- detecting communication problems,
- recovering synchronization,
- and verifying that the display actually contains the expected data.

---

## Robust Communication

Many HD44780 LCD libraries assume that communication always works correctly after initialization.

In practice this is not always true.

Electrical noise, unstable power, timing problems, unexpected resets, or software crashes may leave the LCD controller and software driver out of synchronization.

StableLCD therefore attempts to:

- detect communication failures,
- avoid infinite blocking,
- recover synchronization,
- and verify display correctness during runtime.

Timeout handling is integrated into the communication layer to prevent software from hanging indefinitely if the LCD controller stops responding.

---

## Display Verification

Traditional LCD libraries normally assume that LCD writes succeed.

StableLCD instead supports verification by reading display memory back from the LCD controller and comparing it against the expected data.

This allows software to detect situations where:

- characters were not written correctly,
- the controller lost synchronization,
- or display contents no longer match the expected state.

Verification may be useful in:

- long-running embedded systems,
- watchdog supervised systems,
- industrial control,
- monitoring systems,
- or applications where display correctness matters.

Example:

```cpp
lcd.home();

lcd.verifyBegin();
lcd.print("StableLCD");
lcd.verifyEnd();

if (!lcd.verifyOk()) {
  // Recovery action
}
```

Verification mode intentionally reuses the normal `Print` write path.

Instead of introducing separate verification APIs, normal write operations are transparently converted into LCD read-and-compare operations.

This follows the same “do equal things equally” philosophy used throughout StableLCD, where verification intentionally reuses the same transfer paths as normal runtime communication.

---

## Recovery Support

StableLCD includes support for recovering communication with the LCD controller.

The library distinguishes between:

- normal display clearing (`clear()`),
- controller reinitialization (`initClear()`),
- and full power-cycle recovery (`end()` / `begin()`).

This allows recovery strategies to be adapted to the severity of communication failures.

Example:

```cpp
if (!lcd.verifyOk()) {
  lcd.initClear();
}
```

For more severe failures:

```cpp
lcd.end();
delay(100);
lcd.begin(16,2);
```

---

## Layered Architecture

StableLCD is intentionally organized in layers.

The purpose of the layered architecture is to separate:

- physical hardware access,
- HD44780 protocol handling,
- LCD logic,
- and user-facing API functionality.

This separation improves:

- portability,
- maintainability,
- testing,
- reuse,
- and robustness.

The architecture currently consists of four logical layers:

| Layer | Responsibility |
|---|---|
| `HD44780PHY` | Physical bus/protocol communication |
| `HD44780PIN` | Arduino GPIO backend |
| `LiquidCrystalBase` | LCD logic and API |
| `StableLCD` | User-facing wrapper/binding layer |

The layered design also allows future PHY implementations to be added without changing the LCD logic layer.

---

## Consistent Design Principles

StableLCD was developed around a simple design principle:

***Do equal things equally.***

Instead of implementing separate mechanisms for:

- writing,
- verification,
- custom characters,
- and recovery,

StableLCD attempts to reuse the same code paths whenever possible.

Examples include:

- verification using the normal `Print` write path,
- custom character verification using normal data transfers,
- and recovery using the same synchronization mechanisms as initialization.

The goal is to improve consistency, reduce hidden corner cases, and simplify long-term maintenance.

---
# Architecture

StableLCD uses a layered driver architecture similar to designs commonly used in larger embedded systems.

The purpose of the layered design is to separate:

- physical hardware access,
- HD44780 transport and protocol handling,
- LCD logic,
- and user-facing API functionality.

This separation improves:

- robustness,
- portability,
- maintainability,
- reuse,
- testing,
- and long-term flexibility.

Simple LCD drivers may sometimes be implemented as a single class.

For more advanced or robust drivers, however, explicit separation between hardware access and LCD logic often makes the code easier to maintain and extend.

The StableLCD architecture is divided into four logical layers:

| Layer | Responsibility |
|---|---|
| `HD44780PHY` | Physical transport and protocol layer |
| `HD44780PIN` | Arduino GPIO backend |
| `LiquidCrystalBase` | LCD logic and Arduino-compatible API |
| `StableLCD` | Wrapper and layer binding |

---

## HD44780PHY

`HD44780PHY` implements the low-level communication layer used to access the HD44780 controller.

Responsibilities include:

- raw bus transfers,
- 4-bit and 8-bit communication,
- bus direction handling,
- busy flag polling,
- timeout handling,
- controller synchronization,
- initialization,
- enable/disable state handling,
- and optional display power control.

The PHY layer intentionally contains no LCD user logic.

Its purpose is only to provide reliable communication with the HD44780 controller.

This layer is responsible for the robust initialization and synchronization algorithm used by StableLCD.

Instead of relying entirely on fixed startup delays, the PHY layer attempts to actively synchronize with the controller state using busy polling and nibble-phase detection.

---

## HD44780PIN

`HD44780PIN` is the default Arduino GPIO backend implementation.

This layer connects the generic synchronization-aware PHY layer to Arduino GPIO operations.

It implements the hardware interface required by `HD44780PHY` using:

- `digitalWrite()`,
- `digitalRead()`,
- and `pinMode()`.

Responsibilities include:

- RS control,
- RW control,
- Enable control,
- bus direction switching,
- and physical data bus access.

The current implementation has primarily been tested on AVR architecture using Arduino Nano boards.

---

## LiquidCrystalBase

`LiquidCrystalBase` implements the LCD logic layer and Arduino-compatible API.

Responsibilities include:

- cursor positioning,
- display control,
- text direction handling,
- scrolling,
- custom character handling,
- display state tracking,
- verification support,
- recovery support,
- and `Print` integration.

This layer contains no direct hardware access.

All communication with the LCD controller occurs through abstract virtual functions.

`LiquidCrystalBase` tracks the internal LCD state in software and can restore this state during recovery operations using `initClear()`.

The verification system is also implemented in this layer.

Verification mode reuses the normal `Print` write path and transparently replaces write operations with LCD read-and-compare operations.

---

## StableLCD

`StableLCD` is the user-facing wrapper implementation.

It combines:

- `LiquidCrystalBase`
- and the `HD44780PIN` backend

into a simple single-class interface.

The internal structure is:

```cpp
class StableLCD : public LiquidCrystalBase {
private:
  HD44780PIN lcd;
};
```

This allows:

- clean architectural separation internally,
- while still presenting a simple single-class API to the application.

---

## Alternative PHY Implementations

The layered architecture allows alternative PHY implementations to be added without changing the LCD logic layer.

Possible future PHY implementations could include:

- shift register interfaces,
- I2C expanders,
- SPI interfaces,
- memory mapped buses,
- FastIO backends,
- direct port access implementations,
- or runtime selectable PHY drivers.

Because the LCD logic layer is separated from the transport layer, robustness features such as:

- verification,
- recovery,
- timeout handling,
- and state restoration

may be reused across different hardware implementations.

---

## Why the Architecture Matters

The layered architecture is an important part of the StableLCD design philosophy.

By separating:

- hardware access,
- transport logic,
- LCD behavior,
- and user API functionality

into independent layers, the library attempts to reduce hidden dependencies and special-case behavior.

This also allows the same synchronization, verification, and recovery mechanisms to be reused consistently throughout the library.

---
# Robust Initialization Algorithm

One of the primary design goals of StableLCD is reliable synchronization with the HD44780 controller.

Traditional HD44780 initialization sequences commonly used in existing LCD libraries are largely based directly on the controller datasheet.

These sequences normally assume:

- startup timing compatible with the datasheet,
- sufficiently long initialization delays,
- and successful synchronization after the initialization sequence has been transmitted.

In practice this is not always guaranteed.

Unexpected resets, brownouts, software crashes, noise, or power instability may leave the LCD controller and software driver out of synchronization.

In such situations initialization may fail because the software no longer knows:

- whether the controller is currently busy,
- whether the controller is operating in 4-bit or 8-bit mode,
- or whether the next transfer is expected to be a high nibble or low nibble.

StableLCD therefore uses a synchronization-oriented initialization algorithm instead of relying entirely on fixed startup delays.

---

## Traditional HD44780 Initialization

The classical HD44780 initialization sequence transmits the `0x3` synchronization command three times in order to force the controller toward 8-bit operation before optionally switching to 4-bit mode.

This method is significantly more robust than relying only on power-on-reset defaults, because it attempts to synchronize with the controller regardless of its previous communication state.

However, the traditional sequence still depends heavily on fixed timing delays.

These delays must be long enough to handle:

- slow LCD startup,
- long reset conditions,
- unstable power rise times,
- and worst-case controller timing.

Many implementations therefore use conservative fixed delays to improve reliability.

While this approach often works well, it also has limitations:

- initialization always consumes a fixed amount of time,
- delays may still be too short in some situations,
- the synchronization attempt is normally only performed once,
- and the algorithm does not actively continue recovery if synchronization fails.

StableLCD uses the same classical synchronization mechanism.

However, instead of performing the sequence only once with fixed delays, the complete 3×`0x3` synchronization sequence is repeated as long as unstable busy-flag behavior is observed.

This allows synchronization recovery to continue until stable operation is reached.

---

## Busy Flag Synchronization

An HD44780 operation is not complete until the busy flag (`DB7`) becomes zero.

The busy flag therefore acts as the controller's synchronization signal.

While busy is active, the controller may still be processing a previous command and communication timing may not yet be stable.

To read the busy flag, the LCD data bus must first be configured for read mode.

StableLCD configures the MCU data pins as `INPUT_PULLUP` while reading from the LCD.

This is important because during the LCD controller's internal power-on reset period the LCD data bus may be in a high impedance state (`HI-Z`).

With pullups enabled:

- an inactive or floating LCD bus reads as logic high,
- and the busy flag therefore appears active.

This causes the initialization algorithm to continue waiting until the controller actively drives the bus and synchronization becomes possible.

Without pullups, a floating bus could produce random values and falsely indicate that the controller is ready.

---

## Possible DB7 States

During synchronization the algorithm repeatedly samples `DB7`.

The following situations are possible:

| DB7 Samples | Meaning |
|---|---|
| `1,1` | Busy is certainly active, phase unknown |
| `1,0` | Busy/phase unknown |
| `0,1` | Busy/phase unknown |
| `0,0` | Busy is certainly inactive, phase still unknown |

The important observation is that only `0,0` guarantees that the controller is no longer busy.

However, even in the `0,0` case, nibble phase may still be unknown.

---

## Nibble Synchronization

In 4-bit mode, each byte transfer consists of:

1. high nibble,
2. followed by low nibble.

If synchronization is lost, the software and controller may disagree about which nibble is currently being transferred.

This creates a nibble-phase mismatch where:

- a low nibble may be interpreted as a high nibble,
- or a high nibble may be interpreted as a low nibble.

The initialization algorithm therefore waits until the first observed `DB7=0` condition before immediately sampling `DB7` again.

This leaves two practically relevant situations:

| Samples | Meaning | Action |
|---|---|---|
| `0,0` | Controller is not busy. Phase may still be unknown. | Send repeated 3×`0x3` synchronization sequence. |
| `0,1` | Busy/phase uncertain. | Send repeated 3×`0x3` synchronization sequence and continue synchronization loop. |

The repeated 3×`0x3` synchronization sequence forces the controller toward 8-bit mode regardless of previous nibble alignment.

Once stable 8-bit operation is reached, busy polling eventually stabilizes as either:

- `1,1` while busy,
- or `0,0` when no longer busy.

The synchronization loop therefore continues until stable `0,0` is reached.

---

## Why Repeated 3×0x3 Synchronization Is Used

The repeated 3×`0x3` synchronization sequence is inherited from the traditional HD44780 initialization procedure.

Its purpose is to repeatedly force the controller toward a known 8-bit command state regardless of previous synchronization loss.

Repeated synchronization writes are important because:

- the controller may currently expect either a high nibble or low nibble,
- previous commands may have been incomplete,
- or communication may have stopped mid-transfer.

By repeatedly transmitting the 3×`0x3` synchronization sequence, the software eventually forces the controller back into a predictable command interpretation state.

StableLCD combines this classical synchronization technique with active busy polling.

---

## Timeout Handling

The initialization algorithm includes timeout protection.

Synchronization relies on active busy-flag polling, while timeout handling provides protection against permanent synchronization failure.

Under normal operation the timeout mechanism should never trigger unless a physical communication problem occurs, such as:

- disconnected hardware,
- damaged communication lines,
- missing LCD power,
- permanent controller reset,
- or a defective controller that never leaves the busy state.

If synchronization cannot be achieved within the configured timeout period, initialization fails instead of blocking indefinitely.

This prevents software lockups in situations where communication with the controller is no longer possible.

---

## Fast Initialization

Traditional HD44780 initialization sequences are delay based.

The actual HD44780 instructions normally complete much faster than the conservative startup delays often used by libraries.

For example, many commands complete within a few milliseconds, but after power-up the display module may not be ready to communicate immediately.

Some implementations therefore use startup delays such as `50 ms` to improve compatibility.

This improves reliability, but it also makes every initialization slow even when the display is ready much earlier.

The StableLCD initialization algorithm contains no fixed delay-based waiting loops apart from the very small bus timing delays required by the HD44780 hardware interface itself.

Synchronization instead relies on active busy-flag polling.

When combined with a FastIO backend, initialization and synchronization may therefore execute extremely quickly.

Fast initialization is also important because StableLCD supports repeated runtime reinitialization through `initClear()`.

Unlike a traditional clear operation, `initClear()` performs:

- controller synchronization,
- initialization,
- state restoration,
- and display clear.

This allows applications to periodically reestablish synchronization with the LCD controller during normal runtime operation.

Because the initialization algorithm actively polls the controller instead of relying on large fixed delays, repeated synchronization may be performed without introducing excessive delays into the application.

The algorithm therefore attempts to provide both:

- improved robustness,
- and improved startup performance.

---

## Why the Algorithm Matters

The purpose of the StableLCD initialization algorithm is not only fast startup.

Its primary purpose is reliable synchronization with an LCD controller operating in an unknown or partially synchronized state.

This allows StableLCD to support:

- communication recovery,
- runtime reinitialization,
- watchdog-oriented verification strategies,
- and more robust long-running embedded applications.

The same synchronization mechanisms are reused consistently throughout the library.

---
# Class Responsibilities and Implementation Binding

StableLCD is built as a layered driver architecture where each class has a clearly defined responsibility.

The architecture separates:

- hardware communication,
- synchronization logic,
- display functionality,
- and application API handling

into independent layers.

This section describes:

- the purpose of each class,
- which virtual functions must be implemented,
- which functionality each class provides,
- and how the layers are connected together.

---

## HD44780PHY

`HD44780PHY` is the low-level physical communication layer.

This class implements:

- HD44780 bus protocol handling,
- busy flag polling,
- synchronization,
- timeout handling,
- initialization logic,
- command/data transfers,
- and optional power control integration.

The PHY layer contains most of the synchronization intelligence used by StableLCD.

It does not know:

- display geometry,
- cursor positions,
- text formatting,
- verification policy,
- or application logic.

Its only responsibility is reliable communication with the HD44780 controller.

---

### HD44780PHY Required Virtual Functions

The PHY layer communicates with hardware through a small abstract hardware interface.

The following functions must be implemented by the backend:

```cpp
virtual void initPins() = 0;            // Initialize physical LCD pins.
virtual bool is4bitMode() = 0;          // Return true for 4-bit interface, false for 8-bit.

virtual void setEN(bool high) = 0;      // Control LCD Enable signal.
virtual void setRW(bool high) = 0;      // Control LCD Read/Write signal.
virtual void setRS(bool high) = 0;      // Control LCD Register Select signal.

virtual void writeState() = 0;          // Set data bus direction to write mode.
virtual void readState() = 0;           // Set data bus direction to read mode.

virtual void writeBus(uint8_t val) = 0; // Write value to LCD data bus.
virtual uint8_t readBus() = 0;          // Read current value from LCD data bus.
```

These functions provide:

- GPIO control,
- bus direction handling,
- and data transfer access.

The synchronization and protocol logic itself is implemented entirely inside `HD44780PHY`.

---

### Optional PHY Functions

Several PHY functions provide default implementations but may optionally be overridden for optimization or hardware-specific behavior.

Example:

```cpp
virtual bool readDB7() {
  return (readBus() & 0x80) != 0;
}
```

The default implementation reads `DB7` through `readBus()`.

However, backends may override this function for faster direct busy polling.

Optional display power control may also be implemented:

```cpp
virtual void power(bool) { }
```

This hook is automatically called by:

- `enable()`
- and `disable()`

allowing optional LCD power switching to integrate directly into the communication layer.

---

### HD44780PHY Provided Functions

After the hardware interface has been implemented, `HD44780PHY` provides:

```cpp
bool enable();                          // Enable LCD communication and optional display power.
void disable();                         // Disable LCD communication and optional display power.

bool init();                            // Synchronize and initialize LCD controller.

bool command(uint8_t value);            // Send command byte to LCD controller.
bool writeData(uint8_t value);          // Write data byte to DDRAM or CGRAM.

uint8_t readData();                     // Read data byte from DDRAM or CGRAM.

bool waitBusy();                        // Wait until busy flag clears.
bool readBusy();                        // Read current busy flag state.

uint8_t readStatus();                   // Read current controller status byte.
uint8_t readAC();                       // Wait for ready state and read address counter.
```

These functions provide:

- synchronization,
- initialization,
- command handling,
- busy polling,
- timeout protection,
- and low-level controller communication.

`readBusy()` provides an optimized busy-flag read operation.

Unlike `readStatus()`, which reads the complete HD44780 status byte, `readBusy()` may internally use the optional `readDB7()` optimization hook.

This allows hardware backends to implement very fast busy polling by reading only the `DB7` signal directly instead of performing a complete bus read.

`readStatus()` reads the current controller status directly without waiting for the busy flag to clear.

`readAC()` waits until the controller becomes ready and then reads the current address counter value.

---

## LiquidCrystalBase

`LiquidCrystalBase` implements the high-level LCD API.

It provides:

- Arduino `Print` compatibility,
- text output,
- cursor handling,
- display state management,
- verification support,
- recovery support,
- and compatibility with traditional `LiquidCrystal` style APIs.

This layer contains the user-facing LCD functionality.

Unlike the PHY layer, it does not directly manipulate GPIO pins or hardware timing.

Instead, all controller communication occurs through abstract virtual functions.

---

### LiquidCrystalBase Required Virtual Functions

The following functions must be implemented:

```cpp
virtual bool command(uint8_t value) = 0;      // Send command byte to LCD controller.
virtual bool writeData(uint8_t value) = 0;    // Write data byte to DDRAM or CGRAM.

virtual uint8_t readData() = 0;               // Read data byte from DDRAM or CGRAM.

virtual bool enableLCD() = 0;                 // Enable LCD display and optional power control.
virtual void disableLCD() = 0;                // Disable LCD display and optional power control.

virtual bool initLCD() = 0;                   // Synchronize and initialize LCD controller.
```

These functions bind the high-level LCD API to the lower-level PHY communication layer.

---

### LiquidCrystalBase Provided Functions

Once the communication layer has been connected, `LiquidCrystalBase` provides the complete LCD API.

#### Initialization

```cpp
bool begin(uint8_t cols, uint8_t rows,
           uint8_t charsize = LCD_5x8DOTS);  // Configure, initialize and clear display.

void end();                                  // Stop display and optional power control.
```

#### Display Control

```cpp
bool noDisplay();                            // Turn display off without clearing memory.
bool display();                              // Turn display on.

bool noBlink();                              // Disable blinking cursor block.
bool blink();                                // Enable blinking cursor block.

bool noCursor();                             // Hide cursor underline.
bool cursor();                               // Show cursor underline.
```

#### Text Direction and Scrolling

```cpp
bool leftToRight();                          // Set text direction left to right.
bool rightToLeft();                          // Set text direction right to left.

bool autoscroll();                           // Enable automatic display scrolling.
bool noAutoscroll();                         // Disable automatic display scrolling.

bool scrollDisplayLeft();                    // Scroll display one position left.
bool scrollDisplayRight();                   // Scroll display one position right.
```

#### Cursor and Clear Functions

```cpp
bool home();                                 // Move cursor to home position.

bool clear();                                // Clear display using normal LCD clear command.
bool initClear();                            // Re-synchronize, initialize, restore state and clear display.

bool setCursor(uint8_t col, uint8_t row);    // Set cursor position.

void setRowOffsets(int row0, int row1,
                   int row2, int row3);      // Set DDRAM row offsets.
```

#### Custom Characters

```cpp
bool createChar(uint8_t, uint8_t[]);         // Define custom character in CGRAM.
```

#### Verification

```cpp
void verifyBegin();                          // Enter verify mode and clear previous verify errors.
void verifyEnd();                            // Leave verify mode and return to normal write mode.

void verifyClr();                            // Clear verify error state.
bool verifyOk();                             // Return true if no verify error has occurred.
```

This layer therefore provides:

- Arduino-compatible LCD functionality,
- runtime synchronization support,
- verification,
- recovery,
- and display state management.

---

## StableLCD

`StableLCD` is the default user-facing implementation.

It combines:

- `LiquidCrystalBase`
- and the `HD44780PIN` backend

into a simple single-class interface.

---

### StableLCD PHY Binding

`HD44780PIN` implements the required PHY hardware interface:

```cpp
bool HD44780PIN::is4bitMode() {              // Return true for 4-bit interface, false for 8-bit.
  return mode4bit;
}

void HD44780PIN::setEN(bool en) {            // Control LCD Enable signal.
  digitalWrite(e_pin, en);
}

void HD44780PIN::setRW(bool rw) {            // Control LCD Read/Write signal.
  digitalWrite(rw_pin, rw);
}

void HD44780PIN::setRS(bool rs) {            // Control LCD Register Select signal.
  digitalWrite(rs_pin, rs);
}
```

Similarly, bus transfers are implemented through:

```cpp
void HD44780PIN::writeBus(uint8_t val);      // Write value to LCD data bus.
uint8_t HD44780PIN::readBus();               // Read current value from LCD data bus.
```

This fully binds:

- synchronization logic,
- protocol handling,
- and HD44780 communication

to the physical hardware implementation.

---

### StableLCD LiquidCrystalBase Binding

`StableLCD` binds the high-level API to the backend implementation:

```cpp
bool StableLCD::command(uint8_t value) {     // Send command byte to LCD controller.
  return lcd.command(value);
}

bool StableLCD::writeData(uint8_t value) {   // Write data byte to DDRAM or CGRAM.
  return lcd.writeData(value);
}

uint8_t StableLCD::readData() {              // Read data byte from DDRAM or CGRAM.
  return lcd.readData();
}

bool StableLCD::enableLCD() {                // Enable LCD display and optional power control.
  return lcd.enable();
}

void StableLCD::disableLCD() {               // Disable LCD display and optional power control.
  lcd.disable();
}

bool StableLCD::initLCD() {                  // Synchronize and initialize LCD controller.
  return lcd.init();
}
```

This connects:

- the high-level display API,
- to the synchronization-aware PHY layer,
- through a very small and explicit interface binding.

---

### Constructor and Initialization Philosophy

StableLCD intentionally separates:

- object construction,
- from hardware initialization.

Constructors are configuration only.

Example:

```cpp
StableLCD lcd(rs, rw, en, d4, d5, d6, d7);
```

The constructor:

- stores pin configuration,
- initializes internal state,
- and configures software structures.

It does not:

- access hardware,
- communicate with the LCD,
- perform synchronization,
- execute blocking operations,
- or perform operations that may fail.

Actual hardware initialization occurs later through:

```cpp
lcd.begin(16,2);
```

Constructors should ideally remain:

- fast,
- simple,
- non-blocking,
- failure free,
- and independent of hardware state.

Constructors should generally avoid direct hardware access because hardware resources represent shared external state whose initialization order may not yet be defined.

This is particularly important in embedded systems where:

- global object construction order is not guaranteed,
- shared state may still be undefined,
- `volatile` variables may not yet be initialized,
- peripherals may not yet be initialized,
- clocks may not yet be stable,
- interrupts may not yet be configured,
- DMA systems may not yet be operational,
- hardware registers may not yet contain valid state,
- and startup sequences may still be incomplete.

Hardware communication and synchronization should therefore normally occur during explicit runtime initialization rather than during object construction.

For this reason, StableLCD constructors only store configuration.

Actual hardware communication, synchronization, and failure-prone initialization are performed explicitly by `begin()`.

---

### Why the Layering Matters

The layered architecture allows:

- synchronization logic to remain reusable,
- hardware implementations to evolve independently,
- applications to remain portable,
- and recovery behavior to remain consistent throughout the library.

Alternative PHY implementations may easily be created for:

- direct port I/O,
- FastIO,
- I2C GPIO expanders,
- SPI interfaces,
- memory mapped interfaces,
- or platform-specific optimized drivers.

The same synchronization philosophy is reused consistently across:

- initialization,
- verification,
- recovery,
- and runtime communication.

---
# Porting and Custom Backends

StableLCD is designed so that the synchronization and recovery logic normally does not need to be modified when porting the library to new hardware platforms or interfaces.

The synchronization algorithms are implemented inside `HD44780PHY`.

Porting therefore typically consists only of implementing a new physical backend layer.

Examples:

- direct GPIO access,
- FastIO,
- I2C GPIO expanders,
- SPI interfaces,
- memory mapped interfaces,
- DMA assisted interfaces,
- or platform-specific optimized drivers.

The synchronization, initialization, timeout, verification, and recovery logic remains unchanged.

---

## Backend Architecture

The architecture separates:

- protocol and synchronization logic,
- from physical hardware access.

`HD44780PHY` contains:

- initialization synchronization,
- busy polling,
- timeout handling,
- command sequencing,
- and runtime synchronization logic.

A backend implementation only provides the physical bus operations required by the PHY layer.

This separation makes backend implementations relatively small and simple.

---

## Implementing a Custom Backend

A custom backend normally derives from `HD44780PHY` and implements the required virtual hardware interface.

Example:

```cpp
class HD44780I2C : public HD44780PHY {
protected:
  virtual void initPins() override;
  virtual bool is4bitMode() override;

  virtual void setEN(bool high) override;
  virtual void setRW(bool high) override;
  virtual void setRS(bool high) override;

  virtual void writeState() override;
  virtual void readState() override;

  virtual void writeBus(uint8_t val) override;
  virtual uint8_t readBus() override;
};
```

The PHY layer then automatically provides:

- synchronization,
- initialization,
- busy polling,
- timeout handling,
- and command sequencing.

Only the physical interface itself must be implemented.

---

## Reference Backend: HD44780PIN

`HD44780PIN` is the default Arduino GPIO backend included with StableLCD.

Example:

```cpp
void HD44780PIN::setEN(bool en) {
  digitalWrite(e_pin, en);
}

void HD44780PIN::setRW(bool rw) {
  digitalWrite(rw_pin, rw);
}

void HD44780PIN::setRS(bool rs) {
  digitalWrite(rs_pin, rs);
}
```

Similarly, the data bus interface is implemented through:

```cpp
void HD44780PIN::writeBus(uint8_t val);      // Write value to LCD data bus.
uint8_t HD44780PIN::readBus();               // Read current value from LCD data bus.
```

This backend therefore connects the generic synchronization-aware PHY layer directly to Arduino GPIO hardware.

---

## Fast Busy Polling

Efficient busy polling is important for StableLCD performance.

The PHY layer therefore supports an optional optimized `DB7` read function:

```cpp
virtual bool readDB7()                       // Optional optimized DB7 read.
```

When implemented, `readBusy()` may use this function directly instead of performing a complete status read.

Example:

```cpp
bool HD44780PIN::readDB7() {                 // Optional optimized DB7 read.
  return digitalRead(db_pin[7]);
}
```

This allows:

- faster busy polling,
- reduced bus overhead,
- and significantly improved synchronization performance.

This optimization becomes particularly important for:

- FastIO implementations,
- high-speed MCUs,
- and heavily synchronized runtime environments.

---

## 4-Bit and 8-Bit Interfaces

The PHY layer supports both:

- 4-bit interfaces,
- and 8-bit interfaces.

The backend controls this through:

```cpp
virtual bool is4bitMode()                    // Return true for 4-bit interface, false for 8-bit.
```

The PHY layer automatically adapts:

- initialization,
- synchronization,
- command transfers,
- and busy polling

to the selected interface width.

---

## Runtime Power Control

Optional LCD power control may also be integrated directly into the backend.

Example:

```cpp
virtual void power(bool) override          // Optional display power control.
```

When implemented:

- `enable()` automatically powers the display on,
- and `disable()` automatically powers the display off.

This allows:

- hard controller resets,
- low-power operation,
- and runtime recovery strategies

to integrate naturally into the synchronization framework.

---

## Wrapper Layer Integration

The user-visible `StableLCD` class combines:

- `LiquidCrystalBase`,
- and the `HD44780PIN` backend

into a simple single-class interface.

Example:

```cpp
class StableLCD : public LiquidCrystalBase {
private:
  HD44780PIN lcd;
};
```

`StableLCD` itself contains almost no synchronization logic.

Its primary purpose is:

- API exposure,
- layer binding,
- and simplified user integration.

---

## Why the Backend Architecture Matters

Because the synchronization logic is separated from the physical hardware layer:

- new backends automatically inherit the synchronization framework,
- initialization algorithms remain consistent,
- recovery behavior remains reusable,
- and platform-specific optimization becomes isolated to small backend implementations.

The same PHY synchronization logic may therefore be reused across many different hardware platforms and interface technologies.

---
# Runtime Synchronization and Recovery

Traditional HD44780 libraries typically assume that synchronization only needs to occur once during startup.

StableLCD instead treats synchronization as an ongoing runtime property.

This allows the library to:

- recover from communication loss,
- recover from partial resets,
- recover from delayed controller startup,
- detect timeout conditions,
- and maintain synchronization during long-running operation.

---

## Synchronization During Runtime

The HD44780 controller itself has no guaranteed mechanism for detecting communication phase loss.

If:

- the MCU resets,
- the LCD resets,
- power becomes unstable,
- electrical noise corrupts transfers,
- or communication becomes interrupted,

then the controller and software may lose synchronization.

Traditional libraries typically assume this never happens after initialization.

StableLCD instead allows synchronization to be restored dynamically during runtime.

---

## initClear()

`initClear()` combines:

- synchronization,
- initialization,
- and display clear

into a single operation.

Example:

```cpp
lcd.initClear();
```

Unlike a traditional `clear()`, `initClear()` first performs a complete synchronization and initialization sequence before clearing the display.

This allows:

- communication recovery,
- controller resynchronization,
- and runtime reinitialization

to occur automatically.

Because the synchronization algorithm uses active busy polling rather than long fixed delays, the operation normally completes quickly when the controller is already operating correctly.

---

## Recovery Philosophy

StableLCD intentionally treats synchronization as a recoverable runtime state rather than a startup-only event.

This means:

- synchronization may safely be repeated,
- initialization may safely be repeated,
- and recovery may safely occur during runtime operation.

The synchronization algorithm is therefore designed to tolerate:

- repeated execution,
- partial controller state loss,
- and uncertain communication phase conditions.

---

## Timeout Handling

StableLCD uses timeout protection to prevent permanent blocking if communication fails.

Timeout conditions normally indicate:

- hardware faults,
- permanent reset conditions,
- disconnected displays,
- defective controllers,
- or controllers that never leave the busy state.

Under normal operation, timeout handling should rarely or never activate.

---

## Verification and Runtime Recovery

Verification mode allows software to confirm that:

- commands reached the controller,
- data reached the controller memory,
- and communication remains synchronized.

Example:

```cpp
lcd.verifyBegin();

lcd.print("SYSTEM OK");

lcd.verifyEnd();

if (!lcd.verifyOk()) {
  lcd.initClear();
}
```

This allows:

- runtime recovery,
- watchdog integration,
- and fault-tolerant display handling.

---

## Watchdog Integration

In safety-oriented systems, the display itself may contain important operational or fault information.

Verification therefore allows software to confirm that:

- the controller accepted the transmitted data,
- and communication remains synchronized.

Example:

```cpp
lcd.verifyBegin();
lcd.print("ERROR");
lcd.verifyEnd();

if (!lcd.verifyOk()) {
  systemWatchdogFault();
}
```

The exact watchdog or fault handling mechanism is application specific and therefore intentionally not implemented by the library itself.

---

## Repeated Synchronization

Because synchronization is fast and fully runtime-safe, some applications may intentionally perform repeated synchronization periodically.

Examples:

- after communication errors,
- after power instability,
- after watchdog recovery,
- after partial resets,
- or during periodic runtime maintenance.

In many systems, repeated synchronization through `initClear()` may provide significantly more reliable long-term operation than relying on startup-only initialization assumptions.

---

## Runtime Robustness

StableLCD is designed around the principle that synchronization should remain actively maintainable throughout runtime operation.

Rather than assuming:

- ideal startup timing,
- perfect hardware behavior,
- or permanent synchronization,

the library continuously supports:

- recovery,
- verification,
- timeout protection,
- and deterministic controller communication.

---
# Performance Considerations

StableLCD is designed around active synchronization rather than fixed delays.

This allows the library to operate:

- robustly,
- deterministically,
- and often significantly faster than traditional HD44780 libraries.

---

## Busy Polling vs Fixed Delays

Traditional HD44780 libraries commonly use conservative fixed delays.

Example:

- waiting a fixed time after every command,
- or waiting long startup delays during initialization.

This approach is simple but inefficient because:

- the controller is often ready much sooner,
- delays accumulate during runtime,
- and worst-case timing must always be assumed.

StableLCD instead uses active busy polling.

Operations therefore complete:

- exactly when the controller becomes ready,
- rather than after a predetermined delay interval.

---

## Initialization Performance

Traditional initialization sequences often use long startup delays to guarantee operation under uncertain power conditions.

In practice, these delays may become much larger than the actual controller startup time.

StableLCD instead continuously polls the busy state during synchronization.

This allows:

- fast initialization when the controller is already ready,
- while still tolerating delayed startup conditions,
- unstable reset timing,
- and uncertain synchronization states.

The initialization time therefore automatically adapts to the actual controller behavior.

---

## Runtime Synchronization Cost

Because synchronization uses active polling instead of large fixed delays, runtime resynchronization may often be performed with relatively small overhead.

This is particularly important for:

```cpp
lcd.initClear();
```

which combines:

- synchronization,
- initialization,
- and display clear.

In many systems, the operation is fast enough that it may safely be used periodically or after communication uncertainty without causing significant runtime slowdown.

---

## readDB7() Optimization

Busy polling performance depends heavily on the speed of reading the busy flag (`DB7`).

StableLCD therefore supports an optional optimized backend hook:

```cpp
virtual bool readDB7()
```

When implemented efficiently:

- complete status reads may be avoided,
- GPIO overhead may be reduced,
- and synchronization speed may improve significantly.

This optimization becomes particularly important for:

- FastIO backends,
- direct register access,
- and high-speed MCUs.

---

## FastIO and Direct Register Access

The default `HD44780PIN` backend uses:

- `digitalWrite()`,
- `digitalRead()`,
- and `pinMode()`.

While portable, these functions may be relatively slow on some platforms.

Custom backends may instead use:

- direct register access,
- FastIO libraries,
- memory mapped GPIO,
- or platform-specific optimized hardware access.

Because the synchronization logic remains inside `HD44780PHY`, such optimizations may often be implemented entirely inside the backend layer.

---

## 4-Bit vs 8-Bit Interfaces

StableLCD supports both:

- 4-bit interfaces,
- and 8-bit interfaces.

8-bit interfaces:

- transfer data faster,
- but require more GPIO pins.

4-bit interfaces:

- reduce pin count,
- but require additional bus cycles.

The synchronization framework itself operates identically in both modes.

---

## Delay-Free Runtime Operation

StableLCD intentionally avoids large fixed delays during normal operation.

Only very small hardware timing delays remain:

- bus setup timing,
- enable pulse timing,
- and required HD44780 bus timing constraints.

This allows the library to operate efficiently even in heavily synchronized or frequently verified systems.

---

## Robustness and Performance

StableLCD does not treat:

- robustness,
- synchronization,
- and performance

as conflicting goals.

Instead, active synchronization allows:

- improved reliability,
- deterministic behavior,
- reduced unnecessary waiting,
- and efficient runtime recovery

to coexist within the same driver architecture.

---
# Hardware Considerations

StableLCD is designed for robust HD44780 communication, but reliable operation still depends on correct hardware design.

The library can detect and recover from many communication problems, but it cannot compensate for fundamentally incorrect wiring, missing signals, unstable power, or unsupported hardware configurations.

---

## R/W Signal Required

StableLCD requires access to the HD44780 `RW` signal.

Unlike many simple LCD libraries that only write to the display, StableLCD reads from the controller in order to support:

- busy flag polling,
- controller synchronization,
- display verification,
- `readData()`,
- and optimized runtime recovery.

Because of this, `RW` must be connected to a controllable MCU pin.

Tying `RW` permanently low disables readback and prevents StableLCD from using its main robustness features.

---

## Data Bus Pullups

When the LCD bus is configured for reading, StableLCD uses input pullups in the default GPIO backend.

This is important because the LCD data bus may be floating during:

- power-up,
- controller reset,
- disconnected hardware,
- or high impedance bus conditions.

With pullups enabled, a floating `DB7` line reads as high.

This causes busy polling to interpret the controller as not ready, which is safer than falsely assuming that the controller is ready.

---

## Power and Reset Behavior

LCD modules may not become ready immediately after power is applied.

Slow power rise time, controller reset delay, unstable supply voltage, or module-specific behavior may affect when communication becomes possible.

StableLCD handles this by actively polling the controller instead of relying only on fixed startup delays.

If optional power control is implemented in the backend, `begin()` may power the display on and `end()` may power it off.

This can be useful for:

- low-power systems,
- hard recovery after communication failure,
- and systems where the LCD power supply is controlled by the MCU.

---

## Signal Integrity

HD44780 parallel buses are usually simple and reliable at short distances.

However, communication problems may occur with:

- long wires,
- poor grounding,
- noisy environments,
- weak power supply decoupling,
- loose connectors,
- or high bus capacitance.

For best reliability:

- keep LCD wires short,
- use proper ground connections,
- add decoupling close to the LCD module,
- avoid routing LCD wires near noisy power switching signals,
- and ensure that all signal voltage levels are compatible.

---

## 4-Bit and 8-Bit Wiring

StableLCD supports both 4-bit and 8-bit interfaces.

In 4-bit mode only `DB7..DB4` are used.

In 8-bit mode all data pins `DB7..DB0` are used.

The selected constructor determines which mode is used, and `HD44780PHY` automatically adapts command transfers, reads, busy polling, and initialization to the selected bus width.

---

## LCD Clones and Compatible Controllers

Many character LCD modules use HD44780-compatible controllers rather than original HD44780 chips.

Most compatible controllers behave sufficiently similarly for normal operation, but timing, reset behavior, and edge cases may vary.

StableLCD attempts to handle such differences by:

- avoiding fixed delay assumptions where possible,
- using busy flag polling,
- applying timeout protection,
- and supporting runtime resynchronization.

---

## What Hardware Cannot Be Verified

StableLCD can verify that expected data exists inside the LCD controller memory.

It cannot directly verify the optical output of the display.

Software verification cannot detect:

- broken LCD glass,
- missing backlight,
- poor contrast adjustment,
- damaged segment drivers,
- or physical display defects.

For such failures, external sensing or visual inspection would be required.

StableLCD verification therefore confirms controller memory state, not the physical appearance of the display.

---
# Example Usage Patterns

StableLCD supports both:

- simple Arduino-style usage,
- and more advanced synchronized runtime operation.

The following examples show common usage patterns.

---

## DEMO1 - Basic Usage

Basic usage is intentionally simple and similar to traditional Arduino LCD libraries.

```cpp
#include <StableLCD.h>          // Include the library code

// Initialize pins
const int rs = 6, rw = 7, en = 8;               // Control pins
const int d4 = 9, d5 = 10, d6 = 11, d7 = 12;    // Data bus pins

StableLCD lcd(rs, rw, en, d4, d5, d6, d7);

void setup() {
  lcd.begin(16, 2);             // Set up the LCD's number of columns and rows
  lcd.print("StableLCD");       // Output text
}

void loop() {
}
```

The library automatically performs:

- synchronization,
- initialization,
- busy polling,
- and timeout handling.

---

## DEMO2 - Verification Example

Verification mode allows software to verify that expected data exists inside the LCD controller memory.

```cpp
#include <StableLCD.h>          // Include the library code

// Initialize pins
const int rs = 6, rw = 7, en = 8;               // Control pins
const int d4 = 9, d5 = 10, d6 = 11, d7 = 12;    // Data bus pins

StableLCD lcd(rs, rw, en, d4, d5, d6, d7);

void setup() {
  lcd.begin(16, 2);             // Set up the LCD's number of columns and rows
  lcd.print("StableLCD");       // Output text

  lcd.home();                   // Place cursor on text to verify

  lcd.verifyBegin();            // Verify mode begins
  lcd.print("StableLCD");       // Verify text is StableLCD
  lcd.verifyEnd();              // Verify mode ends

  lcd.setCursor(0,1);           // Set cursor on first position, line 2

  if (lcd.verifyOk()) {         // Output result of verification
    lcd.print("Verify ok");
  } else {
    lcd.print("Verify error");
  }
}

void loop() {
}
```

The verification state remains active until `verifyEnd()` is called.

`verifyOk()` returns:

- true if verification succeeded,
- or false if any verification mismatch or timeout occurred.

---

## DEMO3 - Runtime Recovery Example

Verification may also be used during continuous runtime operation.

```cpp
#include <StableLCD.h>          // Include the library code

// Initialize pins
const int rs = 6, rw = 7, en = 8;               // Control pins
const int d4 = 9, d5 = 10, d6 = 11, d7 = 12;    // Data bus pins

StableLCD lcd(rs, rw, en, d4, d5, d6, d7);

void setup() {
  lcd.begin(16, 2);             // Set up the LCD's number of columns and rows
  lcd.print("StableLCD");       // Output text
}

void loop() {                   // Do continuous verification

  lcd.home();                   // Place cursor on text to verify

  lcd.verifyBegin();            // Verify mode begins
  lcd.print("StableLCD");       // Verify text is StableLCD
  lcd.verifyEnd();              // Verify mode ends

  lcd.setCursor(0,1);           // Set cursor on first position, line 2

  if (lcd.verifyOk()) {         // Output result of verification
    lcd.print("Verify ok");
  } else {
    lcd.print("Verify error");

    delay(1000);                // Demo delay only.

    lcd.initClear();            // Re-initialize and clear display
    lcd.print("StableLCD");     // Output text

    delay(1000);                // Demo delay only.
  }
}
```

This demonstrates:

- runtime verification,
- communication fault detection,
- and automatic recovery through `initClear()`.

The delays are included only to make recovery behavior visible during demonstration.

---

## Runtime Reinitialization

`initClear()` combines:

- synchronization,
- initialization,
- and display clear

into a single operation.

```cpp
lcd.initClear();
lcd.print("Recovered");
```

Because synchronization uses active busy polling rather than large fixed delays, runtime reinitialization may often be performed with relatively small overhead.

---

## Power Controlled Displays

If the backend implements:

```cpp
virtual void power(bool)
```

then display power control becomes integrated into the synchronization framework.

```cpp
lcd.end();     // Disable LCD and optional power control.
lcd.begin();   // Re-enable and synchronize display.
```

This may be useful for:

- low-power systems,
- battery powered systems,
- and hard recovery after communication failure.

---

## Custom Backends

Applications may also use custom hardware backends.

```cpp
class MyLCD : public LiquidCrystalBase {
private:
  HD44780I2C lcd;
};
```

The synchronization and recovery logic remains unchanged because it is implemented inside `HD44780PHY`.

Only the physical hardware backend changes.

---
# Conclusion

The HD44780 controller architecture originates from an era where:

- fixed delays,
- startup assumptions,
- and simple write-only communication

were generally considered acceptable.

For decades, most HD44780 software has therefore relied primarily on:

- conservative timing delays,
- startup-only synchronization,
- and assumptions that communication remains permanently synchronized after initialization.

StableLCD instead treats synchronization as an active runtime property.

Rather than relying primarily on fixed delays and startup assumptions, the library continuously supports:

- busy flag polling,
- synchronization recovery,
- runtime verification,
- timeout protection,
- and deterministic controller communication.

The synchronization framework is designed to tolerate:

- uncertain startup timing,
- partial resets,
- communication phase loss,
- unstable power conditions,
- and long-running runtime operation.

At the same time, the architecture separates:

- synchronization logic,
- from physical hardware access.

This allows:

- backend portability,
- hardware-specific optimization,
- and reusable synchronization behavior

across multiple platforms and interface technologies.

StableLCD therefore demonstrates that even a very old controller architecture such as the HD44780 may still support:

- modern synchronization principles,
- robust runtime behavior,
- verification,
- recovery,
- and efficient operation

when the driver itself is designed around synchronization rather than delay assumptions.

The library intentionally attempts to combine:

- traditional Arduino simplicity,
- deterministic embedded-system behavior,
- and synchronization-oriented driver design.

Although originally developed for HD44780-compatible displays, many of the same principles also apply more generally to embedded communication design:

- synchronize actively,
- verify when possible,
- avoid unnecessary assumptions,
- and treat recovery as part of normal runtime operation rather than as an exceptional condition.

***Do equal things equally.***

---
## Contact

If you have any questions, feedback, or need further assistance, feel free to [Contact Me](https://www.arduino.one/contact.html) through my online form.

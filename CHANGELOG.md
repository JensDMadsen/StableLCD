# Changelog

All notable changes to this project will be documented in this file.

## [1.0.0] - 2026-05-25
### Initial Release
- First stable release
- HD44780 LCD support with robust synchronization (no fixed delays)
- 4-bit and 8-bit interface support
- Runtime verification mode
- Recovery via initClear()
- Timeout protection (200ms)
- Full LiquidCrystal API compatibility
- Arduino GPIO backend included
- LGPL 2.1 licensed

### Requirements
- RW pin must be connected

### Tested
- Arduino Nano (16MHz AVR)
- 16x2 and 20x4 displays
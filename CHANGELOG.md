# Changelog

All notable changes to this project will be documented in this file.

## [1.0.2] - 2026-06-07
### Fixed
- Removed compiler warnings reported by newer Arduino IDE toolchains
- Fixed unused parameter warnings in optional virtual backend hooks
- Fixed constructor initialization order warnings in HD44780PIN

## [1.0.1] - 2026-05-29
### Metadata
- Updated library metadata for improved Library Manager search.

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
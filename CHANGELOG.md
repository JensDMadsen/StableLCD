# Changelog

All notable changes to this project will be documented in this file.

## [1.2.0] - 2026-06-14
### Added
- Added LCD DDRAM text reading functions:
  - `read()`
  - `readLine()`
  - `readUntil()`
- Added logical position and address helper functions:
  - `getCols()`
  - `getRows()`
  - `getPos()`
  - `getLine()`
  - `getAddress()`
  - `setAddress()`
- Added `DEMO6_ReadFunctions` example demonstrating:
  - DDRAM text reading
  - token parsing
  - separator handling
  - logical cursor/address handling
  - DDRAM address save/restore

### Changed
- Expanded README documentation for:
  - reading display memory
  - logical cursor/address functions
  - text parsing behavior
  - separator and terminator handling
- Updated `keywords.txt` with new API functions
- Improved example SRAM usage using `F()` flash string storage

## [1.1.0] - 2026-06-12
### Added
- Optional LCD power control support via backend power pin
- Runtime LCD power support in begin(), end(), enable(), and disable()
- HD44780PIN now waits for upper LCD data bus lines to read high after power-up before synchronization continues
- DEMO4_PowerCycle example for repeated LCD power-cycle verification
- DEMO5_SlowRiseTest example for slow LCD supply rise testing

### Changed
- Improved HD44780 synchronization and initialization robustness
- Added repeated synchronization confirmation rounds during initialization
- Improved documentation of synchronization and recovery behavior
- Updated README documentation and examples
- Updated demo naming for clearer purpose description

### Tested
- Long-duration repeated LCD power-cycle verification testing
- Repeated slow-rise LCD supply power-cycle verification testing

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
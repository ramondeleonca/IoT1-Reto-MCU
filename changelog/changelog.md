# Changelog

## [2026-10-11 19:21:55] Reject invalid time-of-flight readings
- **Prompt:** "this code is always outputting 10912.00mm, 10912.00mm; make the most minimal, necessary changes to make this code work and output the real distance"
- **Summary:** Wait for both VL53L0X sensors to boot; initialize measurement data and reject failed or invalid results instead of printing uninitialized values in [tof.hpp](../src/impl/tof.hpp).

## [2026-10-08 13:16:00] Fix speed calculation and timing logic
- **Prompt:** "fix the code, it just says 999"
- **Summary:** Converted velocity calculations to microsecond precision, added edge-triggered state machine for photogate inputs, fixed distance/time unit conversions, and added timeout handling in [main.hpp](file:///Users/ramon/Documents/Code/IoT-Reto/mcu/src/impl/main.hpp).

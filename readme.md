# Psyco
 
An Arduino library for PSYCO programmable synth controller dev board having:
- 18x analog inputs (10-bits): 0-17	{0-15: ADC, 16:A6, 17:A7}
- 3x GPIO (INPUT,INPUT_PULLUP, OUTPUT): {0:A3, 1:A4, 2:A5}
- 4x CV out (5v @ 12-bit): 0-3
- 4x buffered Gate out: 0-3 {0:D6, 1:D7, 2:D8, 3:D9}
- 1x unbuffered digital out: D10
- MINI IN  (D0)
- MIDI OUT (D1)

Designed specifically to work with PSYCO board which has: 
- Arduino Nano v3.x
- 10-bit ADC: 2x MCP3008 
- 12-bit DAC: 2x MCP-4922
- Adafruit rotary encoder (24 step incremental quadrature + switch)
- 64 RGB RGB led panel (8x8)

Author: Fahd Al Riachi - KHAFA 2021

MIT license, all text above must be included in any redistribution

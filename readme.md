# Psyco
 
An Arduino library for PSYCO programmable synth controller dev board having:
- 21x analog inputs (10-bits): 0-17
- 3x GPIO (INPUT,INPUT_PULLUP, OUTPUT): {0:A3, 1:A4, 2:A5}
- 4x CV out (5v @ 12-bit): 0-3
- 4x buffered Gate out: 0-3 {0:D6, 1:D7, 2:D8, 3:D9}
- 1x unbuffered digital out: D10
- MINI IN  (D0)
- MIDI OUT (D1)

Designed specifically to work with 
- Arduino Nano v3.x
- 10-bit ADC: MCP3008 
- 12-bit DAC: MCP-49xx
- Adafruit rotary encoder (24 step incremental quadrature + switch)

MIT license, all text above must be included in any redistribution

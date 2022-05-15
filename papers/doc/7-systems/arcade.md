# Arcade (Yamaha YM2151/PCM)

this chip combination was used in the Sega OutRun, X and Y arcade boards, and perhaps a few others.
the actual PCM chip had 16 channels, but the number has been cut to 5 in DefleMask for seemingly arbitrary reasons.

# effects

- `10xx`: set noise frequency of channel 8 operator 4. 00 disables noise while 01 to 20 enables it.
- `11xx`: set feedback of channel.
- `12xx`: set operator 1 level.
- `13xx`: set operator 2 level.
- `14xx`: set operator 3 level.
- `15xx`: set operator 4 level.
- `16xy`: set multiplier of operator.
  - `x` is the operator (1-4).
  - `y` is the mutliplier.
- `17xx`: set LFO speed.
- `18xx`: set LFO waveform. `xx` may be one of the following:
  - `00`: saw
  - `01`: square
  - `02`: triangle
  - `03`: noise
- `19xx`: set attack of all operators.
- `1Axx`: set attack of operator 1.
- `1Bxx`: set attack of operator 2.
- `1Cxx`: set attack of operator 3.
- `1Dxx`: set attack of operator 4.
- `1Exx`: set AM depth.
- `1Fxx`: set PM depth.
- `20xx`: set PCM frequency.
  - only works on the PCM channels.
  - `xx` is a 256th fraction of 31250Hz.

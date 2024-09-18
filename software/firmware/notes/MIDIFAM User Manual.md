MIDIFAM User Manual
===================


# Learn Button

## Midi Channel Settings

*** MIDI CHANNEL SETTING
TODO: should be based on what channel the next message is received on
TODO: click the button should cycle through outputs, not exit mode



By default:
- CV output A receives MIDI on channel #1
- CV output B receives MIDI on channel #2
- DFAM "keyboard-controlled sequencer" mode receives MIDI on channel #10

To change the MIDI channel settings, **tap the learn button once**.

When the learn button is pressed, the module enters "Learn MIDI channel" mode and the module listens for MIDI Note On messages *on any MIDI channel*. The LEDs will indicate which output is currently being configured. MIDI note number 60 (i.e. middle C) corresponds to MIDI channel 1, note 61 represents channel 2, and so on, increasing
chromatically until MIDI note number 75 (channel 16). See the table below.

| MIDI note # | MIDI channel selection |
| -----------   | ----------------- |
| 60 (middle C) | ch. 1 |
| 61          | ch. 2 |
| 62          | ch. 3 |
| 63          | ch. 4 |
| 64          | ch. 5 |
| 65          | ch. 6 |
| 66          | ch. 7 |
| 67          | ch. 8 |
| 68          | ch. 9 |
| 69          | ch. 10 |
| 70          | ch. 11 |
| 71          | ch. 12 |
| 72          | ch. 13 |
| 73          | ch. 14 |
| 74          | ch. 15 |
| 75          | ch. 16 |

Any MIDI note outside this range will advance to the next output without changing the selected MIDI channel.

The first MIDI note will select CV output A's MIDI channel.
The second MIDI note will select output B's MIDI channel.
The third MIDI note will select the MIDI channel for "Keyboard-Controlled Sequencer Mode" (which appears at the *Adv/Clock Out* output when the "mode" switch ).

After receiving the third Note On message, the module will **save the new settings** so that the MIDI channel
selections will be restored after the device is power cycled. In addition to saving the MIDI channel selections, **all other settings will also be saved** (e.g. portamento, vibrato, polyphony, and any other configuration made using MIDI CC messages).

## Channel-specific CC messages

These will only apply when they are sent on the MIDI channel to which the CV output is assigned.

**Portamento**

|CC | Function           | Values                         | Notes |
|---|--------------------|--------------------------------|-------|
|65  |   Enable or disable portamento | On/Off | |
|18	|   Time (desc.) | 0 to 2 sec | Duration of slides for descending notes |
|19	|   Time (asc.) | 0 to 2 sec |  Duration of slides for ascending notes |
|5	|   Time (master) | 0 to 2 sec | Master duration  |


**Gate/Trig Behavior**

|CC | Function           | Values                         | Notes |
|---|--------------------|--------------------------------|--------- |
|16 | Trigger duration   | 1ms to about 50ms              | |
|80 | Retrigger behavior | Off, Highest, Lowest, Latest   | What note to re-trigger if any notes are still held when a Note Off message is received |
|81 | Gate/Trig mode     | Off, Trigger, Gate             | "Trigger" means a short pulse for every MIDI Note On. "Gate" means the output is high as long as a note is held. |


**Vibrato**

|CC | Function       | Values                         | Notes|
|---|----------------|--------------------------------|-------------|
|76 | Rate           | 5 sec to 200 ms period    |  |
|77 | Depth          | 0 to 1 semitone           | What note to re-trigger if any notes are still held when a Note Off message is received|
|78 | Delay          | 0 to 3 seconds            | How long to wait before beginning the vibrato|

**Pitch Bend**


|CC | Function       | Values                         | Notes|
|---|----------------|--------------------------------|-------------|
|17 | Pitch bend range           | 0 to 12 semitones   |  | 


## Global CC Messages

Defined in MidiController
    CC_AdvClockWidth  MIDI_NAMESPACE::GeneralPurposeController7 // cc #82
    CC_ClockDiv		  MIDI_NAMESPACE::GeneralPurposeController8 // cc #83


Additional/miscellaneous:
    - OmniModeOff, OmniModeOn
    - MonoModeOn, PolyModeOn
    - AllNotesOff
    - ResetAllControllers

Calibration specific:
    - Calibrate coarse      CC# 102
    - Calibrate fine        CC# 103
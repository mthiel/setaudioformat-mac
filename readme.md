# MacOS - setAudioFormat

A command-line utility for macOS that allows you to modify the audio format settings of your default output device, including sample rate, bit depth, and channel count.

## Why?

*Does your Mac completely forget how many channels your home theater receiver has every time you wake it up?*

*Do you frequently need to switch between two, five, or eight channel modes?*

*Do you just want an alternative to opening Audio MIDI Tool to change formats? Prefer something that can be automated easily?*

All of these were me, and I wasn't able to find an existing tool to help. So, here's one.


## Installation

1. Clone this repository
2. Run `make` to build the release version or `make debug` for a debug build
3. The executable `setAudioFormat` will be created in the current directory

## Usage

This utility works best if you are changing to a format that you've previously configured in Audio MIDI Setup previously. It does not modify the speaker layout when adjusting the number of channels, so MacOS defaults to the layout that was used previously with the requested format.

### Examples:

    # Set the audio format to: 2 ch 16-bit Integer 44.1 KHz
    setAudioFormat --rate=44100 --bits=16 --channels=2
    
    # Set only the sample rate to 192.0 KHz
    setAudioFormat -r 192000
    
    # 8 ch 24-bit Integer 48.0 KHz
    setAudioFormat -r 48000 -b 24 -c 8

    # 2 ch 24-bit Integer 192.0 KHz
    setAudioFormat -r 192000 -b 24 -c 2
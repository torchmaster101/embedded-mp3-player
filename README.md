# Embedded MP3 Player

## Overview

This project is an embedded MP3 player built on an **ATmega328P** microcontroller. The system plays audio files stored on a microSD card using a serial MP3 player module. A **16×2 LCD** displays the current system state and selected track, while an **RGB LED** provides visual feedback for the player's operating mode.

Audio output is routed through an external amplifier and speaker. The volume can be adjusted using the amplifier's built-in potentiometer.

---

## Features

* Play multiple MP3 tracks stored on a microSD card
* LCD interface displaying:

  * Current player state
  * Selected track
* RGB LED status indicator
* Track selection using dedicated buttons
* Play/Pause control
* Adjustable audio volume using the amplifier's potentiometer
* Track wrap-around navigation (cycling from the last track back to the first, and vice versa)

---

## Technical Highlights

This project incorporates several embedded systems concepts:

* Finite-state machine (FSM) architecture for system control
* Independent management of the LCD's top and bottom display lines
* RGB LED state indication based on the current playback mode
* Button-driven track selection and playback control
* Serial communication with the MP3 player module using the ATmega328P's hardware **TX/RX** interface
* Integration of multiple hardware peripherals into a single embedded application

---

## User Guide

### Startup

When powered on, the system enters the **READY** state. The LCD displays the current status and the selected track.

### Track Selection

* **Right button:** Next track
* **Left button:** Previous track

Track selection wraps around automatically. For example:

* Pressing **Previous** while on Track 1 selects the last track.
* Pressing **Next** while on the last track returns to Track 1.

### Playback

Press the **green button** to begin playback.

The system enters the **PLAYING** state and plays the selected MP3 file from the inserted microSD card.

### Pause

Press the **green button** again to pause playback.

When playback resumes, the currently selected track restarts from the beginning.

### Volume Control

Use the potentiometer on the amplifier module to adjust the playback volume.

---

## Hardware Components

* ATmega328P
* DIYables Serial MP3 Player Module
* 16×2 LCD
* RGB LED
* HiLetgo TDA2822M Amplifier
* DWEII 3W 8Ω Speaker
* Potentiometer
* Push Buttons
* Resistors
* Jumper wires

---

## Software

This project was developed in **C/C++** using AVR.

No external software libraries were used beyond the standard AVR libraries. Communication with the MP3 module is implemented using the serial command protocol provided by the manufacturer.

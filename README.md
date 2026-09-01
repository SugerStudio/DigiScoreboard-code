# Digital Scoreboard

A compact, reusable digital scoreboard built for fast-paced indoor sports (badminton, table tennis, squash) at ASHS, replacing single-use manual scorecards and old whiteboards. NCEA Level 3 Digital Technologies project.

## Overview

The scoreboard uses an e-ink display for high visibility and long battery life, with physical tactile buttons for score input — chosen after user research showed 85.7% of respondents preferred a clicky physical press over touch or silent input.

## Hardware

- **Microcontroller:** Seeed XIAO ESP32-C3
- **Display:** 7.5" 3-colour (black/white/red) e-paper, 800x480, MT-DEPG0750RWU790F30 (Microtips Technology), UC8179 controller
- **Display driver:** Seeed ePaper Driver Board for XIAO V2
- **Buttons:** 5x Kailh Navy tactile switches
- **Power:** 10000mAh USB-C rechargeable battery
- **PCB:** Custom board designed in KiCad
- **Enclosure:** 3D printed in PLA, designed in Blender and Fusion 360

## Pin Mapping

| Signal | XIAO Pin | GPIO |
|---|---|---|
| RST | D0 | GPIO2 |
| CS | D1 | GPIO3 |
| BUSY | D2 | GPIO4 |
| DC | D3 | GPIO5 |
| SCK | D8 | GPIO8 |
| MOSI | D10 | GPIO10 |
| Button 1 | D6 | GPIO21 |
| Button 2 | D9 | GPIO9 |
| Button 3 | D4 | GPIO6 |
| Button 4 | D5 | GPIO7 |
| Button 5 | D7 | GPIO20 |

## Software

- **Arduino IDE** with the **esp32 by Espressif Systems** board package, board: XIAO_ESP32C3
- **Libraries:** GxEPD2, Adafruit GFX, Adafruit BusIO
- Display driver class: `GxEPD2_750c_GDEY075Z08`

## Repository Structure

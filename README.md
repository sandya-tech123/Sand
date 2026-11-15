# GSM Based Fire Alert System
 
A fire detection and alert system that monitors temperature, smoke and flame, sounds a local buzzer, and sends an SMS to registered phone numbers through a GSM module.
 
This repository contains:
 
| Path | What it is |
|------|-----------|
| `index.html` | Browser-based software simulator (no install needed, just open the file) |
| `firmware/gsm_fire_alert.ino` | Arduino Uno firmware for the real hardware |
| `docs/PROJECT_REPORT.md` | Full project document (abstract, design, algorithm, testing, results) |
 
## Live simulator
 
Open `index.html` in any browser, or enable **GitHub Pages** (Settings -> Pages -> Deploy from branch `main`, folder `/root`) to get a public link.
 
The simulator lets you change temperature, smoke and flame inputs, and shows the controller state, LCD text, buzzer status, the AT commands sent to the GSM module, and the SMS received on a phone.
 
## How it works
 
| State | Condition | Action |
|-------|-----------|--------|
| NORMAL | All readings below thresholds | Idle |
| WARNING | Temperature **or** smoke above threshold | LED on, buzzer beeps |
| ALARM | Flame detected, **or** temperature **and** smoke above thresholds | Continuous buzzer, SMS to all numbers, repeated every 30 s |
 
## Hardware
 
- Arduino Uno
- SIM800L GSM module (with a separate 4V / 2A supply)
- LM35 temperature sensor
- MQ-2 smoke sensor
- Flame sensor module
- 16x2 I2C LCD
- Buzzer, red LED, 220 ohm resistor
- Voltage divider (1k + 2k) on the Arduino TX -> SIM800L RX line
## Running the firmware
 
1. Install the **LiquidCrystal I2C** library in the Arduino IDE.
2. Open `firmware/gsm_fire_alert.ino`.
3. Edit `ALERT_NUMBERS` with real phone numbers (with country code).
4. Calibrate `SMOKE_THRESHOLD` for your MQ-2 in clean air.
5. Upload to the Arduino Uno.
## Upload to GitHub
 
```bash
cd gsm-fire-alert
git init
git add .
git commit -m "GSM based fire alert system"
git branch -M main
git remote add origin https://github.com/<your-username>/gsm-fire-alert.git
git push -u origin main
```
 
## Safety note
 
This is a learning project. It is not a certified fire safety device and must not replace approved smoke detectors or fire alarm systems.
 
## License
 
MIT
 

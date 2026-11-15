# GSM Based Fire Alert System — Project Report

## 1. Abstract

Fires in homes, shops and server rooms cause serious loss when nobody is present to notice them early. This project designs a low-cost system that continuously monitors temperature, smoke and flame, and on detecting a fire it sounds a local alarm and sends an SMS to pre-registered mobile numbers over the GSM network. Because it uses SMS, it works without internet access and reaches the owner anywhere there is mobile coverage. A browser-based software simulator accompanies the Arduino firmware so the logic can be tested without hardware.

## 2. Objectives

- Detect fire early using more than one sensor to reduce false alarms.
- Alert people nearby with a buzzer and LCD message.
- Alert remote people with an SMS containing the sensor readings.
- Provide a software simulator and open-source firmware.

## 3. Block diagram

```
 +-----------+
 | LM35 Temp |--+
 +-----------+  |
 +-----------+  |     +-----------------+     +-----------+
 | MQ-2 Smoke|--+---> |  Arduino Uno    |---> | 16x2 LCD  |
 +-----------+  |     |  (controller)   |     +-----------+
 +-----------+  |     |                 |     +-----------+
 | Flame Sens|--+     |                 |---> | Buzzer/LED|
 +-----------+        |                 |     +-----------+
                      |                 |     +-----------+      +-------+
                      |                 |<--> | SIM800L   |~~~~~>| Phone |
                      +-----------------+     +-----------+ SMS  +-------+
```

## 4. Components

| Component | Purpose |
|-----------|---------|
| Arduino Uno | Reads sensors, runs decision logic |
| SIM800L GSM module | Sends SMS using AT commands |
| LM35 | Temperature, 10 mV per degree C |
| MQ-2 | Smoke and combustible gas level |
| Flame sensor | Detects infrared from flame |
| 16x2 I2C LCD | Local display |
| Buzzer and LED | Local audible and visual alarm |

## 5. Working principle

1. The controller reads temperature, smoke and flame every 500 ms.
2. Readings are compared with thresholds (default 55 degrees C temperature, ADC 400 smoke).
3. The system moves between three states: NORMAL, WARNING and ALARM.
4. In ALARM state the buzzer stays on and an SMS is sent to every stored number. The SMS is repeated every 30 seconds while the alarm persists.
5. When readings fall back to normal the system resets.

## 6. Algorithm

```
loop:
    t     = read temperature
    smoke = read smoke level
    flame = read flame sensor
    hot   = t >= TEMP_THRESHOLD
    smoky = smoke >= SMOKE_THRESHOLD

    if flame or (hot and smoky): state = ALARM
    elif hot or smoky:           state = WARNING
    else:                        state = NORMAL

    if state == ALARM:
        buzzer on, LED on
        if no SMS sent yet or 30 s elapsed:
            send SMS to each number
    elif state == WARNING:
        LED on, short beep
    else:
        buzzer off, LED off
```

## 7. GSM communication

The module is driven by standard AT commands:

| Command | Meaning |
|---------|---------|
| `AT` | Check module responds |
| `AT+CPIN?` | SIM card ready |
| `AT+CREG?` | Network registration |
| `AT+CMGF=1` | Select text mode |
| `AT+CMGS="number"` | Start sending SMS, message ends with Ctrl+Z |

## 8. Software simulator

`index.html` reproduces the controller logic in the browser. Inputs are sliders for temperature and smoke and a flame toggle. Outputs show the system state, LCD text, buzzer status, the AT command log, and a mock phone inbox. A one-click fire scenario raises the readings gradually to show the NORMAL, WARNING and ALARM transitions.

## 9. Test cases

| # | Temperature | Smoke | Flame | Expected state | SMS |
|---|-------------|-------|-------|----------------|-----|
| 1 | 28 C | 80 | No | NORMAL | No |
| 2 | 65 C | 100 | No | WARNING | No |
| 3 | 30 C | 600 | No | WARNING | No |
| 4 | 65 C | 600 | No | ALARM | Yes |
| 5 | 28 C | 80 | Yes | ALARM | Yes |
| 6 | Alarm persists 30 s | - | - | ALARM | Repeat SMS |

## 10. Advantages and limitations

**Advantages:** works without internet, low cost, multi-sensor decision, reaches multiple people, easy to extend.

**Limitations:** depends on mobile network coverage and SIM balance, MQ-2 needs warm-up and calibration, SIM800L needs a stable 2 A supply, SMS can be delayed during network congestion.

## 11. Future scope

- Automatic voice call to the first number.
- GPS location in the SMS.
- Water sprinkler or relay control.
- Cloud dashboard through an ESP32.
- Battery backup and solar charging.

## 12. Conclusion

The system shows that a reliable fire alert can be built from inexpensive parts. Combining sensors reduces false alarms, and GSM SMS gives a way to notify owners anywhere. The simulator makes the design easy to demonstrate and test.

## 13. References

- SIM800 Series AT Command Manual
- Arduino documentation, `SoftwareSerial` library
- LM35 and MQ-2 sensor datasheets

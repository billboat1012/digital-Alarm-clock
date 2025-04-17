#include <Wire.h>
    #include <RTClib.h>
    #include <TM1637Display.h>
    #include <Servo.h>
    
    // Display pins
    const int CLK = 6;
    const int DIO = 7;
    TM1637Display display(CLK, DIO);
    
    // RTC & Servo
    RTC_DS3231 rtc;
    Servo myServo;
    const int servoPin = 8;
    
    // Button pins
    const int setButtonPin = 13;
    const int incButtonPin = A0;
    const int decButtonPin = A1;
    
    // Mode handling
    enum Mode { NORMAL, SET_TIME, SET_ALARM };
    Mode currentMode = NORMAL;
    
    int editHour = 0;
    int editMinute = 0;
    
    // Alarm
    int alarmHour = 7;
    int alarmMinute = 0;
    bool alarmEnabled = true;
    
    // Timing
    unsigned long lastInteraction = 0;
    const unsigned long timeoutDuration = 5000;
    bool lastSetButtonState = HIGH;
    bool lastIncButtonState = HIGH;
    bool lastDecButtonState = HIGH;
    
    void setup() {
      pinMode(setButtonPin, INPUT_PULLUP);
      pinMode(incButtonPin, INPUT_PULLUP);
      pinMode(decButtonPin, INPUT_PULLUP);
      myServo.attach(servoPin);
      display.setBrightness(7);
      Wire.begin();
      rtc.begin();
    
      // Uncomment once to set the initial time
      // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    
    void loop() {
      DateTime now = rtc.now();
    
      // Timeout to go back to normal mode
      if (currentMode != NORMAL && (millis() - lastInteraction > timeoutDuration)) {
        currentMode = NORMAL;
      }
    
      // --- BUTTON: MODE SWITCH ---
      bool setPressed = digitalRead(setButtonPin) == LOW;
      if (setPressed && lastSetButtonState == HIGH) {
        if (currentMode == NORMAL) {
          currentMode = SET_TIME;
          editHour = now.hour();
          editMinute = now.minute();
        } else if (currentMode == SET_TIME) {
          rtc.adjust(DateTime(now.year(), now.month(), now.day(), editHour, editMinute, 0));
          currentMode = SET_ALARM;
          editHour = alarmHour;
          editMinute = alarmMinute;
        } else if (currentMode == SET_ALARM) {
          alarmHour = editHour;
          alarmMinute = editMinute;
          currentMode = NORMAL;
        }
        lastInteraction = millis();
      }
      lastSetButtonState = !setPressed;
    
      // --- BUTTON: INCREMENT ---
      bool incPressed = digitalRead(incButtonPin) == LOW;
      if (incPressed && lastIncButtonState == HIGH) {
        editMinute++;
        if (editMinute >= 60) {
          editMinute = 0;
          editHour = (editHour + 1) % 24;
        }
        lastInteraction = millis();
      }
      lastIncButtonState = !incPressed;
    
      // --- BUTTON: DECREMENT ---
      bool decPressed = digitalRead(decButtonPin) == LOW;
      if (decPressed && lastDecButtonState == HIGH) {
        editMinute--;
        if (editMinute < 0) {
          editMinute = 59;
          editHour = (editHour - 1 + 24) % 24;
        }
        lastInteraction = millis();
      }
      lastDecButtonState = !decPressed;
    
      // --- DISPLAY ---
      if (currentMode == NORMAL) {
        display.showNumberDecEx(now.hour() * 100 + now.minute(), 0b01000000, true);
      } else {
        display.showNumberDecEx(editHour * 100 + editMinute, 0b01000000, true);
      }
    
      // --- ALARM CHECK ---
      if (alarmEnabled && now.hour() == alarmHour && now.minute() == alarmMinute && now.second() == 0) {
        triggerAlarm();
      }
    
      delay(100); // Reduce flicker
    }
    
    void triggerAlarm() {
      myServo.write(90);
      delay(1000);
      myServo.write(0);
    }
    

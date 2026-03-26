 🎓 Multi-Factor Authenticated Digital Attendance System

 📌 Overview

This project presents a **secure digital attendance system** designed to eliminate proxy attendance in academic environments.
The system uses **multi-factor authentication** combining:

* Registration Number
* Personal PIN (DOB-based)
* One-Time Password (OTP)

It is implemented using an Arduino Uno and demonstrates key concepts of **Digital Electronics**, including **combinational and sequential circuits**.



 🚀 Features

* ✅ Three-factor authentication (Reg No + PIN + OTP)
* ✅ Proxy prevention mechanism
* ✅ Real-time attendance monitoring via Serial Monitor
* ✅ Automatic session locking when limit is reached
* ✅ Finite State Machine (FSM) based control
* ✅ LCD display interface for user interaction
* ✅ Buzzer & LED feedback system


 🛠️ Hardware Components

* Arduino Uno (ATmega328P)
* 4x4 Matrix Keypad
* I2C 16x2 LCD Display
* Buzzer
* LED
* Power Supply


⚙️ System Architecture

The system is based on:

* **Combinational Circuits** → OTP & PIN comparison, keypad decoding
* **Sequential Circuits** → FSM, counters, timers, memory storage

A Finite State Machine (FSM) controls all system operations, ensuring structured and reliable authentication.


 🔐 Authentication Process

1. Student enters registration number
2. System verifies student identity
3. Student enters personal PIN
4. System validates PIN
5. Student enters session OTP
6. If correct → marked **PRESENT**
7. If incorrect → access denied


 📊 Output

* Real-time attendance log on Serial Monitor
* Display of PRESENT/ABSENT students
* Automatic report generation at session end


 🧪 Testing

The system was tested under multiple scenarios:

* Correct and incorrect credentials
* Duplicate attendance attempts
* Timeout handling
* Session limit enforcement

All test cases passed successfully ✅


 ⚠️ Limitations

* Limited memory (Arduino constraints)
* No encryption for stored PINs
* Single authentication station may cause delays
* Data loss on power failure


🔮 Future Improvements

* EEPROM data storage
* Fingerprint sensor integration
* GSM-based OTP delivery
* Real-Time Clock (RTC) for timestamps
* Multiple input stations
* PIN encryption


🎯 Conclusion

This project provides a **low-cost, efficient, and secure solution** for attendance management using embedded systems.
It demonstrates practical application of digital electronics concepts in real-world problem solving.

 👨‍💻 Authors

Group 06 – BENG24ETE-3
Dar es Salaam Institute of Technology (DIT)

* Faida Sylivester Mosses
* Lameck Halle Mbetwa
* Kelvin Pantaleo Massawe
* Isack Festo Lubigisa
* Brenda Edward Kimaro
* Debora Nelson Moshi
* John Julius John

This project is for academic and educational purposes.


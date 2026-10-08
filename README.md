# Smart-Attendance-System
ESP32 based RFID Smart Attendance System with Google Sheets integration

## 📌 About the Project

This project is designed to automate attendance using RFID technology. 
An RFID card is scanned using the RC522 RFID reader, and the ESP32 
identifies the registered student and records their attendance.

The attendance data can be sent to Google Sheets for easy storage 
and monitoring.

## ⚙️ How It Works

1. Student scans their RFID card.
2. RC522 RFID reader reads the card UID.
3. ESP32 checks the UID against registered students.
4. If the card is registered, attendance is marked.
5. The result is displayed to the user.
6. Attendance data is sent to Google Sheets.

## 🛠️ Components Used

- ESP32
- RC522 RFID Reader
- RFID Cards/Tags
- OLED Display
- Buzzer
- Jumper Wires
- Breadboard

## 💻 Software Used

- Arduino IDE
- Google Sheets
- Google Apps Script

## ✨ Features

- RFID-based student identification
- Automatic attendance marking
- OLED display feedback
- Buzzer notification
- Google Sheets integration
- ESP32-based implementation

## 🚀 Future Improvements

- Prevent duplicate attendance
- Add a web dashboard
- Add authentication
- Improve anti-proxy attendance mechanisms
- Store additional student information

## 👩‍💻 Author

Arshia Jain

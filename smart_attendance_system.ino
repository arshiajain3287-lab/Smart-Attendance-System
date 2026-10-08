
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <time.h>

// ===============================
// Wi-Fi
// ===============================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";   // Put your wifi password here

// ===============================
// Google Apps Script Web App
// ===============================
const char* googleScriptURL =
"YOUR_GOOGLE_SHEET_URL";   /*

  GOOGLE SHEETS SETUP
  -------------------

  This sends attendance data to Google Sheets.

  STEP 1:
  Create a new Google Sheet.

  STEP 2:
  Rename the sheet/tab to:

          Class 1

  STEP 3:
  In Google Sheets, go to:

          Extensions → Apps Script

  STEP 4:
  Delete the existing code in Apps Script and paste
  the following code:

  ------------------------------------------------------------

  function doGet(e) {

    var sheet = SpreadsheetApp
                    .getActiveSpreadsheet()
                    .getSheetByName("Class 1");

    var name = e.parameter.name;
    var date = e.parameter.date;
    var time = e.parameter.time;
    var status = e.parameter.status;

    sheet.appendRow([
      "",
      name,
      date,
      time,
      status
    ]);

    return ContentService
           .createTextOutput("Attendance recorded successfully");
  }

  ------------------------------------------------------------

  STEP 5:
  Click:

          Deploy → New deployment

  STEP 6:
  Select:

          Web app

  STEP 7:
  Set:

          Execute as: Me
          Who has access: Anyone

  STEP 8:
  Click "Deploy".

  STEP 9:
  Google will give you a Web App URL that looks like:

  https://script.google.com/macros/s/XXXXXXXXXXXX/exec

  Copy this URL.

  STEP 10:
  Paste YOUR URL into the variable below:

  ------------------------------------------------------------
*/


// ============================================================
// GOOGLE SHEETS URL
// ============================================================

// Replace the text below with YOUR OWN Google Apps Script
// Web App URL.
//
// Example:
// String scriptURL =
// "https://script.google.com/macros/s/XXXXXXXXXXXX/exec";



// ===============================
// RFID Pins
// ===============================
#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);

// ===============================
// Buzzer
// GPIO 4 -> 1k resistor -> BC547 Base
// BC547 Collector -> Buzzer negative
// BC547 Emitter -> GND
// Buzzer positive -> 3.3V
// ===============================
#define BUZZER_PIN 4

// ===============================
// Student Information
// ===============================
struct Student {
  String uid;
  String name;
  bool present;
  String attendanceDate;
  String attendanceTime;
};

Student students[] = {
  {"A5 1A F0 06", "Arshia Jain", false, "", ""},
  {"1C 5E 26 07", "Pushkar Singh", false, "", ""}
};

int totalStudents = 2;


// =====================================================
// BUZZER FUNCTIONS
// =====================================================

// One short beep = successful attendance
void beepOnce() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(300);
  digitalWrite(BUZZER_PIN, LOW);
}

// Two short beeps = error/already marked
void beepTwice() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);

  delay(150);

  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
}


// =====================================================
// SEND ATTENDANCE TO GOOGLE SHEET
// =====================================================

void sendToGoogleSheet(String name, String date, String time, String status) {

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  HTTPClient http;

  // Encode special characters
  name.replace(" ", "%20");
  date.replace("/", "%2F");
  time.replace(" ", "%20");
  time.replace(":", "%3A");
  status.replace(" ", "%20");

  String url = String(googleScriptURL);

  url += "?name=" + name;
  url += "&date=" + date;
  url += "&time=" + time;
  url += "&status=" + status;

  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  http.begin(url);

  // Send request silently
  http.GET();

  http.end();
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // RFID
  SPI.begin();
  rfid.PCD_Init();

  Serial.println();
  Serial.println("Connecting to Wi-Fi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // India time = UTC + 5:30
  configTime(19800, 0, "pool.ntp.org", "time.nist.gov");

  struct tm timeinfo;

  Serial.println("Getting date and time...");

  while (!getLocalTime(&timeinfo)) {
    Serial.println("Waiting for time...");
    delay(1000);
  }

  Serial.println("Date and time synchronized!");

  Serial.println();
  Serial.println("================================");
  Serial.println("     RFID ATTENDANCE SYSTEM");
  Serial.println("================================");
  Serial.println();

  Serial.println("Google Sheet connected.");
  Serial.println("Scan your RFID card...");
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // Check for RFID card
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Create UID string
  String uid = "";

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10) {
      uid += "0";
    }

    uid += String(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) {
      uid += " ";
    }
  }

  uid.toUpperCase();

  Serial.println();
  Serial.println("--------------------------------");

  int studentIndex = -1;

  // Find student
  for (int i = 0; i < totalStudents; i++) {

    if (uid == students[i].uid) {

      studentIndex = i;
      break;
    }
  }


  // ===================================================
  // UNAUTHORIZED CARD
  // ===================================================

  if (studentIndex == -1) {

    Serial.println("Unauthorized RFID card!");
    Serial.println("Attendance NOT marked.");

    beepTwice();
  }


  // ===================================================
  // ALREADY PRESENT
  // ===================================================

  else if (students[studentIndex].present == true) {

    Serial.println(
      students[studentIndex].name +
      " is already marked present."
    );

    Serial.print("Previous date: ");
    Serial.println(students[studentIndex].attendanceDate);

    Serial.print("Previous time: ");
    Serial.println(students[studentIndex].attendanceTime);

    beepTwice();
  }


  // ===================================================
  // NEW ATTENDANCE
  // ===================================================

  else {

    struct tm timeinfo;

    if (getLocalTime(&timeinfo)) {

      char dateBuffer[20];
      char timeBuffer[20];

      // Date
      strftime(
        dateBuffer,
        sizeof(dateBuffer),
        "%d/%m/%Y",
        &timeinfo
      );

      // Time
      strftime(
        timeBuffer,
        sizeof(timeBuffer),
        "%I:%M:%S %p",
        &timeinfo
      );


      // Save locally
      students[studentIndex].present = true;

      students[studentIndex].attendanceDate =
        String(dateBuffer);

      students[studentIndex].attendanceTime =
        String(timeBuffer);


      // Serial Monitor
      Serial.println(
        students[studentIndex].name +
        " marked PRESENT."
      );

      Serial.print("Date: ");
      Serial.println(dateBuffer);

      Serial.print("Time: ");
      Serial.println(timeBuffer);

      Serial.println("Attendance marked successfully.");


      // One beep for successful attendance
      beepOnce();


      // Send to Google Sheet
      sendToGoogleSheet(
        students[studentIndex].name,
        String(dateBuffer),
        String(timeBuffer),
        "Present"
      );
    }

    else {

      Serial.println("Could not get date and time.");

      beepTwice();
    }
  }


  Serial.println("--------------------------------");
  Serial.println("Scan another RFID card...");


  // Stop RFID communication
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(1500);
}
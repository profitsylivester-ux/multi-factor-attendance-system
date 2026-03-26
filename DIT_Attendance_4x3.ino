// ==========================================================
//  DIT DIGITAL ATTENDANCE SYSTEM  (4×3 KEYPAD VERSION)
//  Module  : Digital Electronics (ETU 07322)
//  Lecturer: Dr Georgia Rugumira
//  Class   : BENG24ETE-3  |  Group 06
// ==========================================================
//  WIRING:
//  I2C LCD  -> GND=GND, VCC=5V, SDA=A4, SCL=A5
//  KEYPAD   -> Rows: 9,8,7,6 | Cols: 5,4,3   (3 cols only)
//  BUZZER   -> Pin 10   |   LED -> Pin 13
// ==========================================================
//  4×3 KEYPAD LAYOUT:
//  [ 1 ][ 2 ][ 3 ]
//  [ 4 ][ 5 ][ 6 ]
//  [ 7 ][ 8 ][ 9 ]
//  [ * ][ 0 ][ # ]
// ==========================================================
//  CONTROLS:
//  #      = Confirm / Next
//  *      = Backspace / Cancel (go back to READY)
//  **     = End session early  (press * twice quickly)
//  At IDLE: press # to begin session setup
// ==========================================================

#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ── 4×3 Keypad ──────────────────────────────────────────
const byte ROWS = 4, COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS]  = {5, 4, 3};          // adjust to your wiring
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ── Hardware ─────────────────────────────────────────────
#define BUZZER      10
#define LED         13
#define TIME_LIMIT  30000   // 30 s per student

// ── Student roster ───────────────────────────────────────
// PIN = DDMM of birthday  e.g. 15 March → 1503
struct Student { String reg6, pin, name; };
Student students[] = {
  {"465425","DDMM","Lameck Mbetwa"},
  {"438059","DDMM","Kelvin Massawe"},
  {"414043","DDMM","Isack Lubigisa"},
  {"474625","0412","Faida Mosses"},
  {"474914","DDMM","Brenda Kimaro"},
  {"455798","DDMM","Debora Moshi"},
  {"487791","DDMM","John Julius"},
};
const int TOTAL = 7;

// ── State machine ────────────────────────────────────────
enum State {
  S_IDLE, S_ENTER_COUNT, S_READY,
  S_ENTER_REG, S_ENTER_PIN, S_ENTER_OTP, S_DONE
};
State currentState = S_IDLE;

// ── Session variables ────────────────────────────────────
String sessionOTP       = "";
int    physicalCount    = 0;
int    presentCount     = 0;
String inputBuffer      = "";
int    currentStudent   = -1;
bool   signed_in[7]     = {false};
unsigned long studentStartTime = 0;

// Double-star (end-session) detection
unsigned long lastStarTime = 0;
bool          firstStar    = false;
#define DOUBLE_STAR_MS 600   // max gap between two * presses

// ── Helpers ──────────────────────────────────────────────
void beepOK()   { tone(BUZZER,1200,150); delay(200); tone(BUZZER,1600,150); }
void beepFail() { tone(BUZZER,300,600); }
void beepStart(){ for(int i=0;i<3;i++){ tone(BUZZER,800+i*200,100); delay(150); } }

bool isNumber(char k){
  return k>='0' && k<='9';
}

void lcdShow(String top, String bot){
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(top);
  lcd.setCursor(0,1); lcd.print(bot);
}

String stars(int n){
  String s="";
  for(int i=0;i<n;i++) s+="*";
  return s;
}

String progressBar(){
  return "Present:"+String(presentCount)+"/"+String(physicalCount);
}

// ── End session ──────────────────────────────────────────
void endSession(){
  currentState = S_DONE;
  lcdShow("Session Closed!","Check Serial...");
  beepStart();
  Serial.println(F("============================================="));
  Serial.println(F("           ATTENDANCE REPORT                "));
  Serial.println(F("============================================="));
  Serial.print(F("Physical Count : ")); Serial.println(physicalCount);
  Serial.print(F("Signed In      : ")); Serial.println(presentCount);
  Serial.print(F("Absent         : ")); Serial.println(TOTAL - presentCount);
  Serial.println(F("\nPRESENT:"));
  for(int i=0;i<TOTAL;i++)
    if(signed_in[i]){ Serial.print("  V "); Serial.println(students[i].name); }
  Serial.println(F("\nABSENT:"));
  bool anyAbsent = false;
  for(int i=0;i<TOTAL;i++)
    if(!signed_in[i]){ Serial.print("  X "); Serial.println(students[i].name); anyAbsent=true; }
  if(!anyAbsent) Serial.println(F("  None - Full attendance!"));
  Serial.println(F("============================================="));
  delay(4000);
  lcdShow("Press # to","start new session");
  currentState = S_IDLE;
}

// ── Timeout helper (called every loop) ───────────────────
void checkTimeout(){
  if(currentState==S_ENTER_REG ||
     currentState==S_ENTER_PIN ||
     currentState==S_ENTER_OTP){
    if(millis()-studentStartTime > TIME_LIMIT){
      lcdShow("Time Expired!","Step aside");
      beepFail(); digitalWrite(LED,LOW);
      inputBuffer=""; currentStudent=-1;
      delay(2000);
      currentState = S_READY;
      lcdShow(progressBar(),"# = My turn");
    }
  }
}

// ── Cancel / return to READY ──────────────────────────────
void cancelToReady(){
  inputBuffer=""; currentStudent=-1;
  currentState = S_READY;
  lcdShow(progressBar(),"# = My turn");
}

// ── Setup ────────────────────────────────────────────────
void setup(){
  Serial.begin(9600);
  keypad.setDebounceTime(50);
  lcd.init(); lcd.backlight();
  pinMode(BUZZER,OUTPUT); pinMode(LED,OUTPUT);
  lcdShow("DIT Attendance","Press # = Start");
}

// ── Main loop ────────────────────────────────────────────
void loop(){
  checkTimeout();

  char key = keypad.getKey();
  if(!key) return;

  // ── Double-star detection (end session from READY) ──────
  if(key=='*'){
    if(firstStar && (millis()-lastStarTime < DOUBLE_STAR_MS)){
      // Double-star pressed → end session
      firstStar = false;
      if(currentState==S_READY || currentState==S_ENTER_REG ||
         currentState==S_ENTER_PIN || currentState==S_ENTER_OTP){
        endSession();
        return;
      }
    } else {
      firstStar = true;
      lastStarTime = millis();
    }
  } else {
    firstStar = false;   // reset if any other key pressed
  }

  // ── State machine ───────────────────────────────────────
  switch(currentState){

    // ── IDLE ─────────────────────────────────────────────
    case S_IDLE:
      if(key=='#'){
        inputBuffer=""; physicalCount=0; presentCount=0;
        for(int i=0;i<TOTAL;i++) signed_in[i]=false;
        lcdShow("How many here?","Type + press #");
        currentState = S_ENTER_COUNT;
      }
      break;

    // ── ENTER PHYSICAL COUNT ──────────────────────────────
    case S_ENTER_COUNT:
      if(isNumber(key) && inputBuffer.length()<3){
        inputBuffer += key;
        lcdShow("How many here?", inputBuffer);
      } else if(key=='*' && inputBuffer.length()>0){
        inputBuffer.remove(inputBuffer.length()-1);
        lcdShow("How many here?", inputBuffer+" ");
      } else if(key=='#' && inputBuffer.length()>0){
        physicalCount = inputBuffer.toInt();
        if(physicalCount==0){
          lcdShow("Count must be","at least 1!");
          delay(2000);
          inputBuffer="";
          lcdShow("How many here?","Type + press #");
          break;
        }
        inputBuffer="";
        randomSeed(analogRead(A4)+millis());
        sessionOTP = String(random(100000,999999));
        Serial.println(F("============================================="));
        Serial.print(F("  SESSION OTP : ")); Serial.println(sessionOTP);
        Serial.println(F("  >>> Announce this OTP to students <<<"));
        Serial.println(F("============================================="));
        lcdShow("OTP on Serial!","Tell students");
        beepStart();
        delay(3000);
        currentState = S_READY;
        lcdShow(progressBar(),"# = My turn");
      }
      break;

    // ── READY (waiting for next student) ─────────────────
    case S_READY:
      if(key=='#'){
        inputBuffer=""; currentStudent=-1;
        studentStartTime = millis();
        lcdShow("Reg (last 6):","");
        currentState = S_ENTER_REG;
      }
      // double-star handled above
      break;

    // ── ENTER REG NUMBER ──────────────────────────────────
    case S_ENTER_REG:
      if(isNumber(key) && inputBuffer.length()<6){
        inputBuffer += key;
        lcd.setCursor(0,1); lcd.print(inputBuffer+"      ");
      } else if(key=='*'){
        if(inputBuffer.length()>0){
          inputBuffer.remove(inputBuffer.length()-1);
          lcd.setCursor(0,1); lcd.print(inputBuffer+"      ");
        } else {
          cancelToReady();   // * on empty = cancel
        }
      } else if(key=='#' && inputBuffer.length()==6){
        currentStudent = -1;
        for(int i=0;i<TOTAL;i++)
          if(students[i].reg6==inputBuffer){ currentStudent=i; break; }
        inputBuffer="";

        if(currentStudent==-1){
          lcdShow("Not Found!","Check reg no.");
          beepFail(); delay(2000); cancelToReady();
        } else if(signed_in[currentStudent]){
          lcdShow("Already signed!",
                  students[currentStudent].name.substring(0,16));
          beepFail(); delay(2000); cancelToReady();
        } else {
          lcdShow("Birthday PIN:","(4 digits + #)");
          currentState = S_ENTER_PIN;
        }
      }
      break;

    // ── ENTER PIN ────────────────────────────────────────
    case S_ENTER_PIN:
      if(isNumber(key) && inputBuffer.length()<4){
        inputBuffer += key;
        lcd.setCursor(0,1); lcd.print(stars(inputBuffer.length())+"    ");
      } else if(key=='*'){
        if(inputBuffer.length()>0){
          inputBuffer.remove(inputBuffer.length()-1);
          lcd.setCursor(0,1); lcd.print(stars(inputBuffer.length())+"    ");
        } else {
          // back to reg entry
          inputBuffer=""; currentStudent=-1;
          studentStartTime = millis();
          lcdShow("Reg (last 6):","");
          currentState = S_ENTER_REG;
        }
      } else if(key=='#' && inputBuffer.length()==4){
        if(inputBuffer != students[currentStudent].pin){
          lcdShow("Wrong PIN!","Access Denied");
          beepFail(); inputBuffer=""; currentStudent=-1;
          delay(2000); cancelToReady();
        } else {
          inputBuffer="";
          lcdShow("Session OTP:","(6 digits + #)");
          currentState = S_ENTER_OTP;
        }
      }
      break;

    // ── ENTER OTP ────────────────────────────────────────
    case S_ENTER_OTP:
      if(isNumber(key) && inputBuffer.length()<6){
        inputBuffer += key;
        lcd.setCursor(0,1); lcd.print(stars(inputBuffer.length())+"      ");
      } else if(key=='*'){
        if(inputBuffer.length()>0){
          inputBuffer.remove(inputBuffer.length()-1);
          lcd.setCursor(0,1); lcd.print(stars(inputBuffer.length())+"      ");
        } else {
          // back to PIN entry
          inputBuffer="";
          lcdShow("Birthday PIN:","(4 digits + #)");
          currentState = S_ENTER_PIN;
        }
      } else if(key=='#' && inputBuffer.length()==6){
        unsigned long secs = (millis()-studentStartTime)/1000;
        if(inputBuffer==sessionOTP){
          signed_in[currentStudent] = true;
          presentCount++;
          inputBuffer="";
          lcdShow("PRESENT!",
                  students[currentStudent].name.substring(0,16));
          beepOK(); digitalWrite(LED,HIGH); delay(2000); digitalWrite(LED,LOW);
          Serial.print(presentCount); Serial.print(". ");
          Serial.print(students[currentStudent].name);
          Serial.print(F(" | PRESENT | "));
          Serial.print(secs); Serial.println(F("s"));
          currentStudent=-1;
          if(presentCount >= physicalCount){
            lcdShow("LIMIT REACHED!","Locking now...");
            for(int b=0;b<3;b++){ tone(BUZZER,1800,150); delay(200); }
            delay(1500); endSession();
          } else {
            currentState = S_READY;
            lcdShow(progressBar(),"# = My turn");
          }
        } else {
          lcdShow("Wrong OTP!","See Lecturer");
          beepFail(); inputBuffer=""; currentStudent=-1;
          delay(2500); cancelToReady();
        }
      }
      break;

    default: break;
  }
}

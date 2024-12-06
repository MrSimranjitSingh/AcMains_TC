#include <Arduino.h>
#include <esp_task_wdt.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

#define LED_PIN 2
#define AC_MAINS_FREQ 60  //In Hz
#define ZCD_PIN 19        //Zero cross detection pin from optocoupler
#define ENABLE_PIN 13     //Enable pin for gate drivers

#define ENABLE_HIGH HIGH  //can be used to invert esp32 output
#define ENABLE_LOW LOW

#define TRIGGER FALLING

uint8_t broadcastAddress[] = { 0xE4, 0x65, 0xB8, 0x27, 0x39, 0xB4 };  //MAC address of sender Esp32

//Must match the data structure sent by sender
typedef struct struct_message_in {
  float on_time;
  float frequency;
} struct_message_in;

typedef struct struct_message_out {
  uint16_t message;
} struct_message_out;

struct_message_out outgoingMessage;  // To hold data to be sent
struct_message_in incomingMessage;   // To hold incoming messages

// Define variables to store incoming readings
float incoming_on_time;
float incoming_frequency;

uint16_t counter = 0;
esp_now_peer_info_t peerInfo;

bool delay_compensated = false;
unsigned long DELAY_TIME_US = 830;  //Time to adjust for ZCD switching

unsigned long DUTY_CYCLE_US = 0;                               //Duty cycle time in micro-seconds
const float time_half_wave_us = (0.5 / AC_MAINS_FREQ) * 1000 * 1000;  // 8.33ms for 60Hz
const long error_margin_us = 1000.0;                           //Margin time used to disable interrupts to prevent false triggers
float SKIP_TIME_US = 1 * time_half_wave_us + error_margin_us;  //Time for which to disable interrupts (number of AC cycles to ignore)

hw_timer_t* Zcd_Delay_Timer = NULL;
hw_timer_t* Duty_Cycle_Timer = NULL;
hw_timer_t* Skip_Timer = NULL;

struct struct_zcd {
  const uint8_t PIN;
  bool has_interrupted;
};

struct_zcd zcd = { ZCD_PIN, false };


//This is executed when pusle from
//ZCD circuit was triggered by esp32
void IRAM_ATTR Zcd_ISR() {
  detachInterrupt(zcd.PIN);
  zcd.has_interrupted = true;
}

//This is executed when ZCD pulse is triggered
//and time is adjusted because ZCD circuit triggers
//before zero crossing
void IRAM_ATTR Zcd_Delay_ISR() {
  delay_compensated = true;
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(ENABLE_PIN, ENABLE_HIGH);  //Enable the gate drivers
}

//This is executed after on_time is passed
void IRAM_ATTR Duty_Cycle_ISR() {
  digitalWrite(LED_PIN, LOW);
  digitalWrite(ENABLE_PIN, ENABLE_LOW);  //Disable the gate drivers
}

//This is executed after number of AC cycles
//set by user have been skipped
void IRAM_ATTR Skip_ISR() {
  attachInterrupt(zcd.PIN, Zcd_ISR, TRIGGER);
}


void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(ZCD_PIN, INPUT);

  digitalWrite(ENABLE_PIN, ENABLE_LOW);
  digitalWrite(LED_PIN, LOW);

  //esp_task_wdt_init(15, true);
  //esp_task_wdt_add(NULL);
  //esp_task_wdt_reset();
  disableCore0WDT();
  disableCore1WDT();

  setup_timers();
  update_data();

  WiFi.mode(WIFI_STA);
  Serial.print("\r\nMy MAC address is: ");
  Serial.println(WiFi.macAddress());

  setup_espnow();
}

void loop() {

  if (zcd.has_interrupted) {
    timerRestart(Zcd_Delay_Timer);      // reset the duty-cycle timer counter
    timerAlarmEnable(Zcd_Delay_Timer);  // start the duty-cycle timer
    zcd.has_interrupted = false;
  }
  if (delay_compensated) {
    delay_compensated = false;
    timerRestart(Duty_Cycle_Timer);      // reset timer counter
    timerAlarmEnable(Duty_Cycle_Timer);  // start the timer

    timerRestart(Skip_Timer);      // reset timer counter
    timerAlarmEnable(Skip_Timer);  // start the timer
  }
}
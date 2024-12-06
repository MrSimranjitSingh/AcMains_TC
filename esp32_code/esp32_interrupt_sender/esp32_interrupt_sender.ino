#include <Arduino.h>
#include <esp_task_wdt.h>
#include <U8x8lib.h>
#include "AiEsp32RotaryEncoder.h"
#include "AiEsp32RotaryEncoderNumberSelector.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

#define LED_PIN 2
#define AC_MAINS_FREQ 60  //In Hz

#define RE_1_A_PIN 13
#define RE_1_B_PIN 14
#define RE_1_BUTTON_PIN 27

#define RE_2_A_PIN 25
#define RE_2_B_PIN 26
#define RE_2_BUTTON_PIN 33

#define RE_VCC_PIN -1  //Put -1 if Rotary encoder Vcc is connected directly to 3.3V
#define RE_STEPS 2     //depending on your encoder - try 1,2 or 4 to get expected behaviour

//Duty cycle limit % for interrupter
#define RE_1_MIN 0
#define RE_1_MAX 50
#define RE_1_STEP 5

//Pulse skipping
#define RE_2_MIN 1
#define RE_2_MAX 119
#define RE_2_STEP 1

uint8_t broadcastAddress[] = { 0x34, 0x5F, 0x45, 0xA9, 0x2C, 0x94 };  //MAC address of receiver Esp32

//Must match the data structure sent by receiver
typedef struct struct_message_in {
  uint16_t message;
} struct_message_in;


typedef struct struct_message_out {
  float on_time;
  float frequency;
} struct_message_out;

struct_message_out outgoingMessage;  // To hold data to be sent
struct_message_in incomingMessage;   // To hold incoming messages

// Define variables to store incoming readings
uint16_t incoming_message;
esp_now_peer_info_t peerInfo;

const int default_RE_DC = RE_1_MAX;
const float default_RE_Skip = RE_2_MAX;

AiEsp32RotaryEncoder* RE_1 = new AiEsp32RotaryEncoder(RE_1_A_PIN, RE_1_B_PIN, RE_1_BUTTON_PIN, RE_VCC_PIN, RE_STEPS);
AiEsp32RotaryEncoder* RE_2 = new AiEsp32RotaryEncoder(RE_2_A_PIN, RE_2_B_PIN, RE_2_BUTTON_PIN, RE_VCC_PIN, RE_STEPS);
AiEsp32RotaryEncoderNumberSelector numberRE1Selector = AiEsp32RotaryEncoderNumberSelector();
AiEsp32RotaryEncoderNumberSelector numberRE2Selector = AiEsp32RotaryEncoderNumberSelector();

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE);

void IRAM_ATTR RE_1_ISR() {
  RE_1->readEncoder_ISR();
}
void IRAM_ATTR RE_2_ISR() {
  RE_2->readEncoder_ISR();
}


void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  //esp_task_wdt_init(15, true);
  //esp_task_wdt_add(NULL);
  //esp_task_wdt_reset();
  //disableCore0WDT();
  //disableCore1WDT();

  setup_encoders();
  setup_lcd();
  delay(1000);
  update_data();

  WiFi.mode(WIFI_STA);
  Serial.print("\r\nMy MAC address is: ");
  Serial.println(WiFi.macAddress());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop() {
  //in loop call your custom function which will process rotary encoder values
  rotary_loop();

  static unsigned long previousMillis = 0;
  static const long interval = 2000;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&outgoingMessage, sizeof(outgoingMessage));

    /*String s;
    if (result == ESP_OK) {
      s = "Send: ok  ";
    } else {
      s = "Send: fail";
    }

    const char* foo = s.c_str();
    u8x8.drawString(0, 5, foo);*/
  }
}
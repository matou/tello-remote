#include <WiFi.h>
#include <WiFiUdp.h>

// includes wifi password, etc.; that file shouldn't go into the repository
#include "secrets.h"

// settings
#include "settings.h"


// WiFi network name and password:
const char * networkName = MY_SSID;
const char * networkPswd = MY_WIFI_PWD;

//IP address to send UDP data to:
// either use the ip address of the server or 
// a network broadcast address
//const char * telloAddress = "192.168.178.53";
const char * telloAddress = "192.168.10.1";
const int telloPort = 8889;
const int ownUdpPort = 9000;

// Are we currently connected to WiFi?
boolean wifiConnected = false;

// Do we have a Tello connection, yet? 
boolean telloConnected = false;

//The udp library class
WiFiUDP udp;

// toggle switch states
boolean arm=false, fly=false, fast=false; 
boolean arm_old=false, fly_old=false, fast_old=false;

// PicoPlanet states
int planet0 = 0, planet1 = 0, planet2 = 0;
int planet0_old = 0, planet1_old = 0, planet2_old = 0;

// joystick values
int left_right, forw_back, up_down, yaw; 

// a buffer for incoming packets
char packetBuffer[255];

// all LED pins
int leds[] = {LED_LAND, LED_FLY, LED_SLOW, LED_FAST, LED_PLANET_0, LED_PLANET_2, LED_POWER, LED_DISARM, LED_ARM};
int n_led = 9;              

// joystick calibration
int min_left_right = MIN_LEFT_RIGHT;
int max_left_right = MAX_LEFT_RIGHT;
int min_forw_back = MIN_FORW_BACK;
int max_forw_back = MAX_FORW_BACK;
int min_up_down = MIN_UP_DOWN;
int max_up_down = MAX_UP_DOWN;
int min_yaw = MIN_YAW;
int max_yaw = MAX_YAW;

// armed?
boolean armed = false;

void setup(){
    // Initilize hardware serial:
    Serial.begin(115200);

    // Connect to the WiFi network
    connectToWiFi(networkName, networkPswd);

    // initialize PINS
    initPins();
    
    arm = digitalRead(SW_ARM) == 0; 
    fly = digitalRead(SW_FLY) == 0 ;
    fast = digitalRead(SW_FAST) == 0;

    if (!fast) {
        Serial.println("LED test");
        for (int i=0; i<n_led; i++) 
            digitalWrite(leds[i], LOW);
        for (int n=0; n<3; n++) {
            for (int i=0; i<n_led; i++) {
                digitalWrite(leds[i], HIGH);
                delay(50);
            }
            delay(500);
            for (int i=0; i<n_led; i++) {
                digitalWrite(leds[i], LOW);
                delay(50);
            }
            delay(500);
        }
    }

    // set correct LEDs
    if (arm) digitalWrite(LED_ARM, HIGH); 
    else digitalWrite(LED_DISARM, HIGH);
    if (fly) digitalWrite(LED_FLY, HIGH); 
    else digitalWrite(LED_LAND, HIGH);
    if (fast) digitalWrite(LED_FAST, HIGH); 
    else digitalWrite(LED_SLOW, HIGH);

    digitalWrite(LED_POWER, HIGH);
    digitalWrite(LED_PLANET_0, HIGH);
    digitalWrite(LED_PLANET_1, HIGH);
    digitalWrite(LED_PLANET_2, HIGH);

    min_left_right = max_left_right = left_right = analogRead(JOY_LEFT_RIGHT);
    min_forw_back = max_forw_back = forw_back = analogRead(JOY_FORW_BACK);
    min_up_down = max_up_down = up_down = analogRead(JOY_UP_DOWN);
    min_yaw = max_yaw = yaw = analogRead(JOY_YAW);

    Serial.println("init done");
}

void loop(){
    // Tello not yet connected? Try to connect. 
    if (!telloConnected) {
        if (wifiConnected) {
            delay(500);
            telloConnected = connect();
            delay(1000);
            Serial.println("Tello connected");
            
            for (int i=0; i<5; i++) {
                digitalWrite(LED_PLANET_0, LOW);
                digitalWrite(LED_PLANET_1, LOW);
                digitalWrite(LED_PLANET_2, LOW);
                delay(300);
                digitalWrite(LED_PLANET_0, HIGH);
                digitalWrite(LED_PLANET_1, HIGH);
                digitalWrite(LED_PLANET_2, HIGH);
                delay(300);
            }
        }
    }

    // check toggle switch states
    arm = digitalRead(SW_ARM) == 0; 
    fly = digitalRead(SW_FLY) == 0 ;
    fast = digitalRead(SW_FAST) == 0;

    // check joystick values
    left_right = analogRead(JOY_LEFT_RIGHT);
    forw_back = analogRead(JOY_FORW_BACK);
    up_down = analogRead(JOY_UP_DOWN);
    yaw = analogRead(JOY_YAW);

    // ongoing calibration
    if (left_right < min_left_right) min_left_right = left_right;
    if (left_right > max_left_right) max_left_right = left_right;
    if (forw_back < min_forw_back) min_forw_back = forw_back;
    if (forw_back > max_forw_back) max_forw_back = forw_back;
    if (up_down < min_up_down) min_up_down = up_down;
    if (up_down > max_up_down) max_up_down = up_down;
    if (yaw < min_yaw) min_yaw = yaw;
    if (yaw > max_yaw) max_yaw = yaw;

    // check PicoPlanet inputs
    planet0 = digitalRead(SW_PLANET_0);
    planet1 = digitalRead(SW_PLANET_1);
    planet2 = digitalRead(SW_PLANET_2);

    // any changes in toggle switches? 
    if (arm != arm_old) {
        if (arm) {
            digitalWrite(LED_ARM, HIGH); 
            digitalWrite(LED_DISARM, LOW);
            armed = true;
        }

        if (!arm) {
            emergency(); // turn off motors 
            digitalWrite(LED_ARM, LOW);
            digitalWrite(LED_DISARM, HIGH);
            armed = false;
        }
        arm_old = arm;
    }

    if (fly != fly_old) {
        // only take off if armed
        if (fly && armed) {
            digitalWrite(LED_FLY, HIGH);
            // TODO maybe a short delay?
            takeoff();
            digitalWrite(LED_LAND, LOW);
        } else {
            land();
            digitalWrite(LED_FLY, LOW);
            digitalWrite(LED_LAND, HIGH);
        }
        fly_old = fly;
    }

    if (fast != fast_old) {
        // TODO maybe restrict speed in rc command if slow?  
        if (fast) {
            digitalWrite(LED_FAST, HIGH);
            digitalWrite(LED_SLOW, LOW);
        } else {
            digitalWrite(LED_FAST, LOW);
            digitalWrite(LED_SLOW, HIGH);
        }
        fast_old = fast;
    }

    // send updated joystick values if armed and flying
    if (armed && fly) {
        left_right = normalize(left_right, min_left_right, max_left_right);
        forw_back = normalize(forw_back, min_forw_back, max_forw_back) * (-1);
        up_down = normalize(up_down, min_up_down, max_up_down) * (-1);
        yaw = normalize(yaw, min_yaw, max_yaw);
    
        if (fast) {
            Serial.printf("%d, %d, %d, %d\n", left_right, forw_back, up_down, yaw);
            delay(300);
        }

        rc(left_right, forw_back, up_down, yaw); 
    }

    // do aerobatics
    if (armed && fly && planet0 != planet0_old && planet0) {
        left_right = forw_back = up_down = yaw = 0;
        rc(left_right, forw_back, up_down, yaw); 
        stop();
        delay(3000);
        planet0_old = planet0;
        flip_f();
        delay(500);
    }
    if (armed && fly && planet1 != planet1_old && planet1) {
        left_right = forw_back = up_down = yaw = 0;
        rc(left_right, forw_back, up_down, yaw); 
        stop();
        delay(3000);
        planet1_old = planet1;
        flip_b();
        delay(500);
    }
    if (armed && fly && planet2 != planet2_old && planet2) {
        left_right = forw_back = up_down = yaw = 0;
        rc(left_right, forw_back, up_down, yaw); 
        stop();
        delay(3000);
        planet2_old = planet2;
        flip_lr();
        delay(500);
    }

    delay(10);
}

void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          udp.begin(WiFi.localIP(),ownUdpPort);
          wifiConnected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          wifiConnected = false;
          telloConnected = false;
          break;
      default: break;
    }
}

int a = -100;
int b = 100;
int normalize(int value, int minimum, int maximum) {
    //Serial.printf("val: %d, min: %d, max: %d\n", value, minimum, maximum);
    int normalized = (b-a) * ((float)(value - minimum) / (float)(maximum - minimum)) + a;
    if (normalized > -JOY_THRESHOLD && normalized < JOY_THRESHOLD) return 0;
    else return normalized; 
}

void telloCommand(char *cmd) {
    if (wifiConnected) {
        udp.beginPacket(telloAddress, telloPort);
        udp.printf(cmd);
        udp.endPacket();
        //Serial.printf("Sent [%s] to Tello.\n", cmd);
    }
    else {
        Serial.println("Tello not connected; command not sent");
    }
}

int connect() {
    telloCommand("command");
    // TODO: confirm connection
    return true;
}

void emergency() {
    // turns off motors immediately
    telloCommand("emergency");
}

void takeoff() {
    telloCommand("takeoff");
}

void stop() {
    telloCommand("speed 0");
}

void land() {
    telloCommand("land");
}

void rc(int l_left_right, int l_forw_back, int l_up_down, int l_yaw) {
    char cmd[50];   // rc -100 -100 -100 -100 
    sprintf(cmd, "rc %d %d %d %d", l_left_right, l_forw_back, l_up_down, l_yaw);
    telloCommand(cmd);
}

void flip_f() {
    telloCommand("flip f");
}

void flip_b() {
    telloCommand("flip b");
}

void flip_lr() {
    int r = random(2); // coin flip 
    if (r == 0) 
        telloCommand("flip l");
    else 
        telloCommand("flip r");
}

void initPins() {
    Serial.println("Setting pin modes");
    pinMode(SW_ARM, INPUT_PULLUP);
    pinMode(SW_FLY, INPUT_PULLUP);
    pinMode(SW_FAST, INPUT_PULLUP);

    pinMode(SW_PLANET_0, INPUT);
    pinMode(SW_PLANET_1, INPUT);
    pinMode(SW_PLANET_2, INPUT);

    pinMode(LED_ARM, OUTPUT);
    pinMode(LED_DISARM, OUTPUT);
    pinMode(LED_FAST, OUTPUT);
    pinMode(LED_SLOW, OUTPUT);
    pinMode(LED_FLY, OUTPUT);
    pinMode(LED_LAND, OUTPUT);
    pinMode(LED_PLANET_0, OUTPUT);
    pinMode(LED_PLANET_1, OUTPUT);
    pinMode(LED_PLANET_2, OUTPUT);
    pinMode(LED_POWER, OUTPUT);

    pinMode(JOY_LEFT_RIGHT, INPUT);
    pinMode(JOY_FORW_BACK, INPUT);
    pinMode(JOY_UP_DOWN, INPUT);
    pinMode(JOY_YAW, INPUT);
}

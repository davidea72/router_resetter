/*
 * 
 * PROJECT TO SWITCH OFF AND ON A ROUTER WHEN THE VPN AND INTERNET CONNECTIVITY GOES DOWN
 * 
 * ROUTER RESETTER
 * 
 * 
 * in this project based on the code from sparkydave
 * https://forum.arduino.cc/index.php?topic=518673.msg3534763#msg3534763
 * 
 * i've used this rele board
 * https://it.aliexpress.com/item/32890526507.html
 * the board use the GPIO 0 to activare the rele
 * the led on the board is ON when the rele is energized
 * 
 * power supply 5V
 * the power comsumation is :
 * 
 * 6 mA when the rele is not energized 
 * 18 mA when the rele is energized
 * 
 * 
 * the project use this library :     https://github.com/dancol90/ESP8266Ping
 * i put it directly in the project dir
 * 
 * BE CAREFULL , on github there is an open issue #24 for memory corruption , 
 *  "it run out of memory, some wahtchdog restarte de DSP32"
 *  https://github.com/dancol90/ESP8266Ping/issues/24
 *  i don't know if it affect the ESP01 , but i think of yes ....
 * 
 * 
 * at startup , the resetter wait firstDelay before start , this time is due to left the router complete the boot process and
 * start the connection (mine is 4G router)
 * 
 * the it wait for wifiTimeWaitMinute checking for wifi signal, if tis time is over, it maybe the router has some problem and
 * it try to reset the router again
 * 
 * once it's connect to the wifi
 * 
 * 
 * it checks the connectivity  every minuteBetweenCheck period (configurable)  to two ip address
 * 
 * the primary ip is the VPN server  , every minuteBetweenCheck period
 * the second IP is google , only if the primary ip fails
 * 
 * be carefull, this time is misured with internal clock, 
 * then it's not accurate!!!
 * during the time between two check , the led flash every 0,5 sec
 * it is OFF during the ping check
 * if the ping fails for googleFailsMax and vpnFailsMax , then whe change the rele status , and 
 * we hold the status of the rele for about 10 second , in this meantime , the led flash every 0,1 sec
 * 
 * GPIO2 pin 2 LED
 * GPIO0 pin 0 rele
 *  
 * the program has a DEBUG feature , if enabled , we use the serial port to output some data
 * be carefull, the esp01 has serial port , but if you use the rele board from aliexpress , it hasn't serial comunication
 * 
 * i use to set the RELE pin output to HIGH in normal state , becouse in my rele board when the output is HIGH 
 * the rele is NOT ENERGIZED then i use the NC output of the rele, becouse if the power supply fail , the load
 * continue to work, i loose only the ping functionality!!!!
 * 
 * 
 * *************************           function            *************************
 *  setup_wifi() , to connect to the wifi network
 *  delay_check(), the delay between two check
 *  cicle_rele() , to cicle te rele on and off , change HIGH to LOW if you want your rele act on NC or NO side
 * 
 * *************************          endfunction            *************************
 * * 
 * 
 * 
 * 
 * to be done :
 * 
 * delay the first check to permit to the router to power up : DONE , documentation
 * 
 * check if there is a method to control if the wifi doesn't giv up
 * 
 * 
 * 
 * 
 * 
 * power on
 * firstDelay         5 minute                            LED ON STEADY
 * wifiTimeWaitMinute 5 minute                            LED BLINK 0.5 Sec
 * minuteBetweenCheck          5 minute                            LED BLINK 0.1 0.1 0.4 0.4
 * if 3 minuteBetweenCheck fails              15 minute
 * during ping check                                      LED OFF
 * cicle_Rele         10 Sec                              LED BLINK 0.1 Sec
 * 
 * test no wifi signal with 1 min delay         WORK!!!!!
 * 
 * start  at 00:26:00 , estimed timeout firstDelay              STEADY ON OK
 *        at 00:27:00 , estimed timeout wifiTimeWaitMinute      BLINK OK
 *        at 00:28:00 - 28:10           cicle_Rele              BLINK OK
 *        
 *        
 * test with wifi signal with 1 min delay         WORK!!!!!
 * 
 * 
 * start  at 01:00:10 , power on , estimed timeout firstDelay      01:01:10           STEADY ON OK
 *        at            no wait for wifiTimeWaitMinute
 *        at 01:01:10 , minuteBetweenCheck                         01:02:10            BLINK OK
 *        at 01:02:20 , first ping                                 01:02:30            LED OFF          
 *        at 01:02:20 , minuteBetweenCheck                         01:03:20            BLINK OK
 *        at 01:03:20 , second ping                                01:03:40            LED OFF          
 *        at 01:03:40 , minuteBetweenCheck                         01:04:40            BLINK OK
 *        at 01:04:40 , third ping                                 01:04:50            LED OFF          
 *        at 01:04:50 , cicle_rele                               - 01:05:00            BLINK OK
 *        
 */
#include <ESP8266WiFi.h>
#include "ESP8266Ping/src/ESP8266Ping.h"
#define LED 2
#define RELE 0
//#define DEBUG True

// Update these with values suitable for your network.

const char* ssid = "wifissid";
const char* password = "wifipassword";

int firstDelay = 5;   //delay at startup before checking wifi signal minutes
int wifiTimeWaitMinute = 5;   //delay waiting wifi signal before reset
int minuteBetweenCheck = 5;  //minute between two ping check , check every

int eslapedWifiWait = 0;

const IPAddress google_ip(8, 8, 8, 8);
int googleFails = 0;      // variable to store the number of failed ping
int googleFailsMax = 3;   // max muber of failed ping before reset


const IPAddress vpn_ip(192, 168, 30, 11 );
int vpnFails = 0;       // variable to store the number of failed ping
int vpnFailsMax = 3;    // max muber of failed ping before reset


int flashing=0;         // count flashing during the rele energization

int eslapedCheck=0;      //count the second during the delay check
;
//WiFiClient espClient;

void setup() {
  
  pinMode(LED, OUTPUT);     // Initialize the GPIO5 pin as an output
  pinMode(RELE, OUTPUT);     // Initialize the GPIO4 pin as an output
  digitalWrite(RELE, HIGH);   // Turn RELE OFF (Note that LOW or HIGH is the voltage level
    // but actually the Relay is on; this is because
    // it is acive low on the ESP-01)
  digitalWrite(LED, LOW);

#ifdef DEBUG
  Serial.begin(115200);
#endif

delay(firstDelay*60000);

setup_wifi();

}






void loop() {

  
while(vpnFails < vpnFailsMax and googleFails < googleFailsMax){

    //delay between two check cicle
    delay_check();
    
    #ifdef DEBUG
    Serial.print("Pinging vpn Server ");
    Serial.println(vpn_ip);
    #endif
    
    if(Ping.ping(vpn_ip)){
        #ifdef DEBUG
        Serial.print("Success! Will try ping again in ");
        Serial.print(minuteBetweenCheck);
        Serial.println(" minutes");
        #endif
        vpnFails=0;       // if the ping is OK , the we reset the vpn and google fails counter
        googleFails=0;
    }
    else {
        #ifdef DEBUG
        Serial.println("PING FAILED!");
        Serial.print("Incrementing vpnFail count to ");
        Serial.println(vpnFails);
        #endif
        ++vpnFails;     // Increment vpnFails counter
        if (!Ping.ping(google_ip)){
          googleFails++;
    }
  }
}     
#ifdef DEBUG
Serial.println("Switching relay 1 OFF for 10 seconds flashing led every 100 ms to reboot router and resetting googleFails to zero");
#endif
cicle_rele();

vpnFails = 0;
googleFails=0;
flashing=0;
}





/*
 * to connect to the wifi network
 */

void setup_wifi() {

  
  #ifdef DEBUG
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
#endif

  WiFi.begin(ssid, password);

////////////////////////////////////////////////////////////////////////////////////////////
  while (WiFi.status() != WL_CONNECTED) {
     
     if(eslapedWifiWait  < wifiTimeWaitMinute*60) {
        digitalWrite(LED,!digitalRead(LED));
        delay(500);           
        eslapedWifiWait++;  
        digitalWrite(LED,!digitalRead(LED));
        delay(500);           
                 
        }
      else{
        cicle_rele();
        }
        
    
#ifdef DEBUG
    Serial.print(".");
#endif
    digitalWrite(LED,!digitalRead(LED));
  }

  //randomSeed(micros());

#ifdef DEBUG
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#endif

  digitalWrite(LED,HIGH);
  delay(10000);
}

/*
 * the delay between two check
 */

void delay_check() {
  while(eslapedCheck  < minuteBetweenCheck*60) {
        digitalWrite(LED,!digitalRead(LED));
        delay(100);
        digitalWrite(LED,!digitalRead(LED));
        delay(100);                   
        eslapedCheck++;  
        digitalWrite(LED,!digitalRead(LED));
        delay(400);           
        digitalWrite(LED,!digitalRead(LED));
        delay(400);           
        }
    eslapedCheck = 0;

  }


/*
 * to cicle te rele on and off , change HIGH to LOW if you want your rele act on NC or NO side
 */

void cicle_rele() {

digitalWrite(RELE, LOW);  // Turn GPIO5 off
    while(flashing < 100) {
    digitalWrite(LED,!digitalRead(LED));
    delay(50);           // Delay 100 ms * 100 = 10 seconds
    flashing++;  
    delay(50);           // Delay 100 ms * 100 = 10 seconds
    }
digitalWrite(RELE, HIGH);   // Turn RELE OFF
  
}

 
# ROUTER RESETTER 
## PROJECT TO SWITCH OFF AND ON A ROUTER WHEN THE VPN AND INTERNET CONNECTIVITY GOES DOWN


in this project based on the code from **sparkydave**

[link to inspiration code from arduino forum](https://forum.arduino.cc/index.php?topic=518673.msg3534763#msg3534763)

i've used this [rele board from aliexpress](https://it.aliexpress.com/item/32890526507.html)



![wifi module1](https://github.com/davidea72/router_resetter/blob/master/ESP8266_ESP-01S_5V_WIFI_RELE_MODULE.jpg "wifi module")

![wifi module connection](https://github.com/davidea72/router_resetter/blob/master/ESP8266_ESP-01S_5V_WIFI_RELE_MODULE_CONNECTION.jpg "wifi module connection" )

![wifi module dimension](https://github.com/davidea72/router_resetter/blob/master/ESP8266_ESP-01S_5V_WIFI_RELE_MODULE_DIMENSION.jpg "wifi module dimension")


the board use the GPIO 0 to activate the rele

the led on the board is ON when the rele is energized


power supply 5V

the power comsumation is :

+ 6 mA when the rele is not energized 
+ 18 mA when the rele is energized


the project use this library :     https://github.com/dancol90/ESP8266Ping

i put it directly in the project dir

### BE CAREFULL , on github there is an open issue #24 for memory corruption , "it run out of memory, some wahtchdog restarte de DSP32"
[issue 24](https://github.com/dancol90/ESP8266Ping/issues/24)

 i don't know if it affect the ESP01 , but i think of yes ....
---
---
---
---
---
 
**plese fill the SSID and WPA fields with the appropriate value for your wifi network**



at startup , the resetter wait **firstDelay** (express in minutes) before start , this time is to give to the router the time to complete the boot process and start the connection (mine is 4G router)

then it wait **wifiTimeWaitMinute** (express in minutes) waiting for wifi signal, if this time is over, maybe the router has some problem to generate the wifi signal and it tries to reset the router again

once it's connect to the wifi


it checks the connectivity  every **minuteBetweenCheck** period  to two ip address

the primary ip it's the VPN server (mine is 192.168.30.11) , and if it fails, it checks the second ip address

the second IP is google (8.8.8.8), checked only if the primary ip fails


notice that **minuteBetweenCheck** time is measured with internal clock, then it's not accurate!!!

if the ping fails 3 times for **googleFailsMax** and **vpnFailsMax** , then it changes the rele status , and it holds the status of the rele for about 10 second , in this meantime , the led flash every 0,1 sec
---
---
the program has a DEBUG feature (not tested), if enabled , we use the serial port to output some data

notice that the esp01 has serial port , but if you use the rele board from aliexpress , it hasn't serial comunication port cabled

i use to set the RELE pin output to HIGH in normal state , becouse in my rele board when the output is HIGH the rele is NOT ENERGIZED then i use the NC output of the rele, becouse if the power supply fail , the load continue to work, i loose only the ping functionality!!!!


************************           function           ************************

setup_wifi() , to connect to the wifi network

delay_check(), the delay between two check

cicle_rele() , to cicle te rele on and off , change HIGH to LOW if you want your rele act on NC or NO side
 
************************          endfunction           ************************




|state                       |  time duration  |  LED STATE                 |
|----------------------------|-----------------|----------------------------|
|power on                    |                 |                            |
|firstDelay                  |      5 minutes  | LED ON STEADY              |
|wifiTimeWaitMinute          |  max 5 minutes  | LED BLINK 0.5 Sec          |
|minuteBetweenCheck          |  every 5 minute | LED BLINK 0.1 0.1 0.4 0.4  |
|during ping check           |                 | LED OFF                    |
|cicle_Rele                  |     10 Sec      | LED BLINK 0.1 Sec          |



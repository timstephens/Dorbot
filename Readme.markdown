Dorbot
======

What is it?
------------

Dorbot is an authentication and entry control system based on Arduino and Raspberry Pi. The system uses RFID cards (based on the MF-RC522 card reader that's cheaply available on eBay) as access tokens. The card reader is attached to an Arduino, which is also connected to an nRF24L01+ mini radio transceiver. 

A second transceiver is connected to a Raspberry Pi, which acts as a base-station. In this way, several doors/accesses can be controlled from a single controller. The Pi runs the server code, and a MySQL database which contains a list of allowed keys. When a card is scanned on the door entry, the Arduino there broadcasts to the server to ask if the key-holder is allowed in. The server checks the permissions in the database and replies with affirmative or negative. Additionally, each access attempt is logged.

Requirements
------------

There are several requirements, both in terms of hardware and software. 

###Hardware

* Arduino
* Raspberry Pi
* MF-RC522 card reader
* 2x nRF24L01+ radio transceivers

###Software

* [RF24] (https://www.github.com/maniacbug/RF24) library for radio transceivers (Arduino and Pi)
* [MF-RC522](https://github.com/miguelbalboa/rfid) library for Arduino 
* MySQL 5 or later
* mysqlclient-dev
* SPI


Installation on Raspberry Pi
----------------------------
Connect up the radio to the GPIO on the Pi. 

Enable access to the SPI module by commenting out the line in the blacklist file
	 /etc/modprobe.d/raspi-blacklsit.conf
 	sudo reboot

 After rebooting, at the terminal prompt type:
 	lsmod
You should see spi in the list of available modules.


Install MySQL and libmysqlclient-dev:

Raspberry Pi

    sudo apt-get update
    sudo apt-get dist-upgrade

//Allow the upgrade to take place, then 
    sudo apt-get install mysql-server mysql-client

//Install C API connector
    sudo apt-get install libmysqlclient-dev

    mysql -u root -p
>password

	
Load the mysql dump file into the database. In the mysql client:
	source Dorbot/RPi/db.mysql

Get the RF24 library, and install as required.
	git clone https://www.github.com/maniacbug/RF24.git

I copied the .h, .cpp and .o files into the Dorbot folder rather than specifying their installation locations.

Next, build the .cpp
	cd Dorbot
	 g++ -Wall -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -L./librf24/  -lrf24 dorbot_server.cpp -o dorbot_server `mysql_config --cflags --libs`

Run the server 
	sudo ./server
	
You should see MySQL version information printed out, and then the server should wait for incoming connections from the Arduino.

Installation on Arduino
-----------------------

Connect the radio and card reader to the SPI ports on the Arduino. The pins are

###Radio
Radio -- Arduino
CE -- 9
CSN -- 10
Others as usual.
Note: Don't connect the radio to the 5V pin. You'll cause damage

###Card Reader
RFID -- Arduino
SS -- 7
RST -- 6
Others as usual.

Connect Piezo buzzer between pin 3 and ground

Load the RFID_Door_Check code into your Arduino IDE, and upload the code. Open the Serial Monitor to see what information is being sent backwards and forwards. For the first version of this, the beeper sounds different tones depending on whether the user is permitted into the room or not. 

Release Notes
-------------

###Version 0.1

This version is full of security holes. The server is vulnerable to SQL injection attacks (although the maximum length of the data that can be sent via the radio link is limited to 12 bytes). The Arduino on the door end is vulnerable to a replay attack, and it might be possible to open the door without sending information to the database (although you'd need to scan a card, and then send the response before the server replies (i.e. 0.2s)).

There are likely other issues, but this is meant as a demonstration of what's possible and is expected to be improved before deployment.

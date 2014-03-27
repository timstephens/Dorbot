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

###Hardware Setup

You need to have SPI enabled on your Raspberry Pi. You can do this with `raspi-config`

	sudo raspi-config
	
Scroll to the 'Advanced Options' and select 'enable SPI'

Connect up the radio to the GPIO on the Pi. The pins are:

	Radio -- Raspberry Pi
	1 GND -- 6
	2 3v3 -- 1
	3 CE -- 22
	4 CSN -- 24
	5 SCK -- 23
	6 MOSI -- 19
	7 MISO -- 21
	8 IRQ -- NC


Enable access to the SPI module by commenting out the line in the blacklist file

	/etc/modprobe.d/raspi-blacklist.conf
	sudo reboot

 After rebooting, at the terminal prompt type:
 
	lsmod
	
You should see spi in the list of available modules.

###Software Setup

Install MySQL and libmysqlclient-dev:

	sudo apt-get update
	sudo apt-get dist-upgrade

//Allow the upgrade to take place, then

	sudo apt-get install mysql-server mysql-client

//Install C API connector

	sudo apt-get install libmysqlclient-dev

If you've not done this yet, clone the Dorbot repository to a directory on your Raspberry Pi. 

	git clone https://github.com/timstephens/Dorbot.git
	
	
Log in to mysql using the password that you created for root during the installation.

	mysql -u root -p
	
> enter your password

	
Create the database and load the mysql dump file into the database. In the mysql client:

	create database door;
	use door;
	source Dorbot/RPi/db.mysql
	
Grant permission to the dorbot user to access the databases

	GRANT SELECT, UPDATE, INSERT on door.users to 'dorbot'@'localhost' IDENTIFIED BY 'password';
	GRANT SELECT, UPDATE, INSERT on door.access to 'dorbot'@'localhost' IDENTIFIED BY 'password';
	
NB: You'll need to enter the password that you compile into the dorbot_server.cpp file.

Get the RF24 library, and install as required. (Instructions as [here](http://arduino-for-beginners.blogspot.it/2013/02/setup-nordic-nrf24l01-rf-modules-to.html))

	git clone git clone https://github.com/stanleyseow/RF24.git rf24
	cd rf24/librf24-rpi/librf24
	make
	sudo make install
	sudo ldconfig -v | grep librf
	

I copied the .h, .cpp and .o files into the Dorbot folder rather than specifying their installation locations.

Next, build the .cpp

	cd Dorbot/RPi
	make
	
Run the server 

	sudo ./dorbot_server

You should see MySQL version information printed out, and then the server should wait for incoming connections from the Arduino.

Installation on Arduino
-----------------------

Connect the radio and card reader to the SPI ports on the Arduino. The pins are

###Radio

	Radio -- Arduino
	1 GND -- GND  
	2 3v3 -- 3v3
	3 CE -- 9
	4 CSN -- 10
	5 SCK -- 13
	6 MOSI -- 11
	7 MISO -- 12
	8 IRQ -- NC

	
	
	
	Radio -- Raspberry Pi
	1 GND -- P6  
	2 3v3 -- P1
	3 CE -- P22
	4 CSN -- P24
	5 SCK -- P23
	6 MOSI -- P19
	7 MISO -- P21
	8 IRQ -- NC
	

	Note: Don't connect the radio +VDD to the 5V pin. You'll cause damage.

###Card Reader

	RFID -- Arduino
	SS/SDA -- 6
	IRQ - NC
	RST -- 7
	SCK -- 13
	MOSI -- 11
	MISO -- 12
	GND -- GND
	3v3 -- 3v3
	
Connect Piezo buzzer between pin 3 and ground

Load the RFID_Door_Check code into your Arduino IDE, and upload the code. Open the Serial Monitor to see what information is being sent backwards and forwards. For the first version of this, the beeper sounds different tones depending on whether the user is permitted into the room or not. 


Running Automatically
---------------------

To make the server run automatically whenever the Pi is booted, use supervisord. You need to install supervisord with atp-get:

	apt-get install supervisord

Then, add the following lines to /etc/supervisor/supervisord.conf

	[program:dorbot_server]
	command=/home/pi/Dorbot/RPi/dorbot_server >> /home/pi/dorbot_log
	numprocs = 1
	autostart = true
	autorestart = true
	startsecs = 10
	startretries = 3
	stopsignal = TERM
	stdout_logfile = /home/pi/dorbot_log 

When the Pi boots, the server code will be running (and log to the text file so you can see that it's working).



Release Notes
-------------

###Version 0.2
I added AES128 encryption to the communications between the two devices. The password needs to be set in the code at each end (and needs to be the same). There's also a makefile for the Raspberry Pi, which will make compiling a little easier. 


###Version 0.1

This version is full of security holes. The server is vulnerable to SQL injection attacks (although the maximum length of the data that can be sent via the radio link is limited to 12 bytes). The Arduino on the door end is vulnerable to a replay attack, and it might be possible to open the door without sending information to the database (although you'd need to scan a card, and then send the response before the server replies (i.e. 0.2s)).

There are likely other issues, but this is meant as a demonstration of what's possible and is expected to be improved before deployment.

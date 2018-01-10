# EC500-Voice-Controlled-Communication-Over-XMPP
As a final project for EC500 Connected Devices at BU, my partner and I developed a voice controlled XMPP communication system running on Raspberry Pis.

The objective of this project was to set up three Raspberry Pis that communicated over XMPP and were voice controlled. The user is able to click a button and speak a command into a microphone. The audio is converted to text and sent to another Pi over XMPP. Pis receiving messages play the message out loud. Each Pi is also able to have a different language so, if necessary, translation may occurs between the message transfer. We use the Prosody for our XMPP server, the ALSA library for our audio recording and playback, Gloox for our XMPP library, and WiringPi for our GPIO interactions (Buttons & LEDs). Audio to text, text to audio, and translation was done through Google APIs while the Natural Language Processing (NLP) was done using RegEx.

Everything on the Raspberry Pis is contained in the folder Pi. This folder has a few variables that must be customized per pi. The first is the pi client_name at the top of the main_pi file. It should be changed to fit whatever pi is being configured. Also, the file itself should be changed to match whatever pi is being configured (ie. main_pi2.cpp). This file name must also be updated in the Makefile. Finally, the voice and language that the pi uses when reading messages can be changed in the XMPP_Client.cpp file in the message handler function.

To setup, simply download the repository, make the changes above, and run the makefile and its output. Once done, the pi is ready to go. Make sure it is connnected to the buttons and the LEDs on the right pins (Specified in GPIO_Manager.h, these coincide with the WiringPi numbers), a microphone that handles 16kHz, 16 bit signed little endian, and mono input, and a speaker via the audio out jack.

The white Ready LED will come on when the pi is connected and ready to go. To increase the number of pis in the network, you must first add them to the server (detailed below), make them all "buddies" (easily done in a messaging app like Apple messages), and set them up as explained above. 

#########################################################

XMPP Server and Client

=============== Sources and References ==================

Gloox Library Example:
https://blog.knatten.org/2012/03/23/basic-gloox-tutorial/

Gloox Library Homepage:
https://camaya.net/gloox/

Prosody Server Homepage (downlaod, installation, configuration):
https://prosody.im/

======================= SERVER ==========================

Host Setup:
- AWS EC2 micro server instance running Ubuntu Server 16.04
- Public DNS is ec2-52-14-237-126.us-east-2.compute.amazonaws.com
- Free tier for at least a month, then maybe need to pay
- Ports for inbound traffic at home ip of 209.6.73.212/32 are
	- TCP: 22 (SSH), 5222, 5223, 5269, 5298, 8010 (XMPP)
	- UDP: 5298 (XMPP)
- Ports for inbound traffic at BU ip of 155.41.0.0/16 are
	- TCP: 0 - 65535 (SSH, XMPP - Temporary while testing)
- Ports for inbound traffic at other appartment of 209.6.68.19/32 are
	- TCP: 0 - 65535 (SSH, XMPP - Temporary while testing)
- Ports will be cleaned up as project solidifies
- Access through SSH by the following: 
	- ssh -i ./XMPPServer.pem ubuntu@ec2-52-14-237-126.us-east-2.compute.amazonaws.com
- Link to management page: https://us-east-2.console.aws.amazon.com/ec2/v2/home?region=us-east-2#SecurityGroups:sort=groupId

Server Setup:
- Prosody XMPP server
    -  sudo apt-get install prosody
    -  sudo vi /etc/prosody/prosody.cfg.lua 
    -  prosodyctl register me EC500.com password
   	-  sudo prosodyctl register pi1 ec2-52-14-237-126.us-east-2.compute.amazonaws.com password
    -  sudo prosodyctl register pi2 ec2-52-14-237-126.us-east-2.compute.amazonaws.com password
    -  sudo prosodyctl register pi3 ec2-52-14-237-126.us-east-2.compute.amazonaws.com password
    -  sudo prosodyctl register bot ec2-52-14-237-126.us-east-2.compute.amazonaws.com password
    -  sudo prosodyctl register test ec2-52-14-237-126.us-east-2.compute.amazonaws.com password
    -  sudo prosodyctl restart

- Configuration file can be found in the prosody.cfg.lua file in this directory
- XMPP Server set up with 5 client accounts
	- pi1@ec2-52-14-237-126.us-east-2.compute.amazonaws.com password
	- pi2@ec2-52-14-237-126.us-east-2.compute.amazonaws.com password
	- pi3@ec2-52-14-237-126.us-east-2.compute.amazonaws.com password
	- bot@ec2-52-14-237-126.us-east-2.compute.amazonaws.com password
	- test@ec2-52-14-237-126.us-east-2.compute.amazonaws.com password

- All client accounts have added each other as buddies so any can message any

======================= CLIENT ==========================

Overview
- C++ code
- Uses gloox as an XMPP library

Mac gloox installation either through Homebrew or manual
- Homebrew using the command line
	-	/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
	-	brew install gloox
- Manually
	- Download version 1.0.20 from https://camaya.net/gloox/download/
	- Unpack the directory
	- Install by going into the directory and running:
		-	./configure
		-	make
		-	make install

Manual Raspbian gloox installation on the Raspberry Pi
- Download version 1.0.20 from https://camaya.net/gloox/download/
- Unpack the directory, clean up, and install using the following commands:
	- tar xvfj gloox-1.0.20.tar.bz2
	- rm gloox-1.0.20.tar.bz2
	- cd gloox-1.0.20
	- ./configure
	- make
	- sudo make install 							// Install the librarys
	- sudo ldconfig									// Reconfig ld paths
	- sudo apt-get install libpthread-stubs0-dev	// Install pthreads which are required for gloox

Client
- Client/Bot code is found in xmppBotTest.cpp
- Bot responds to everything sent to it with hello world
- Compile and run by
	-	g++ xmppBotTest.cpp -o main -I/usr/local/include -L/usr/local/lib -lgloox -pthread -std=c++0x
	-	./main

#########################################################

Audio acquisition and playback via ALSA

=============== Sources and References ==================

Library Reference:
http://www.alsa-project.org/alsa-doc/alsa-lib/modules.html

Leveraged Examples:
Capture/Play: http://www.linuxjournal.com/article/6735
Volume: http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code

============= Installation and Compiling ================

Installing ALSA library on the raspberry pi
- sudo apt-get install alsa-utils 			// For command line ALSA controls
- sudo apt-get install libasound2-dev		// For ALSA code libraries
- Compile and run by
	- g++ audioTest.cpp -o main -lasound
	- ./main


#########################################################

GPIO pin interaction via wiringPi

=============== Sources and References ==================

Library Reference:
https://projects.drogon.net/raspberry-pi/wiringpi/functions/

Leveraged Examples:
http://cs.smith.edu/dftwiki/index.php/Tutorial:_Interrupt-Driven_Event-Counter_on_the_Raspberry_Pi

============= Installation and Compiling ================

Installing wiringPi on the Raspberry Pi
- git clone git://git.drogon.net/wiringPi
- cd wiringPi
- ./build

Compile and run by
- g++ wiringPiTest.cpp -o main -I/usr/local/include -L/usr/local/lib -lwiringPi
         

all : out main_pi2.o Mixer.o XMPP_Client.o GPIO_Manager.o base64.o

out : main_pi2.o Mixer.o GPIO_Manager.o XMPP_Client.o base64.o
	g++ -o out main_pi2.o Mixer.o GPIO_Manager.o XMPP_Client.o base64.o -I/usr/local/include -L/usr/local/lib -lgloox -pthread -std=c++11 -lasound -lwiringPi

main_pi2.o : main_pi2.cpp GPIO_Manager.h XMPP_Client.h Mixer.h base64.h
	g++ -c main_pi2.cpp -std=c++11

base64.o : base64.cpp
	g++ -c base64.cpp

Mixer.o : Mixer.cpp GPIO_Manager.h
	g++ -c Mixer.cpp

XMPP_Client.o : XMPP_Client.cpp
	g++ -c XMPP_Client.cpp -std=c++11

GPIO_Manager.o : GPIO_Manager.cpp
	g++ -c GPIO_Manager.cpp


clean:
	rm out main_pi2.o Mixer.o GPIO_Manager.o XMPP_Client.o audio.wav base64.o encoded.json

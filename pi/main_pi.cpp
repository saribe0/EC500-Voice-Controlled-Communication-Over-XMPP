#include "XMPP_Client.h"
#include "Mixer.h"
#include "GPIO_Manager.h"
#include <fstream>
#include <istream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <regex>
#include "base64.h"
#include <stdexcept>

using namespace std;

const string server = "@ec2-52-14-237-126.us-east-2.compute.amazonaws.com";
const string client_name = "pi2";
const string password = "password";

string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}


void NLP(string command, XMPP_Client* client, GPIO_Manager* gpio, Mixer* mix)
{
	smatch text_match;
	regex send_message("send a message to.*(Alex |Sam |professor )(.*)", regex_constants::icase);
	regex lights("(Alex |Sam |professor )?(turn|switch) (on|off).*(green|red|yellow)", regex_constants::icase);
	regex vol("(Alex |Sam |professor )?(raise|turn up|increase)?(lower|turn down|decrease)?(set|change)?.*volume( to \\d+)?", regex_constants::icase);

	//Send a message
	if (regex_search(command, text_match, send_message))
	{
		cout << "Destination: " + text_match.str(1) << endl;
		cout << "Message: " + text_match.str(2) << endl;
		string pi_num = text_match.str(1);
		if (pi_num == "Alex ") client->sendMessage(text_match.str(2), "pi1" + server);
		if (pi_num == "Sam ") client->sendMessage(text_match.str(2), "pi2" + server);
		if (pi_num == "Professor ") client->sendMessage(text_match.str(2), "pi3" + server);
	}

	//Toggle lights
	else if (regex_search(command, text_match, lights))
	{
		//if sending the command to another pi
		if (text_match.str(1) != "")
		{
			string destination = "";
			if (text_match.str(1) == "Alex ") destination = "pi1";
			if (text_match.str(1) == "Sam ") destination = "pi2";
			if (text_match.str(1) == "Professor ") destination = "pi3";
			client->sendMessage("*" + text_match.str(3) + text_match.str(4) + "*", destination + server);
		}
		else
		{
			int led = 0;
			int toggle = 0;
			
			if (text_match.str(3) == "on") toggle = 1;
			if (text_match.str(3) == "off") toggle = 0;
			if (text_match.str(4) == "green") led = 0;
			if (text_match.str(4) == "red") led = 1;
			if (text_match.str(4) == "yellow") led = 2;
			gpio->setLED(led, toggle);
		}
	}

	//Adjust Volume
	else if (regex_search(command, text_match, vol))
	{
		int currentVol = mix->getVolume();

		//if sending the command to another pi
		if (text_match.str(1) != "")
		{
			string destination = "";
			if (text_match.str(1) == "Alex ") destination = "pi1";
			if (text_match.str(1) == "Sam ") destination = "pi2";
			if (text_match.str(1) == "Professor ") destination = "pi3";
			client->sendMessage("*" + text_match.str(2) + text_match.str(3) + text_match.str(4) + "volume" + text_match.str(5) + "*", destination + server);
		}
		else if (text_match.str(2) != "" && currentVol < 10) mix->setVolume(currentVol + 1);
		else if (text_match.str(3) != "" && currentVol > 0) mix->setVolume(currentVol - 1);
		else if (text_match.str(4) != "" && text_match.str(5) != "")
		{
			int setVol = 0;
			if (text_match.str(5) == " to 0") setVol = 0;
			if (text_match.str(5) == " to 1") setVol = 1;
			if (text_match.str(5) == " to 2") setVol = 2;
			if (text_match.str(5) == " to 3") setVol = 3;
			if (text_match.str(5) == " to 4") setVol = 4;
			if (text_match.str(5) == " to 5") setVol = 5;
			if (text_match.str(5) == " to 6") setVol = 6;
			if (text_match.str(5) == " to 7") setVol = 7;
			if (text_match.str(5) == " to 8") setVol = 8;
			if (text_match.str(5) == " to 9") setVol = 9;
			if (text_match.str(5) == " to 10") setVol = 10;
			mix->setVolume(setVol);
		}
	}
}

int main()
{
	cout << "Hello, I am Sam" << endl;
	cout << "Instantiating an instance of the Mixer, GPIO_Manager, and XMPP_Client" <<endl;
	

	Mixer* mix;
	GPIO_Manager* gpio;
	XMPP_Client* client;

	mix = new Mixer(gpio);
	gpio = new GPIO_Manager();
	client = new XMPP_Client(client_name+server, password, gpio, mix);

	cout << "Connecting to the xmpp server" << endl;
	do {
		client->refresh();
		sleep(1);
	} while(!client->isConnected());

	cout << "Connected to the server, ready to go" << endl;

	gpio->setLED(3, 1);

	while(true)
	{
		if(gpio->startPressed() && gpio->stopPressed())
		{
			break;
		}
		if(gpio->startPressed())
		{
			cout << "Starting to record, press stop button to stop" <<endl;
			int length = mix->capture("audio.wav");
			
			ifstream ifs("audio.wav");
  			string content((std::istreambuf_iterator<char>(ifs)),(std::istreambuf_iterator<char>()));
  			string encoded = base64_encode(reinterpret_cast<const unsigned char*>(content.c_str()), content.length());

			ofstream myfile;
			myfile.open ("encoded.json");
			myfile << "{'config': {'encoding':'LINEAR16','sampleRateHertz' : 16000,'languageCode' : 'en-US'},'audio' : {'content':'";
			myfile << encoded;
			myfile << "'}}";
			myfile.close();
  
  			string curl_stt = "curl -s -k -H \"Content-Type: application/json\" https://speech.googleapis.com/v1/speech:recognize?key=AIzaSyDQCyIvaAkWwnITqug8Q6B-ZA-4mY0rcCM -d @encoded.json";
  			string stt_json = exec(curl_stt.c_str());

			regex stt_rgx("\"transcript\": \"(.*)\"");
			smatch stt_match;
			regex_search(stt_json, stt_match, stt_rgx);
			string transcript = stt_match.str(1);

			cout << "String to Text: " + transcript << endl;

			string curl_translate = "curl -s -k -H \"Content-Type: application/json\" https://translation.googleapis.com/language/translate/v2?key=AIzaSyDQCyIvaAkWwnITqug8Q6B-ZA-4mY0rcCM -d \"{'q':'" + transcript + "','target':'en' }\"";

			string translation_json = exec(curl_translate.c_str());
  			regex translate_rgx("\"translatedText\": \"(.*)\"");
			smatch translate_match;
			regex_search(translation_json, translate_match, translate_rgx);
			string translation = translate_match.str(1);

			cout << "Translation to English:" + translation << endl;

			NLP(translation, client, gpio, mix);
		}
		client->refresh(); // Process incoming messages in the recieved message handler
		usleep(50000);
	}

	gpio->setLED(3,0);

	return 0;
}
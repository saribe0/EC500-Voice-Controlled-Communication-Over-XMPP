// XMPP_Client.cpp

#include "XMPP_Client.h"

string XMPP_Client::exec(const char* cmd) {
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


// Constructor to get the client's connected variable
ConnListener::ConnListener(bool *connected)
{
	setConnection = connected;
}

// Called when the client is connected, sets connection to true
void ConnListener::onConnect() 
{
	cout << "ConnListener::onConnect()" << endl;
	*setConnection = true;
}

// Called when the client is disconnected, sets connection to false
void ConnListener::onDisconnect(ConnectionError e) 
{
	cout << "ConnListener::onDisconnect() " << e << endl;
	*setConnection = false;
}

// Called when the client is connected with TLS, sets connection to true
bool ConnListener::onTLSConnect(const CertInfo& info) 
{
	cout << "ConnListener::onTLSConnect()" << endl;
	*setConnection = true;
	return true;
}

// Constructor that creates a client using the provided jabber id and password
XMPP_Client::XMPP_Client(string jabber_id, string password, GPIO_Manager* new_gpio, Mixer* new_mix)
{
	mix = new_mix;
	gpio = new_gpio;
	connected = false;
	JID jid( jabber_id );
	j = new Client( jid, password );
	ConnListener* connListener = new ConnListener(&connected);
	j->registerMessageHandler( this );
	j->registerConnectionListener(connListener);
	j->connect(false);
	cout << "XMPP_Client created" << endl;
}

// Function called to handle incoming messages
// - Allows the client to react to incoming messages and call the right functions
// - Messages will only come in after refresh is called
void XMPP_Client::handleMessage( const Message& stanza, MessageSession* session)
{
	cout << "Recieved Message: " + stanza.body() << endl;

	smatch text_match;
	regex lights("\\*(on|off)(green|red|yellow)\\*", regex_constants::icase);
	regex vol("\\*(raise|turn up|increase)?(lower|turn down|decrease)?(set|change)?.*volume( to \\d+)?\\*", regex_constants::icase);
	if (regex_search(stanza.body(), text_match, lights))
	{
		int led = 0;
		int toggle = 0;
		if (text_match.str(1) == "on") toggle = 1;
		if (text_match.str(1) == "off") toggle = 0;
		if (text_match.str(2) == "green") led = 0;
		if (text_match.str(2) == "red") led = 1;
		if (text_match.str(2) == "yellow") led = 2;
		gpio->setLED(led, toggle);
	}
	else if (regex_search(stanza.body(), text_match, vol))
	{
		int currentVol = mix->getVolume();

		if (text_match.str(1) != "" && currentVol < 10) mix->setVolume(currentVol + 1);
		else if (text_match.str(2) != "" && currentVol > 0) mix->setVolume(currentVol - 1);
		else if (text_match.str(3) != "" && text_match.str(4) != "")
		{
			int setVol = 0;
			if (text_match.str(4) == " to 0") setVol = 0;
			if (text_match.str(4) == " to 1") setVol = 1;
			if (text_match.str(4) == " to 2") setVol = 2;
			if (text_match.str(4) == " to 3") setVol = 3;
			if (text_match.str(4) == " to 4") setVol = 4;
			if (text_match.str(4) == " to 5") setVol = 5;
			if (text_match.str(4) == " to 6") setVol = 6;
			if (text_match.str(4) == " to 7") setVol = 7;
			if (text_match.str(4) == " to 8") setVol = 8;
			if (text_match.str(4) == " to 9") setVol = 9;
			if (text_match.str(4) == " to 10") setVol = 10;
			mix->setVolume(setVol);
		}
	}
	else
	{
		string curl_translate = "curl -s -k -H \"Content-Type: application/json\" https://translation.googleapis.com/language/translate/v2?key=AIzaSyDQCyIvaAkWwnITqug8Q6B-ZA-4mY0rcCM -d \"{'q':'" + stanza.body() + "','target':'en' }\"";

		string translation_json = exec(curl_translate.c_str());
		regex translate_rgx("\"translatedText\": \"(.*)\"");
		smatch translate_match;
		regex_search(translation_json, translate_match, translate_rgx);
		string translation = translate_match.str(1);

		cout << "Translation to foreign language: " + translation << endl;		

		string tts_jwt = exec("curl -s -k -X POST -H \"Ocp-Apim-Subscription-Key : 1b4fc3b3c8b345c2ac2fb5ce252c6440\" https://api.cognitive.microsoft.com/sts/v1.0/issueToken --data \"{}\"");
		string curl_tts = "curl -s -k https://speech.platform.bing.com/synthesize -d \"<speak version='1.0' xml:lang='en-US'><voice xml:lang='en-US' xml:gender='Female' name='Microsoft Server Speech Text to Speech Voice (en-US, ZiraRUS)'>" + translation + "</voice></speak>\" -H \"Authorization : Bearer " + tts_jwt + "\" -H \"Content-Type: application/ssml+xml\" -H \"X-Microsoft-OutputFormat : raw-16khz-16bit-mono-pcm\" -o \"translation_audio.wav\"";
			
		string file = exec(curl_tts.c_str());
		cout << file << endl;
			
		mix->play("translation_audio.wav",9999999);
	}
}

// Function to allow the client to send messages
// - First checks to make sure the client is connected, if so, sends the message
// - If not connected, returns -1
// - Requires input of contents and the jabber_id (username@server)
int XMPP_Client::sendMessage(string contents, string jabber_id)
{
	if (connected)
	{
		JID sid(jabber_id);
		Message msg(Message::MessageType(16), sid, contents);
		j->send( msg );
		cout << "Message Sent" << endl;
		return 0;
	}
	else
	{
		cout << "Message Failed, Not Connected" << endl;
		return -1;
	}
}

// Refresh function which has the client check for messges waiting at the server
// - Must be called to recieve messages
void XMPP_Client::refresh()
{
	j->recv(10);
}

// Returns whether the client is currently connected to the server or not
bool XMPP_Client::isConnected()
{
	return connected;
}

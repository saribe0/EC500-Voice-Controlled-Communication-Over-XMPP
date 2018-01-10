// XMPP_Client.h

#include <gloox/client.h>
#include <gloox/message.h>
#include <gloox/messagehandler.h>
#include <gloox/connectionlistener.h>
#include "Mixer.h"
#include "GPIO_Manager.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <regex>
#include <fstream>
#include <stdexcept>
#include <stdio.h>

using namespace gloox;
using namespace std;

// Connection listener class to work with XMPP Clients
// - Takes a pointer to the clients connected variable
// - When a client connects or disconnects, this class handles the connection and modifies the variable
class ConnListener : public ConnectionListener 
{
	public:
		// Constructor to get the client's connected variable
		ConnListener(bool *connected);

		// Called when the client is connected, sets connection to true
		virtual void onConnect();

		// Called when the client is disconnected, sets connection to false
		virtual void onDisconnect(ConnectionError e);

		// Called when the client is connected with TLS, sets connection to true
		virtual bool onTLSConnect(const CertInfo& info) ;

	private:
		bool* setConnection;
};

// Class for an XMPP Client to connect to the server and handle messages
// - Client has to be refreshed periodically to check for messages
// - Messages can only be send after connection has been made, must be refreshed a few times before connection is made
class XMPP_Client : public MessageHandler
{
	public:
		// Constructor that creates a client using the provided jabber id and password
		XMPP_Client(string jabber_id, string password, GPIO_Manager* new_gpio, Mixer* new_mix);

		// Function called to handle incoming messages
		// - Allows the client to react to incoming messages and call the right functions
		// - Messages will only come in after refresh is called
		virtual void handleMessage( const Message& stanza, MessageSession* session = 0 );

		// Function to allow the client to send messages
		// - First checks to make sure the client is connected, if so, sends the message
		// - If not connected, returns -1
		// - Requires input of contents and the jabber_id (username@server)
		int sendMessage(string contents, string jabber_id);

		// Refresh function which has the client check for messges waiting at the server
		// - Must be called to recieve messages
		void refresh();

		// Returns whether the client is currently connected to the server or not
		bool isConnected();

	private:
		Client* j;
		bool connected;
		Mixer* mix;
		GPIO_Manager* gpio;
		string exec(const char* cmd);
};
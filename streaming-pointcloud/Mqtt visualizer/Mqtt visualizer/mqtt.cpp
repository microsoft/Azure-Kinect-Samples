#include "mqtt.h"
#include "windowWrapper.h"
#include <boost/algorithm/string.hpp>

extern mqtt mosquittoWrapper;
extern openCvWrapper window;
extern bool merged;


//Converts an std::string into an array of chars, syntax is very similar to how strcpy works
//Used for getting std::/toml::string into a char*
void stringToChar(char*& dest, std::string source) {
	if (strlen(source.c_str())) {
		size_t len = strlen(source.c_str()) + 1;
		dest = new char[len];
		strcpy_s(dest, len, source.c_str());
		dest[len - 1] = '\0';
	}
}

/*
The constructor sets all pointers to null and sets the non pointer variables to the default by mosquitto's standards
port = 1883
keepalive = 60
clean_session = true
*/
mqtt::mqtt() {
	this->host = nullptr;
	this->topic = nullptr;
	this->port = 1883;
	this->keepalive = 60;
	this->clean_session = true;
	this->mosq = nullptr;
	this->id = nullptr;
	this->userdata = nullptr;
}

/*
The constructor frees up all of the pointer's memory and then set them to point at 0
All other non-pointer data variables are set to 0.
*/
mqtt::~mqtt() {
	delete[] this->host;
	this->host = 0;
	delete[] this->topic;
	this->topic = 0;
	this->port = 0;
	this->keepalive = 0;
	this->clean_session = false;
	delete this->userdata;
	this->userdata = nullptr;
	mosquitto_destroy(this->mosq);
	this->mosq = 0;
}

/*
Frees up all memory for the pointer fields before allocating the data onto host, topic, and id
Sets all the other non pointer variables to their respective fields.
It will then go through and call init_mosquitto() which will initialize the mosq variable,
allowing it to connect() right after create()
*/
bool mqtt::create(char* host, char* topic, int port, char* id, void* userdata, int keep_alive, bool clean_session, int num_cameras) {
	delete[]this->host;
	delete[]this->topic;
	delete[]this->id;
	this->host = new char[strlen(host) + 1];
	strcpy_s(this->host, (strlen(host) + 1), host);
	this->topic = new char[strlen(topic) + 1];
	strcpy_s(this->topic, (strlen(topic) + 1), topic);
	this->port = port;
	this->keepalive = keepalive;
	this->clean_session = clean_session;
	this->id = new char[strlen(id) + 1];
	strcpy_s(this->id, (strlen(id) + 1), id);

	return initMosquitto();
}

/*
connect() is a wrapper that will check to make sure that the connection succeeded, if it didn't it will return 1
succeeding will return 0
*/
bool mqtt::connect(void) {
	if (mosquitto_connect(this->mosq, this->host, this->port, this->keepalive)) {
		fprintf(stderr, "Unable to connect.\n");
		return false;
	}
	return true;
}

/*
The my_function_callback are functions that will help in the debug process. They are the functions that are called as if there
were on_event() type syntax. These are added to the callback function when calling the mosquitto_log_function_set(). The
mosquitto_log_function_set() takes the mosquitto structure and these my_function_callback() as the arguments.
*/
void mqtt::callbacks(void) {
	//mosquitto_log_callback_set(this->mosq, my_log_callback);
	//mosquitto_connect_callback_set(this->mosq, my_connect_callback);
	mosquitto_message_callback_set(this->mosq, my_message_callback);
	//mosquitto_subscribe_callback_set(this->mosq, my_subscribe_callback);
}

/*
Send message is just a simple wrapper to publish/send a message over the mqtt stream with the
topic that the varible was initialized with.
The required arguments are:
payload_len: size of payload to be sent over
payload: the actual payload to be sent over
qos: 0, 1 or 2 indicating the quality of service to be used for the message
Optional arguments are:
retain: if message is desired to be kept
mid: number for the message id specifically sent over, can be NULL and the library will increment its own.
*/
void mqtt::sendMessage(int payload_len, const void* payload, char* topic, int qos, bool retain, int* mid) {
	if(topic != nullptr){
		mosquitto_publish(this->mosq, mid, topic, payload_len, payload, qos, retain);
	}
	else {
		mosquitto_publish(this->mosq, mid, this->topic, payload_len, payload, qos, retain);
	}
}

/*
Finish mosquitto is a wrapper that calls the mosquitto_lib_cleanup() function
Only do this at the end of the program.
*/
void mqtt::finishMosquitto(void) {
	mosquitto_lib_cleanup();
}

/*
Wrapper that is used to initialize the mosquitto library. Only needs to be called once.
*/
void mqtt::initMqttLib(void) {
	mosquitto_lib_init();
}

/*
Wrapper to help create a new mosquitto instance.
There are no required parameters for this function as it already assumes that the values for the class
has been populated in some fashion prior to calling this.
*/
int mqtt::initMosquitto(void) {
	this->mosq = mosquitto_new(this->id, this->clean_session, this->userdata);
	if (!mosq) {
		fprintf(stderr, "Unable to connect.\n");
		return 0;
	}
	return 1;
}

/*
ParseArgs takes in the section of toml that is relevant for mqtt info.
It will create the variables right from the file and then initialize the values to those provided from the
config.toml file.
Returns the success or failure of init_mosquitto()
*/
bool mqtt::ParseArgs(toml::Table table) {
	stringToChar(this->host, toml::get<toml::String>(table.at("host")));
	stringToChar(this->id, toml::get<toml::String>(table.at("id")));
	std::string topicString(toml::get<toml::String>(table.at("topic")));
	topicString.append("/");
	topicString.append(id);
	stringToChar(this->topic, topicString);
	this->port = (int)toml::get<toml::Integer>(table.at("port"));
	try {
		this->keepalive = (int)toml::get<toml::Integer>(table.at("keepalive"));
	}
	catch (std::exception & e) {
		e; //Removes unreferenced local veriable warning
		this->keepalive = 60;
	}
	try {
		this->clean_session = toml::get<toml::Boolean>(table.at("clean_session"));
	}
	catch (std::exception & e) {
		e; //Removes unreferenced local veriable warning
		this->clean_session = true;
	}

	return initMosquitto();
}

/*
Give the current mosquitto instance back, useful for:
mosquitto_loop_start();
*/
mosquitto* mqtt::giveMosquitto() {
	return this->mosq;
}

/*
Subscribe to the topic that is stored in the class
*/
void mqtt::subscribe() {
	mosquitto_subscribe(this->mosq, NULL, this->topic, 0);
}

/*
Subscribe to a topic that is not stored in the class
Can do either to be '(this->topic)/newtopic' in terms of appending
or its own topic to something that is not tied to this->topic
*/
void mqtt::subscribe(char* topic, bool append) {
	std::string subTopic;
	if (append) {
		subTopic = std::string(this->topic);
		subTopic.append("/");
		subTopic.append(topic);
	}
	else {
		subTopic = std::string(topic);
	}
	mosquitto_subscribe(this->mosq, NULL, subTopic.c_str(), 0);
}

/*
Disconnect the mosquitto client from the broker
*/
void mqtt::disconnect() {
	mosquitto_disconnect(this->mosq);
}

	/*
	Return the stored topic in the class
	*/
std::string mqtt::giveTopic() {
	return this->topic;
}

bool mqtt::unsubscribe(char* topic) {
	if (topic == nullptr) {
		return false;
	}

	mosquitto_unsubscribe(this->mosq, NULL, topic);

	return true;
}

/*
Function is provided by the mosquitto.org documentation
*/
void my_message_callback(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message)
{
	if (message->payloadlen) {
		
		if (strcmp((char*)message->topic, TOPIC_CONTROL) == 0 ) {
			
			if (strcmp((char*)message->payload, COMMAND_SHUTDOWN) == 0) {
				printf("Closing down the application..\n");
				window.setDone(true);
			}
			else if (strcmp((char*)message->payload, COMMAND_RESTART) == 0) {
				printf("Restarting the application..                          \n");
				window.setRestart(true);
			}
		}
		else if (strcmp((char*)message->topic, TOPIC_MERGER) == 0) {
			
			// Merger is responding with a list of 1 or more topics from which
			// point frames will be broadcast (either the list of individual 
			// cameras, or the merged frames)

			std::string messagePayload((char*)message->payload);
			std::vector<std::string> messageVector;
			//String split using boost 'Solution 2': https://www.fluentcpp.com/2017/04/21/how-to-split-a-string-in-c/
			boost::split(messageVector, messagePayload, [](char c) {return c == ' '; });
			if (messageVector.size() != 0) {
				window.setTopicNames(messageVector);
			}
		}
		else if ( window.isActiveTopic(message->topic) ) {
			//For Debugging purposes
			//printf("%s %d bytes\n", (char*)message->topic, message->payloadlen);
			std::string topicString((char*)message->topic);
			msg newPoints;
			char* buffer = (char*)message->payload;
			int totalBuffer = message->payloadlen - 13;
			int totalPoints = 0;
			memcpy(&totalPoints, &buffer[8], 4);
			totalPoints *= 3;
			int depthSize = totalPoints * 2;
			newPoints.message = new int16_t[totalPoints];
			newPoints.size = totalPoints;
			memcpy(&newPoints.message[0], &buffer[13], depthSize);
			window.importPoints(topicString, newPoints);
			delete[]newPoints.message;
			newPoints.message = nullptr;
		}
	}
}

/*
Function is provided by the mosquitto.org documentation
*/
void my_connect_callback(struct mosquitto* mosq, void* userdata, int result)
{
	if (!result) {
		/* Subscribe to broker information topics on successful connect. */
		mosquitto_subscribe(mosq, NULL, "$SYS/#", 2);
	}
	else {
		fprintf(stderr, "Connect failed\n");
	}
}

/*
Function is provided by the mosquitto.org documentation
*/
void my_subscribe_callback(struct mosquitto* mosq, void* userdata, int mid, int qos_count, const int* granted_qos)
{
	int i;

	printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for (i = 1; i < qos_count; i++) {
		printf(", %d", granted_qos[i]);
	}
	printf("\n");
}

/*
Function is provided by the mosquitto.org documentation
*/
void my_log_callback(struct mosquitto* mosq, void* userdata, int level, const char* str)
{
	/* Pring all log messages regardless of level. */
	printf("%s\n", str);
}
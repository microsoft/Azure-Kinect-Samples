#include "mqtt.h"
#include "messageBuffer.h"
#include "pointcloudmerger.h"
#include <boost/algorithm/string.hpp>

extern mqtt mosquittoWrapper;
extern pointCloudMerger merger;

//Converts an std::string into an array of chars, syntax is very similar to how strcpy works
//Used for getting std/toml string into a char*
void stringToChar(char*& dest, std::string source) {
	if (strlen(source.c_str())) {
		size_t len = strlen(source.c_str()) + 1;
		dest = new char[len];
		strcpy_s(dest, len, source.c_str());
		dest[len - 1] = '\0';
	}
}

//-------------------------------public functions-------------------------------//


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
	this->mosq = nullptr;
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
		fprintf(stderr, "mqtt::connect() Unable to connect.\n");
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
	//log_callback, connect_callback, and subscribe_callback are used for debugging
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
		fprintf(stderr, "mqtt::initMosquitto() Unable to connect.\n");
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
bool mqtt::ParseArgs(toml::Table data) {
	toml::Table table = toml::get<toml::Table>(data.at("MqttInfo"));
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
	catch (std::exception& e) {
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

	table = toml::get<toml::Table>(data.at("CameraInfo"));
	this->parseFPS(toml::get<toml::String>(table.at("fps")));
	this->parseDepthMode(toml::get<toml::String>(table.at("depth_mode")));
	this->parseColorResolution(toml::get<toml::String>(table.at("color_resolution")));
	this->cameraConfig.color = toml::get<bool>(table.at("color"));

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
Unsubscribe from a specific topic.
*/
void mqtt::unsubscribe(char* topic, bool append) {
	std::string subTopic;
	if (append) {
		subTopic = std::string(this->topic);
		subTopic.append("/");
		subTopic.append(topic);
	}
	else {
		subTopic = std::string(topic);
	}
	mosquitto_unsubscribe(this->mosq, NULL, subTopic.c_str());
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

/*
* Handle incoming MQTT messages.
*/
void my_message_callback(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message)
{
	if (message->payloadlen) {
		
		//Camera's topic
		if (strcmp((char*)message->topic, TOPIC_CONTROL) == 0) {

			// Program is over with, set done
			if (strcmp((char*)message->payload, COMMAND_SHUTDOWN) == 0) {
				printf("Closing down the application..\n");
				merger.setDone(true);
			}

			// Program needs to restart
			else if (strcmp((char*)message->payload, COMMAND_RESTART) == 0) {
				merger.setRestart(true);
				printf("Restarting the application..\n");
				std::fflush(stdout);
			}
		}
		else if (strcmp((char*)message->topic, TOPIC_CAMERA_AVAILABLE) == 0) {
			// Cameras announce the MQTT topic upon which they will broadcast frames.
			// Subscribe to the topic and send back the configuration so the camera
			// can initialize and start sending data.

			std::string messagePayload((char*)message->payload);
			merger.createNewIndex(messagePayload);
			mosquittoWrapper.subscribe((char*)message->payload);
			
			uint8_t configArr[4] = { 
				(uint8_t)mosquittoWrapper.cameraConfig.fps, 
				(uint8_t)mosquittoWrapper.cameraConfig.depthMode,
				(uint8_t)mosquittoWrapper.cameraConfig.colorMode,
				(uint8_t)mosquittoWrapper.cameraConfig.color
			};
			mosquittoWrapper.sendMessage(4, configArr, (char*)TOPIC_CAMERA_CONFIGURE);
		}//End topic
		else {
			//Viewer Topic
			if (strcmp((char*)message->topic, TOPIC_VIEWER) == 0) {

				// Viewer is asking for the list of camera topics
				if (strcmp((char*)message->payload, COMMAND_GET_CAMERA_TOPICS) == 0) {
					std::string cameraTopics;
					if (merger.giveIndexNames(cameraTopics)) {
						mosquittoWrapper.sendMessage((int)cameraTopics.size(), cameraTopics.c_str(), (char*)TOPIC_MERGER);
					}
				}
				//Viewer is asking for the merger's topic
				else if (strcmp((char*)message->payload, COMMAND_GET_MERGED_TOPIC) == 0) {
					std::string mergerTopic(mosquittoWrapper.giveTopic());
					mosquittoWrapper.sendMessage((int)mergerTopic.size(), mergerTopic.c_str(), (char*)TOPIC_MERGER);
				}

			}//End viewer topic
			else if (message->payloadlen >= 13) {
				//Else the message must be a camera sending a frame over
				char* payload = (char*)message->payload;

				int bytesPerPoint = 6;
				//Get and create the msg structure
				messageBuffer* msg = new messageBuffer;
				memcpy(&msg->timestamp,  &payload[0],  8);
				memcpy(&msg->pointCount, &payload[8],  4);
				memcpy(&msg->color,      &payload[12], 1);
				if (msg->color) {
					bytesPerPoint = 9;
				}
				msg->size = msg->pointCount * bytesPerPoint;
				msg->points = new char[msg->size];
				memcpy(&msg->points[0], &payload[13], msg->size);
				if (!merger.addMessage(message->topic, msg)) {
					delete msg;
				}

			}
			else {
				//For Debugging
				//printf("\n%s %d\t\t%u\n", (char*)message->topic, message->payloadlen, timestamp);
			}
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
		fprintf(stderr, "mqtt::my_connect_callback() Connect failed\n");
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

//-------------------------------private functions-------------------------------//

/*
Parse the fps from the toml file and set it into a pseudo enum state.
*/
bool mqtt::parseFPS(std::string fpsSetting) {
	int mode = 0;
	if (fpsSetting.compare("5") == 0) {
		mode = 0;
	}
	else if (fpsSetting.compare("15") == 0) {
		mode = 1;
	}
	else if (fpsSetting.compare("30") == 0) {
		mode = 2;
	}
	else {
		this->parseErrorOutput("fps");
		return false;
	}

	this->cameraConfig.fps = mode;
	return true;
}

/*
Parse the depth mode from the toml file and set it into a pseudo enum state.
*/
bool mqtt::parseDepthMode(std::string depthSetting) {
	int mode = 0;
	if (depthSetting.compare("OFF") == 0) {
		mode = 0;
	}
	else if (depthSetting.compare("NFOV_BINNED") == 0) {
		mode = 1;
	}
	else if (depthSetting.compare("NFOV_UNBINNED") == 0) {
		mode = 2;
	}
	else if (depthSetting.compare("WFOV_BINNED") == 0) {
		mode = 3;
	}
	else if (depthSetting.compare("WFOV_UNBINNED") == 0) {
		mode = 4;
	}
	else if (depthSetting.compare("PASSIVE_IR") == 0) {
		mode = 5;
	}
	else {
		this->parseErrorOutput("depth_setting");
		return false;
	}

	this->cameraConfig.depthMode = mode;
	return true;
}

/*
Parse the color resolution from the toml file and set it into a pseudo enum state.
*/
bool mqtt::parseColorResolution(std::string colorResolutionSetting) {
	int mode = 0;
	if (colorResolutionSetting.compare("OFF") == 0) {
		mode = 0;
	}
	else if (colorResolutionSetting.compare("720P") == 0) {
		mode = 1;
	}
	else if (colorResolutionSetting.compare("1080P") == 0) {
		mode = 2;
	}
	else if (colorResolutionSetting.compare("1440P") == 0) {
		mode = 3;
	}
	else if (colorResolutionSetting.compare("1536P") == 0) {
		mode = 4;
	}
	else if (colorResolutionSetting.compare("2160P") == 0) {
		mode = 5;
	}
	else if (colorResolutionSetting.compare("3072P") == 0) {
		mode = 6;
	}
	else {
		this->parseErrorOutput("color_resolution");
		return false;
	}

	this->cameraConfig.colorMode = mode;
	return true;
}

/*
Parse the error explaining the wrong user input from the toml file and set it into a pseudo enum state.
*/
void mqtt::parseErrorOutput(std::string errorString) {
	fprintf(stderr, "error, cannot parse the %s correctly, please ensure there are no typos\n", errorString.c_str());
}
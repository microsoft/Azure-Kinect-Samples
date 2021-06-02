#pragma once
#pragma once
#include <stdio.h>
#include <mosquitto.h>
#include "..//packages/TOMLParser-master/toml.hpp"

#define TOPIC_CAMERA_AVAILABLE "cameras/available"
#define TOPIC_CAMERA_CONFIGURE "cameras/configure"
#define TOPIC_CONTROL "cameras/control"
#define TOPIC_VIEWER "viewer"
#define TOPIC_MERGER "merger"

#define COMMAND_SHUTDOWN "done"
#define COMMAND_RESTART "restart"

#define COMMAND_GET_CAMERA_TOPICS "get_camera_topics"
#define COMMAND_GET_MERGED_TOPIC "get_merged_topic"


//Wrapper to hold the configuration to send off to cameras when they come online
struct cameraConfiguration {
	int fps = 0;
	int depthMode = 0;
	int colorMode = 0;
	bool color = false;

	~cameraConfiguration() {
		this->fps = 0;
		this->depthMode = 0;
		this->colorMode = 0;
		this->color = false;
	}
};

/*
These callback functions are not included within the class as they are used to be passed as pointers to the functions
for mosquitto to use them properly
*/
void my_message_callback(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message);
void my_connect_callback(struct mosquitto* mosq, void* userdata, int result);
void my_subscribe_callback(struct mosquitto* mosq, void* userdata, int mid, int qos_count, const int* granted_qos);
void my_log_callback(struct mosquitto* mosq, void* userdata, int level, const char* str);

/*
mqtt class is a wrapper for the mosquitto library
It holds just about all the information to be able to send data with the exception of some sort of message
variable.
Once the class has been created, the next steps to send a message are connect(), then send_message()
With send_message(), be sure to include the payload length, the payload, and the quality of service that would be desired.
*/
class mqtt {
public: //Functions
	mqtt();
	~mqtt();
	bool create(char* host, char* topic, int port, char* id, void* userdata, int keep_alive, bool clean_session, int num_cameras);
	bool connect(void);
	void disconnect();
	void callbacks(void);
	void sendMessage(int payload_len, const void* payload, char* topic = nullptr, int qos = 0, bool retain = false, int* mid = NULL);
	void finishMosquitto(void);
	void initMqttLib(void);
	int initMosquitto(void);
	bool ParseArgs(toml::Table table);
	void subscribe(void);
	void subscribe(char* topic, bool append = false);
	void unsubscribe(char* topic, bool append = false);
	std::string giveTopic();
	mosquitto* giveMosquitto();
	struct cameraConfiguration cameraConfig;


private: //Functions
	bool parseFPS(std::string fpsSetting);
	bool parseDepthMode(std::string depthSetting);
	bool parseColorResolution(std::string colorResolutionSetting);
	void parseErrorOutput(std::string errorString);

private: //Variables
	char* host;
	char* topic;
	int port;
	int keepalive;
	bool clean_session;
	char* id;
	void* userdata;
	struct mosquitto* mosq;
};
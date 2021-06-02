#include <iostream>
#include "windowWrapper.h"
#include "mqtt.h"
#include <Eigen/Core>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <boost/format.hpp>

float translationIncrement = 0;
float rotationIncrement = 0;
int adjustmentDetailLevel = 1;
float adjustmentDetailLevels[4] = { 0.1f, 1.0f, 10.0f, 100.0f };

uint16_t index = 0;
std::vector<std::string> topics;

//Window instance
openCvWrapper window("Pointcloud Renderer");
mqtt mosquittoWrapper;
bool displayHelpText = true;

std::string helptTextWidgetName("helptext");
std::string helpTextString("\
n, N      : Help menu\n\
m, M      : Restart capture\n\
Tab       : Switch cameras\n\
l,L       : Save cameras\n\n\
Cloud Movement\n\
Directions relative to camera\n\
  Up      : move positive y-axis\n\
  Down    : move negative y-axis\n\
  Right   : move positive x-axis\n\
  Left    : move negative x-axis\n\
  Home    : move positive z-axis\n\
  End     : move negative z-axis\n\
  2       : rotate positive x-axis\n\
  8       : rotate negative x-axis\n\
  6       : rotate positive y-axis\n\
  4       : rotate negative y-axis\n\
  9       : rotate positive z-axis\n\
  7       : rotate negative z-axis\n\
");


void sendCalibrationAdjustment(float tx, float ty, float tz, float rx, float ry, float rz, std::string topic) {
	float arr[6] = { tx, ty, tz, rx, ry, rz };
	mosquittoWrapper.sendMessage(sizeof(float) * 6, arr, (char*)topic.c_str());
}

//Keyboard events for the cv::viz window created
void KeyboardViz3d(const cv::viz::KeyboardEvent& w, void* t) {
	//There was a key pressed
	//Go through and look at which key was pressed and act accordingly by which key does what.
	if (w.action) {
		//Debug to get viz name in order to bind keys to proper instructions
		//std::cout << "you pressed " << w.code << " = " << w.symbol << " in viz window " << "\n";
		std::string topicString;
		std::string lastCommand("Last Command Entered: ");
		std::string command;

		float translateAmount = translationIncrement * adjustmentDetailLevels[adjustmentDetailLevel];
		float rotateAmount = rotationIncrement * adjustmentDetailLevels[adjustmentDetailLevel];

		if (topics.size() > index) {
			topicString.append(topics[index]);
		}
		topicString.append("/calibrate");
		
		if (w.symbol == "Up") {
			sendCalibrationAdjustment(0, translateAmount, 0, 0, 0, 0, topicString);
		}
		else if (w.symbol == "Down") {
			sendCalibrationAdjustment(0, -translateAmount, 0, 0, 0, 0, topicString);
		}
		else if (w.symbol == "Right") {
			sendCalibrationAdjustment(-translateAmount, 0, 0, 0, 0, 0, topicString);
		}
		else if (w.symbol == "Left") {
			sendCalibrationAdjustment(translateAmount, 0, 0, 0, 0, 0, topicString);
		}
		else if (w.symbol == "End") {
			sendCalibrationAdjustment(0, 0, translateAmount, 0, 0, 0, topicString);
		}
		else if (w.symbol == "Home") {
			sendCalibrationAdjustment(0, 0, -translateAmount, 0, 0, 0, topicString);
		}
		//Rotate the point cloud that is currently in focus, everything is relative to the global coordinates
		else if (w.symbol == "2") {
			sendCalibrationAdjustment(0, 0, 0, rotateAmount, 0, 0, topicString);
		}
		else if (w.symbol == "8") {
			sendCalibrationAdjustment(0, 0, 0, -rotateAmount, 0, 0, topicString);
		}
		else if (w.symbol == "6") {
			sendCalibrationAdjustment(0, 0, 0, 0, rotateAmount, 0, topicString);
		}
		else if (w.symbol == "4") {
			sendCalibrationAdjustment(0, 0, 0, 0, -rotateAmount, 0, topicString);
		}
		else if (w.symbol == "7") {
			sendCalibrationAdjustment(0, 0, 0, 0, 0, -rotateAmount, topicString);
		}
		else if (w.symbol == "9") {
			sendCalibrationAdjustment(0, 0, 0, 0, 0, rotateAmount, topicString);
		}
		else if (w.symbol == "l" || w.symbol == "L") {
			mosquittoWrapper.sendMessage(4, (char*)COMMAND_SAVE, (char*)TOPIC_CONTROL);
		}
		else if (w.symbol == "Prior") {
			// page up
			adjustmentDetailLevel++;
			if (adjustmentDetailLevel > 3) {
				adjustmentDetailLevel = 3;
			}
		}
		else if (w.symbol == "Next") {
			// page down
			adjustmentDetailLevel--;
			if (adjustmentDetailLevel < 0) {
				adjustmentDetailLevel = 0;
			}
		}
		else if (w.symbol == "n" || w.symbol == "N") {
			displayHelpText = !displayHelpText;
			if (displayHelpText) {
				window.drawText(helpTextString, helptTextWidgetName, 100, 0);
			}
			else {
				window.clearWidgetByName(helptTextWidgetName);
			}
		}
		else if (w.symbol == "m" || w.symbol == "M") {
			mosquittoWrapper.sendMessage(7, (char*)COMMAND_RESTART, (char*)TOPIC_CONTROL);
		}
		else if (w.symbol == "Tab") {
			index = (index + 1) % (topics.size());
			std::cout << "\33[2KCamera: " << topics[index] <<"\r";
		}
		//Display the extra options onto the help message
		else if (w.symbol == "H" || w.symbol == "h") {
			std::cout<< helpTextString << std::endl;
		}
		else if (w.symbol == "q" || w.symbol == "Q" || w.symbol == "e" || w.symbol == "E") {
			window.setDone(true);
		}
		
		if (topics.size() != 0) {
			lastCommand.append(w.symbol);
			lastCommand.append(" on source ");
			lastCommand.append(topics[index]);		

			window.drawText(lastCommand, std::string("lastCommand"));
		}
	}
}

int main() {
	//Set the window's background and keyboard callbacks
	window.setBackgroundColor(cv::viz::Color::black());
	window.registerKeyboardCallback(KeyboardViz3d);

	//Parse the toml file
	std::ifstream file("settings.toml");
	if (!file.good()) {
		fprintf(stderr, "Error opening the settings.toml file");
		exit(1);
	}
	toml::Data data = toml::parse(file);
	mosquittoWrapper.initMqttLib();
	
	translationIncrement = (float)toml::get<toml::Float>(data.at("translation_increment"));
	rotationIncrement = (float)toml::get<toml::Float>(data.at("rotation_increment"));
	
	bool merged = toml::get<toml::Boolean>(data.at("merged"));

	mosquittoWrapper.ParseArgs(toml::get<toml::Table>(data.at("MqttInfo")));
	//Set up the mosquitto client
	mosquittoWrapper.callbacks();
	mosquittoWrapper.connect();

	mosquittoWrapper.subscribe((char*)TOPIC_MERGER);
	mosquittoWrapper.subscribe((char*)TOPIC_CONTROL);
	mosquitto_loop_start(mosquittoWrapper.giveMosquitto());

	window.parseArgs(toml::get<toml::Table>(data.at("Threshold")));

	std::string commandGetCameraTopics(COMMAND_GET_CAMERA_TOPICS);
	std::string commandGetMergedTopic(COMMAND_GET_MERGED_TOPIC);
	bool stopped = false;
	std::string state("Cameras:\n");
	std::string stats("Stats:\n");
	int pointCount = 0;

	while (!stopped && !window.isDone()) {
		window.drawThreshold();
		window.sphere(10);

		while (topics.size() == 0 && !window.isDone() && !stopped) {
			std::string temp = state;
			temp.append("Waiting for topics from merger\n");
			window.drawText(temp, state, 0, 0.70);
			temp = stats;
			temp.append("Points: 0");
			window.drawText(temp, stats, 0, 0.90);
			if (displayHelpText) {
				window.drawText(helpTextString, helptTextWidgetName);
			}
			window.showWindow(1, false);
			mosquittoWrapper.sendMessage((int)commandGetCameraTopics.size(), commandGetCameraTopics.c_str(), (char*)TOPIC_VIEWER);
			std::this_thread::sleep_for(std::chrono::seconds(1));
			topics = window.getIndexNames();
			stopped = window.wasStopped();
		}
		std::string displayTopics = state;
		for (auto iter : topics) {
			displayTopics.append(iter);
			displayTopics.append("\n");
		}
		if (merged) {
			window.unsubscribeTopics();
			mosquittoWrapper.sendMessage((int)commandGetMergedTopic.size(), commandGetMergedTopic.c_str(), (char*)TOPIC_VIEWER);
		}
		while (!stopped && !window.isRestart() && !window.isDone()) {
			int newPoints = window.createWidgets();
			if (newPoints != 0) {
				pointCount = newPoints;
			}
			std::string displayStats("Points: ");
			displayStats.append(std::to_string(pointCount));
			displayStats.append("\n\nSelected Camera:       ");
			displayStats.append(topics[index]);

			if (!merged) {
				displayStats.append(" - ");
				switch (index) {
				case 0:
					displayStats.append("red");
					break;
				case 1:
					displayStats.append("green");
					break;
				case 2:
					displayStats.append("blue");
					break;
				default:
					displayStats.append("white");
				}
			}
			displayStats.append("\n");
			displayStats.append("Rotation increment:    ");

			std::stringstream currentRotationValue;
			currentRotationValue << std::fixed << std::setprecision(2) << (rotationIncrement * adjustmentDetailLevels[adjustmentDetailLevel]);
			std::stringstream currentTranslationValue;
			currentTranslationValue << std::fixed << std::setprecision(2) << (translationIncrement * adjustmentDetailLevels[adjustmentDetailLevel]);

			displayStats.append(currentRotationValue.str());
			displayStats.append(" degrees\n");

			displayStats.append("Translation increment: ");
			displayStats.append(currentTranslationValue.str());
			displayStats.append(" mm");


			window.drawText(displayStats, stats, 0.0, 0.85);
			window.drawText(displayTopics, state, 0, 0.65);
			if (displayHelpText) {
				window.drawText(helpTextString, helptTextWidgetName);
			}
			window.showWindow(1, false);
			stopped = window.wasStopped();
		}
		window.setRestart(false);
		window.unsubscribeTopics();
		window.clearAllWidgets();
		topics.clear();
		index = 0;
		if (!stopped) {
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
	}

	//Disconnect the mosquitto client from the broker
	//Clean up the library
	mosquittoWrapper.disconnect();
	mosquittoWrapper.finishMosquitto();

	return 0;
}
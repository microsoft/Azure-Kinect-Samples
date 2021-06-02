#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <opencv2/viz.hpp>
#include "../packages/TOMLParser-master/toml.hpp"
#include "mqtt.h"
#include <unordered_map>

//-----openCvWrapper------//
/*
This class is used in order to wrap the cv::viz library into an easier to use API.
Just has the basic functionality of taking a couple different vectors of data types
and converting them into the cv::Point3d type so it can be used with the cv::viz::WCloud
*/
class openCvWrapper {
public:
	openCvWrapper();
	openCvWrapper(std::string windowName);
	~openCvWrapper();
	bool setName(std::string windowName);
	bool getWindowByName(std::string windowName);
	bool showWindow();
	bool showWindow(int ms, bool redraw = false);
	bool importPoints(std::string index, msg& message);
	int createWidgets();
	void clearAllWidgets();
	void clearWidgetByName(std::string widgetName);
	bool wasStopped();
	bool setBackgroundColor(cv::viz::Color color = cv::viz::Color::white());
	void resetCamera();
	void registerKeyboardCallback(cv::viz::Viz3d::KeyboardCallback callback);	
	void sphere(double size);
	bool parseArgs(toml::Table table);
	void drawThreshold(cv::viz::Color color = cv::viz::Color::amethyst());
	bool setTopicNames(std::vector<std::string> newTopicNames);
	std::vector<std::string> getIndexNames();
	bool unsubscribeTopics();
	void drawText(std::string text, std::string widgetName, float widthPercent = 0.0, float heightPercent = 0.0, int textSize = 16);
	void drawHelpText(std::string widgetName);
	bool isActiveTopic(std::string topic);
	bool isDone();
	bool setDone(bool toSet);
	bool isRestart();
	bool setRestart(bool toSet);


private: //functions
	void makeWidget(std::vector<cv::Point3d>& points, std::string widgetName, cv::viz::Color color = cv::viz::Color::white());

private: //variables
	cv::viz::Viz3d window;						//Window holds the cv::viz::Viz3d window instance
	std::string windowName;						//Window name stores the window's name
	std::vector<cv::Point3d> threshold;
	std::vector<std::string> topicNames;
	std::unordered_map<std::string, msg*> cameraSources;
	std::vector<std::vector<cv::Point3d>> currentPoints;
	//Program state variables
	bool done;
	bool restart;
};
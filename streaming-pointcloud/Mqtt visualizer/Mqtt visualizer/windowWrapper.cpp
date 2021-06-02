#pragma once
#include "windowWrapper.h"
#include "mqtt.h"

extern mqtt mosquittoWrapper;

//-------------------------------public functions-------------------------------//


//-----Default constructor------//
/*
Sets all of the data types to their zero respective or empty values
*/
openCvWrapper::openCvWrapper() {
	this->window;
	this->windowName.clear();
	this->topicNames.clear();
	this->threshold = std::vector<cv::Point3d>(2);
	this->done = false;
	this->restart = false;
}

//-----Name constructor------//
/*
Sets all of the data types to their zero respective or empty values.
Gets the window all set up and ready to go in order to display
*/
openCvWrapper::openCvWrapper(std::string windowName) {
	this->openCvWrapper::openCvWrapper();
	this->windowName = windowName;
	this->window = cv::viz::Viz3d(this->windowName);
}

//-----Deconstructor------//
/*
Sets all of the data types to their zero respective or empty values in order to be cleared
*/
openCvWrapper::~openCvWrapper() {
	this->windowName.clear();
	this->threshold.clear();

	this->done = false;
	this->restart = false;

	this->unsubscribeTopics();
	this->cameraSources.clear();
	this->topicNames.clear();

}

//-----SetName------//
/*
Set name will set the WindowName variable
Also sets this->window with the windowName, so its ready to go when showing.
*/
bool openCvWrapper::setName(std::string windowName) {
	if (windowName.size() == 0) {
		return false;
	}
	this->windowName.clear();
	this->windowName = windowName;
	this->window = cv::viz::Viz3d(this->windowName);
	return true;
}

bool openCvWrapper::isActiveTopic(std::string topic) {
	return std::find(this->topicNames.begin(), this->topicNames.end(), topic) != this->topicNames.end();
}

//-----getWindowByName------//
/*
This will take a windowName that is not 0 in length and set that as the new window focus.
Will also reset the camera so everything should be visable if the camera is not in the correct
position to see the new window's object.
*/
bool openCvWrapper::getWindowByName(std::string windowName) {
	if (windowName.size() == 0) {
		return false;
	}
	
	this->windowName.clear();
	this->windowName = windowName;
	this->window = cv::viz::getWindowByName(this->windowName);
	this->window.resetCamera();

	return true;
}

//-----showWindow------//
/*
This will display the window indefinately, until the user hits q,Q,e, or E in order to
have it exit the window. Will only show the window if this->windowName is already set
*/
bool openCvWrapper::showWindow() {
	if (this->windowName.size() == 0) {
		return false;
	}
	this->window.spin();
	return true;
}

//-----showWindow------//
/*
This is an overloaded function that will take 1-2 inputs.
int ms = the number of milliseconds to show the screen
bool redraw = force to redraw the image every time its shown
*/
bool openCvWrapper::showWindow(int ms, bool redraw) {
	if (this->windowName.size() == 0) {
		return false;
	}
	this->window.spinOnce(ms, redraw);
	return true;
}

//-----importPoints------//
/*
Check to make sure that the index exists and its possible to lock the mutex

Go ahead and copy the message over into the buffer class and unlock the mutex
*/
bool openCvWrapper::importPoints(std::string index, msg& message) {
	if (this->cameraSources.find(index) != this->cameraSources.end() && !this->cameraSources[index]->mut.try_lock()) {
		return false;
	}

	this->cameraSources[index]->size = message.size;
	delete[]this->cameraSources[index]->message;
	this->cameraSources[index]->message = new int16_t[message.size];
	memcpy(&this->cameraSources[index]->message[0], &message.message[0], message.size*2);
	this->cameraSources[index]->changed = true;

	this->cameraSources[index]->mut.unlock();

	return true;
}

//-----createWidget------//
/*
This function will create a widget object and put it into the window that has already
been set in place. Will fail if either the points field or windowName are empty
fields. Will succeed if both of those values are populated prior to calling this function.
*/
int openCvWrapper::createWidgets() {
	if (this->topicNames.size() == 0) {
		return 0;
	}

	int pointCount = 0;
	int count = 0;
	//Set the color depending on how many point clouds there are.
	for (auto iter : this->cameraSources) {
		cv::viz::Color color;
		switch (count) {
		case 0:
			color = color.red();
			break;
		case 1:
			color = color.green();
			break;
		case 2:
			color = color.blue();
			break;
		default:
			color = color.white();
		}
		//If the lock can be obtained, update the point cloud and unlock
		if (iter.second->mut.try_lock()) {
			if (iter.second->changed && iter.second->message != nullptr) {
				this->currentPoints[count].clear();
				int16_t* points = iter.second->message;
				int size = (int)iter.second->size / 3;
				for (int i = 0; i < size; ++i) {
					this->currentPoints[count].emplace_back(cv::Point3d(points[i * 3 + 0], points[i * 3 + 1], points[i * 3 + 2]));
				}
				pointCount += size;
				iter.second->changed = false;
				this->makeWidget(this->currentPoints[count], iter.first, color);

			}
			iter.second->mut.unlock();
		}

		++count;
	}

	return pointCount;
}

//-----clearAllWidgets------//
/*
This function is a simple wrapper to get rid of all the widgets on the window.
*/
void openCvWrapper::clearAllWidgets() {
	this->window.removeAllWidgets();

}

//-----clearAllWidgets------//
/*
This function is a simple wrapper to get rid of the matching widget name on the window.
*/
void openCvWrapper::clearWidgetByName(std::string widgetName) {
	this->window.removeWidget(widgetName);
}

//-----wasStopped------//
/*
This function is a simple wrapper for the wasStopped function on the window, which
checks to see if the user wanted to try and get close out of the window. If the value
is true, then the user did want to exit the window.
*/
bool openCvWrapper::wasStopped() {
	return this->window.wasStopped();
}
//-----setBackgroundColor------//
/*
This function will set the color if the window has already been set up
*/
bool openCvWrapper::setBackgroundColor(cv::viz::Color color) {
	if (this->windowName.size() == 0) {
		return false;
	}
	this->window.setBackgroundColor(color);
	return true;
}

//-----setBackgroundColor------//
/*
This function is a wrapper to reset the camera for the window.
*/
void openCvWrapper::resetCamera() {
	this->window.resetCamera();
}

//-----setBackgroundColor------//
/*
This function is a wrapper to pass in more keyboard events.
*/
void openCvWrapper::registerKeyboardCallback(cv::viz::Viz3d::KeyboardCallback callback) {
	this->window.registerKeyboardCallback(callback, &this->window);
}

//-----sphere------//
/*
Show a blue sphere that is larger than the normal points

The intent of this sphere is to show the origin and move the point cloud over to have the same origin
*/
void openCvWrapper::sphere(double size) {
	std::vector<cv::Point3d> p(1, cv::Point3d(0,0,0));
	cv::viz::WCloud sphere(p, cv::viz::Color::yellow());
	sphere.setRenderingProperty(cv::viz::POINT_SIZE, size);
	this->window.showWidget("sphere", sphere);
}

//-----parseArgs------//
/*
Get the min and max vectors to be used for a threshold cube, used in drawThreshold
*/
bool openCvWrapper::parseArgs(toml::Table table) {
	
	std::vector<std::int_least64_t> temp = toml::get<std::vector<toml::Integer>>(table.at("min"));
	threshold[0] = cv::Point3d((int)temp[0], (int)temp[1], (int)temp[2]);
	temp = toml::get<std::vector<toml::Integer>>(table.at("max"));
	threshold[1] = cv::Point3d((int)temp[0], (int)temp[1], (int)temp[2]);

	return true;
}

//-----drawThreshold------//
/*
Show the bounding box that the current threshold values are within the world origin.

Move point clouds around within the box in order to get it set to the spot it would like to be.

Uses the cv::viz::WCylinder to get the 'pipes' or the edges of the cube that threshold makes
*/
void openCvWrapper::drawThreshold(cv::viz::Color color) {
	cv::Point3d allmin = this->threshold[0];
	cv::Point3d allmax = this->threshold[1];


	this->window.showWidget("side1", cv::viz::WCylinder(cv::Point3d(allmin.x, allmin.y, allmin.z), cv::Point3d(allmax.x, allmin.y, allmin.z), 10, 30, color));
	this->window.showWidget("side2", cv::viz::WCylinder(cv::Point3d(allmin.x, allmin.y, allmin.z), cv::Point3d(allmin.x, allmax.y, allmin.z), 10, 30, color));
	this->window.showWidget("side3", cv::viz::WCylinder(cv::Point3d(allmin.x, allmin.y, allmin.z), cv::Point3d(allmin.x, allmin.y, allmax.z), 10, 30, color));
	this->window.showWidget("side4", cv::viz::WCylinder(cv::Point3d(allmin.x, allmax.y, allmin.z), cv::Point3d(allmin.x, allmax.y, allmax.z), 10, 30, color));
	this->window.showWidget("side5", cv::viz::WCylinder(cv::Point3d(allmin.x, allmax.y, allmin.z), cv::Point3d(allmax.x, allmax.y, allmin.z), 10, 30, color));
	this->window.showWidget("side6", cv::viz::WCylinder(cv::Point3d(allmin.x, allmax.y, allmax.z), cv::Point3d(allmin.x, allmin.y, allmax.z), 10, 30, color));
	this->window.showWidget("side7", cv::viz::WCylinder(cv::Point3d(allmin.x, allmax.y, allmax.z), cv::Point3d(allmax.x, allmax.y, allmax.z), 10, 30, color));
	this->window.showWidget("side8", cv::viz::WCylinder(cv::Point3d(allmax.x, allmax.y, allmax.z), cv::Point3d(allmax.x, allmax.y, allmin.z), 10, 30, color));
	this->window.showWidget("side9", cv::viz::WCylinder(cv::Point3d(allmax.x, allmax.y, allmax.z), cv::Point3d(allmax.x, allmin.y, allmax.z), 10, 30, color));
	this->window.showWidget("side10", cv::viz::WCylinder(cv::Point3d(allmax.x, allmin.y, allmax.z), cv::Point3d(allmin.x, allmin.y, allmax.z), 10, 30, color));
	this->window.showWidget("side11", cv::viz::WCylinder(cv::Point3d(allmax.x, allmin.y, allmax.z), cv::Point3d(allmax.x, allmin.y, allmin.z), 10, 30, color));
	this->window.showWidget("side12", cv::viz::WCylinder(cv::Point3d(allmax.x, allmax.y, allmin.z), cv::Point3d(allmax.x, allmin.y, allmin.z), 10, 30, color));
}

//-----setTopicNames------//
/*
Make sure that newTopicNames is a size of nonZero, if it is size zero, return false, bad list

Unsubscribe to all topics before doing anything to the new ones.

Set class member vector to the newTopics, if its greater than 1 there will be an extra 'blank' at the end from parsing the space separated string

Make the unordered_map entry first before subscribing

Set currentPoints to fit the new amount of topics
*/
bool openCvWrapper::setTopicNames(std::vector<std::string> newTopicNames) {
	if (newTopicNames.size() == 0) {
		return false;
	}

	this->unsubscribeTopics();

	this->topicNames = newTopicNames;
	
	this->currentPoints = std::vector<std::vector<cv::Point3d>>(this->topicNames.size());

	for (auto iter: this->topicNames) {
		this->cameraSources[iter] = new msg;
		mosquittoWrapper.subscribe((char*)iter.c_str());
		//Debugging to ensure which topics the visualizer has subscribed to
		//std::cout <<"|" <<iter <<"|" << std::endl;
	}

	return true;
}

//-----getIndexNames------//
/*
Return the current list of indexNames
*/
std::vector<std::string> openCvWrapper::getIndexNames() {
	return this->topicNames;
}

//-----unsubscribeTopics------//
/*
Checks that there are topic names (must have been subscribed already), error out if zero

Go through the list and unsubscribe from them all, obtain the lock and delete everything. This is a blocking lock to ensure no thread is accessing the current message.

Clear out the rest of the vectors
*/
bool openCvWrapper::unsubscribeTopics() {
	if (this->topicNames.size() == 0) {
		return false;
	}

	for (auto iter : this->topicNames) {
		mosquittoWrapper.unsubscribe((char*)iter.c_str());
		this->cameraSources[iter]->mut.lock();
		this->cameraSources[iter]->mut.unlock();
		delete this->cameraSources[iter];
	}

	this->topicNames.clear();
	this->cameraSources.clear();
	this->currentPoints.clear();

	return true;
}

void openCvWrapper::drawText(std::string text, std::string widgetName, float widthPercent, float heightPercent, int textSize) {
	cv::Size s = this->window.getWindowSize();
	cv::Point point((int)(s.width * widthPercent), (int)(s.height * heightPercent));
	cv::viz::WText textWidget(text, point, textSize);
	this->window.showWidget(widgetName, textWidget);
}

//-----isDone, setDone, isRestart, setRestart------//
/*
Simple getters and setters that manage program state
*/
bool openCvWrapper::isDone() {
	return this->done;
}

bool openCvWrapper::setDone(bool toSet) {
	return this->done = toSet;
}

bool openCvWrapper::isRestart() {
	return this->restart;
}

bool openCvWrapper::setRestart(bool toSet) {
	return this->restart = toSet;
}

//-------------------------------private functions-------------------------------//

//-----makeWidget------//
/*
Put the point cloud up into the viz window with the color decided from createWidgets
*/
void openCvWrapper::makeWidget(std::vector<cv::Point3d>& points, std::string widgetName, cv::viz::Color color) {
	if (points.size() == 0) {
		return;
	}
	cv::viz::WCloud cloudWidget(points, color);
	this->window.showWidget(widgetName, cloudWidget);
}

// async_subscribe.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// This application is an MQTT subscriber using the C++ asynchronous client
// interface, employing callbacks to receive messages and status updates.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker.
//  - Subscribing to a topic
//  - Receiving messages through the callback API
//  - Receiving network disconnect updates and attempting manual reconnects.
//  - Using a "clean session" and manually re-subscribing to topics on
//    reconnect.
//

/*******************************************************************************
 * Copyright (c) 2013-2017 Frank Pagliughi <fpagliughi@mindspring.com>
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Frank Pagliughi - initial implementation and documentation
 *******************************************************************************/

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include "mqtt/async_client.h"

const int	QOS = 0;

/////////////////////////////////////////////////////////////////////////////

// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.

class action_listener : public virtual mqtt::iaction_listener
{
	std::string name_;

	void on_failure(const mqtt::token& tok) override {
		std::cout << name_ << " failure";
		if (tok.get_message_id() != 0)
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
		std::cout << std::endl;
	}

	void on_success(const mqtt::token& tok) override {
		std::cout << name_ << " success";
		if (tok.get_message_id() != 0)
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
		auto top = tok.get_topics();
		if (top && !top->empty())
			std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
		std::cout << std::endl;
	}

public:
	action_listener(const std::string& name) : name_(name) {}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Local callback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */
class callback : public virtual mqtt::callback,
	public virtual mqtt::iaction_listener

{
	// Counter for the number of connection retries
	int nretry_;
	std::chrono::time_point< std::chrono::steady_clock> lasttrytime;

	// The MQTT client
	mqtt::async_client& cli_;
	// Options to use if we need to reconnect
	mqtt::connect_options& connOpts_;
	// An action listener to display the result of actions.
	action_listener subListener_;

	// This deomonstrates manually reconnecting to the broker by calling
	// connect() again. This is a possibility for an application that keeps
	// a copy of it's original connect_options, or if the app wants to
	// reconnect with different options.
	// Another way this can be done manually, if using the same options, is
	// to just call the async_client::reconnect() method.
	void reconnect() {
		auto now= std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lasttrytime).count() > 2500) {
			lasttrytime = now;
			try {
				bool connected = cli_.connect(connOpts_, nullptr, *this)->try_wait();
			}
			catch (const mqtt::exception & exc) {
				std::cerr << "Error: " << exc.what() << std::endl;
				exit(1);
			}
		}
	}

	// Re-connection failure
	void on_failure(const mqtt::token& tok) override {
		std::cout << "Connection attempt failed" << std::endl;
		nretry_++;
		reconnect();
	}

	// (Re)connection success
	// Either this or connected() can be used for callbacks.
	void on_success(const mqtt::token& tok) override {}

	// (Re)connection success
	void connected(const std::string& cause) override {
		std::cout << "\nConnection success" << std::endl;
		std::cout << "\nSubscribing to topic '" << topic << std::endl;

		lasttrytime = std::chrono::steady_clock::now();

		cli_.subscribe(topic, QOS, nullptr, subListener_);
		cli_.start_consuming();
	}

	// Callback for when the connection is lost.
	// This will initiate the attempt to manually reconnect.
	void connection_lost(const std::string& cause) override {
		std::cout << "\nConnection lost" << std::endl;
		if (!cause.empty())
			std::cout << "\tcause: " << cause << std::endl;

		std::cout << "Reconnecting..." << std::endl;
		nretry_ = 0;
		reconnect();
	}

	// Callback for when a message arrives.
	void message_arrived(mqtt::const_message_ptr msg) override { }

	void delivery_complete(mqtt::delivery_token_ptr token) override {}

public:
	callback(mqtt::async_client& cli, mqtt::connect_options& connOpts)
		: nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription") {}

	std::string topic = "frames";
};

/////////////////////////////////////////////////////////////////////////////


//Buffer to receive "raw" mqtt data into
struct TextureData {

	uint64_t timestamp;
	uint32_t pointcount;
	bool hasColor;

	//position data
	std::vector<int16_t>* depth_data;

	//rgb data
	std::vector<uint8_t>* color_data;

};

//Wrap mqtt client and provide ability to parse mqtt packet
class MQTTClient {

	mqtt::async_client* client;
	mqtt::connect_options* connOpts;
	callback* cb;
	int messagecount = 0;


public:
	MQTTClient() {
		MQTTAsync_nameValue* v = MQTTAsync_getVersionInfo();
		std::cout << v[0].value << " v:" << v[1].value << std::endl;

		//we start out wit nothing
		client = NULL;
		cb = NULL;
		connOpts = NULL;
	}

	~MQTTClient() {
		
	}

	void Connect(const std::string& url, const std::string& clientid, const std::string& topic) {
		if (client != NULL ){
			if (client->is_connected()) {
				Disconnect();
			}
		}
		
		//async client plumbing
		connOpts = new mqtt::connect_options();
		connOpts->set_keep_alive_interval(5);
		connOpts->set_clean_session(true);
		connOpts->set_connect_timeout(5);

		client = new mqtt::async_client(url, clientid);

		cb = new callback(*client, *connOpts);
		cb->topic = topic;
		client->set_callback(*cb);

		// Start the connection.
		// When completed, "callback" will subscribe to topic.

		try {
			std::cout << "Connecting to the MQTT server..." << std::flush;
			mqtt::token_ptr t = client->connect(*connOpts, nullptr, *cb);
		}
		catch (const mqtt::exception & exc) {
			std::cerr << "\nERROR: Unable to connect to MQTT server: '"
				<< url << "' error:" << exc.what() << std::endl;
		}
	}

	void Disconnect() {
		if (client != NULL) {

			//tear it down
			if (client->is_connected()) {
				client->disconnect()->wait();
			}
			
			delete cb;
			delete client;		
			delete connOpts;

			client = NULL;
			cb = NULL;
			connOpts = NULL;
		}
	}

	//copy mqtt payload into a struct that is passed in, returns the pointcount
	//clears out the struct if it has points from previous frames
	int GetData( TextureData& data) 
	{
		if (client  == NULL) {
			return -1;
		}

		if (!this->client->is_connected())
		{
			return -1;
		}

		//try and get a package from the mqtt queue
		mqtt::const_message_ptr ptr;
		auto messageConsumed = this->client->try_consume_message(&ptr);
		if (messageConsumed) {
			
			//work with the payload
			mqtt::binary payload = ptr->get_payload();

			//get timestamp
			int header = 0; 
			memcpy(&data.timestamp, &payload[0], sizeof(uint64_t));
			header += sizeof(uint64_t);

			//get point count
			uint32_t headerPointCount;
			memcpy(&headerPointCount, &payload[header], sizeof(uint32_t));
			header += 4;

			//does the packet have depth and color
			memcpy(&data.hasColor, &payload[header], sizeof(bool));
			header += 1;

			uint32_t payloadPointCount = (payload.size() - header) / 9;
			//clear our buffers if we have less points
			if (payloadPointCount < data.pointcount) {
				std::fill(data.depth_data->begin(), data.depth_data->end(), 0);
				std::fill(data.color_data->begin(), data.color_data->end(), 0);
			}

			//payload count trumps the count in the header, just in case the header value is wrong
			if (payloadPointCount != headerPointCount) {
				headerPointCount = payloadPointCount;
			}

			//play it safe, don't go larger than what fits in TD texture
			if (headerPointCount > data.depth_data->size() / 3) {
				headerPointCount = data.depth_data->size() / 3;
			}
			data.pointcount = headerPointCount;

			//copy over depth data
			int positionssize = sizeof(int16_t) * ((__int64)data.pointcount* 3);
			memcpy(data.depth_data->data(), &payload[header], positionssize);

			//if color is included, copy this too
			if (data.hasColor) {
				int colorsize = sizeof(uint8_t) * ((__int64)data.pointcount * 3);
				int depthEnd = sizeof(int16_t)* ((__int64)payloadPointCount * 3);
				memcpy(data.color_data->data(), &payload[(__int64)header + (__int64)depthEnd], colorsize);
			}

			//clean up
			ptr.reset();
			
			//we need to clear out the queue to prevent a memory leak. 
			//we don't have a way to know the current queue size.
			//so we only process the first message (oldest) above and clear the rest out below
			//the chances that we will create a queue in runtime is small as touch runs faster than the network
			int queuesize = 0;
			while (this->client->try_consume_message(&ptr)) {
				queuesize++;
				ptr.reset();
			}

			return data.pointcount;
		}
		else {			
			ptr.reset();

			return 0;
		}
	}

};
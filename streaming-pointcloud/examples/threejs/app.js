const config    = require("./config.json");
const mqtt      = require("mqtt");
const express   = require("express");
const socketio  = require("socket.io");

// Start up the HTTP and websocket server

const app = express();

app.use('/', express.static(__dirname + "/public"));

const server = app.listen(config.http_server.port, "0.0.0.0", function() {
    console.log("HTTP server running on port: ", server.address().port);
});

const io = socketio(server);

// Connect to the MQTT broker and subscribe to the topic for point cloud frames.

const mqttClient = mqtt.connect(config.mqtt);

mqttClient.on("connect", function () {
    console.log("Connected to MQTT broker.");
    mqttClient.subscribe(config.points_topic);
});

mqttClient.on("message", function(topic, payload){
    // Forward point frames on as websocket messages.
    if(topic == config.points_topic) {
        io.emit("points", payload);
    }
});

# Point Cloud Visualizer

## Introduction

The Point Cloud Visualizer will display multiple kinect capture sources and show the depth data. The visualizer will also adjust/calibrate the camera's coordinate system into the real world coordinate system by keypresses.

**Note:** This application provides a visualizer for the depth data source(s) and does not do much by itself. It's meant to be used as a calibration and debugging tool.

# Prerequisites

In order to use this application, you will need the following:

1. An MQTT broker such as [Mosquitto](https://mosquitto.org/download)
1. The executable `'Mqtt visualizer.exe'` which is available in this repository or can be compiled from the source.
1. The executable `'merger.exe'` which is also available in this repository or can be combiled from the source.


# Usage
This is a command line application that also has a GUI visualization window. The program does not take any command line arguments.
```
'Mqtt visualizer.exe'
```


# Configuration
For configuration, the application will look for a file named `settings.toml`.

## Settings TOML
The contents of the TOML file are as follows:

| Property | Description | Values |
| -------- | ----------- | ------ |
|`unit`| amount to shift the point cloud by a single keypress (in millimeter) | Any whole integer|
|`merged`| Choose between viewing cameras individually or the merged point cloud | `true` or `false`|


## [MqttInfo] Section

Connection settings for the MQTT client are stored in the `[MqttInfo]` section.

| Property | Description |
| -------- | ----------- |
| `host` | the IP address of the MQTT broker |
| `port` | the port for the MQTT broker (typically this is 1883) |
| `id` | the MQTT client ID for this application (this must be unique among clients). |
| `topic` | the MQTT topic upon which the depth frames will be published. |
| `keepalive` | The MQTT keepalive interval, in seconds |
| `clean_session` | The `clean_session` value to use when connecting to the broker (see MQTT spec)

### [Threshold] Section

Threshold settings are used to visualize the bounding box that the `kinect Capture` software uses for the culling points. This is not the actual bounding box for culling points, it is a debugging representation on the thresholding for the sources.

| Property | Description |
| -------- | ----------- |
| `min` | The minimum X, Y, and Z boundary, in millimeters, for bounding box. |
| `max` | The maximum X, Y, and Z boundary, in millimeters, for bounding box. |


# Point cloud manipulation

![Coordinate system for the Microsoft Kinect for Azure](https://docs.microsoft.com/en-us/azure/kinect-dk/media/concepts/concepts-coordinate-systems/coordinate-systems-camera-features.png)

| Key | Description of command |
| --- | ---------------------- |
| ↑ | Move the point cloud up |
| ↓	| Move the point cloud down |
| → | Move the point cloud right |
| ←	| Move the point cloud left |
| Home | Move the point cloud away (positive z) |
| End | Move the point cloud closer (negative z) |
| 8	| Move the point cloud counter clockwise along the x axis |
| 2	| Move the point cloud clockwise along the x axis |
| 4	| Move the point cloud counter clockwise along the y axis |
| 6	| Move the point cloud clockwise along the y axis |
| 7	| Move the point cloud counter clockwise along the z axis |
| 9	| Move the point cloud clockwise along  the z axis |

If viewing the camera sources individually, they will show the following color (not the actual color of the rgb points) for their assigned ordering:
| Device Number | Color |
| ------------- | ----- |
| 1 | red |
| 2 | green |
| 3 | blue |
| 4<sup>+</sup> | white |

**Note:** If there is only one color present, but multiple cameras going, the viewer is set to look at the merged point cloud

# Keybindings
In addition to the point cloud manipulation commands, the visualizer has some other helpful commands present.

| Key   | Description of command |
| :---: | ---------------------- |
|   Q   | Exit out of the program |
|   L   | Have `kinect Capture` applications save their transformations |
|   N   | Toggle onscreen help text |
|   M   | Restart the programs without closing |
|  TAB  | Cycle active selected camera |
|   H   | Display the help text in the terminal |

 # Compiling from Source
To compile this application from source, you will need the following:

+ Visual Studio 2019 with MSVC v142 or greater

+ Install the following packages via [Vcpkg](https://github.com/microsoft/vcpkg) 
  + boost:x64-windows
  + mosquitto:x64-windows
  + opencv[contrib,core,vtk]:x64-windows

+ If you experience problems compiling the projects related to the Kinect SDK or libraries, it may be that you need to   manually add the Microsoft Azure Kinect Sensor package. You can do this from within Visual Studio via the Nuget Package   Manager.
  + In Visual Studio, open the `Tools` menu and select `NuGet Package Manager > Manage NuGet Packages for Solution...`
  + Search for `Microsoft.Azure.Kinect.Sensor` and install it.

+ [TomlParser](https://github.com/ToruNiina/TOMLParser) Library
  + Download or clone the repository and copy the contents to ```packages/```
  + Insert ```#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS``` to the top of ```src/toml_parser.hpp```

Once you have all of these prerequisites, open `Mqtt visualizer.sln` in Visual Studio to build the application.
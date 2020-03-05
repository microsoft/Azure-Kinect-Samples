# Azure Kinect Capture

This application will capture depth frames from the Azure Kinect camera and transmit them as a set of 3D points via MQTT, with or without the matching RGB colors for each point. It can also perform translation and rotation of the depth points prior to transmission in order to align multiple cameras in the same phsyical space.

**Note:** This application provides a depth data source and does not do much by itself. It's meant to be used in conjunction with other applications that make use of the depth data.

# Prerequisites

In order to use this application, you will need the following:

1. An Azure Kinect camera
1. An MQTT broker such as [Mosquitto](https://mosquitto.org/download)
1. The executable `'kinect capture.exe'` which is available in this repository or can be compiled from source.


# Usage

This is a command line application with two optional arguments.

```
'kinect capture.exe' [device index] [timeout]
```

+ `[device index]` The numeric index of the device (used when there are multiple cameras connected to a single computer). Default is zero.
+ `[timeout]` The timeout, in milliseconds, when waiting for frames (see _insert link to kinect documentation_ for more information). Default is 10 milliseconds.


# Configuration

For configuration, the application will look for a file named `settings[device index].toml` where `[device index]` matches the optional device index command line argument.

This means that the application will attempt to open `settings0.toml` if no device index is specified.

## Settings TOML

The settings TOML file contains the following values.

| Property | Description |
| ---- | --- |
| `file`| The name of the TOML file containing camera transform and clipping values (see below). |

### [MqttInfo] Section

Connection settings for the MQTT client are stored in the `[MqttInfo]` section.

| Property | Description |
| ---- | --- |
| `host` | the IP address of the MQTT broker |
| `port` | the port for the MQTT broker (typically this is 1883) |
| `id` | the MQTT client ID for this application (this must be unique among clients). |
| `topic` | the MQTT topic upon which the depth frames will be published. |
| `keepalive` | The MQTT keepalive interval, in seconds |
| `clean_session` | The `clean_session` value to use when connecting to the broker (see MQTT spec) |


## Camera Transform TOML

Camera translation, rotation, and clipping values are specified in a separate TOML file. The name of this file is configurable via the `file` property in the settings TOML (see above).

By specifying these values you can compensate for the position of the camera in physical space, and cull any points that fall outside a specific region of interest (such as distant objects, walls, floor, ceiling, etc).

The transformation values are applied first, then the transformed points are tested 

The file must contain the following values:

### [Camera] Section

| Property | Description |
| ---- | --- |
| `translation` | The X, Y, and Z translation, in millimeters, to apply to all points. |
| `rotation` | The X, Y, and Z rotation, in degrees, to apply to all points. |
| `min` | The minimum X, Y, and Z boundary, in millimeters, for culling points. |
| `max` | The maximum X, Y, and Z boundary, in millimeters, for culling points. |

# Compiling from Source

To compile this application from source, you will need the following:

+ Visual Studio 2019 with MSVC v142 or greater

+ Install the following packages via [Vcpkg](https://github.com/microsoft/vcpkg) 
  + boost:x64-windows
  + eigen3:x64-windows
  + mosquitto:x64-windows

+ If you experience problems compiling the projects related to the Kinect SDK or libraries, it may be that you need to   manually add the Microsoft Azure Kinect Sensor package. You can do this from within Visual Studio via the Nuget Package   Manager.
  + In Visual Studio, open the `Tools` menu and select `NuGet Package Manager > Manage NuGet Packages for Solution...`
  + Search for `Microsoft.Azure.Kinect.Sensor` and install it.

+ [TomlParser](https://github.com/ToruNiina/TOMLParser) Library
  + Download or clone the repository and copy the contents to ```packages/```
  + Insert ```#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS``` to the top of ```src/toml_parser.hpp```

Once you have all of these prerequisites, open `kinect capture.sln` in Visual Studio to build the application.

# License
The Eigen3 library is under the MPL2.0 License, information regarding the license can be found [here](./Eigen.MPL2).

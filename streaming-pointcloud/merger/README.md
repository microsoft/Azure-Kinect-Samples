# Point Cloud Merger Application

The Point Cloud Merger Application will consume MQTT data from one or more Kinect Capture instances and publish the merged data for use in interactive applications.

# Prerequisites

In order to use this application, you will need the following:

1. One or more Azure Kinect cameras
1. An MQTT broker such as [Mosquitto](https://mosquitto.org)
1. One or more instances of the `kinect capture` application configured to publish data to your MQTT broker.
1. The executable `'merger.exe'`, which is available in this repository or can be compiled from source.


# Usage

This is a console application which takes one optional argument:

```
merger.exe [settings_filename]
```

`[settings_filename]` - the name of the TOML file containing the application settings, default is `settings.toml`


# Configuration

Configuration for the merger application is stored in a TOML file. By default the application will look for `settings.toml` or you may specify the file name via the optional command line parameter (see above).

The contents of the TOML file are as follows:

## [CameraInfo] Section

The `merger` application communicates with the `kinect capture` instances to set the desired camera configuration (resolution, frame rate, etc). The capture settings are specified in the `[CameraInfo]` section of the settings.toml file.


| Property | Description | Values |
| ---- | --- | --- |
| `fps` | Camera frame rate | `'5'`, `'15'`, or `'30'` |
| `depth_mode` | The depth mode for the camera * | `'NFOV_BINNED'`, `'NFOV_UNBINNED'`, `'WFOV_BINNED'`, `'WFOV_UNBINNED'` |
| `color` | Capture the RGB color data for each 3D point | `True`, `False` |
| `color_resolution` | The resolution of the color camera input ** | `'2160P'`, `'1440P'`, `'1080P'`, `'720P'`, `'3072P'`, `'1536P'` |

\* All depth modes can operate up to 30 FPS except `'NFOV_UNBINNED'` which has a maximum frame rate of `'15'`

** All color modes can operate up to 30 FPS except `'3072P'` which has a maximum frame rate of `'15'`


## [MqttInfo] Section

Connection settings for the MQTT client are stored in the `[MqttInfo]` section.

| Property | Description |
| ---- | --- |
| `host` | the IP address of the MQTT broker |
| `port` | the port for the MQTT broker (typically this is 1883) |
| `id` | the MQTT client ID for this application (this must be unique among clients). |
| `topic` | the MQTT topic upon which the depth frames will be published. |
| `keepalive` | The MQTT keepalive interval, in seconds |
| `clean_session` | The `clean_session` value to use when connecting to the broker (see MQTT spec)


# Compiling from Source

To compile this application from source, you will need the following:

+ Visual Studio 2019 with MSVC v142 or greater

+ Install the following packages via [Vcpkg](https://github.com/microsoft/vcpkg) 
  + mosquitto:x64-windows
  + boost:x64-windows

+ If you experience problems compiling the projects related to the Kinect SDK or libraries, it may be that you need to   manually add the Microsoft Azure Kinect Sensor package. You can do this from within Visual Studio via the Nuget Package   Manager.
  + In Visual Studio, open the `Tools` menu and select `NuGet Package Manager > Manage NuGet Packages for Solution...`
  + Search for `Microsoft.Azure.Kinect.Sensor` and install it.

+ [TomlParser](https://github.com/ToruNiina/TOMLParser) Library
  + Download or clone the repository and copy the contents to ```packages/```
  + Insert ```#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS``` to the top of ```src/toml_parser.hpp```

Once you have all of these prerequisites, open `merger.sln` in Visual Studio to build the application.

# Building

You can find detailed documentation for each of the specific components in their respective README files, however there are several commonalities and shared requirements among the projects which are described here.

## Vcpkg

You may install and build the required libraries manually but it is simpler to use the [Vcpkg](https://github.com/microsoft/vcpkg) package management tool provided by Microsoft.

### Installing and Setting up Vcpkg

You can install Vcpkg via the command line using the following steps:

| Powershell command | Description |
|--------------------|-------------|
|`'git clone git@github.com:microsoft/vcpkg.git'` | Clone the repo |
|`'cd vcpkg'` | Change to the vcpkg directory |
|`'.\bootstrap-vcpkg.bat'` | Run the bootstrapper |
|`'.\vcpkg.exe integrate install'` | Set the PATH for visual studio |

After running `vcpkg.exe integrate install` you should see the following:
```
PS C:\vcpkg> .\vcpkg.exe integrate install
Applied user-wide integration for this vcpkg root.

All MSBuild C++ projects can now #include any installed libraries.
Linking will be handled automatically.
Installing new libraries will make them instantly available.

CMake projects should use: "-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
```


### Installing the libraries

Once you have installed Vcpkg, you can install the dependencies for building the various C++ projects.

| Powershell command | Description |
|--------------------|-------------|
| `.\vcpkg.exe install boost:x64-windows` | Install 64-bit version of Boost |
| `.\vcpkg.exe install eigen3:x64-windows` | Install 64-bit version of Eigen3 |
| `.\vcpkg.exe install mosquitto:x64-windows` | Install 64-bit version of mosquitto |
| `.\vcpkg.exe install opencv[contrib,core,vtk]:x64-windows` | Install 64-bit version of Opencv (May take time) |

After installing these libraries, run the following command prior to opening Visual Studio:

`vcpky.exe integrate install`

## Visual Studio 2019

This respository includes the source code and Visual Studio solutions to be used with Visual Studio 2019. You must ensure that you have the 

### MSCV v142

To ensure that MSCV v142 is installed on the machine, open `Visual Studio Installer` and click `modify` within `Visual Studio 2019`

Go to the `Individual components` tab located at the top and search for `MSVC v142 - VS 2019 C++ x64/x86 build tools (v14.20)`

Click `Modify` at the bottom right of the screen and wait for the installer to finish

### NuGet - Install the Azure Kinect Package

If you experience problems compiling the projects related to the Kinect SDK or libraries, it may be that you need to manually add the Microsoft Azure Kinect Sensor package. You can do this from within Visual Studio via the Nuget Package Manager.

To get there open the `Tools` menu and select `NuGet Package Manager > Manage NuGet Packages for Solution... > Search`

Search for `Microsoft.Azure.Kinect.Sensor` and install it.

## Toml Parser

This project uses Toml files to store configuration, to build the project you will need to place the [TomlParser](https://github.com/ToruNiina/TOMLParser) source in the `packages` folder of each project you wish to build. In order to avoid warnings when building the project, you will need to modify the Toml source code as follows:

Edit `src/toml_parser.hpp` and add the following line to the top of the file:

```#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS```

There is a zipped up version of the github repo used for the [TomlParser](https://github.com/ToruNiina/TOMLParser) availble at the root directory which already contains this modification.

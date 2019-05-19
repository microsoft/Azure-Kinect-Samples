
# Azure Kinect Streaming Service - Samples

Azure Kinect Streaming Service is executable deployed as Docker container, that exposes Azure Kinect streams using [WebSocket](https://www.websocket.org/) protocol, [RFC 6455](https://tools.ietf.org/html/rfc6455). 
Websocket is supported by most popular languages to write machine learning code: 

![C#](images/logo_Csharp.png) ![NodeJS](images/logo_nodejs.png)	![Python](images/logo_python.png) ![Java](images/logo_java.png)	![Go](images/logo_go.png)

Azure Kinect Streaming Service provides is fastest way to integrate your Azure Kinect device with [Microsoft Cognitive Services](https://azure.microsoft.com/en-us/services/cognitive-services/).

## Getting Started (Windows)
These instructions will guide you throug initial Docker setup, installing Azure Kinect Streaming Service container and run samples on Windows operating system.

### Prerequisites
- Latest RS5 [Windows Operating System](https://www.microsoft.com/en-us/software-download/windows10) Installed (Pro or Enterprise)

### Install Docker (Windows)
- Follow [instructions](https://docs.microsoft.com/en-us/virtualization/windowscontainers/quick-start/quick-start-windows-10) to enable Containers and Hyper-V feature on installed Windows OS.
- Install Docker for Windws:
  - Please follow [instructions](https://hub.docker.com/editions/community/docker-ce-desktop-windows) to download, install and run **Edge** version of docker for windows.
  - Switch to use windows containers. Detailed instructions can be found [here](https://docs.docker.com/docker-for-windows/#docker-settings-dialog), section *Switch between Windows and Linux containers*. 
### Install Azure Kinect Streaming Service Container
- Run cmd with elevated privilege (As Administrator)
- Determine Windows Version
  - Run cmd
    ```
    winver
    ```
  - The following window will show up. Number next to Version is your windows version number. Yellow marked area on image shows where is version number.
 
    ![C#](images/winver.png)
    - **NOTE:**: The following version images are currently supported: 1809, 1903
- Login to Azure Container Registry (ACR) and get image. [Anonymous read-only access]((https://feedback.azure.com/forums/903958-azure-container-registry/suggestions/31655977-configure-permissions-at-a-repository-level)) is not yet available. Replace *\<version number>* with your windows version.
	```
	cd c:\docker
	docker login samplecontainers.azurecr.io -u samplecontainers -p dRiESWMFW36DE3iqazuCq+xk3FOwTn8u
	docker pull samplecontainers.azurecr.io/k4astreaming:<version number>
	``` 
### Run Azure Kinect Streaming Service in Container
- Plug in Azure Kinect Camera
- Run cmd with elevated privilege (As Administrator). Replace *\<version number>* with your windows version.
	```
	cd c:\docker
	docker run --device "class/F18A0E88-C30C-11D0-8815-00A0C906BED8" --device "class/A5DCBF10-6530-11D2-901F-00C04FB951ED" --device "class/875CFCC8-678E-4C12-BA03-5119D706A3F0" --device "class/65E8773D-8F56-11D0-A3B9-00A0C9223196" --device "class/875CFCC8-678E-4C12-BA03-5119D706A3F0" --device "class/AAF9A6D4-E299-4D55-990D-6F6F7324A717" --device "class/F97EE10A-5022-44BF-B0D0-729D453A33C6" --publish 8888:8888 --isolation process -it samplecontainers.azurecr.io/k4astreaming:<version number>
	```
    - *--device* argument lists all hardware devices from host OS to be forwarded to container. They represent all USB hardware devices used by Azure Kinect.
- To verify installation, click on link http://localhost:8888. On web page, select Camera Samples/Color Stream, pres Start and you should see color stream from camera.
- **NOTE:** Please see *Known Issues* section for known issues with running Azure Kinect in Windows Containers

### Setup Azure Cognitive Services Access
- Please follow up [instructions](https://docs.microsoft.com/en-us/azure/cognitive-services/cognitive-services-apis-create-account) to create Cognitive Services account, and obtain **Endpoint** (for example https://westus.api.cognitive.microsoft.com).
- Please follow up [instructions](https://docs.microsoft.com/en-us/azure/cognitive-services/authentication) to obtain **Service Subscription Key**.
- To verify access please follow C# sample instructions below to place **Endpoint** and **Service Subscription Key** to proper places in code.

# Samples
![NodeJS](images/logo_nodejs.png)

We provided rich set of NodeJS samples for you. You can find all NodeJS samples at [nodejs](nodejs) folder.  
Please see start page at http://localhost:8888, once you run Azure Kinect Streaming Service container to run sample and learn more how to use Websocket URL parameters to access Azure Kinect streams.  
http://localhost:8888 web page provides instructions how to build container with your own NodeJS solution, using Azure Kinect Streaming Service!  
  
![C#](images/logo_Csharp.png)

[Face Detection](c-sharp) sample demonstrates how to build C# solution to access Azure Kinect color stream remotely and use it with [Cognitive Services](https://azure.microsoft.com/en-us/services/cognitive-services/) to detect face.  
Pattern in sample can be easily used to access other Azure Kinect streams and use them with other Cognitive Servcies to build rich computer vision and speech applications.
- **Instructions** for Face Recognition to work:
	- Please complete steps outlined at Setup **Azure Cognitive Services Access** section.
    - In [DisplayStream.cs](c-sharp/FaceDetection/DisplayStream.cs), search for **\<YOUR SUBSCRIPTION KEY>** string and replace it with **Service Subscription Key** for your subscription.
    - In [DisplayStream.cs](c-sharp/FaceDetection/DisplayStream.cs), search for **\<YOUR ENDPOINT>** string and replace it with **Endpoint** for your subscription.
- For more information how to access other Azure Kinect streams, please run Azure Kinect Streaming Servcie container, and see http://localhost:8888, API menu option.
- For more information how to use other [Cognitive Services](https://azure.microsoft.com/en-us/services/cognitive-services/), please follow up tutorials for [Cognitive Services](https://azure.microsoft.com/en-us/services/cognitive-services/) of your interest.

![Python](images/logo_python.png)

[colorStream.py](python/colorStream.py) is simple Python application to demonstrate how to access Azure Kinect color stream, exposed by Azure Kinect Streaming Service.
- **Instructions** to install dependencies necessary for colorStream.py to work:
  - If you don't have Python installed on your computer, please follow [instructions](https://www.python.org/) to install Python. We recommend Python 3.7 but sample works with other versions as well.
  - Install dependencies from cmd line:
	```
	pip install websockets
	python -m pip install numpy
	pip install opencv-python
	``` 
- For more information how to access other Azure Kinect streams, please run Azure Kinect Streaming Servcie container, and see http://localhost:8888, API menu option.  

# Known Issues
### 1. Consecutive attempt to run container with Streaming Service fails.
If you follow procedure how to run Azure Kinect Streaming Service, and try to start it multiple times, start might faile with the following trace message:
```
[2019-03-28 13:22:40.022] [error] [t=4196] [Color] Failed to activate source in open camera: 0x80070003
```
If you encounter this issue you will likely have to restart host computer.
This is known issue with windows ks.sys driver and Windows team is working to provide fix for issue.
#### Workaround
Start Azure Streaming Service Container using the following command:
```
docker run --device "class/F18A0E88-C30C-11D0-8815-00A0C906BED8" --device "class/A5DCBF10-6530-11D2-901F-00C04FB951ED" --device "class/875CFCC8-678E-4C12-BA03-5119D706A3F0" --device "class/65E8773D-8F56-11D0-A3B9-00A0C9223196" --device "class/875CFCC8-678E-4C12-BA03-5119D706A3F0" --device "class/AAF9A6D4-E299-4D55-990D-6F6F7324A717" --device "class/F97EE10A-5022-44BF-B0D0-729D453A33C6" --publish 8888:8888 --isolation process -it samplecontainers.azurecr.io/k4astreaming cmd
```
Command adds **cmd** at the end. This will start container in c:\k4astreaming folder without starting Azure Kinect Streaming Service. All service binaries are in that folder. Now you can run StreamingService.exe as many times as you would like without encountering issue.

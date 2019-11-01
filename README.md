# Azure Kinect DK Code Samples Repository

This GitHub repository contains code samples that demonstrate how to use Microsoft's Azure Kinect DK Sensor and Body
Tracking SDKs.

For more information about the Azure Kinect DK and available documentation, see [Azure.com/Kinect]
(https://azure.microsoft.com/services/kinect-dk/)

Each sample includes a README file that explains how to run and use the sample.

[![Build Status](https://microsoft.visualstudio.com/Analog/_apis/build/status/Analog/AI/depthcamera/microsoft.Azure-Kinect-Samples?branchName=master)]
(https://microsoft.visualstudio.com/Analog/_build/latest?definitionId=41402&branchName=master)

## Contribute

We welcome your contributions and suggestions! Please open new issues in our [Azure Kinect Sensor SDK repository]
(https://github.com/microsoft/Azure-Kinect-Sensor-SDK/issues). Most contributions require you to agree to a Contributor
License Agreement (CLA) declaring that you have the right to, and actually do, grant us the rights to use your 
contribution. For details, visit https://cla.microsoft.com. When you submit a pull request, a CLA-bot will 
automatically determine whether you need to provide a CLA and decorate the PR appropriately (e.g., label, comment). 
Simply follow the instructions provided by the bot. You will only need to do this once across all repos using our CLA.

## Join Our Developer Program

Complete your developer profile [here](https://mixedreality.microsoftcrmportals.com/signup/) to get connected with our Mixed Reality Developer Program. You will receive the latest on our developer tools, events, and early access offers.

## Building

Due to the samples being provided from various sources they may only build for Windows or Linux. To keep the barrier for adding new samples low we only require that the sample works in one place.

### Windows

If the project has Visual Studio solution open it and build. CMake is not currently supported for Windows, though we support the community expanding on this.

### Linux

For building with Linux, CMake is used. Build from a `git clone`, we do not support building from a ZIP file.

Install the prerequisites
```
apt install libk4abt0.9-dev
apt install libxi-dev
```

From the root of the git project
```
mkdir build
cd build
cmake .. -GNinja
ninja
```


## Microsoft Code of Conduct

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact opencode@microsoft.com with any additional questions or comments.

## License

[MIT License](LICENSE)

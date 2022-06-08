# Walkthrough of Azure Kinect and Cognitive Services Face API

This project demonstrates how to read data from an Azure Kinect DK, and identify faces using the Azure Cognitive Services Face API.

## Overview

* Using the Azure Kinect Sensor SDK with C# in Visual Studio
* Filter images with depth data
* Call Cognitive Services Face API with images from the Azure Kinect DK
* Map face positions to depth distances

## Before Beginning

You will need a Cognitive Services Face API key.

Visit http://aka.ms/faceapi
* Click "Try Face"
* Follow the prompts for a free trial, or to add the Face API to your subscription

Add your API key to your environment:

```shell
setx FACE_SUBSCRIPTION_KEY 1234abcd....
setx FACE_ENDPOINT https://westcentralus.api.cognitive.microsoft.com/face/v1.0
```

## Open the Project

Open AzureKinectFaceAPI.sln in Visual Studio 2019. Make sure that Visual Studio is up to date.

The solution has a number of projects for different stages of development.

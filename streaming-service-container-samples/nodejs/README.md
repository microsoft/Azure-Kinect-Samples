# NodeJS Samples
![NodeJS](../images/logo_nodejs.png)

Set of samples for to demonstrate how to access Azure Kinect DK streams from javascript. Samples have minimal UI to demonstrate code functionality.

## Prerequisites
- NodeJS installed (Instructions [HERE](https://nodejs.org/en/))
- Azure Kinect Streaming Service running

## Run Samples
- Open cmd window
- Navigate cmd window to nodejs folder.
- Type the following:
  ```
  npm install
  npm start
  ```
- Web browser will pop up with address in address bar: http://localhost:3001. Web browser will display index.html with links to each sample.
- You can run separate sample by simply add sample html file after URL: For example, to run color camera sampe, type http://localhost:3001/color.html.

## Cognitive Services Sample
In order to run Cognitive Services - Face Detection sample you have to setup Cognitive Services Access. 

- Please follow [instructions](../README.md), **Setup Congitive Services Access** section.
- Replace *\<Service Subscription Key\>* and *\<Endpoint\>* strings, in cognition.html file, with values from previous step.



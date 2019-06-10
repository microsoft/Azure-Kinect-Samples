// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

var CognitiveServices = function(uri, subscriptionKey)
{
	"use strict";

    var uriBase = uri;
    var subscriptionKey = subscriptionKey;
    
    var requestInProgress = false;          // Ensures not to send new request before outstanding request is processed
    var contentCache;                       // Cacheing frame that was sent to cognitive services.
    var lastRequestedService;               // Last service that was requested (face recognition, speech recognition, ...)
    var imageUrl;                           // Cached image url object, if content is image
    var services;

    const faceRecognitionParams = "returnFaceId=true&returnFaceLandmarks=false" +
    "&returnFaceAttributes=age,gender,headPose,smile,facialHair,glasses," +
    "emotion,hair,makeup,occlusion,accessories,blur,exposure,noise";

    services = this.service =
    {
        faceRecognition:    1,
        objectRecognition:  2,
        speechRecognition:  3,
    }

    // Requesting congnitive service processing.
    this.CognitiveServiceRequest = function (serviceRequested, content)
    {
        // Don't request new reqest if there is outstanding one
        if (requestInProgress == true)
        {
            return;
        }
        
        requestInProgress = true;

        var queryURL = uriBase + '/face/v1.0/detect?' + faceRecognitionParams;

        var xhr = new XMLHttpRequest();
        xhr.open('POST', queryURL, true);
        xhr.setRequestHeader('Ocp-Apim-Subscription-Key', subscriptionKey);
        xhr.setRequestHeader('Content-Type', 'application/octet-stream');

        // [TODO]: Implement in streaming services inserting Azure Kinect user agent in javascript. This is needed
        // for traffic measuring purposes, i.e. to know how much traffic to cognitive services is comming from streaming
        // service.
        xhr.setRequestHeader('User-Agent', 'k4astreamservice');
    
        xhr.onload = ProcessSuccess;
        xhr.onerror = ProcessError;
        xhr.onabort = ProcessAbort;
        
        contentCache = content;
        lastRequestedService = serviceRequested;
        xhr.send(content);
    }
    
    function ProcessSuccess(event)
    {
        requestInProgress = false;
        var retContent;

        if (this.status == 200 && this.readyState == 4){
            var jsonData = JSON.parse(this.response);
    
            if (lastRequestedService == services.faceRecognition ||
                lastRequestedService == services.objectRecognition)
            {
                if (imageUrl !== undefined)
                {
                    URL.revokeObjectURL(imageUrl);
                }

                imageUrl = URL.createObjectURL(contentCache);
                retContent = imageUrl;
            }

            var event = new CustomEvent('CognitiveServiceResponse', {'detail': {
                response: jsonData,
                service: lastRequestedService,
                content: retContent
            }});
    
            window.dispatchEvent(event);
        }
        else
        {
            console.log("ERROR: Failed to process successful response from cognitive service. Status = " + this.status);
        }
    }
    
    function ProcessError(event)
    {
        requestInProgress = false;
        console.log("ERROR: Cognitive services responded with error. Status = " + event.status);
    }
    
    function ProcessAbort(event)
    {
        requestInProgress = false;
        console.log("ERROR: Cognitive services request aborted. Status = " + event.status);
    }
}
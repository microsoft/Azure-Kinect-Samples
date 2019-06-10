// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

var FaceRecognitionVisualizer = function(surfaceElementId, contentElementId)
{
    "use strict";

    var surfaceId = surfaceElementId;
    var contentId = contentElementId;

    this.Visualize = function(data)
    {
    	// Remove group element with previous frames
	    d3.selectAll("#faceDataGroup").remove();

        var surfaceElement = document.getElementById(surfaceId);
        var imageElement = document.getElementById(contentId);


        var horizontalScaleFactor = imageElement.clientWidth / imageElement.width;
        var verticalScaleFactor = imageElement.clientHeight / imageElement.height;

        const LINE_HEIGHT = 14;
        const PADDING = 1;
        const TEXT_WIDTH = 170;

        // Draw new frames
        var userDataGroup = d3.select("#" + surfaceId)
            .append("g")
                .attr("id", "faceDataGroup")
                .selectAll("g")
                    .data(data)
                    .enter().append("g")

        // Draw frame around face
        userDataGroup.append("rect")				
            .attr("class", "objectRect")
            .attr("x", function(d) { return d.faceRectangle.left * horizontalScaleFactor; })
            .attr("y", function(d) { return d.faceRectangle.top * verticalScaleFactor; })
            .attr("height", function(d) { return d.faceRectangle.height * verticalScaleFactor; })
            .attr("width", function(d) { return d.faceRectangle.width * horizontalScaleFactor; })

        // Draw text background
        userDataGroup.append("rect")				
            .attr("class", "faceTextRect")
            .attr("x", function(d) { return d.faceRectangle.left * horizontalScaleFactor; })
            .attr("y", function(d) 
            { 
                return (d.faceRectangle.top * verticalScaleFactor) + (d.faceRectangle.height * verticalScaleFactor); 
            })
            .attr("height", 17)
            .attr("width", TEXT_WIDTH)

        var txt = userDataGroup.append("text")

        txt
		.append("tspan")
			.attr("x", function(d) { return (d.faceRectangle.left * horizontalScaleFactor) + PADDING; })
			.attr("y", function(d) 
			{ 
				var retVal = (d.faceRectangle.top * verticalScaleFactor) + (d.faceRectangle.height * verticalScaleFactor); 
				retVal += PADDING;
				retVal += LINE_HEIGHT;
				return retVal; 
			})
			.text(function(d) { return "Emotion: " + GetEmotions(d.faceAttributes); })
    }

    // Face recognition response processing helpers
    function GetEmotions(jsonFile)
    {
        var retVal = "";
        var maxValue = 0;
    
        if (jsonFile.emotion.anger > maxValue)
        {
            maxValue = jsonFile.emotion.anger;
            retVal = "anger (" + Round(jsonFile.emotion.anger, 2) + ")";
            Math.roun
        }
        
        if (jsonFile.emotion.contempt > maxValue)
        {
            maxValue = jsonFile.emotion.contempt;
            retVal = "contempt (" + Round(jsonFile.emotion.contempt, 2) + ")";
        }
    
        if (jsonFile.emotion.disgust > maxValue)
        {
            maxValue = jsonFile.emotion.disgust;
            retVal = "disgust (" + Round(jsonFile.emotion.disgust, 2) + ")";
        }
    
        if (jsonFile.emotion.fear > maxValue)
        {
            maxValue = jsonFile.emotion.fear;
            retVal = "fear (" + Round(jsonFile.emotion.fear, 2) + ")";
        }
    
        if (jsonFile.emotion.happiness > maxValue)
        {
            maxValue = jsonFile.emotion.happiness;
            retVal = "happiness (" + Round(jsonFile.emotion.happiness, 2) + ")";
        }
        
        if (jsonFile.emotion.neutral > maxValue)
        {
            maxValue = jsonFile.emotion.neutral;
            retVal = "neutral (" + Round(jsonFile.emotion.neutral, 2) + ")";
        }
    
        if (jsonFile.emotion.sadness > maxValue)
        {
            maxValue = jsonFile.emotion.sadness;
            retVal = "sadness (" + Round(jsonFile.emotion.sadness, 2) + ")";
        }
    
        if (jsonFile.emotion.surprise > maxValue)
        {
            maxValue = jsonFile.emotion.surprise;
            retVal = "surprise (" + Round(jsonFile.emotion.surprise, 2) + ")";
        }
    
        return retVal;
    }

    function Round(value, decimals) 
    {
        return Number(Math.round(value+'e'+decimals)+'e-'+decimals);
    }
}

// Body Tracking Visualization
var AudioVisualizer = function(surfaceCanvasId, samplingRate, numOfChannels)
{
    "use strict";

    var surfaceId = surfaceCanvasId;
    var surfaceContext = surfaceId.getContext('2d');
    var samplingRate = samplingRate;
    var numOfChannels = numOfChannels;

    const AUDIO_CONTENT_SIZE = 5;
    const VISUALIZATION_INTERVAL = 100;                  // We will try to draw frames for number of milliseconds showed here.
    var audioContent = new Array(AUDIO_CONTENT_SIZE);   
    var readingPosition = 0;
    var pushingPosition = 0;
    
    // Drawing members
    var timerInterval;                                          // Reference to interval we set for drawing wave length 
    var readingPointer = 0;                                     // Sample reading pointer.

    this.PushAudioData = function(audioData)
    {
        audioContent[pushingPosition++] = audioData;

        pushingPosition %= AUDIO_CONTENT_SIZE;
    }

    this.Start = function()
    {
        timerInterval = window.setInterval(VisualizeAudio, VISUALIZATION_INTERVAL);
        console.log(surfaceContext);
    }

    this.Stop = function()
    {
        window.clearInterval(timerInterval);
    }

    // Function will check if there are enough samples to read from audioContent.
    function IsThereEnoughSamples(numOfSamples)
    {
        var retVal = false;
        var availableSamples = 0;
        var currentPos = readingPosition;

        if (currentPos != pushingPosition)
        {
            availableSamples = (audioContent[currentPos].length - readingPointer) / numOfChannels;
            if (availableSamples < numOfSamples)
            {
                ++currentPos;
                currentPos %= AUDIO_CONTENT_SIZE;
                while (currentPos != pushingPosition)
                {
                    availableSamples += audioContent[currentPos++].length / numOfChannels;
                    currentPos %= AUDIO_CONTENT_SIZE;
                    if (availableSamples >= numOfSamples)
                    {
                        retVal = true;
                        break;
                    }
                }
            }
            else
            {
                retVal = true;
            }
        }

        return retVal;
    }

    function VisualizeAudio()
    {
        if (readingPosition != pushingPosition)
        {
            var frameInterval = samplingRate / VISUALIZATION_INTERVAL;

            while(IsThereEnoughSamples(frameInterval) == true)
            {
                surfaceContext.clearRect(0, 0, surfaceId.width, surfaceId.height);
                var stepX = surfaceId.width / frameInterval;
                var stepY = surfaceId.height / (numOfChannels + 1);
                surfaceContext.beginPath();
                
                var tempReadingPointer = readingPointer;
                var teampReadingPosition = readingPosition;

                for (var channelNo = 0; channelNo < numOfChannels; channelNo++)
                {
                    readingPointer = tempReadingPointer;
                    readingPosition = teampReadingPosition;
                    for (var i = 0; i < frameInterval; i++)
                    {
                        const x = i * stepX;
                        const y = (channelNo * stepY) + (0.5 + (audioContent[readingPosition][readingPointer + channelNo] / 2)) * 200;
                        
                        readingPointer += numOfChannels;
                        if (readingPointer > audioContent[readingPosition].length)
                        {
                            readingPointer = 0;
                            ++readingPosition;
                            readingPosition %= AUDIO_CONTENT_SIZE;
                        }

                        if (i == 0)
                        {
                            surfaceContext.moveTo(x, y);
                        }
                        else
                        {
                            surfaceContext.lineTo(x, y);
                        }
                    }
                }
                surfaceContext.stroke();
            }
        }
    }
}


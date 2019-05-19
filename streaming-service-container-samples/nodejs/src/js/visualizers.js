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


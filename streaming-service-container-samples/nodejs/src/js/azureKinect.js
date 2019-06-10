// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

var AzureKinect = function()
{
	"use strict";

    // Private variables
    const METADATA_SIZE = 14;

    var fileReader = new FileReader();      // Helper to convert blob to Uint8Array
    var lastMetadata;                       // Last received metadata either received or set.
    var imageUrl;                           // Cached image url object
    var header;                             // Uint8Array converted metadata header

    // Public members
    var streamFormat = this.streamFormat = 
    {
        unknown:            0,
        minImageFormat:     0,
        mjpg:               1,
        nv12:               2,
        yuv2:               3,
        bgra32:             4,
        depth:              5,
        infrared:           6,
        maxImageFormat:     6,

        minAudioFormat:     50,
        audio70:            50,
        maxAudioFormat:     50,
    }

    // Sets default format. When request to streaming service doesn't specifies to use
    // metadata, this will tell to object what frame format to expect.
    this.SetFormat = function(format)
    {
        lastMetadata = 
        {
            format: format,
            timestamp: 0,
            image: undefined
        }
    }

    function IsAudio(format)
    {
        return (format >= streamFormat.minAudioFormat && format <= streamFormat.maxAudioFormat) ? true : false;
    };

    function IsImage(format)
    {
        return (format >= streamFormat.minImageFormat && format <= streamFormat.maxImageFormat) ? true : false;
    };

    this.ProcessWebsocketMessage = function(event)
    {
        var retVal;

        var metadata = ProcessMetadata(event.data);
        if (metadata != undefined)
        {
            lastMetadata = metadata;
        }

        if (IsImage(lastMetadata.format) == true)
        {
            var image = CreateImage(event.data);
            if (image !== undefined && lastMetadata !== undefined)
            {
                retVal = 
                {
                    format: lastMetadata.format,
                    timestamp: lastMetadata.timestamp,
                    image: image
                }
            }
        }
        else if (IsAudio(lastMetadata.format) == true)
        {
            retVal = 
            {
                format: lastMetadata.format,
                timestamp: lastMetadata.timestamp,
                content: new Float32Array(event.data)
            }
        }        

        return retVal;
    }

    // Private members
    function ProcessMetadata(data)
    {
        var retVal;
        if (data !== undefined && data.size == METADATA_SIZE)
        {
            fileReader.readAsArrayBuffer(data);

            if (header !== undefined &&
                header[0] == 'M'.charCodeAt(0) &&
                header[1] == 'E'.charCodeAt(0) &&
                header[2] == 'T'.charCodeAt(0) &&
                header[3] == 'A'.charCodeAt(0))
            {
                var format = header[4];

                // Get timestamp from header
                var timestamp = 0;
                var pos = 6;
                var i = 0;
                for (i = 0; i < 8; i++)
                {
                    timestamp += header[i + pos] * Math.pow(256, i);
                }

                retVal = 
                {
                    format: format,
                    timestamp: timestamp
                }
            }
        }

        return retVal;
    }

    // Function will create image from data, if data contains image and return it.
    function CreateImage(data)
    {
        var retVal;

        if (data !== undefined && 
            data.size != METADATA_SIZE &&
            (lastMetadata.format == streamFormat.mjpg ||
            lastMetadata.format == streamFormat.nv12 ||
            lastMetadata.format == streamFormat.yuv2 ||
            lastMetadata.format == streamFormat.bgra32 ||
            lastMetadata.format == streamFormat.depth ||
            lastMetadata.format == streamFormat.infrared)
            )
        {
            if (imageUrl != undefined)
            {
                URL.revokeObjectURL(imageUrl);
            }
            imageUrl = URL.createObjectURL(data);
            retVal = imageUrl;
        }

        return retVal;
    }

    fileReader.onload = function(event)
    {
        header = new Uint8Array(event.target.result);
    }

}
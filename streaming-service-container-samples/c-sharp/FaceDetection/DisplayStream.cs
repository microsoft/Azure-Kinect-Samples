// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Json;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Windows.Forms;

namespace FaceDetection
{
    public partial class DisplayStream : Form
    {
        // Both face detection and training are asynchronous operations. Program is designed to run only one asynchronous
        // operation of same type at time.
        static private bool     faceDetectionInProgress = false;        // Status variable to ensure there is only one request active at time.

        // Delegate that is called when new color frame arrives for processing in context of this form thread.
        public delegate void        DisplayImage(byte[] rawImageData);
        public DisplayImage         displayImage;

        // Face detection members
        private FaceDetection       faceDetection = new FaceDetection();
        private JsonValue           faceData = null;

        // UI adjustment members
        bool                        screenSizeAdjusted = false;

        // Members set by outside threads to notify form about some important changes. 
        public UInt16               distanceFromCamera = 0;
        private const UInt16        FACE_RECOGNITION_THRESHOLD = 400;
        public bool                 networkIsUp = true;

        public DisplayStream()
        {
            InitializeComponent();

            displayImage = new DisplayImage(DisplayImageMethod);
        }

        /// <summary>
        /// Method will display image from Azure Kinect, as well as reslut from face recognition service in form of
        /// rectangle and text.
        /// </summary>
        /// <param name="rawImageData">Buffer with raw image data from Azure Kinect.</param>
        public void DisplayImageMethod(byte[] rawImageData)
        {
            var inStream = new MemoryStream(rawImageData);
            var image = Image.FromStream(inStream);

            if (distanceFromCamera <= FACE_RECOGNITION_THRESHOLD)
            {
                statusDistance.ForeColor = Color.FromArgb(0, 120, 212);
            }
            else
            {
                faceData = null;
                statusDistance.ForeColor = Color.DarkRed;
            }
            statusDistance.Text = $"{distanceFromCamera} mm";

            if (screenSizeAdjusted == false)
            {
                this.Height = image.Height + mainStatusStrip.Height;
                this.Width = displayBox.Width = image.Width;

                screenSizeAdjusted = true;
            }

            // Draw rectangle and text if there is face recognition result from cognitive services.
            if (faceData != null)
            {
                if (faceData.JsonType == JsonType.Object && faceData.ContainsKey("error") == true)
                {
                }
                else if(faceData.ContainsKey("faceData") == true && faceData["faceData"].JsonType == JsonType.Array)
                {
                    Rectangle faceFrame = new Rectangle();

                    foreach (JsonValue face in faceData["faceData"])
                    {
                        JsonValue faceRectangle = face["faceRectangle"];
                        int left = faceRectangle["left"];
                        int top = faceRectangle["top"];
                        int width = faceRectangle["width"];
                        int height = faceRectangle["height"];

                        Graphics imageGraphics = Graphics.FromImage(image);
                        faceFrame = new Rectangle(left, top, width, height);
                        Pen pen = new Pen(Color.FromArgb(0, 120, 212), 3);
                        SolidBrush brush = new SolidBrush(Color.FromArgb(0, 120, 212));
                        Rectangle textRectangle = new Rectangle(left, top + height, width < 120 ? 120 : width, 25);
                        Font textFont = new Font("Calibri", 12, FontStyle.Bold);
                        SolidBrush textBrush = new SolidBrush(Color.Black);
                        PointF textPoint = new PointF(left + 10, top + height + 4);

                        imageGraphics.DrawRectangle(pen, faceFrame);
                    }
                }
                else
                {
                }
            }

            // Show Color image in window
            displayBox.Image = image;

            if (faceDetectionInProgress == false && distanceFromCamera <= FACE_RECOGNITION_THRESHOLD)
            {
                statusServiceUrl.Text = CognitiveServices.uriBase;
                faceDetectionInProgress = true;
                faceDetection.DetectFace(rawImageData, this);
            }
        }

        /// <summary>
        /// Face detection asynchronous process will report result upon completion here.
        /// </summary>
        /// <param name="response"></param>
        internal void DetectFaceResult(JsonValue response)
        {
            faceData = response;
            faceDetectionInProgress = false;
        }
    }
}

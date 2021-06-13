using Microsoft.Azure.CognitiveServices.Vision.Face.Models;
using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;

namespace AzureKinectFaceApi
{
    // Represents a rectangle to draw
    class StatusRectangle
    {
        public StatusRectangle(Rect rect, string text)
        {
            this.PixelRectangle = rect;
            this.Text = text;
        }

        public StatusRectangle(FaceRectangle rect, string text)
        {
            this.PixelRectangle = new Rect(rect.Left, rect.Top, rect.Width, rect.Height);
            this.Text = text;
        }

        public Rect PixelRectangle { get; set; }

        public string Text { get; set; }
    }
}

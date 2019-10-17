using Microsoft.Azure.Kinect.Sensor;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace AzureKinectFaceApi
{
    class KinectHelper
    {
        public static BitmapSource GetBitmapSource(Microsoft.Azure.Kinect.Sensor.Capture capture)
        {
            BitmapSource bitmapSource = BitmapSource.Create(
                            capture.Color.WidthPixels,
                            capture.Color.HeightPixels,
                            300, 300,
                            PixelFormats.Bgra32,
                            null,
                            capture.Color.Memory.ToArray(),
                            capture.Color.StrideBytes);

            return bitmapSource;
        }

        static readonly int MaxRange = 1200;
        static readonly int MinRange = 1000;
        static readonly int BackgroundRange = 4000;

        public static Presence GetPresenceForDepth(int depth)
        {
            if (depth > BackgroundRange || depth == 0)
                return Presence.NotInView;
            if (depth < MinRange)
                return Presence.TooClose;
            if (depth > MaxRange)
                return Presence.TooFar;

            return Presence.JustRight;
        }
    }
}

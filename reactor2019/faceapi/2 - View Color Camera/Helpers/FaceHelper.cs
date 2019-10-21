#if true
using Microsoft.Azure.CognitiveServices.Vision.Face;
using Microsoft.Azure.CognitiveServices.Vision.Face.Models;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;

namespace AzureKinectFaceApi
{
    static class FaceHelper
    {
        public static IFaceClient Authenticate()
        {
            string SUBSCRIPTION_KEY = Environment.GetEnvironmentVariable("FACE_SUBSCRIPTION_KEY");
            string ENDPOINT = Environment.GetEnvironmentVariable("FACE_ENDPOINT");

            if (string.IsNullOrEmpty(SUBSCRIPTION_KEY))
            {
                throw new Exception("FACE_SUBSCRIPTION_KEY environment variable must be set");
            }

            if (string.IsNullOrEmpty(ENDPOINT))
            {
                throw new Exception("FACE_ENDPOINT environment variable must be set");
            }

            string suffix = "face/v1.0";
            if (ENDPOINT.EndsWith(suffix))
            {
                ENDPOINT = ENDPOINT.Substring(0, ENDPOINT.Length - suffix.Length);
            }

            return Authenticate(ENDPOINT, SUBSCRIPTION_KEY);
        }

        public static IFaceClient Authenticate(string endpoint, string key)
        {
            return new FaceClient(new ApiKeyServiceClientCredentials(key)) { Endpoint = endpoint };
        }

        public static async Task<IList<DetectedFace>> GetFaces(IFaceClient client, Stream image)
        {
            CancellationTokenSource cancellation = new CancellationTokenSource(TimeSpan.FromSeconds(30));

            return await client.Face.DetectWithStreamAsync(
                image,
                returnFaceAttributes: new[] { FaceAttributeType.Emotion, FaceAttributeType.Smile },
                returnFaceLandmarks: true,
                cancellationToken: cancellation.Token
                );
        }

        public static string EmotionToString(Emotion emotion)
        {
            string emotionString = "";

            if (emotion.Anger > 0.2) emotionString += "😠";
            if (emotion.Contempt > 0.2) emotionString += "😒";
            if (emotion.Disgust > 0.2) emotionString += "🤢";
            if (emotion.Fear > 0.2) emotionString += "😨";
            if (emotion.Happiness > 0.2) emotionString += "😊";
            if (emotion.Neutral > 0.2) emotionString += "😐";
            if (emotion.Sadness > 0.2) emotionString += "😢";
            if (emotion.Surprise > 0.2) emotionString += "😲";
            
            return emotionString;
        }


    }
}
#endif
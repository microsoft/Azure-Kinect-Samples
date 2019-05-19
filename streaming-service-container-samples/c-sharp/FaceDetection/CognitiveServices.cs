using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Json;
using System.Net.Http;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using Microsoft.Azure.CognitiveServices.Vision.Face;
using Microsoft.Azure.CognitiveServices.Vision.Face.Models;


namespace FaceDetection
{
    internal enum CognitiveServiceUrlType
    {
        cloud,
        container
    };

    internal enum CognitiveResult
    {
        resultOK = 0,
        resultFailed = -1
    }

    internal class CognitiveServices
    {
        protected const string subscriptionKey = "<Service Subscription Key>";          // Subscription key obtained from Azure Portal when subscribing to Cognitive Services
        protected const string containerUriBase = "<Local Container Endpoint>";         // Local container URL to send Cognitive Service requests (Face)
        protected const string azureUriBase = "<Azure Endpoint>";                       // URL of Cognitive Serivces, Azure Cloud. URL is obtained when 

        public static string uriBase { get; protected set; }

        internal CognitiveServices()
        {
            // [CHANGE] Decide here if you want to use container or cloud version of Cognitive Services.
            SetCognitiveServiceUrl(CognitiveServiceUrlType.cloud);
        }

        /// <summary>
        /// Gets URL type (cloud or container) based on current network conditions
        /// </summary>
        /// <returns></returns>
        internal CognitiveServiceUrlType GetCognitiveServiceUrlType()
        {
            CognitiveServiceUrlType retVal = CognitiveServiceUrlType.cloud;

            if (uriBase == containerUriBase)
            {
                retVal = CognitiveServiceUrlType.container;
            }

            return retVal;
        }

        /// <summary>
        /// Sets which service to use to send Face requests (cloud or container)
        /// </summary>
        /// <param name="type"></param>
        internal void SetCognitiveServiceUrl(CognitiveServiceUrlType type)
        {
            if (type == CognitiveServiceUrlType.cloud)
            {
                uriBase = azureUriBase;
            }
            else if (type == CognitiveServiceUrlType.container)
            {
                uriBase = containerUriBase;
            }
            else
            {
                Console.WriteLine("Unknown Cognitive Service URL type.");
            }
        }
    }

    internal class FaceDetection : CognitiveServices
    {
        // Onboarding fields
        private const string peopleGroupName = "akdksample";
        private const string recognitionModel = "recognition_01";

        // Face detection API client instance we are using to talking to Cognitive Services
        // (NuGet: Microsoft.Azure.CognitiveServices.Vision.Face)
        private static FaceClient faceServiceClient = null;

        internal FaceDetection()
        {
            // Initializing clients to communicate with remote Cognitive Services
            HttpClient client = new HttpClient();
            ApiKeyServiceClientCredentials credentials = new ApiKeyServiceClientCredentials(subscriptionKey);
            faceServiceClient = new FaceClient(credentials, client, true);
        }

        ~FaceDetection()
        {
            if (faceServiceClient != null)
            {
                faceServiceClient.Dispose();
                faceServiceClient = null;
            }
        }

        internal void DetectFace(byte[] image, DisplayStream dialog)
        {
            MakeFaceDetectRequest(image, dialog);
        }

        /// <summary>
        /// Method sends requests to Cognitive Service first to detect faces on image.
        /// </summary>
        /// <param name="image">MJPG raw image to recognize faces</param>
        /// <param name="dialog">Main dialog instance to report progress</param>
        internal static async void MakeFaceDetectRequest(byte[] image, DisplayStream dialog)
        {
            JsonValue responseContent = null;
            JsonValue finalResponse = new JsonObject();

            try
            {
                string contentString;

                // Preparing and sending Cognitive Service request to recognize faces. Response will be used to draw rectangles around detected faces.
                faceServiceClient.Endpoint = uriBase;
                MemoryStream stream = new MemoryStream(image);
                List<FaceAttributeType> attributes = new List<FaceAttributeType> { FaceAttributeType.Age, FaceAttributeType.Gender, FaceAttributeType.Emotion };
                var faces = await faceServiceClient.Face.DetectWithStreamWithHttpMessagesAsync(stream, true, false, attributes, recognitionModel);

                // Process response to get JSON data from response. JSON data contain face rectangle
                contentString = await faces.Response.Content.ReadAsStringAsync();
                responseContent = JsonObject.Parse(contentString);
                // Attach all face data to final response sent back to main dialog to display results.
                finalResponse["faceData"] = responseContent;
            }
            catch (Exception exception)
            {
                Console.WriteLine(exception.ToString());
            }

            dialog.DetectFaceResult(finalResponse);
        }
    }

    class DepthCalculation
    {
        public UInt16 CalculateDepth(byte[] depthImageData)
        {
            UInt16 retVal = 0;

            UInt16[] depth = new UInt16[depthImageData.Length / 2];

            for (int i = 0; i < depth.Length; i++)
            {
                // b16g (16-bit Grayscale, Big-endian)
                depth[i] = (UInt16)((depthImageData[i * 2] & 0xFF) | ((depthImageData[i * 2 + 1] & 0xFF) << 8));
            }

            Array.Sort<UInt16>(depth);
            int index = 0;

            // Get the first index that distance > 0
            for (int i = 0; i < depth.Length; i++)
            {
                if (depth[i] > 0)
                {
                    index = i;
                    break;
                }
            }

            // Get the index of 5 percentile (exclude 0)
            index += (depth.Length - index) / 100 * 5;
            retVal = depth[index];

            return retVal;
        }
    }

    internal class OnboardTeam
    {
        internal List<TeamMember> team = new List<TeamMember>();

        internal struct TeamMember
        {
            internal string     name;
            internal byte[]     image;
            internal Rectangle  imageRect;
            internal bool       fromFile;
        }

        public void LoadTeam()
        {
            Regex rx = new Regex(@"(.+\\)(.+)\.(.+)$", RegexOptions.Compiled | RegexOptions.IgnoreCase);
            string imagesPath = Directory.GetCurrentDirectory() + @"\Images";

            string[] imageFiles = Directory.GetFiles(imagesPath);
            foreach (string image in imageFiles)
            {
                MatchCollection matches = rx.Matches(image);
                foreach (Match match in matches)
                {
                    GroupCollection groups = match.Groups;
                    if (groups.Count > 2)
                    {
                        TeamMember teamMember = new TeamMember();
                        teamMember.name = groups[groups.Count - 2].Value;

                        using (FileStream fileStream = new FileStream(image, FileMode.Open, FileAccess.Read))
                        {
                            BinaryReader binaryReader = new BinaryReader(fileStream);
                            teamMember.image =  binaryReader.ReadBytes((int)fileStream.Length);
                        }

                        teamMember.fromFile = true;
                        team.Add(teamMember);
                    }
                }
            }
        }

        public void ClearTeam()
        {
            // Clear only team members from file, so we can preserve onbarded team members from application
            List<TeamMember> tempTeam = new List<TeamMember>();

            foreach(TeamMember teamMember in team)
            {
                if (teamMember.fromFile == false)
                {
                    tempTeam.Add(teamMember);
                }
            }

            team.Clear();
            team = tempTeam;
        }
    }
}
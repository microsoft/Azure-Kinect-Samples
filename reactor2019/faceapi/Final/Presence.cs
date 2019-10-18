using System;
using System.Collections.Generic;
using System.Text;

namespace AzureKinectFaceApi
{
    enum Presence
    {
        TooFar,
        TooClose,
        NotInView,
        JustRight
    }

    static class PresenceMethods
    {
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

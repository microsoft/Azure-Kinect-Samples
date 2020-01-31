using System.Collections.Generic;

public class Smoother
{
    // Max allowed size of history list.
    private int maxSize = 100;

    // In case smoother has enough frames in history to perform smooth action.
    private bool hasEnoughForSmoothing = false;

    // Number of the latest frames used to smooth current position; default 5.
    public int NumberSmoothingFrames { get; set; } = 5;

    // Holds received data about moves.
    private List<SkeletonPosition> rawData = new List<SkeletonPosition>();

    // Holds received which are smoothened a little bit.
    private List<SkeletonPosition> smoothenedData = new List<SkeletonPosition>();

    // Process skeleton position and sends back smoothened or raw based on passed parameter.
    public SkeletonPosition ReceiveNewSensorData(SkeletonPosition newData, bool smoothing)
    {
        // In case list is too big.
        if(rawData.Count > maxSize)
        {
            Resize();
        }

        // Add new frame data to raw data used for smoothing.
        rawData.Add(newData);

        // In case value for smoothing is invalid just return original raw frame.
        if (NumberSmoothingFrames <= 1)
        {
            return rawData[rawData.Count - 1];
        }

        // Mark that smoother has enough frames for smoothing.
        if (rawData.Count > NumberSmoothingFrames)
        {
            hasEnoughForSmoothing = true;
        }

        if (smoothenedData.Count == 0)
        {
            smoothenedData.Add(newData);
        }
        else
        {
            SkeletonPosition temp = smoothenedData[smoothenedData.Count - 1] + newData;
            if(hasEnoughForSmoothing)
            {
                temp = temp - rawData[rawData.Count - NumberSmoothingFrames];
            }
            smoothenedData.Add(temp);
        }

        // Smoothened timestamp has the same timestamp as the latest received.
        smoothenedData[smoothenedData.Count - 1].Timestamp = rawData[rawData.Count - 1].Timestamp;

        return smoothing && hasEnoughForSmoothing
            ? smoothenedData[smoothenedData.Count - 1] / (float)NumberSmoothingFrames
            : rawData[rawData.Count - 1];
    }

    // Deletes old position data from list which do not have more impact on smoothing algorithm.
    public void Resize()
    {
        if (rawData.Count > NumberSmoothingFrames)
        {
            rawData.RemoveRange(0, rawData.Count - NumberSmoothingFrames);
        }
        if (smoothenedData.Count > NumberSmoothingFrames)
        {
            smoothenedData.RemoveRange(0, smoothenedData.Count - NumberSmoothingFrames);
        }
    }
}

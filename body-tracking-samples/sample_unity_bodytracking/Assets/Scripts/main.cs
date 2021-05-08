using System.Collections.Generic;
using UnityEngine;
using Microsoft.Azure.Kinect.BodyTracking;
using System.Threading;

public class main : MonoBehaviour
{
    // Handler for SkeletalTracking thread.
    public GameObject m_tracker;
    private BackgroundDataProvider m_backgroundDataProvider;
    public BackgroundData m_lastFrameData = new BackgroundData();
    private CancellationTokenSource _cancellationTokenSource;
    private CancellationToken _token;

    void Start()
    {
        EditorClose();
        _cancellationTokenSource = new CancellationTokenSource();
        _token = _cancellationTokenSource.Token;
        SkeletalTrackingProvider m_skeletalTrackingProvider = new SkeletalTrackingProvider();

        //tracker ids needed for when there are two trackers
        const int TRACKER_ID = 0;
        m_skeletalTrackingProvider.StartClientThread(TRACKER_ID, _token);
        m_backgroundDataProvider = m_skeletalTrackingProvider;
    }

    private void onQuit()
    {
        // do something
        if(_cancellationTokenSource != null)
        {
            _cancellationTokenSource.Cancel();
        }
        Debug.Log("goodbye world");
    }

    private void EditorClose()
    {
        Debug.Log("adding close down callback");
        UnityEditor.EditorApplication.quitting += onQuit;
    }

    void Update()
    {
        if (m_backgroundDataProvider.IsRunning)
        {
            if (m_backgroundDataProvider.GetCurrentFrameData(ref m_lastFrameData))
            {
                if (m_lastFrameData.NumOfBodies != 0)
                {
                    m_tracker.GetComponent<TrackerHandler>().updateTracker(m_lastFrameData);
                }
            }
        }
    }

    void OnApplicationQuit()
    {
        // Stop background threads.
        if (m_backgroundDataProvider != null)
        {
            _cancellationTokenSource.Cancel();
            _cancellationTokenSource = null;
            m_backgroundDataProvider.StopClientThread();
        }
    }
}

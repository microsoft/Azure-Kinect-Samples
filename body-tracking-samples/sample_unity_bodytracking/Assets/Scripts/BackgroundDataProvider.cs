using System;
using System.Threading.Tasks;

public abstract class BackgroundDataProvider
{
    protected volatile bool m_runBackgroundThread;
    private BackgroundData m_frameBackgroundData = new BackgroundData();
    private bool m_latest = false;
    object m_lockObj = new object();
    public bool IsRunning { get; set; } = false;

    public void StartClientThread(int id)
    {
        m_runBackgroundThread = true;
        Task.Run(() => RunBackgroundThreadAsync(id));
    }

    protected abstract void RunBackgroundThreadAsync(int id);

    public void StopClientThread()
    {
        UnityEngine.Debug.Log("Stopping BackgroundDataProvider thread.");
        m_runBackgroundThread = false;
    }

    public void SetCurrentFrameData(ref BackgroundData currentFrameData)
    {
        lock (m_lockObj)
        {
            var temp = currentFrameData;
            currentFrameData = m_frameBackgroundData;
            m_frameBackgroundData = temp;
            m_latest = true;
        }
    }

    public bool GetCurrentFrameData(ref BackgroundData dataBuffer)
    {
        lock (m_lockObj)
        {
            var temp = dataBuffer;
            dataBuffer = m_frameBackgroundData;
            m_frameBackgroundData = temp;
            bool result = m_latest;
            m_latest = false;
            return result;
        }
    }
}

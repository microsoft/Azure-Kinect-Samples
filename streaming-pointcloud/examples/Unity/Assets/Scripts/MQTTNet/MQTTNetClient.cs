using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;
using MQTTnet.Client;
using MQTTnet;
using MQTTnet.Diagnostics;
using System.Threading.Tasks;
using MQTTnet.Client.Options;
using System;
using System.Threading;
using System.Text;
using MQTTnet.Client.Publishing;
using MQTTnet.Client.Receiving;
using System.Collections.Concurrent;

public class MQTTNetClient : MonoBehaviour
{
    public string Broker;
    public int Port;
    public string Topic;
    private IMqttClient _client;
    public PointCloudLoader pointLoader;


    public class MqttMessage
    {
        public string path;
        public byte[] payload;
    }

    //public ConcurrentQueue<MqttMessage> queue = new ConcurrentQueue<MqttMessage>();

    public BlockingCollection<MqttMessage> queue = new BlockingCollection<MqttMessage>(new ConcurrentQueue<MqttMessage>());

    public bool IsConnected()
    {
        if (_client == null) return false;

        return _client.IsConnected;
    }

    public async void Publish(string topic, string message)
    {
        try
        {
            await PublishAsync(topic, message);
        }
        catch (Exception e)
        {
            Debug.LogError("[MQTTNetClient] publish error " + e.Message);
        }
    }

    private async Task<MQTTnet.Client.Publishing.MqttClientPublishResult> PublishAsync(string topic, string message)
    {
        if (_client != null && _client.IsConnected)
        {
            
            var mqttmsg = new MqttApplicationMessageBuilder()
            .WithTopic(topic)
            .WithPayload(message)
            .Build();

            MqttClientPublishResult result = await _client.PublishAsync(mqttmsg);
            return result;
        }else
        {
            return null;
        }
    }

    private Thread receiveEventThread;
    private bool isRunning;

    public async void Connect()
    {
        await StartClientAsync().ConfigureAwait(false);
    }

    private void ReceiveEventThread()
    {
        Connect();
    }

    private void Start()
    {
        // start thread for raising received message event from broker
        this.receiveEventThread = new Thread(this.ReceiveEventThread);
        this.receiveEventThread.Start();

    }

    bool aborting;
    private void OnDisable()
    {
        Debug.Log("disconnect");
        aborting = true;
        this.receiveEventThread.Abort();
        _client.DisconnectAsync().ConfigureAwait(false);
        _client.Dispose();
    }

    private void Update()
    {
        
        bool latest = false;
        
        MqttMessage message = new MqttMessage();
        while(queue.TryTake(out message, TimeSpan.FromMilliseconds(1)))
        {
            latest = (queue.Count == 0);
            if (latest)
            {
                pointLoader.LoadPoints(message.payload, message.payload.Length);
            }            
        }       
    }

    public async Task<IMqttClient> StartClientAsync()
    {
        var options = new MqttClientOptionsBuilder()
        .WithTcpServer(Broker, Port)        
        .WithCommunicationTimeout(System.TimeSpan.FromSeconds(5))
        .Build();
        
        ((MqttClientTcpOptions)(options.ChannelOptions)).NoDelay = true;
        ((MqttClientTcpOptions)(options.ChannelOptions)).BufferSize = 4096 * 1000;

        var adapterFactory = new MQTTnet.Implementations.MQTTNetCustomAdapterFactory();

        var factory = new MqttFactory();
        _client = factory.CreateMqttClient(adapterFactory);

        /*
        _client.UseDisconnectedHandler(async e =>
        {
            if (aborting) return;

            Debug.Log("[MQTTNetClient] Disconnected, reconnect");
            await Task.Delay(TimeSpan.FromSeconds(.5));

            try
            {
                await _client.ConnectAsync(options, CancellationToken.None);
            }
            catch
            {
                Debug.Log("[MQTTNetClient] reconnect failed");
            }
        });
        */

        _client.UseApplicationMessageReceivedHandler( e =>
        {
            if (aborting) return;

            if (e.ApplicationMessage.Topic == Topic)
            {
                //Debug.Log($"+ Topic = {e.ApplicationMessage.Topic} {queue.Count}");
                MqttMessage message = new MqttMessage();
                message.path = e.ApplicationMessage.Topic;
                message.payload = new byte[e.ApplicationMessage.Payload.Length];
                Array.Copy(e.ApplicationMessage.Payload, 0, message.payload, 0, e.ApplicationMessage.Payload.Length);

                queue.Add(message);

                //pointLoader.LoadPoints(e.ApplicationMessage.Payload);
            }
        });
        
            _client.UseConnectedHandler(async e =>
        {
            if (aborting) return;

            Debug.Log("[MQTTNetClient] connected");

            await _client.SubscribeAsync(new TopicFilterBuilder().WithTopic( Topic).WithAtMostOnceQoS().Build());

            Debug.Log("[MQTTNetClient] subscribed");
        });


        await _client.ConnectAsync(options, CancellationToken.None).ConfigureAwait(false);
        return _client;
    }
}

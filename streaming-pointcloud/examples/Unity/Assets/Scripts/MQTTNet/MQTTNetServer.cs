using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using MQTTnet.Server;
using MQTTnet;
using MQTTnet.Diagnostics;
using System.Threading.Tasks;

public class MQTTNetServer : MonoBehaviour
{
    public int Port = 1883;
    IMqttServer server;

    private readonly IMqttNetLogger _serverLogger = new MqttNetLogger("server");

    // Start is called before the first frame update
    void Start()
    {

        var _ = StartServerAsync();
    }

    public async Task<IMqttServer> StartServerAsync()
    {
        MqttFactory mqttFactory = new MqttFactory();

        MqttServerOptionsBuilder options = new MqttServerOptionsBuilder()
            .WithDefaultEndpointPort(Port);

        server = mqttFactory.CreateMqttServer();
        server.StartedHandler = new MqttServerStartedHandlerDelegate(e => {
            Debug.Log($"[MQTTNetServer] server started " + Port);
        });

        /*
        server.ClientSubscribedTopicHandler = new MqttServerClientSubscribedHandlerDelegate(e =>
        {
            Debug.Log($"{e.TopicFilter.Topic} {e.ClientId}");
        });
        */

        await server.StartAsync(options.Build());
        return server;
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}

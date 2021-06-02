using System;
using MQTTnet.Adapter;
using MQTTnet.Client.Options;
using MQTTnet.Diagnostics;
using MQTTnet.Formatter;

namespace MQTTnet.Implementations
{
    public class MQTTNetCustomAdapterFactory : IMqttClientAdapterFactory
    {
        public IMqttChannelAdapter CreateClientAdapter(IMqttClientOptions options, IMqttNetChildLogger logger)
        {
            if (options == null) throw new ArgumentNullException(nameof(options));

            switch (options.ChannelOptions)
            {
                case MqttClientTcpOptions _:
                    {
                        var adapter = new MQTTnet.Adapter.MQTTNetCustomAdapter(new MQTTnet.Implementations.MqttTcpChannel(options), new MQTTnet.Formatter.MqttPacketFormatterAdapter(options.ProtocolVersion), logger)
                        {
                            ReadBufferSize = ((MqttClientTcpOptions)(options.ChannelOptions)).BufferSize
                        };
                        return adapter;
                    }

                case MqttClientWebSocketOptions webSocketOptions:
                    {
                        return new MQTTnet.Adapter.MQTTNetCustomAdapter(new MqttWebSocketChannel(webSocketOptions), new MqttPacketFormatterAdapter(options.ProtocolVersion), logger);
                    }

                default:
                    {
                        throw new NotSupportedException();
                    }
            }
        }
    }
}

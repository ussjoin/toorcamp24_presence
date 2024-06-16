# virtualenv rpi-presence
# source rpi-presence/bin/activate
# pip3 install paho-mqtt

# MacOS:
# brew install mosquitto
# mosquitto has been installed with a default configuration file.
# You can make changes to the configuration by editing:
#     /opt/homebrew/etc/mosquitto/mosquitto.conf
#
# To start mosquitto now and restart at login:
#   brew services start mosquitto
# Or, if you don't want/need a background service you can just run:
#   /opt/homebrew/opt/mosquitto/sbin/mosquitto -c /opt/homebrew/etc/mosquitto/mosquitto.conf
import paho.mqtt.client as mqtt



# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, reason_code, properties):
    print(f"Connected with result code {reason_code}")
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("narwhalbeacon/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    # TODO:
    # Parse payload
    # Beacon to Person
    # Send command to Arduino

# TODO:
# parse YAML

mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqttc.on_connect = on_connect
mqttc.on_message = on_message

mqttc.connect("localhost", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
mqttc.loop_forever()
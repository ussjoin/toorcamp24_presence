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


# https://pypi.org/project/paho-mqtt/
import paho.mqtt.client as mqtt
import yaml
import re

frens = re.compile("^NR7WL-[A-Z0-9]{4}$")

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, reason_code, properties):
    print(f"Connected with result code {reason_code}")
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("narwhalbeacon/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    m = frens.match(msg.payload.decode("utf-8"))
    if m:
        identifier = m.group()
        if identifier in people_lookup:
            person = people_lookup[identifier]
            print(f"Found friend: {person['name']}")
        else:
            print(f"Found someone I don't know: {identifier}")
    else:
        print(f"Got a message incoming I didn't understand: {msg.payload.decode('utf-8')}")
    
    # TODO:
    # Send command to Arduino


mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqttc.on_connect = on_connect
mqttc.on_message = on_message

mqttc.connect("localhost", 1883, 60)

people_lookup = {}

with open("people.yaml", "r") as f:
    people = yaml.safe_load(f)['users']
    for person in people:
        people_lookup[person['beacon']] = person

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
mqttc.loop_forever()
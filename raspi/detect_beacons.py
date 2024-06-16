import time
import paho.mqtt.client as mqtt

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, reason_code, properties):
    print(f"Connected with result code {reason_code}")
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    # client.subscribe("$SYS/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqttc.on_connect = on_connect
mqttc.on_message = on_message

mqttc.connect("localhost", 1883, 60)

# These functions implement a threaded interface to the network loop. Calling loop_start() once, before or after connect*(), runs a thread in the background to call loop() automatically. This frees up the main thread for other work that may be blocking. This call also handles reconnecting to the broker. Call loop_stop() to stop the background thread. The loop is also stopped if you call disconnect().

mqttc.loop_start()

while True:
    uuid = "ohai"
    mqttc.publish("narwhalbeacon", uuid)
    time.sleep(5)
    

mqttc.loop_stop()
import time
import paho.mqtt.client as mqtt
import re
import asyncio
from bleak import BleakScanner
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData

frens = re.compile("^NR7WL-[A-Z0-9]{4}$")
mqttc = mqtt.Client()


# Called when a BLE beacon is detected.
def simple_callback(device: BLEDevice, advertisement_data: AdvertisementData):
    #print(advertisement_data.local_name)
    if advertisement_data.local_name: # it's None if there wasn't one
        m = frens.match(advertisement_data.local_name)
        if m:
            print(f"Found friend: {m.group()}")
            #print("%s: %r", device.address, advertisement_data)
            mqttc.publish("narwhalbeacon", m.group())

# The MQTT callback for when the client receives a CONNACK response from the server.
# Will be called on reconnect, too.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    # client.subscribe("$SYS/#")
    
    # However, we don't actually need this script to receive messages, just send.

# The MQTT callback for when a PUBLISH message is received from the server.
# In this script, should be basically unused.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))


async def main():
    mqttc.on_connect = on_connect
    mqttc.on_message = on_message
    mqttc.connect("localhost", 1883, 60)

    # These functions implement a threaded interface to the network loop. Calling loop_start() once, before or after connect*(), runs a thread in the background to call loop() automatically. This frees up the main thread for other work that may be blocking. This call also handles reconnecting to the broker. Call loop_stop() to stop the background thread. The loop is also stopped if you call disconnect().

    mqttc.loop_start()
    
    stop_event = asyncio.Event()
    # TODO: add something that calls stop_event.set()
    
    async with BleakScanner(simple_callback) as scanner:
        # Important! Wait for an event to trigger stop, otherwise scanner
        # will stop immediately.
        await stop_event.wait()
        
    # Doesn't ever really get here, we just Ctrl-C
    mqttc.loop_stop()

asyncio.run(main())

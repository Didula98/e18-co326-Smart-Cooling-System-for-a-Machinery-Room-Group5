import paho.mqtt.client as mqttclient
import time
import pymysql
import json
import datetime

# check connection
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Client connected")
        global connected
        connected = True
    else:
        print("Not conneted")


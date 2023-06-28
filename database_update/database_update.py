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




def on_message(client, userdata, message):

    # print message
    print("Message:", str(message.payload.decode("utf-8")))
    print("Topic", str(message.topic))

    if str(message.topic) == "UoP/CO/326/E18/5/SendData":
        # if requesting data
        global cursor
        global connection

        # retrive last 20 rows form the table
        cursor.execute('''
        SELECT *
        FROM (
        SELECT *, ROW_NUMBER() OVER () AS rn
        FROM controller_data
        ) AS subquery
        WHERE rn > (SELECT COUNT(*) FROM controller_data) - 20;
        '''
        )

        data = cursor.fetchall()

        # create json format
        jsonlist = []

        for row in data:
            row_dict = {
                "id": row[0],
                "temp": row[1],
                "sent_time": str(row[2]),
                "received_time": str(row[3]),
                "rtc_time": str(row[4])
            }

            jsonlist.append(row_dict)

        # publish data to broker
        client.publish("UoP/CO/326/E18/5/ReceiveData", str(jsonlist))
        cursor.close()
        connection.close()

    
    elif str(message.topic) == "UoP/CO/326/E18/5/ReceiveData":
        # if data received from broker
        data = json.loads(str(message.payload.decode("utf-8")))

        # calculate rtc time
        rtc_time = datetime.datetime.now().replace(microsecond=0) - datetime.datetime.strptime(data["DateTime"], '%Y-%m-%d %H:%M:%S')

        # insert data to the database
        sql = "INSERT INTO controller_data VALUES(uuid(), %s, %s, %s, %s)"
        val = (data["Temperature"], data["DateTime"], datetime.datetime.now().replace(microsecond=0), rtc_time)
        cursor.execute(sql, val)
        connection.commit()






# create connection with database
connection = pymysql.connect(
    host = 'localhost',
    user = 'root',
    password = 'di@62992',
    database = 'smart_cooling_system'
)

# create cursor
cursor = connection.cursor() 

connected = False
MessageRecieved = False

broker_address = "192.168.22.113"
port = 1883
user = "serverCO326"
password = "group5"
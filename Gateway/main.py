import random
import time
import sys
import os
import serial.tools.list_ports
from Adafruit_IO import MQTTClient

AIO_FEED_ID = ["bbc-led", "bbc-fan", "bbc-buzzer"]

# Load credentials from environment variables. Put real values in a local .env (ignored by git).
AIO_USERNAME = os.getenv("AIO_USERNAME", "REPLACE_WITH_USERNAME")
AIO_KEY = os.getenv("AIO_KEY")

client = None

def connected(client):
    print("Ket noi thanh cong...")
    for feed in AIO_FEED_ID:
        client.subscribe(feed)


def subscribe(client , userdata , mid , granted_qos):
    print("Subcribe thanh cong...")


def disconnected(client):
    print("Ngat ket noi...")
    sys.exit (1)


def message(client , feed_id , payload):
    print("Nhan du lieu: " + payload)
    if isMicrobitConnected:
        ser.write((str(payload) + "#").encode())


def publish(feed, value):
    # Safe publish wrapper: only publish when client is configured
    if client:
        try:
            client.publish(feed, value)
        except Exception as e:
            print(f"Publish error: {e}")
    else:
        print(f"Skipping publish (no AIO key): {feed} = {value}")

def getPort():
    ports = serial.tools.list_ports.comports()
    N = len(ports)
    commPort = "None"
    for i in range(0, N):
        port = ports[i]
        strPort = str(port)
        # Tìm cả Micro:bit và ESP32
        if "USB Serial Device" in strPort or "CP210x" in strPort or "CH340" in strPort or "Silicon Labs" in strPort: # test gia lap esp32
            splitPort = strPort.split(" ")
            commPort = (splitPort[0])
    return commPort

isMicrobitConnected = False
if getPort() != "None":
    ser = serial.Serial( port=getPort(), baudrate=115200)
    isMicrobitConnected = True
    
# If AIO_KEY provided, configure MQTT client. Otherwise run in local/test mode and avoid leaking key.
if AIO_KEY:
    client = MQTTClient(AIO_USERNAME , AIO_KEY)
    client.on_connect = connected
    client.on_disconnect = disconnected
    client.on_message = message
    client.on_subscribe = subscribe
    client.connect()
    client.loop_background()
else:
    print("AIO_KEY not set in environment. Running in local/test mode (no publish to Adafruit IO).")
def processData(data):
    data = data.replace("!", "")
    data = data.replace("#", "")
    splitData = data.split(":")
    print(splitData)
    try:
        if splitData[1] == "TEMP":
            temp_value = splitData[2]
            publish("bbc-temp", temp_value)
            print("Cap nhat nhiet do: " + temp_value)
        elif splitData[1] == "HUMI":
            humi_value = splitData[2]
            publish("bbc-humi", humi_value)
            print("Cap nhat do am: " + humi_value)
        elif splitData[1] == "LIGHT":
            light_value = splitData[2]
            publish("bbc-light", light_value)
            print("Cap nhat do sang: " + light_value)
        elif splitData[1] == "PIR":
            pir_value = splitData[2]
            pir_normalized = 1 if pir_value == "True" else 0
            publish("bbc-pir", pir_normalized)
            print("Cap nhat PIR: " + str(pir_normalized))
    except Exception as e:
        print(f"Loi parse data: {e}")
        pass


mess = ""
def readSerial():
    bytesToRead = ser.inWaiting()
    if (bytesToRead > 0):
        global mess
        mess = mess + ser.read(bytesToRead).decode("UTF-8", errors="ignore")
        while ("#" in mess) and ("!" in mess):
            start = mess.find("!")
            end = mess.find("#")
            processData(mess[start:end + 1])
            if (end == len(mess)):
                mess = ""
            else:
                mess = mess[end+1:]

while True:
    if isMicrobitConnected:
        readSerial()
        time.sleep(1)
    else:
        value = random.randint(15, 40)
        print("Cap nhat nhiet do (RANDOM TEST):", value)
        client.publish("bbc-temp", value)

        value_humi = random.randint(30, 90)
        print("Cap nhat do am (RANDOM TEST):", value_humi)
        client.publish("bbc-humi", value_humi)

        value_light = random.randint(40, 150)
        print("Cap nhat do sang (RANDOM TEST):", value_light)
        client.publish("bbc-light", value_light)

        value_pir = random.randint(0, 1)
        print("Cap nhat PIR (RANDOM TEST):", value_pir)
        client.publish("bbc-pir", value_pir)
        time.sleep(30)
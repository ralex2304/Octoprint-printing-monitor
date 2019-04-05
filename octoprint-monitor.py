"""
Octoprint printing monitor by ralex2304
Version: 2.5
date: 05.04.2019
email: admin@ardupy3000.ru
web: https://github.com/ralex2304
"""

API = "---"   #Octoprint API key
#Import section
import time
import json
import requests
import os
import serial
import socket
import datetime
from logMaster import *
#Variable section
port="/dev/ttyACM"
headers = {'X-Api-Key': API,"Content-Type":"application/json"}
headers2 = {'X-Api-Key': API}
url = 'http://127.0.0.1/api/job'
url2 = 'http://127.0.0.1/api/printer?exclude=temperature,sd'
count=4
estimated=False
conn=0
ser_conn=1
finished=False
IP = "No data"
i=0
if API=="---": #checking if API key is added
    print("Please add to code your Octoprint API key")
    print("Press CTRL + C")
    while True:
        pass
print(log("Connecting...")) #reporting about starting the code
#Connecting to Octoprint
while True:
    conn+=1
    try:
        r = requests.get(url2,headers=headers2)
    except:
        pass
    else:
        print(log("Connected. "+str(conn)+" tryings."))
        break
    time.sleep(5)
#Connecting serial
while True:
    try:
        ser = serial.Serial(port+str(i),9600,timeout=2)
    except Exception as e:
        print(log("Serial connect error: "+str(e)))
        if i==4:
            i=-1
        i+=1
        if ser_conn<20:
            continue
    else:
        print(log("Serial connected. "+str(ser_conn)+" tryings."))
        break
    if ser_conn==20:
        print(log("Serial is not responding. "+str(ser_conn)+" tryings."))
        #break
    time.sleep(3) #delay for 3 seconds
    ser_conn+=1
#Main loop
while True:
    inp="" #resetting input variable
    count+=1
    #Reading data from serial
    try:
        inp=str((ser.readline()).decode("utf-8")) #converting input data to string
        inp=inp.replace("\n","") #Replacing "\n" symbol from respond
    except Exception as e:
        print(log("Error reading serial: "+str(e)))
    #Pause handler
    if inp=="pause":
        contents = json.dumps({"command":"pause","action":"pause"})
        requests.post(url, data=contents, headers=headers)
        print(log("Pause"))
        count=5
    #Resume handler
    if inp=="resume":
        contents = json.dumps({"command":"pause","action":"resume"})
        requests.post(url, data=contents, headers=headers)
        print(log("Resume"))
        count=5
    #Cancel handler
    if inp=="cancel":
        print(log("Cancel"))
        contents = json.dumps({"command":"cancel"})
        requests.post(url, data=contents, headers=headers)
        count=5
    #Shutdown handler
    if inp=="shutdown":
        r = requests.get(url2,headers=headers2)
        if json.loads(r.text)["state"]["text"]=="Operational":
            print(log("Shutdown"))
            try:
                ser.write(b"status\n")
                ser.write(b"Shutted down\n") #Sending status
                ser.write(b"estimated\n")
                ser.write(b"Ip: "+bytes(IP)+"\n") #Sending Ip address
            except Exception as e:
                print(log("Serial write error: "+str(e))) #reporting an error
            #optional:
            #os.system("sudo service octoprint stop")
            os.system("sudo shutdown -h now") #shutting down
        else:
            print(log("Shutdown cancelled. State:"+json.loads(r.text)["state"]["text"]))
    #Every five seconds it sends data to arduino device
    if count>=5:
        count=0
        estimated=not estimated
        #Getting data from octoprint
        r = requests.get(url2,headers=headers2)
        r2 = requests.get(url,headers=headers2)
        #Different responses handler
        if r.text=="Printer is not operational":
            try:
                ser.write(b"status\n")
                ser.write(b"Printer is not ok\n")
            except Exception as e:
                print(log("Serial write error: "+str(e))) #reporting an error
            continue
        try:
            var=json.loads(r.text)["state"]["text"]
            var2=json.loads(r2.text)["progress"]["completion"]
            if var != "Operational":
                finished=False
            elif finished==False:
                finished=True
                try:
                    ser.write(b"finished\n")
                except:
                    pass
        except:
            print(log("Error. Uncorrect respond: \""+r.text+"\"")) #reporting an error
            continue
        try:
            ser.write(b"status\n")   #sending status
            if len(str(json.loads(r.text)["state"]["text"]))>20:
                ser.write(bytes(str(json.loads(r.text)["state"]["text"])[0:20],'utf-8')+b"\n")
            else:
                ser.write(bytes(str(json.loads(r.text)["state"]["text"]),'utf-8')+b"\n")
            ser.write(b"estimated\n")   #sending time left or percentage of current work
            if str(json.loads(r.text)["state"]["text"])=="Operational":
                try:
                    if IP=="No data":
                        IP=str((([ip for ip in socket.gethostbyname_ex(socket.gethostname())[2] if not ip.startswith("127.")] or [[(s.connect(("8.8.8.8", 53)), s.getsockname()[0], s.close()) for s in [socket.socket(socket.AF_INET, socket.SOCK_DGRAM)]][0][1]]) + ["no IP found"])[0]) #getting Ip address
                except Exception as e:
                    IP="No data"
                    print(log("Error getting Ip: "+str(e)))
                estim="Ip: "+IP
                ser.write(bytes(estim,'utf-8')+b"\n")
                continue
            if estimated==True:
                estim="Completion: "+str(json.loads(r2.text)["progress"]["completion"])[0:4]+"%"
            else:
                left=str(datetime.timedelta(seconds=int(json.loads(r2.text)["progress"]["printTimeLeft"])))
                estim="Left: "+left
            ser.write(bytes(estim,'utf-8')+b"\n")
        except Exception as e:
            print(log("Serial write error: "+str(e)))
    time.sleep(1)   #delay for 1 second
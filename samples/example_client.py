#!/usr/bin/env python3

import sys, getopt
import json
import os
import websocket
import _thread
import time

filename = ""
drone_type = ""
drone_serial = ""

def on_message(ws, message):
    print("message arrived: " + message)
    msg = json.loads(message)
    client_id = msg["data"]["client_id"]
    if msg["type"] == "hello_resp":
        def run(*args):
            file = open(filename, "r")
            for line in file:
                j_line = json.loads(line)
                j_line["client_id"] = client_id
                adjusted_line = json.dumps({"type":"data_broadcast", "data":j_line})
                print(adjusted_line)
                ws.send(adjusted_line)
                time.sleep(0.1)
            time.sleep(1)
            ws.close()
            print("thread terminating...")

        _thread.start_new_thread(run, ())

def on_error(ws, error):
    print(error)

def on_close(ws):
    print("### closed ###")

def on_open(ws):
    ws.send(json.dumps({"type":"hello","data":{"ctype":0, "drone_name":drone_type, "serial":drone_serial}}))



def main(argv):
    global filename, drone_type, drone_serial
    try:
        opts, args = getopt.getopt(argv, "hf:t:s:", ["file=", "drone_type=", "drone_serial="])
    except getopt.GetoptError:
        print("client.py -f <filename> -t <drone_type> -s <drone_serial>")
    for opt, arg in opts:
        if opt == "-h":
            print("client.py -f <filename> -t <drone_type> -s <drone_serial>")
            sys.exit()
        elif opt in ("-f", "--file"):
            filename = arg
        elif opt in ("-t", "--drone_type"):
            drone_type = arg
        elif opt in ("-s", "--drone_serial"):
            drone_serial = arg
    if filename == "" or drone_type == "" or drone_serial == "":
        print("client.py -f <filename> -t <drone_type> -s <drone_serial>")
        sys.exit(2)

    #websocket.enableTrace(True)
    ws = websocket.WebSocketApp("ws://butcluster.ddns.net:5555",
                              on_message = on_message,
                              on_error = on_error,
                              on_close = on_close)
    ws.on_open = on_open
    ws.run_forever()


if __name__ == "__main__":
    main(sys.argv[1:])



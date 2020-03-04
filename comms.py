import time, mscvcrt
import socket

host = 192.168.4.1
port = 80

while True:
    sock = socket.socket()
    sock.connect((host, port))

    

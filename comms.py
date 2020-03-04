import socket

'''
pip install py-getch
'''

host = "192.168.4.1"
port = 80

cmd = None
prev_cmd = 'TO'
out = None
while True:
    cmd = 'F'
    # cmd can be F, B, R, L

    # connect to paperbot websocket
    sock = socket.socket()
    sock.connect((host, port))
    sock.send(cmd.encode())
    # data = sock.recv(1) is used to ask paperbot for data
    # NOTE: paperbot side of the code has not yet been implemented
    # Everything was implemented using the following information
    # https://techtutorialsx.com/2017/11/13/esp32-arduino-setting-a-socket-server/

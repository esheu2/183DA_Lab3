import time, mscvcrt
import socket

host = 192.168.4.1
port = 80

keypressTimeout = 3

cmd = None
prev_cmd = 'TO'
out = None
while True:
    timeout = time.time()
    if mscvrt.kbhit():
        prev_cmd = cmd
        cmd = msvcrt.getch().decode()
    elif time.time() - timeout > keypressTimeout:
        prev_cmd = cmd
        cmd = 'TO'

    # WASD controls
    if cmd == 'w':
        out = 'F'
    elif cmd == 'a':
        out = 'L'
    elif cmd == 's':
        out = 'B'
    elif cmd == 'd':
        out = 'R'

    # connect to paperbot websocket
    sock = socket.socket()
    sock.connect((host, port))
    sock.send(out.encode())
    # data = sock.recv(1) is used to ask paperbot for data
    # NOTE: paperbot side of the code has not yet been implemented

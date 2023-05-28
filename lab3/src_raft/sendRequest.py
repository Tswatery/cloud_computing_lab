import socket
import random

node_info = [
    ('127.0.0.1', 8001),
    ('127.0.0.1', 8002),
    ('127.0.0.1', 8003)
]

while(1):
    data = input("请输入要发送的数据: ")
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    node = random.choice(node_info)
    ip, port = node[0], node[1]
    sock.connect((ip, port))
    data = f'$ {data}'
    sock.sendall(data.encode())
    print(f'\033[32m向{ip}:{port}发送数据{data}\033[0m')
    response = sock.recv(1024)
    recvstr = response.decode()
    if ':' in recvstr:
        idx = recvstr.find(':')
        ip, port = recvstr[0:idx], int(recvstr[idx + 1:])
        sock1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock1.connect((ip, port))
        print(f'\033[31m向leader{ip}:{port}重新发送{data}\033[0m')
        sock1.sendall(data.encode())
        recv = sock1.recv(1024)
        print(f'\033[32m重新接受到接收:{recv.decode()}\033[0m')
        sock1.close()
    else:
        print(f'\033[32m接收: {response.decode()}\033[0m')
    sock.close()

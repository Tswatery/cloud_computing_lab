import socket

ip = "115.157.200.89"
port = 8001
while(1):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((ip, port))
    data = input("请输入要发送的数据: ")
    sock.sendall(data.encode())
    response = sock.recv(1024)
    print('Received:', response.decode())
    sock.close()

import socket

def send_data(ip, port):
    # 创建一个TCP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        # 连接到目标IP和端口
        sock.connect((ip, port))

        while True:
            # 从终端读入要发送的数据
            data = input("请输入要发送的数据 (输入 'q' 退出): ")

            if data == 'q':
                break

            # 发送数据
            sock.sendall(data.encode())

            # 接收响应
            response = sock.recv(1024)
            print('Received:', response.decode())

    except socket.error as e:
        print('Socket error:', e)

    finally:
        # 关闭socket连接
        sock.close()

# 从终端读入IP和端口
ip = input("请输入IP地址: ")
port = int(input("请输入端口号: "))

# 调用函数发送数据
send_data(ip, port)

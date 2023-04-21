# 2023-04-21
实验要求我们这样输入：`./http-server --ip 127.0.0.1 --port 8888 --threads 8`
因此需要绑定地址和端口号，然后就可以从终端中使用`curl`命令来测试这个http的服务器。

# 2023-04-22
会发现在一开始的实验中，想要看`path`和`method`，是不能直接`printf`的：
```c
printf("method: %s, path: %s", request->method, request->path);
```
必须使用`write`系统调用：
```c
int n1 = write(STDERR_FILENO, request->method, strlen(request->method));
int n2 = write(STDERR_FILENO, request->path, strlen(request->path));
```
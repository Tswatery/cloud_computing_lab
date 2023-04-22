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

惊！在C语言中比较字符串只能使用`strcmp`，相等即返回0，因为在C语言中字符串是字符数组存储的，`==`只能比较二者的地址是否一致。

## 错误
1. 我发现在我的`http-server`中，如果我连续输入两次`get`访问`hello-world.txt`就会出现这个问题，其他的是没有问题的。

<img src="https://image-save-1309598795.cos.ap-nanjing.myqcloud.com/typora/image-20230422223346155.png" alt="image-20230422223346155" style="zoom:33%;" />

吗的，不管了。

2. 使用`curl`命令的时候，如果我加上了`Content-length`就会出现还有多少个字节没有读取的问题。

<img src="https://image-save-1309598795.cos.ap-nanjing.myqcloud.com/typora/image-20230422224550596.png" alt="image-20230422224550596" style="zoom:33%;" />

吗的，不知道为什么，不管了。

可能是因为我的`content-length`字节设置不正确。我该怎么设置呢？


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

3. 在`mac`环境下甚至都不能返回信息。

# 2023-04-23

发现课程组的测试是在`mac`环境下的，我没有`x86`的虚拟机，遂在`mac`环境下继续做实验。

今天解决了单个客户端请求服务的情况，目前来看是没有错误的。

通过`curl`命令发送`http`请求并响应，之前错误`18`的情况是因为：

![image-20230423154733038](https://image-save-1309598795.cos.ap-nanjing.myqcloud.com/typora/image-20230423154733038.png)

把箭头标志的一行写在了最后面，但这其实是`http header`的结束符号`/r/n`。

连续两次`hello-world.txt`出现内存错误的原因也解决了。

今天还需要把文件的判断写一下，比如不存在的文件返回`404.html`，现在通过浏览器也能访问`localhost:8888/index.hrml`了。

目标：

1. 不存在的文件返回`404.html`
2. 不是`GET`或者`POST`返回`501.html`

# 2023-05-04

`pipeline`。贺老师说要主动关闭连接，因此我的解析方式要改，在主函数中，通过`strtok`函数来根据`\r\n\r\n`来分割字符串。然后逐行解析吧。

写到了`main`的`server`逻辑，明早起来把具体逻辑完善一下。

# 2023-05-05

我还是卡死在`pipeline`了。下午看看大佬们怎么实现的吧。

突然发现了我的问题其实是任何操作都是正确的，就是我写不进去套接字`server_socket`，我的任何东西都写不进去套接字。

并且由于我没有设置比如定时关闭的情况，所以会出现死循环卡死。

## 找到bug

关于为什么会卡在`pipeline`呢，在我的简陋服务器中，我疏忽了一个东西：在每次传输完成的时候，服务器一定要主动关闭`socket`。这种在非持续连接中没有任何问题，因为每次客户端会主动关闭，此时服务器没有设置主动关闭也没问题，因为顺序执行后，这个`socket`会被回收。

但是在持续性连接中就出现了问题，如果服务器在一定时间内没有接收到数据的话，它会一直等到因为我们默认的是持续连接；在客户端也会一直等，就出现了没有传输内容的情况。

<img src="./img/log/image-20230505173344334.png" alt="image-20230505173344334" style="zoom:50%;" />

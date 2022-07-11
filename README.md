# Mini Web Server

依照TinyWebServer进行重写的项目，主要用于自己学习WebServer基本运作方式。

在原项目基础上添加了大量注释，去除了个性化选择运行方式的功能，对部分代码进行了修改。

TinyWebServer: https://github.com/qinguoyi/TinyWebServer

遇到的问题:

1、处理完来自客户端的EPOLLIN请求后，不能直接回复报文，需要注册并监听到EPOLLOUT事件后才能回复。

2、回复css资源时，如果content-type设置为text/html，将会导致css无法正常显示，原因不明。

3、客户端用axios发送请求时，url填https会对报文进行加密，处理时需要先解密。

4、在头文件中定义全局变量是不被允许的，头文件中仅可声明，可以用extern声明，再在cpp中定义。
# Mini Web Server

依照TinyWebServer进行重写的项目，主要用于自己学习WebServer基本运作方式。

在原项目基础上添加了大量注释，去除了个性化选择运行方式的功能，对部分代码进行了修改。

TinyWebServer: https://github.com/qinguoyi/TinyWebServer

遇到的问题:

1、处理完来自客户端的EPOLLIN请求后，不能直接回复报文，需要注册并监听到EPOLLOUT事件后才能回复。

2、回复css资源时，如果content-type设置为text/html，将会导致css无法正常显示，原因不明。

3、客户端用axios发送请求时，url填https会对报文进行加密，处理时需要先解密。

4、在头文件中定义全局变量是不被允许的，头文件中仅可声明，可以用extern声明，再在cpp中定义。

5、Jsoncpp中Value的构造函数传入指针变量时，内部会进行指针解引用后拷贝，影响效率，可将指针变量转为整型地址后传入，取出时同理。注意64位机不能用int。

6、调试的时候，根目录是当前执行的语句所在目录，而运行make后的可执行文件，根目录却是可执行文件所在目录。

7、请求头中的keep-alive属性决定，客户端接下来是否继续发送请求报文。

8、epoll_loop中，应该先判断定时器信号，因为定时器信号的条件很苛刻，必须为pipe中有可读事件。该条件完全包含在客户端可读的条件内。
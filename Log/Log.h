#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>

#include <queue>

#include "../Locker/Locker.h"

using namespace std;

class Log {
public:
    // 使用局部变量懒汉
    static Log *get_instance() {
        static Log instance;
        return &instance;
    }

    static void *flush_log_thread(void *args) {
        Log::get_instance()->async_write_log();
        return nullptr;
    }

    bool init(const char* file_name, bool close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0); //可选择的参数有日志文件、日志缓冲区大小、最大行数以及最长日志条队列

    void write_log(const char* level, const char *format, ...); // 同步：直接写日志   异步：将内容加入队列

    void flush(void); // 刷新缓存区

private:
    Log() {
        _count = 0;
        _is_async = false;
    }
    virtual ~Log() {
        if (_fp != nullptr) {
        fclose(_fp);
    }
    }
    void* async_write_log();

private:
    bool _close_log;         // 是否关闭日志
    bool _is_async;         // 是否同步标志位

    char _dir_name[128];    // 路径名
    char _log_name[128];    // log文件名
    int _today;             // 因为按天分类,记录当前时间是那一天
    FILE *_fp;              // 打开log的文件指针    

    int _split_lines;       // 日志最大行数
    int _log_buf_size;      // 日志缓冲区大小
    long long _count;       // 日志行数记录
    char *_buf;

    Locker _mutex;          // 保护 Log日志信息 的互斥锁

    int _max_log_num;
    std::queue<char*> _log_queue;   // 阻塞队列
    Locker _queue_locker;           // 保护 阻塞队列 的互斥锁
};

#define LOG_DEBUG(format, ...)  Log::get_instance()->write_log("[debug]:", format, ##__VA_ARGS__);  Log::get_instance()->flush();
#define LOG_INFO(format, ...)   Log::get_instance()->write_log("[info]:", format, ##__VA_ARGS__);   Log::get_instance()->flush();
#define LOG_WARN(format, ...)   Log::get_instance()->write_log("[warn]:", format, ##__VA_ARGS__);   Log::get_instance()->flush();
#define LOG_ERROR(format, ...)  Log::get_instance()->write_log("[erro]:", format, ##__VA_ARGS__);   Log::get_instance()->flush();

#endif

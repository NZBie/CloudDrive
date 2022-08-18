#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <pthread.h>

#include "Log.h"

using namespace std;

// 日志初始化，异步需要设置阻塞队列的长度，同步不需要设置
bool Log::init(const char* file_name, bool close_log, int log_buf_size, int split_lines, int max_queue_size) {

    // 如果设置了max_queue_size,则设置为异步
    if (max_queue_size >= 1) {
        _is_async = true;
        _max_log_num = max_queue_size;

        // 创建线程异步写日志
        pthread_t tid;
        pthread_create(&tid, nullptr, flush_log_thread, nullptr);
    }
    
    // 初始化成员变量
    _close_log = close_log;
    _log_buf_size = log_buf_size;
    _buf = new char[_log_buf_size];
    memset(_buf, '\0', _log_buf_size);
    _split_lines = split_lines;

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    _today = my_tm.tm_mday;

    // 日志的路径
    const char *p = strrchr(file_name, '/');
    char log_full_name[256] = {0};
    if (p == nullptr) {
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    }
    else {
        strcpy(_log_name, p + 1);
        strncpy(_dir_name, file_name, p - file_name + 1);
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", _dir_name, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, _log_name);
    }
    _fp = fopen(log_full_name, "a");
    if(_fp == nullptr) return false;

    return true;
}

// 同步：直接写日志   异步：将内容加入队列
void Log::write_log(const char* level, const char *format, ...) {

    if(_close_log == true) return;

    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    // 检测是否需要新建日志文件，实现日志 按日期分类，超行分类
    _mutex.lock();
    _count++;

    if(_today != my_tm.tm_mday || _count % _split_lines == 0) {
        
        char new_log[256] = {0};
        fflush(_fp);
        fclose(_fp);
        char tail[16] = {0};
       
        snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);
       
        if(_today != my_tm.tm_mday) {

            snprintf(new_log, 255, "%s%s%s", _dir_name, tail, _log_name);
            _today = my_tm.tm_mday;
            _count = 0;
        }
        else {
            snprintf(new_log, 255, "%s%s%s.%lld", _dir_name, tail, _log_name, _count / _split_lines);
        }
        _fp = fopen(new_log, "a");
    }
    _mutex.unlock();

    // 写入日志的具体内容
    va_list arg_list;
    va_start(arg_list, format);

    _mutex.lock();

    // 标头格式
    int n = snprintf(_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                     my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                     my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, level);
    
    int m = vsnprintf(_buf + n, _log_buf_size - 1, format, arg_list);
    _buf[n + m] = '\n';
    _buf[n + m + 1] = '\0';
    char* log_str = new char[n + m + 2];
    strcpy(log_str, _buf);

    _mutex.unlock();
    _queue_locker.lock();

    // 异步 且 阻塞队列未满，则加入阻塞队列
    if (_is_async && _log_queue.size() < _max_log_num) {
        _log_queue.push(log_str);
        _queue_locker.unlock();
    }

    // 同步 或者 阻塞队列满，则同步写入日志
    else {
        _queue_locker.unlock();
        _mutex.lock();
        fputs(log_str, _fp);
        _mutex.unlock();
    }

    va_end(arg_list);
}

// 刷新缓存区
void Log::flush(void)
{
    _mutex.lock();
    //强制刷新写入流缓冲区
    fflush(_fp);
    _mutex.unlock();
}

// 异步从队列取出内容，写日志
void* Log::async_write_log() {

    //从阻塞队列中取出单条日志，写入文件
    char* single_log;
    while (true) {
        // 阻塞队列为空
        if(_log_queue.empty()) continue;

        _queue_locker.lock();
        if(!_log_queue.empty()) {

            // 取出单条日志
            single_log = _log_queue.front();
            _log_queue.pop();
            _queue_locker.unlock();

            // 写入日志文件
            _mutex.lock();
            fputs(single_log, _fp);
            _mutex.unlock();
        }
        else {
            _queue_locker.unlock();
        }
    }
    return nullptr;
}

#include <iostream>
#include "logger.h"
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <thread>
#include <stdexcept>

const char *LevelString[6] = {"DNS", "DEBUG", "INFO", "WARNING", "ERROR", "FATAL"};

log_ins::log_ins()
{
    fp_ins = fopen("log_ins.log", "w+");
    log_ins_buf = new char[Log_Ins_size];
}

log_ins::~log_ins()
{
    std::cout << "~log_ins" << std::endl;
    if (log_ins_usedlen != 0)
    {
        fwrite(log_ins_buf, 1, log_ins_usedlen, fp_ins);
    }
    if (log_ins_buf != nullptr)
    {
        delete[] log_ins_buf;
    }
}

LogBuffer::LogBuffer(uint32_t _bufsize, int _logbuf_num)
{
    BufSize = _bufsize;
    logbuf_num = _logbuf_num;
    UsedLen = 0;
    state = FREE;
    logbuffer = new char[BUFSIZE];
    if (logbuffer == nullptr)
    {
        throw std::logic_error("logbuffer init err");
    }
}

LogBuffer::~LogBuffer()
{
    if (logbuffer != nullptr)
    {
        delete[] logbuffer;
    }
}

void LogBuffer::append(char const *log, int len)
{
    memcpy(logbuffer + UsedLen, log, len);
    UsedLen += len;
}

void LogBuffer::FlushTofile(FILE *fp)
{
    uint32_t wt_len = fwrite(logbuffer, 1, UsedLen, fp);
    if (wt_len != UsedLen)
    {
        throw std::logic_error("FLUSH TO FILE ERR");
    }
    log_insde->log_ins_append("buf wirte now-->%d\n", logbuf_num);
    std::cout << "buf wirte now-->" << logbuf_num << std::endl;
    UsedLen = 0;
    fflush(fp);
}

logger::logger(/* args */)
{
    level = INFO;
    fp = nullptr;
    buf_num = 0;
    start_flag = false;
}

logger::~logger()
{
    std::cout << "~logger" << std::endl;
    {
        std::lock_guard<std::mutex> lock(log_mtx);
        std::map<std::thread::id, LogBuffer *>::iterator iter;
        for (iter = threadbufmap.begin(); iter != threadbufmap.end(); ++iter)
        {
            iter->second->SetState(LogBuffer::BufState::FLUSH);
            {
                std::lock_guard<std::mutex> lock2(flush_mtx);
                flushbufqueue.push(iter->second);
            }
        }
    }
    flushcond.notify_one();
    start_flag = false;
    flushcond.notify_one();
    if (flushThread.joinable())
        flushThread.join();
    if (fp != nullptr)
    {
        fclose(fp);
    }
    while (!freeBufQueue.empty())
    {
        LogBuffer *p = freeBufQueue.front();
        freeBufQueue.pop();
        delete p;
    }
    while (!flushbufqueue.empty())
    {
        LogBuffer *p = flushbufqueue.front();
        flushbufqueue.pop();
        delete p;
    }
}

void logger::Init(const char *logDir, LoggerLevel _level)
{
    time_t t = time(nullptr);
    struct tm *ptm = localtime(&t);
    char logFilePath[256] = {0};
    snprintf(logFilePath, 255, "%s/log_%d_%d_%d", logDir, ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
    level = _level;
    fp = fopen(logFilePath, "w+");

    if (fp == nullptr)
    {
        throw std::logic_error("log file open fail --ERR");
    }
    flushThread = std::thread(&logger::Flush, this);
    return; //????
}

void logger::Append(int _level, const char *File, int line, const char *func, const char *fmt, ...)
{
    char logline[LogLineSize]; // 简历一个临时char
    struct timeval tv;         // 毫秒
    gettimeofday(&tv, NULL);   // 当前时间
    static time_t lastsec = 0;
    if (lastsec != tv.tv_sec) // 对比毫秒有没有变化，没有比那就就改值
    {
        struct tm *ptm = localtime(&tv.tv_sec);
        lastsec = tv.tv_sec;
        int k = snprintf(save_ymdhms, 64, "%04d-%02d-%02d %02d:%02d:%02d", ptm->tm_year + 1900,
                         ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
        save_ymdhms[k] = '\0';
    }

    std::thread::id tid = std::this_thread::get_id(); // 当前运行的线程ID
    // log内容
    uint32_t n = snprintf(logline, LogLineSize, "[%s][%s.%03ld][%s:%d %s][pid:%lu] ", LevelString[level],
                          save_ymdhms, tv.tv_usec / 1000, File, line, func, std::hash<std::thread::id>()(tid));

    // TO DO ???? //加入变量内容到log
    va_list args;
    va_start(args, fmt);
    int m = vsnprintf(logline + n, LogLineSize - n, fmt, args); // 加入变量的位置，char[]是指针
    va_end(args);
    int len = n + m;                                       // log总长
    LogBuffer *currentlogbuffer = nullptr;                 // log缓存
    std::map<std::thread::id, LogBuffer *>::iterator iter; // map表，存放不同的缓存
    {
        std::lock_guard<std::mutex> lock(log_mtx); // logbuf锁，放置线程串扰
        iter = threadbufmap.find(tid);
        if (iter != threadbufmap.end())
        {
            currentlogbuffer = iter->second; // 不是最后一个buf缓存，取出他的logbuf
        }
        else // 是最后一个缓存，new一个buflog
        {
            threadbufmap[tid] = currentlogbuffer = new LogBuffer(BUFSIZE, logbuf_num++);
            buf_num++;
            std::cout << "-----new buff------" << std::endl;
        }
    }
    if (currentlogbuffer->GetAvailLen() >= len && currentlogbuffer->GetState() == LogBuffer::BufState::FREE) // logbuf足够，写入缓存
    {
        currentlogbuffer->append(logline, len);
    }
    else // 空间不足，写文件
    {
        if (currentlogbuffer->GetState() == LogBuffer::BufState::FREE) // 空闲
        {
            currentlogbuffer->SetState(LogBuffer::BufState::FLUSH); // 写状态
            {
                std::lock_guard<std::mutex> lock(flush_mtx); // 上锁
                flushbufqueue.push(currentlogbuffer);        // 添加到队列的顶部，push_back是添加到尾部
            }
            flushcond.notify_one();                     //???   //唤醒一个线程
            std::lock_guard<std::mutex> lock(free_mtx); // free队列取buf
            if (!freeBufQueue.empty())                  // free队列为不空
            {
                currentlogbuffer = freeBufQueue.front();
                freeBufQueue.pop();
                log_insde->log_ins_append("change buf-->%d\n", currentlogbuffer->get_logbuf_num());
                std::cout << "change buf-->" << currentlogbuffer->get_logbuf_num() << std::endl;
            }
            else // free为空
            {
                if (buf_num * BUFSIZE < MEM_LIMIT) // 检查是否到达内存限制
                {
                    currentlogbuffer = new LogBuffer(BUFSIZE, logbuf_num++); // 没有，就新建buf
                    buf_num++;
                    log_insde->log_ins_append("new buf-->%d\n", logbuf_num);
                    std::cout << "new buf" << logbuf_num << std::endl;
                }
                else // 满了，就。。。
                {

                    std::cout << "buf num ==max drop log" << std::endl;
                    return; // 返回，TODO 优化
                }
            }
            currentlogbuffer->append(logline, len); // 写入file
            {
                std::lock_guard<std::mutex> lock2(log_mtx); // 解锁
                iter->second = currentlogbuffer;            // 配置buf缓存
            }
        }
        else // 状态为flush
        {
            std::lock_guard<std::mutex> lock(free_mtx); // 上锁
            if (!freeBufQueue.empty())                  // 不为空
            {
                currentlogbuffer = freeBufQueue.front(); // 取出第一个buf
                freeBufQueue.pop();                      // 删除第一个部分，也就是刚刚取出的
                {
                    std::lock_guard<std::mutex> lock2(log_mtx); // 上锁
                    iter->second = currentlogbuffer;            // 用新的buf
                }
            }
        }
    }
}

void log_ins::log_ins_append(const char *fmt, ...)
{
    char logline[64];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(logline, 64, fmt, args); // 加入变量的位置，char[]是指针
    va_end(args);
    if (log_ins_usedlen + len < Log_Ins_size)
    {
        memcpy(log_ins_buf + log_ins_usedlen, logline, len);
        log_ins_usedlen += len;
    }
    else
    {
        {
            std::lock_guard<std::mutex> lock(log_ins_mtx);
            fwrite(log_ins_buf, 1, log_ins_usedlen, fp_ins);
            log_ins_usedlen = 0;
        }
        memcpy(log_ins_buf + log_ins_usedlen, logline, len);
        log_ins_usedlen += len;
    }
}

void logger::Flush()
{
    start_flag = true;
    while (true)
    {
        LogBuffer *p;
        {
            std::unique_lock<std::mutex> lock(flush_mtx);
            while (flushbufqueue.empty() && start_flag)
            {
                flushcond.wait(lock);
            }
            if (flushbufqueue.empty() && start_flag == false)
            {
                return;
            }
            p = flushbufqueue.front();
            flushbufqueue.pop();
        }
        p->FlushTofile(fp);
        p->SetState(LogBuffer::BufState::FREE);
        {
            std::lock_guard<std::mutex> lock(free_mtx);
            freeBufQueue.push(p);
        }
    }
}

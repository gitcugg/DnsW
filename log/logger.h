#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <string.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#define LogLineSize 1024
#define Log_Ins_size 4096 * 1
#define BUFSIZE 2 * 1024    // 2K
#define MEM_LIMIT 64 * 1024 // 64K

#define LOG_INIT(logdir, lev)                     \
    do                                            \
    {                                             \
        logger::GetInstance()->Init(logdir, lev); \
    } while (0)

#define LOG(level, fmt, ...)                                                                          \
    do                                                                                                \
    {                                                                                                 \
        if (logger::GetInstance()->GetLevel() <= level)                                               \
        {                                                                                             \
            logger::GetInstance()->Append(level, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__); \
        }                                                                                             \
    } while (0)

enum LoggerLevel
{
    DNS = 0,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class log_ins
{
private:
    FILE *fp_ins;
    char *log_ins_buf;
    int log_ins_usedlen = 0;
    std::mutex log_ins_mtx;

public:
    log_ins(/* args */);
    static log_ins *GetInstance() // modify
    {
        static log_ins log_ins;
        return &log_ins;
    }
    ~log_ins();
    void log_ins_append(const char *fmt, ...);
};

class LogBuffer
{
private:
    uint32_t BufSize = 0;
    uint32_t UsedLen = 0;
    int state;
    char *logbuffer;
    int logbuf_num;
    log_ins *log_insde = log_ins::GetInstance();

public:
    enum BufState
    {
        FREE = 0,
        FLUSH = 1
    };

    LogBuffer(uint32_t bufsize, int _logbuf_num);
    ~LogBuffer();
    int GetUsedLen()
    {
        return UsedLen;
    }
    int GetAvailLen()
    {
        return BUFSIZE - UsedLen;
    }
    int GetState()
    {
        return state;
    }
    void SetState(BufState _state)
    {
        state = _state;
    }
    int get_logbuf_num()
    {
        return logbuf_num;
    }
    void append(const char *log, int len);
    void FlushTofile(FILE *fp);
};

class logger
{
private:
    int level;                                           // 日志等级
    FILE *fp;                                            // 文件指针 TODO 改ofstream
    std::map<std::thread::id, LogBuffer *> threadbufmap; //???
    std::mutex log_mtx;                                  // 同步锁
    int buf_num;                                         // 缓冲区数量
    std::mutex flush_mtx;                                // 写文件锁
    std::condition_variable flushcond;                   // 阻塞线程，等待通知
    std::queue<LogBuffer *> flushbufqueue;               // flush队列
    std::mutex free_mtx;                                 // free锁
    std::queue<LogBuffer *> freeBufQueue;                // free队列
    std::thread flushThread;                             // 写文件线程
    bool start_flag;
    char save_ymdhms[64]; // 日志时刻
    static logger *m_instance;
    int logbuf_num = 0;
    log_ins *log_insde = log_ins::GetInstance();

public:
    enum loggerLevel
    {
        DNS = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        LEVEL_COUNT
    };
    logger(/* args */);
    ~logger();
    // 单例模式
    static logger *GetInstance() // modify
    {
        static logger logger;
        return &logger;
    }
    void Init(const char *logDir, LoggerLevel _level);
    int GetLevel()
    {
        return level;
    }

    void Append(int _level, const char *File, int line, const char *func, const char *fmt, ...);
    void Flush(); // buf写入到文件
};

#endif
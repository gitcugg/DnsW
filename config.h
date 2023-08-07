// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

// 定义DNS服务器的默认配置信息
const int DEFAULT_UDP_PORT = 53;                            // UDP套接字监听的端口号
const int DEFAULT_TCP_PORT = 53;                            // TCP套接字监听的端口号
const int DEFAULT_DOT_PORT = 853;                           // DoT套接字监听的端口号
const int DEFAULT_UPSTREAM_PORT = 53;                       // TCP套接字监听的端口号
const int DEFAULT_LOCAL_PORT = 53;                          // TCP套接字监听的端口号
const std::string DEFAULT_LOG_FILE = "dns.log";             // 日志文件的路径
const std::string DEFAULT_UPSTREAM_SERVER = "119.29.29.29"; // 上游DNS服务器的地址
const int DEFAULT_CACHE_SIZE = 1000;                        // 缓存的最大容量

// 定义DNS服务器的配置结构体，用于存储从配置文件中读取的配置信息
struct Config
{
    int udp_port; // UDP套接字监听的端口号
    int tcp_port; // TCP套接字监听的端口号
    int dot_port; // DoT套接字监听的端口号
    int upstream_port;
    int local_port;
    std::string log_file;                      // 日志文件的路径
    std::vector<std::string> upstream_servers; // 上游DNS服务器的地址列表
    int cache_size;                            // 缓存的最大容量

    // 构造函数，初始化为默认配置信息
    Config()
    {
        udp_port = DEFAULT_UDP_PORT;
        tcp_port = DEFAULT_TCP_PORT;
        dot_port = DEFAULT_DOT_PORT;
        log_file = DEFAULT_LOG_FILE;
        upstream_port = DEFAULT_UPSTREAM_PORT;
        local_port = DEFAULT_LOCAL_PORT;
        upstream_servers.push_back(DEFAULT_UPSTREAM_SERVER);
        cache_size = DEFAULT_CACHE_SIZE;
    }
};

#endif

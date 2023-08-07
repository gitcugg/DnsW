// net.h
#ifndef NET_H
#define NET_H

#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <arpa/inet.h>
#include "config.h"

// 定义网络模块的类，用于创建和管理不同类型的套接字
class Net
{
private:
    Config config;
    int udp_sock;                    // UDP套接字的文件描述符
    int tcp_sock;                    // TCP套接字的文件描述符
    int dot_sock;                    // DoT套接字的文件描述符
    std::vector<int> upstream_socks; // 上游DNS服务器套接字的文件描述符列表
public:
    // 构造函数，根据配置信息创建不同类型的套接字，并连接到上游DNS服务器
    Net(const Config &config);

    // 析构函数，关闭所有的套接字并释放资源
    ~Net();

    // 获取UDP套接字的文件描述符
    int get_udp_sock() const;

    // 获取TCP套接字的文件描述符
    int get_tcp_sock() const;

    // 获取DoT套接字的文件描述符
    int get_dot_sock() const;

    // 获取上游DNS服务器套接字的文件描述符列表
    const std::vector<int> &get_upstream_socks() const;

    // 从指定的套接字读取数据，并返回读取到的数据和地址信息
    std::pair<std::vector<char>, std::string> read_from_sock(int sock);

    // 向指定的套接字写入数据，并返回写入是否成功
    bool write_to_sock(int sock, const std::vector<char> &data, const std::string &addr = "");
    std::vector<char> recv_data(int sock, sockaddr_in &cli_addr);
    int create_upstream_socket(const std::string &ip);
    int create_local_socket();
    void send_data(int sock, const std::vector<char> &data, sockaddr_in &cli_addr);
};

#endif

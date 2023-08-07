// main.cpp
#include "net.h"
#include "epoll.h"
#include "message.h"
#include "cache.h"
#include <iostream>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <map>
#include <hiredis/hiredis.h>
#include "log/logger.h"

void set_nonblocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

int main()
{
    LOG_INIT(".", LoggerLevel::DNS); // 日志初始化
    std::cout << "RAT_DNS_INIT" << std::endl;
    LOG(LoggerLevel::INFO, "RAT_DNS_INIT\n", 1);
    Config _config;   // 创建一个 Config 类的对象，使用默认参数或者使用 JSON 文件作为参数
    Net net(_config); // 创建一个 Net 类的对象，传递 Config 类的对象作为参数
    // Net net;     // 创建一个网络模块对象
    Epoll epoll; // 创建一个epoll模块对象
    Cache cache; // 创建一个缓存模块对象
    std::string supstream_ip = "223.6.6.6";
    std::map<uint16_t, sockaddr_in> client_map;

    int local_sock = net.create_local_socket(); // 创建一个本地DNS服务器fd
    if (local_sock < 0)
    {                                                                  // 如果创建失败
        std::cerr << "Failed to create local socket\n";                //
        LOG(LoggerLevel::ERROR, "Failed to create local socket\n", 1); //
        return -1;                                                     // 返回-1，表示程序异常退出
    }

    int upstream_sock = net.create_upstream_socket(supstream_ip); // 创建一个上游DNS服务器fd，并连接到公共DNS服务器
    if (upstream_sock < 0)
    {                                                                     // 如果创建失败
        std::cerr << "Failed to create upstream socket\n";                //
        LOG(LoggerLevel::ERROR, "Failed to create upstream socket\n", 1); //
        return -1;                                                        // 返回-1，表示程序异常退出
    }
    set_nonblocking(local_sock);
    set_nonblocking(upstream_sock);
    epoll.add_sock(local_sock);    // 将本地DNS服务器fd注册到epoll
    epoll.add_sock(upstream_sock); // 将上游DNS服务器fd注册到epoll实

    sockaddr_in upstream_addr;
    upstream_addr.sin_family = AF_INET;
    upstream_addr.sin_port = htons(53);
    upstream_addr.sin_addr.s_addr = inet_addr(supstream_ip.c_str());

    while (true)
    {                                          // 等待发生事件
        std::vector<int> socks = epoll.wait(); //
        for (int sock : socks)
        { // 遍历发生事件的fd列表中的每个fd

            sockaddr_in cli_addr;
            if (sock == local_sock)
            { // 如果fd是本地DNS服务器fd，表示有客户端发送了DNS请求
                std::cout << "SOCK_TYPE -> local_sock" << std::endl;

                std::vector<char> data = net.recv_data(sock, cli_addr); // 接收客户端发送的数据，并赋值给数据向量
                // std::pair<std::vector<char>, std::string> getData = net.read_from_sock(sock);
                Message request(data); // 根据数据向量解析DNS请求报文，并创建一个报文对象

                std::vector<Question> &questions = request.get_questions(); // 获取DNS请求报文中的问题部分对象列表

                if (!questions.empty())
                {                                      // 如果问题部分对象列表不为空
                    Question &question = questions[0]; // 获取问题部分对象列表中的第一个问题对象
                    std::cout << "QUESTION: " << question.qname << std::endl;

                    // 缓存模式
                    std::vector<ResourceRecord> answers = cache.lookup(question.qname, question.qtype); // 根据问题对象中的查询域名和查询类型，在缓存中查找匹配的资源记录，并赋值给答案部分对象列表

                    if (!answers.empty())
                    {                                         // 如果答案部分对象列表不为空，表示缓存中有匹配的资源记录
                        Header header = request.get_header(); // 获取DNS请求报文中的头部对象
                        header.flags |= 0x8000;               // 将设置为响应类型
                        header.ancount = answers.size();      // 将头部对象中的答案部分记录数字段设置为答案部分对象列表的大小

                        Message response(header, questions, answers, {}, {}); // 根据头部对象、问题部分对象列表和答案部分对象列表构造DNS响应报文，并创建一个报文对象

                        std::vector<char> dataRespone = response.to_data(); // 将DNS响应报文转换为数据向量，并赋值给数据向量

                        net.send_data(local_sock, dataRespone, cli_addr); // 将数据向量发送给客户端
                        std::cout << "CACHE_MODE: " << question.qname << std::endl;
                        LOG(LoggerLevel::DNS, "GET DNS Request socket CacheMode ID:%u, IP:%s, Qname->%s, \n", request.get_header().id, inet_ntoa(cli_addr.sin_addr), question.qname.c_str()); // 输出接受信息
                    }
                    else
                    {                                                      // 如果答案部分对象列表为空，表示缓存中没有匹配的资源记录
                        net.send_data(upstream_sock, data, upstream_addr); // 将数据向量转发给上游DNS服务器
                        client_map[request.get_header().id] = cli_addr;
                        std::cout << "GET LOCAL ID----> " << request.get_header().id << std::endl;
                        LOG(LoggerLevel::DNS, "GET DNS Request socket ID:%u, IP:%s, Qname->%s \n", request.get_header().id, inet_ntoa(cli_addr.sin_addr), question.qname.c_str()); // 输出接受信息
                    }

                    // 直询模式
                    // net.send_data(upstream_sock, data, upstream_addr); // 将数据向量转发给上游DNS服务器
                }
            }
            else if (sock == upstream_sock)
            { // 如果上游DNS服务器fd，表示上游DNS服务器发送了DNS响应
                std::cout << "SOCK_TYPE -> upstream_sock" << std::endl;
                std::vector<char> data = net.recv_data(sock, upstream_addr); // 接收上游DNS服务器发送的数据
                // std::pair<std::vector<char>, std::string> getData = net.read_from_sock(sock); // 接收上游DNS服务器发送的数据，并赋值给数据向量
                Message response(data); // 根据数据向量解析DNS响应报文，并创建一个报文对象
                if (response.get_header().ancount > 0)
                {
                    // response.getRespone(data);
                    std::vector<ResourceRecord> &answers = response.get_answers(); // 获取DNS响应报文中的答案部分对象列表
                    if (!answers.empty())
                    { // 如果答案部分对象列表不为空
                        // response.to_ip();

                        cache.update(answers); // 根据答案部分对象列表，在缓存中插入或更新资源记录，并设置过期时间
                    }
                    std::map<uint16_t, sockaddr_in>::iterator itr;
                    uint16_t messageId = response.get_header().id;
                    std::cout << "GET upstream ID----> " << messageId << std::endl;
                    itr = client_map.find(messageId);
                    if (itr != client_map.end())
                    {
                        sockaddr_in getMapClient = itr->second;
                        net.send_data(local_sock, data, getMapClient); // 将数据转发给客户端
                        // client_map.erase(itr);                                                                                                                                                                      // TODO 增加一个ID<-->CLIENT生存时间
                        LOG(LoggerLevel::DNS, "Send DNS socket ID:%u, clientIP:%s, Qname->%s,QIp->%s \n", messageId, inet_ntoa(getMapClient.sin_addr), answers.data()->name.c_str(), response.get_ipList().data()); // 输出接受信息
                    }
                    else
                    {
                        LOG(LoggerLevel::WARNING, "ERR-> NO ClientAddr :ID:%u, Qname->%s\n", response.get_header().id, response.get_questions()[0].qname.c_str());
                        // LOG(LoggerLevel::ERROR, "ERR-> NO ClientAddr", 1);
                    }
                }
                else
                {
                    std::vector<ResourceRecord> &answers = response.get_answers();
                    std::map<uint16_t, sockaddr_in>::iterator itr;
                    uint16_t messageId = response.get_header().id;
                    itr = client_map.find(messageId);
                    if (itr != client_map.end())
                    {
                        sockaddr_in getMapClient = itr->second;
                        net.send_data(local_sock, data, getMapClient); // 将数据转发给客户端
                        // client_map.erase(itr);// TODO 增加一个生存时间
                        LOG(LoggerLevel::DNS, "ERR-> Send Empty DNS socket ID:%u, clientIP:%s, Qname->%s\n", messageId, inet_ntoa(getMapClient.sin_addr), response.get_questions()[0].qname.c_str()); // 输出接受信息
                    }
                    else
                    {
                        LOG(LoggerLevel::WARNING, "ERR-> Get upstream DNS Empty  & No Client:ID:%u\n", response.get_header().id);
                    }
                }
            }
        }
    }

    return 0; // 返回0，表示程序正常退出
}

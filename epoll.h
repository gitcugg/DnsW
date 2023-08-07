// epoll.h
#ifndef EPOLL_H
#define EPOLL_H

#include <vector>

// 定义epoll模块的类，用于创建和管理epoll实例
class Epoll
{
private:
    int epfd; // epoll实例的文件描述符
public:
    Epoll();

    ~Epoll();

    void add_sock(int sock);
    void del_sock(int sock); // 将指定的fd从epoll实例中删除
    std::vector<int> wait();
};

#endif

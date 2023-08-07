// epoll.cpp
#include "epoll.h"
#include <sys/epoll.h>
#include <unistd.h>

Epoll::Epoll()
{
    epfd = epoll_create(1); // 创建一个epoll实例，并获取fd
}

Epoll::~Epoll()
{
    close(epfd); // 关闭epoll实例
}

// 将指定的fd注册到epoll实例中
void Epoll::add_sock(int sock)
{
    struct epoll_event ev;
    ev.data.fd = sock;                         // 设置fd
    ev.events = EPOLLIN;                       // 设置事件类型为可读
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev); // 将fd和事件类型添加到epoll实例中
}

// 将指定的fd从epoll实例中删除
void Epoll::del_sock(int sock)
{
    struct epoll_event ev;
    ev.data.fd = sock;                         // 设置fd
    ev.events = EPOLLIN;                       // 设置事件类型为无效
    epoll_ctl(epfd, EPOLL_CTL_DEL, sock, &ev); // 将fd和事件类型从epoll实例中删除
}

// 等待epoll实例中的fd发生事件，并返回发生事件的fd列表
std::vector<int> Epoll::wait()
{
    std::vector<int> socks;
    struct epoll_event events[10];            // 10
    int n = epoll_wait(epfd, events, 10, -1); // 无限等待
    if (n > 0)
    { // 如果有事件发生
        for (int i = 0; i < n; i++)
        {                                       // 遍历发生事件的个数
            socks.push_back(events[i].data.fd); // 添加到列表中
        }
    }
    return socks; // 返回发生事件的fd列表
}

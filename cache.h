// cache.h
#ifndef CACHE_H
#define CACHE_H

#include "message.h"
#include <map>
#include <list>

// 定义缓存模块的类，用于缓存DNS报文的答案部分
class Cache
{
private:
    std::map<std::string, std::list<ResourceRecord>> cache; // 用于存储缓存数据的map
public:
    Cache();
    ~Cache();

    // 根据查询域名和查询类型，在缓存中查找匹配的资源记录，并返回资源记录列表
    std::vector<ResourceRecord> lookup(const std::string &qname, uint16_t qtype);

    // 根据DNS报文的答案部分，在缓存中插入或更新资源记录，并设置过期时
    void update(const std::vector<ResourceRecord> &answers);
};

#endif

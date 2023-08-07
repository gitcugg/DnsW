#include "cache.h"
#include <ctime>
#include <iostream>

Cache::Cache()
{
    cache.clear(); // 清空缓存数据
}

Cache::~Cache()
{
    cache.clear(); // 清空缓存数据
}

// 根据查询域名和查询类型，在缓存中查找匹配的资源记录，并返回资源记录列表
std::vector<ResourceRecord> Cache::lookup(const std::string &qname, uint16_t qtype)
{
    std::vector<ResourceRecord> results; // 存储匹配的资源记录列表

    auto it = cache.find(qname); // 在缓存数据中查找查询域名对应的资源记录列表
    if (it != cache.end())
    {                                                    // 如果找到了查询域名对应的资源记录列表
        std::list<ResourceRecord> &records = it->second; // 获取资源记录列表的引用
        for (auto jt = records.begin(); jt != records.end();)
        {                                 // 遍历资源记录列表中的每个资源记录
            ResourceRecord &record = *jt; // 获取资源记录的引用
            if (record.ttl == 0)
            {                           // 如果资源记录已经过期
                jt = records.erase(jt); // 从资源记录列表中删除该资源记录，并更新itr
            }
            else
            { // 如果资源记录未过期
                if (qtype == record.type || qtype == 255)
                {                              // 如果查询类型与资源记录类型匹配，或者查询类型为任意类型（255）
                    results.push_back(record); // 将该资源记录添加到匹配的资源记录列表中
                }
                jt++; // 更新itr位置
            }
        }
        if (records.empty())
        {                    // 如果资源记录列表为空，表示所有资源记录都已过期或删除
            cache.erase(it); // 从缓存数据中删除该查询域名对应的条目，并释放内存空间
        }
    }

    return results; // 返回匹配的资源记录列表
}

// 根据DNS报文的答案部分，在缓存中插入或更新资源记录，并设置过期时间
void Cache::update(const std::vector<ResourceRecord> &answers)
{
    time_t now = time(nullptr); // 获取当前时间

    for (const auto &answer : answers)
    {                                      // 遍历DNS报文答案部分中的每个资源记录
        ResourceRecord record = answer;    // 创建一个资源记录对象，存储答案部分中的各个字段
        auto it = cache.find(record.name); // 在缓存数据中查找该资源记录域名对应的资源记录列表，并获取itr

        if (it != cache.end())
        {                                                    // 如果找到了该资源记录域名对应的资源记录列表
            std::list<ResourceRecord> &records = it->second; // 获取资源记录列表的引用
            bool found = false;                              // 创建一个布尔变量，标记是否找到了相同的资源记录
            for (auto &r : records)
            { // 遍历资源记录列表中的每个资源记录
                if (r.type == record.type && r.class_ == record.class_ && r.rdata == record.rdata)
                {                       // 如果资源记录类型、类别和数据与答案部分中的资源记录相同
                    r.ttl = record.ttl; // 更新该资源记录的生存时间为答案部分中的资源记录的生存时间
                    found = true;       // 将布尔变量设置为真，表示找到了相同的资源记录
                    break;              // 跳出循环，结束查找
                }
            }
            if (!found)
            {                              // 如果没有找到相同的资源记录
                records.push_back(record); // 将答案部分中的资源记录添加到资源记录列表中
            }
        }
        else
        {                                      // 如果没有找到该资源记录域名对应的资源记录列表
            std::list<ResourceRecord> records; // 创建一个新的资源记录列表
            records.push_back(record);         // 将答案部分中的资源记录添加到新的资源记录列表中
            cache[record.name] = records;      // 将新的资源记录列表添加到缓存数据中，并以该资源记录域名为键
        }
    }

    for (auto &pair : cache)
    {                                                     // 遍历缓存数据中的每个键值对
        std::list<ResourceRecord> &records = pair.second; // 获取资源记录列表的引用
        for (auto &record : records)
        { // 遍历资源记录列表中的每个资源记录
            if (record.ttl > 0)
            {                      // 如果该资源记录未过期
                record.ttl -= now; // 将该资源记录的生存时间减去当前时间，得到剩余时间
                if (record.ttl < 0)
                {                   // 如果剩余时间小于0，表示该资源记录已过期
                    record.ttl = 0; // 将该资源记录的生存时间设置为0，表示已过期
                }
            }
        }
    }
}

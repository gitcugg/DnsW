// message.h
#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <vector>
#include <cstdio>

// 定义DNS报文头部的结构体，存储DNS报文头部的各个字段
struct Header
{
    uint16_t id;      // 标识符，标识DNS请求和响应
    uint16_t flags;   // 标志位，表示DNS报文的类型和状态
    uint16_t qdcount; // 问题部分的记录数
    uint16_t ancount; // 答案部分的记录数
    uint16_t nscount; // 权威部分的记录数
    uint16_t arcount; // 附加部分的记录数
};

// 定义DNS报文问题部分的结构体，存储DNS报文问题部分的各个字段
struct Question
{
    std::string qname; // 查询域名
    uint16_t qtype;    // 查询类型
    uint16_t qclass;   // 查询类别
};

// 定义DNS报文资源记录的结构体，存储DNS报文答案部分、权威部分和附加部分的各个字段
struct ResourceRecord
{
    std::string name;        // 资源记录域名
    uint16_t type;           // 资源记录类型
    uint16_t class_;         // 资源记录类别
    uint32_t ttl;            // 资源记录生存时间
    uint16_t rdlength;       // 资源记录数据长度
    std::vector<char> rdata; // 资源记录数据
    // std::vector<char> IP;
};

enum class RRType : uint16_t
{
    A = 1,           // IPv4地址
    NS = 2,          // 名称服务器
    CNAME = 5,       // 别名
    SOA = 6,         // 起始授权
    PTR = 12,        // 指针
    MX = 15,         // 邮件交换
    TXT = 16,        // 文本
    AAAA = 28,       // IPv6地址
    SRV = 33,        // 服务定位
    NAPTR = 35,      // 命名权限指针
    OPT = 41,        // 可选
    DS = 43,         // 委派签名
    RRSIG = 46,      // 资源记录签名
    NSEC = 47,       // 下一安全
    DNSKEY = 48,     // DNS密钥
    NSEC3 = 50,      // 下一安全3
    NSEC3PARAM = 51, // 下一安全3参数
    TLSA = 52,       // TLSA证书关联
    CAA = 257,       // CA授权
};

// 定义DNS报文的类，解析和构造DNS报文
class Message
{
private:
    Header header;                           // DNS报文头部对象
    std::vector<Question> questions;         // DNS报文问题部分对象列表
    std::vector<ResourceRecord> answers;     // DNS报文答案部分对象列表
    std::vector<ResourceRecord> authorities; // DNS报文权威部分对象列表
    std::vector<ResourceRecord> additionals; // DNS报文附加部分对象列表
    std::vector<char> ipList;

public:
    Message(const std::vector<char> &data);

    Message(const Header &header, const std::vector<Question> &questions, const std::vector<ResourceRecord> &answers, const std::vector<ResourceRecord> &authorities, const std::vector<ResourceRecord> &additionals);

    Header &get_header();

    std::vector<Question> &get_questions();

    std::vector<ResourceRecord> &get_answers();

    std::vector<ResourceRecord> &get_authorities();

    std::vector<ResourceRecord> &get_additionals();
    std::vector<char> &get_ipList();

    std::vector<char> to_data();
    std::string to_ip();
    std::string to_Type(RRType _type, void *_rdata);
    std::vector<ResourceRecord> getRespone(const std::vector<char> &data);
};

#endif

// message.cpp
#include "message.h"
#include <arpa/inet.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <iomanip>

// 解析原始数据中的查询域名，返回查询域名字符串，同时更新指针
std::string parse_qname(const char *&p, const char *base)
{
    std::string qname; // 存储查询域名字符串

    while (*p != 0)
    { // 当指针指向的字符不为0时，表示查询域名未结束
        if ((*p & 0xc0) == 0xc0)
        {                                                       // 如果指针指向的字符的前两位为11，表示使用了压缩指针
            uint16_t offset = ntohs(*((uint16_t *)p)) & 0x3fff; // 将指针指向的两个字节转换为网络字节序，去掉前两位，得到偏移量
            const char *q = base + offset;                      // 创建一个新的字符指针，指向偏移量对应的
            qname += parse_qname(q, base);                      // 递归解析新的字符指针指向的查询域名，将结果追加到查询域名字符串中
            p += 2;                                             // 更新原始字符指针，跳过两个字节
            break;                                              // 跳出循环，结束解析
        }
        else
        {                                             // 如果指针指向的字符的前两位不为11，表示使用了普通标签
            uint8_t len = *p;                         // 获取指针指向的字符的值，表示标签的长度
            qname += std::string(p + 1, p + 1 + len); // 将指针后面len个字符复制到查询域名字符串中，表示标签的内容
            p += 1 + len;                             // 更新指针，跳过len+1个字节
            if (*p != 0)
            {                 // 如果指针指向的字符不为0，表示查询域名未结束
                qname += "."; // 在查询域名字符串后面追加一个点号，表示标签之间的分隔符
            }
        }
    }

    if (*p == 0)
    {        // 如果指针指向的字符为0，表示查询域名结束
        p++; // 更新指针，跳过一个字节
    }

    // while (*p != 0)
    // {                         // 遍历所有的标签，直到遇到空终止符
    //     int len = *p;         // 获取当前标签的长度
    //     p++;                  // 将指针移动到下一个
    //     qname.append(p, len); // 将标签内容追加到域名字符串中
    //     p += len;             // 将指针移动到下一个
    //     if (*p != 0)
    //     { // 如果还没有到达空终止符，就在域名字符串中添加一个"."
    //         qname.append(".");
    //     }
    // }
    // p++; // 将指针移动到下一个

    return qname; // 返回查询域名字符串
}

// 将查询域名字符串编码为原始数据，返回原始数据向量
std::vector<char> encode_qname(const std::string &qname)
{
    std::vector<char> data; // 存储原始数据向量

    size_t start = 0;             // 创建一个大小类型变量，存储标签的起始
    size_t end = qname.find('.'); // 创建一个大小类型变量，存储标签的结束

    while (end != std::string::npos)
    {                                                                        // 当结束不为无效值时，表示标签未结束
        uint8_t len = end - start;                                           // 计算标签的长度
        data.push_back(len);                                                 // 将标签的长度添加到原始数据向量中
        data.insert(data.end(), qname.begin() + start, qname.begin() + end); // 将标签的内容添加到原始数据向量中
        start = end + 1;                                                     // 更新起始，跳过点号
        end = qname.find('.', start);                                        // 更新结束，查找下一个点号
    }

    uint8_t len = qname.size() - start;                          // 计算最后一个标签的长度
    data.push_back(len);                                         // 将最后一个标签的长度添加到原始数据向量中
    data.insert(data.end(), qname.begin() + start, qname.end()); // 将最后一个标签的内容添加到原始数据向量中

    data.push_back(0); // 在原始数据向量末尾添加一个0，表示查询域名结束

    return data; // 返回原始数据向量
}

// 构造函数，根据原始数据解析DNS报文，初始化各个字段
Message::Message(const std::vector<char> &data)
{
    const char *p = data.data(); // 创建一个字符指针，指向原始数据向量的首地址

    memcpy(&header, p, sizeof(header)); // 将原始数据中的前12个字节复制到头部结构体中，更新指针
    p += sizeof(header);                // 更新指针

    header.id = ntohs(header.id); // 转换为主机字节序
    header.flags = ntohs(header.flags);
    header.qdcount = ntohs(header.qdcount);
    header.ancount = ntohs(header.ancount);
    header.nscount = ntohs(header.nscount);
    header.arcount = ntohs(header.arcount);

    for (int i = 0; i < header.qdcount; i++)
    {                                                         // 遍历问题部分记录数
        Question question;                                    // 创建一个问题结构体对象，存储问题部分的各个字段
        question.qname = parse_qname(p, data.data());         // 解析原始数据中的查询域名，赋值给问题结构体对象的查询域名字段，更新指针
        memcpy(&question.qtype, p, sizeof(question.qtype));   // 将原始数据中的两个字节复制到问题结构体对象的查询类型字段，更新指针
        p += sizeof(question.qtype);                          // 更新指针
        memcpy(&question.qclass, p, sizeof(question.qclass)); // 将原始数据中的两个字节复制到问题结构体对象的查询类别字段，更新指针
        p += sizeof(question.qclass);                         // 更新指针

        question.qtype = ntohs(question.qtype);   // 将问题结构体对象中的查询类型字段转换为主机字节序
        question.qclass = ntohs(question.qclass); // 将问题结构体对象中的查询类别字段转换为主机字节序

        questions.push_back(question); // 将问题结构体对象添加到问题部分对象列表中
    }

    for (int i = 0; i < header.ancount; i++)
    {                                              // 遍历答案部分记录数
        ResourceRecord answer;                     // 创建一个资源记录结构体对象，存储答案部分的各个字段
        answer.name = parse_qname(p, data.data()); // 解析原始数据中的资源记录域名，赋值给资源记录结构体对象的域名字段，更新指针
        p--;
        memcpy(&answer.type, p, sizeof(answer.type)); // 将原始数据中的两个字节复制到资源记录结构体对象的类型字段，更新指针
        p += sizeof(answer.type);                     // 更新指针
        memcpy(&answer.class_, p, sizeof(answer.class_));
        p += sizeof(answer.class_);
        memcpy(&answer.ttl, p, sizeof(answer.ttl));
        p += sizeof(answer.ttl);
        memcpy(&answer.rdlength, p, sizeof(answer.rdlength));
        p += sizeof(answer.rdlength);     // 更新指针
        answer.type = ntohs(answer.type); // 转换为主机字节序
        answer.class_ = ntohs(answer.class_);
        answer.ttl = ntohl(answer.ttl);
        answer.rdlength = ntohs(answer.rdlength);

        answer.rdata.resize(answer.rdlength);            // 调整资源记录结构体对象的数据向量的大小为数据长度字段的值
        memcpy(answer.rdata.data(), p, answer.rdlength); // 将原始数据中的数据复制到资源记录结构体对象的数据向量中，更新指针
        p += answer.rdlength;                            // 更新指针
        std::string Type_ss = to_Type((RRType)answer.type, answer.rdata.data());
        // if (answer.type == 1 && answer.class_ == 1 && answer.rdlength) // 如果资源记录的类型是A，类别是IN，数据长度是4，表示这是一个IPv4地址
        // {
        //     // RRType getRtype = (RRType)answer.type;

        //     char ip[INET_ADDRSTRLEN];                                     // 定义一个变量存储IP地址的字符串表示
        //     inet_ntop(AF_INET, answer.rdata.data(), ip, INET_ADDRSTRLEN); // 将二进制格式转换为点分十进制格式
        //     // ip = std::string(ip);                       // 将字符串转换为std::string类型
        //     std::cout << "TYPE-->" << Type_ss << " IP: " << ip << std::endl;
        // }

        answers.push_back(answer); // 将资源记录结构体对象添加到答案部分对象列表中
    }

    for (int i = 0; i < header.nscount; i++)
    { // 遍历权威部分记录数
        ResourceRecord authority;
        authority.name = parse_qname(p, data.data());
        memcpy(&authority.type, p, sizeof(authority.type));
        p += sizeof(authority.type);
        memcpy(&authority.class_, p, sizeof(authority.class_));
        p += sizeof(authority.class_);
        memcpy(&authority.ttl, p, sizeof(authority.ttl));
        p += sizeof(authority.ttl);
        memcpy(&authority.rdlength, p, sizeof(authority.rdlength));
        p += sizeof(authority.rdlength);

        authority.type = ntohs(authority.type);
        authority.class_ = ntohs(authority.class_);
        authority.ttl = ntohl(authority.ttl);
        authority.rdlength = ntohs(authority.rdlength);

        authority.rdata.resize(authority.rdlength);
        memcpy(authority.rdata.data(), p, authority.rdlength);
        p += authority.rdlength;

        authorities.push_back(authority); // 将资源记录结构体对象添加到权威部分对象列表中
    }

    for (int i = 0; i < header.arcount; i++)
    {                                                  // 遍历附加部分记录数
        ResourceRecord additional;                     // 存储附加部分的各个字段
        additional.name = parse_qname(p, data.data()); // 原始数据中的资源记录域名
        memcpy(&additional.type, p, sizeof(additional.type));
        p += sizeof(additional.type);
        memcpy(&additional.class_, p, sizeof(additional.class_));
        p += sizeof(additional.class_);
        memcpy(&additional.ttl, p, sizeof(additional.ttl));
        p += sizeof(additional.ttl);
        memcpy(&additional.rdlength, p, sizeof(additional.rdlength));
        p += sizeof(additional.rdlength);

        additional.type = ntohs(additional.type); // 转换为主机字节序
        additional.class_ = ntohs(additional.class_);
        additional.ttl = ntohl(additional.ttl);
        additional.rdlength = ntohs(additional.rdlength);

        additional.rdata.resize(additional.rdlength);
        memcpy(additional.rdata.data(), p, additional.rdlength);
        p += additional.rdlength; // 更新指针

        additionals.push_back(additional); // 将资源记录结构体对象添加到附加部分对象列表中
    }
}

Message::Message(const Header &header, const std::vector<Question> &questions, const std::vector<ResourceRecord> &answers, const std::vector<ResourceRecord> &authorities, const std::vector<ResourceRecord> &additionals)
{
    this->header = header; // 赋值
    this->questions = questions;
    this->answers = answers;
    this->authorities = authorities;
    this->additionals = additionals;
}

// 一个函数，将数字转换为资源记录类型字符串
std::string Message::to_Type(RRType _type, void *_rdata)
{
    switch (_type)
    {
    case RRType::A:
    {
        char ip[INET_ADDRSTRLEN];                        // IPV4
        inet_ntop(AF_INET, _rdata, ip, INET_ADDRSTRLEN); // 将二进制格式转换为点分十进制格式
        this->ipList.insert(ipList.end(), ip, ip + INET_ADDRSTRLEN);
        this->ipList.push_back(' ');
        // ip = std::string(ip);
        std::cout << "TYPE-->A  IP: " << ip << std::endl;
    }
        return "A";
    case RRType::NS:
        return "NS";
    case RRType::CNAME:
        std::cout << "CNAME->" << std::endl;
        return "CNAME";
    case RRType::SOA:
        return "SOA";
    case RRType::PTR:
        return "PTR";
    case RRType::MX:
        return "MX";
    case RRType::TXT:
        return "TXT";
    case RRType::AAAA:
    {
        char ip[INET6_ADDRSTRLEN]; // IPV6
        inet_ntop(AF_INET6, _rdata, ip, sizeof(ip));
        std::cout << "TYPE-->AAAA  IP: " << ip << std::endl;
        this->ipList.insert(ipList.end(), ip, ip + INET_ADDRSTRLEN);
        this->ipList.push_back('+');
    }
        return "AAAA";
    case RRType::SRV:
        return "SRV";
    case RRType::NAPTR:
        return "NAPTR";
    case RRType::OPT:
        return "OPT";
    case RRType::DS:
        return "DS";
    case RRType::RRSIG:
        return "RRSIG";
    case RRType::NSEC:
        return "NSEC";
    case RRType::DNSKEY:
        return "DNSKEY";
    case RRType::NSEC3:
        return "NSEC3";
    case RRType::NSEC3PARAM:
        return "NSEC3PARAM";
    case RRType::TLSA:
        return "TLSA";
    case RRType::CAA:
        return "CAA";
    default:
        return "Unknown";
    }
}

std::vector<ResourceRecord> Message::getRespone(const std::vector<char> &data)
{
    std::vector<ResourceRecord> resourceRecord_Bak;
    std::string name;
    const char *p = data.data();
    // 跳过响应报文中的问题部分，因为我们已经知道了问题的域名
    memcpy(&header, p, sizeof(header)); // 将原始数据中的前12个字节复制到头部结构体中，更新指针

    p += sizeof(header); // 更新指针

    header.id = ntohs(header.id);           // 转换为主机字节序
    header.flags = ntohs(header.flags);     //
    header.qdcount = ntohs(header.qdcount); //
    header.ancount = ntohs(header.ancount); //
    header.nscount = ntohs(header.nscount); //
    header.arcount = ntohs(header.arcount); //
    // p += sizeof(Header);                    // 指向缓冲区中当前的指针
    while (*p != 0)
    {                        // 遍历所有的标签，直到遇到空终止符
        int len = *p;        // 获取当前标签的长度
        p++;                 // 更新指针
        name.append(p, len); // 将内容追加到域名
        p += len;            // 更新指针
        if (*p != 0)
        { // 如果还没有到达空终止符，就在域名字符串中添加一个"."
            name.append(".");
        }
    }
    p++; // 更新指针

    p += 2 * sizeof(ushort); // 跳过问题类型和类别

    std::string ip;

    // 解析响应报文中的答案部分，只处理第一个答案，忽略其他部分
    if (header.ancount > 0)
    {           // 如果答案数量大于0，表示有答案
        p += 2; // 跳过name字段，因为它是一个指针，将指针移动到下一个

        ushort type, _class, rdlength;                                                    // 定义一些变量存储type、class和rdlength字段
        unsigned int ttl;                                                                 //
        memcpy(&type, p, sizeof(ushort));                                                 // type字段
        memcpy(&_class, p + sizeof(ushort), sizeof(ushort));                              // class字段
        memcpy(&ttl, p + 2 * sizeof(ushort), sizeof(unsigned int));                       // ttl字段
        memcpy(&rdlength, p + 2 * sizeof(ushort) + sizeof(unsigned int), sizeof(ushort)); // rdlength字段

        // 将网络字节序转换为主机字节序
        type = ntohs(type);
        _class = ntohs(_class);
        ttl = ntohl(ttl);
        rdlength = ntohs(rdlength);

        p += 2 * sizeof(ushort) + sizeof(unsigned int) + sizeof(ushort); // 移动

        if (type == 1 && _class == 1 && rdlength == 4)
        { // IPv4地址
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, p, ip, INET_ADDRSTRLEN); // 将二进制格式转换为点分十进制格式
            // ip = std::string(ip);
            std::cout << "IP:" << ip << std::endl;
        }
    }
}

// 将资源记录中的数据转换为IP地址
std::string Message::to_ip()
{
    for (auto &itr : answers)
    {
        std::stringstream ss;
        std::cout << "TYPE--->" << itr.type << std::endl;
        if (questions[0].qtype == 1)
        { // A记录，IPv4地址
            for (size_t i = 0; i < itr.rdlength; i++)
            {
                ss << (int)(uint8_t)itr.rdata[i];
                if (i != itr.rdlength - 1)
                {
                    ss << ".";
                }
            }
            std::cout << "IP->" << ss.str() << std::endl;
            return ss.str();
        }
        else if (questions[0].qtype == 28)
        { // AAAA记录，IPv6地址
            for (size_t i = 0; i < itr.rdlength; i += 2)
            {
                ss << std::hex << std::setfill('0') << std::setw(4)
                   << ((int)(uint8_t)itr.rdata[i] << 8 | (int)(uint8_t)itr.rdata[i + 1]);
                if (i != itr.rdlength - 2)
                {
                    ss << ":";
                }
            }
            std::cout << "IP->" << ss.str() << std::endl;
            return ss.str();
        }
        else
        {
            std::cout << "Not an IP address" << std::endl;
            return "Not an IP address";
        }
    }
}

Header &Message::get_header()
{
    return header;
}

std::vector<Question> &Message::get_questions()
{
    return questions;
}

std::vector<ResourceRecord> &Message::get_answers()
{
    return answers;
}

std::vector<ResourceRecord> &Message::get_authorities()
{
    return authorities;
}

std::vector<ResourceRecord> &Message::get_additionals()
{
    return additionals;
}
std::vector<char> &Message::get_ipList()
{
    return ipList;
}

// 将DNS报文转换为原始数据，返回原始数据向量
std::vector<char> Message::to_data()
{
    std::vector<char> data; // 存储原始数据向量

    Header h = header; // 创建一个头部结构体对象，存储头部字段
                       // 转换为网络字节序
    h.id = htons(h.id);
    h.flags = htons(h.flags);
    h.qdcount = htons(h.qdcount);
    h.ancount = htons(h.ancount);
    h.nscount = htons(h.nscount);
    h.arcount = htons(h.arcount);

    data.resize(sizeof(h)); // 调整原始数据向量的大小为头部结构体对象的大小
    memcpy(data.data(), &h, sizeof(h));

    for (const auto &question : questions)
    {                          // 遍历问题部分对象列表
        Question q = question; // 创建一个问题结构体对象，存储问题部分的各个字段

        q.qtype = htons(q.qtype); // 转换为网络字节序
        q.qclass = htons(q.qclass);

        std::vector<char> qname_data = encode_qname(q.qname);
        data.insert(data.end(), qname_data.begin(), qname_data.end());
        data.resize(data.size() + sizeof(q.qtype) + sizeof(q.qclass));
        memcpy(data.data() + data.size() - sizeof(q.qtype) - sizeof(q.qclass), &q.qtype, sizeof(q.qtype));
        memcpy(data.data() + data.size() - sizeof(q.qclass), &q.qclass, sizeof(q.qclass));
    }

    for (const auto &answer : answers)
    {                              // 遍历答案部分对象列表
        ResourceRecord r = answer; // 创建一个资源记录结构体对象，存储答案部分的各个字段

        r.type = htons(r.type); // 换为网络字节序
        r.class_ = htons(r.class_);
        r.ttl = htonl(r.ttl);
        r.rdlength = htons(r.rdlength);

        std::vector<char> name_data = encode_qname(r.name);
        data.insert(data.end(), name_data.begin(), name_data.end());
        data.resize(data.size() + sizeof(r.type) + sizeof(r.class_) + sizeof(r.ttl) + sizeof(r.rdlength) + r.rdlength);
        memcpy(data.data() + data.size() - sizeof(r.type) - sizeof(r.class_) - sizeof(r.ttl) - sizeof(r.rdlength) - r.rdlength, &r.type, sizeof(r.type));
        memcpy(data.data() + data.size() - sizeof(r.class_) - sizeof(r.ttl) - sizeof(r.rdlength) - r.rdlength, &r.class_, sizeof(r.class_));
        memcpy(data.data() + data.size() - sizeof(r.ttl) - sizeof(r.rdlength) - r.rdlength, &r.ttl, sizeof(r.ttl));
        memcpy(data.data() + data.size() - sizeof(r.rdlength) - r.rdlength, &r.rdlength, sizeof(r.rdlength));
        memcpy(data.data() + data.size() - r.rdlength, r.rdata.data(), r.rdlength);
    }

    for (const auto &authority : authorities)
    {                                 // 遍历权威部分对象列表
        ResourceRecord r = authority; // 创建一个资源记录结构体对象，存储权威部分的各个字段

        r.type = htons(r.type); // 换为网络字节序
        r.class_ = htons(r.class_);
        r.ttl = htonl(r.ttl);
        r.rdlength = htons(r.rdlength);

        std::vector<char> name_data = encode_qname(r.name);
        data.insert(data.end(), name_data.begin(), name_data.end());
        data.resize(data.size() + sizeof(r.type) + sizeof(r.class_) + sizeof(r.ttl) + sizeof(r.rdlength) + r.rdlength);
        memcpy(data.data() + data.size() - sizeof(r.type) - sizeof(r.class_) - sizeof(r.ttl) - sizeof(r.rdlength) - r.rdlength, &r.type, sizeof(r.type));
        memcpy(data.data() + data.size() - sizeof(r.class_) - sizeof(r.ttl) - sizeof(r.rdlength) - r.rdlength, &r.class_, sizeof(r.class_));
        memcpy(data.data() + data.size() - sizeof(r.ttl) - sizeof(r.rdlength) - r.rdlength, &r.ttl, sizeof(r.ttl));
        memcpy(data.data() + data.size() - sizeof(r.rdlength) - r.rdlength, &r.rdlength, sizeof(r.rdlength));
        memcpy(data.data() + data.size() - r.rdlength, r.rdata.data(), r.rdlength);
    }

    for (const auto &additional : additionals)
    {                                  // 遍历附加部分对象列表
        ResourceRecord r = additional; // 创建一个资源记录结构体对象，存储附加部分的各个字段

        r.type = htons(r.type);
        r.class_ = htons(r.class_);
        r.ttl = htonl(r.ttl);
        r.rdlength = htons(r.rdlength);

        std::vector<char> name_data = encode_qname(r.name);
        data.insert(data.end(), name_data.begin(), name_data.end());
        data.resize(data.size() + sizeof(r.type) + sizeof(r.class_) + sizeof(r.ttl) + sizeof(r.rdlength) + r.rdlength);
        memcpy(data.data() + data.size() - sizeof(r.type) - sizeof(r.class_) - sizeof(r.ttl) - sizeof(r.rdlength) - r.rdlength, &r.type, sizeof(r.type));
        memcpy(data.data() + data.size() - sizeof(r.class_) - sizeof(r.ttl) - sizeof(r.rdlength) - r.rdlength, &r.class_, sizeof(r.class_));
        memcpy(data.data() + data.size() - sizeof(r.ttl) - sizeof(r.rdlength) - r.rdlength, &r.ttl, sizeof(r.ttl));
        memcpy(data.data() + data.size() - sizeof(r.rdlength) - r.rdlength, &r.rdlength, sizeof(r.rdlength));
        memcpy(data.data() + data.size() - r.rdlength, r.rdata.data(), r.rdlength);
    }

    return data;
}

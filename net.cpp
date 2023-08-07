// net.cpp
#include "net.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <fcntl.h>

int create_udp_socket(int port)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0); // 创建一个IPv4地址族和UDP协议类型的FD
    if (sock < 0)
    { // 如果创建失败，返回-1
        return -1;
    }

    struct sockaddr_in server_addr;           // 创建一个IPv4地址结构体，存储服务器地址
    server_addr.sin_family = AF_INET;         // 设置地址族为IPv4
    server_addr.sin_port = htons(port);       // 设置端口号，并转换为网络字节序
    server_addr.sin_addr.s_addr = INADDR_ANY; // 设置IP地址为任意地址

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // 设置fd选项为允许重用地址

    int n = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)); // 将fd绑定到服务器地址，并获取绑定的结果
    if (n < 0)
    { // 绑定失败
        close(sock);
        return -1;
    }

    return sock; // 返回FD
}

int create_tcp_socket(int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0); // 创建一个IPv4地址族和TCP协议类型的fd
    if (sock < 0)
    { // 如果创建失败，返回-1
        return -1;
    }

    struct sockaddr_in server_addr;           // 创建一个IPv4地址结构体，存储服务器地址
    server_addr.sin_family = AF_INET;         // 设置地址族为IPv4
    server_addr.sin_port = htons(port);       // 设置端口号，并转换为网络字节序
    server_addr.sin_addr.s_addr = INADDR_ANY; // 设置IP地址为任意地址

    int opt = 1;                                                   // 创建一个整型变量，设置fd选项
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // 设置fd选项为允许重用地址

    int n = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)); // 将fd绑定到服务器地址，并获取绑定的结果
    if (n < 0)
    { // 绑定失败
        close(sock);
        return -1;
    }

    n = listen(sock, 5); // 将监听模式，最大连接数为5，
    if (n < 0)
    { // 监听失败，关闭fd并返回-1
        close(sock);
        return -1;
    }

    return sock; // 返回创建的fd的文件描述符
}

int create_dot_socket(int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0); // 创建一个IPv4地址族和TCP协议类型的fd
    if (sock < 0)
    { // 如果创建失败，返回-1
        return -1;
    }

    struct sockaddr_in server_addr;           // 存储服务器地址
    server_addr.sin_family = AF_INET;         // 设置地址族为IPv4
    server_addr.sin_port = htons(port);       // 设置端口
    server_addr.sin_addr.s_addr = INADDR_ANY; // 设置IP地址为任意地址

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // 允许重用端口地址

    int n = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)); // 将fd绑定到服务器地址，并获取绑定的结果
    if (n < 0)
    { // 如果绑定失败，关闭fd并返回-1
        close(sock);
        return -1;
    }

    n = listen(sock, 5); // 将fd设置为监听模式，并设置最大连接数为5
    if (n < 0)
    { // 如果监听失败，关闭fd并返回-1
        close(sock);
        return -1;
    }

    return sock; // 返回创建的fd
}

Net::Net(const Config &_config)
{
    this->config = _config;
    udp_sock = create_udp_socket(config.udp_port); // 创建UDPfd并绑定到指定端口
    tcp_sock = create_tcp_socket(config.tcp_port); // 创建TCPfd并绑定到指定端口
    dot_sock = create_dot_socket(config.dot_port); // 创建DoTfd并绑定到指定端口

    for (const auto &server : config.upstream_servers)
    {                                              // 遍历上游DNS服务器地址列表
        int sock = create_upstream_socket(server); // 创建上游DNS服务器fd并连接到指定地址
        upstream_socks.push_back(sock);            // 将上游DNS服务器fd添加到列表中
    }
}

Net::~Net()
{
    close(udp_sock); // 关闭UDPfd
    close(tcp_sock); // 关闭TCPfd
    close(dot_sock); // 关闭DoTfd

    for (const auto &sock : upstream_socks)
    {                // 遍历上游DNS服务器fd列表
        close(sock); // 关闭上游DNS服务器fd
    }
}

int Net::get_udp_sock() const
{
    return udp_sock;
}

int Net::get_tcp_sock() const
{
    return tcp_sock;
}

int Net::get_dot_sock() const
{
    return dot_sock;
}

const std::vector<int> &Net::get_upstream_socks() const
{
    return upstream_socks;
}

SSL_CTX *create_ssl_context()
{
    SSL_library_init();       // 初始化SSL库
    SSL_load_error_strings(); // 加载SSL错误信息

    const SSL_METHOD *method = TLS_server_method(); // 获取TLS服务器方法对象，指定SSL协议版本和模式
    SSL_CTX *ctx = SSL_CTX_new(method);             // 创建一个SSL上下文对象，并获取其指针

    if (ctx == nullptr)
    { // 创建失败
        return nullptr;
    }

    SSL_CTX_set_ecdh_auto(ctx, 1); // 设置SSL上下文对象为自动选择椭圆曲线算法

    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0)
    {                      // 设置SSL上下文对象使用证书文件，并获取设置的结果
        SSL_CTX_free(ctx); // 释放SSL上下文对象
        return nullptr;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0)
    {                      // 设置SSL上下文对象使用私钥文件，并获取设置的结果
        SSL_CTX_free(ctx); // 释放SSL上下文对象
        return nullptr;
    }

    return ctx; // 返回创建的SSL上下文对象
}

// 从指定的fd读取数据，并返回读取到的数据和地址信息
std::pair<std::vector<char>, std::string> Net::read_from_sock(int sock)
{
    std::vector<char> data; // 存储读取到的数据
    std::string addr;       // 存储地址信息

    if (sock == udp_sock)
    {                                                                                                    // 如果是UDP
        char buffer[512];                                                                                // 创建一个缓冲区，大小为512字节
        struct sockaddr_in client_addr;                                                                  // 存储客户端地址
        socklen_t client_len = sizeof(client_addr);                                                      // 获取客户端地址结构体的大小
        int n = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_len); // 从UDP-fd接收数据，
        if (n > 0)
        {                                           // 如果接收成功
            data.assign(buffer, buffer + n);        // 将缓冲区中的数据复制到数据向量中
            addr = inet_ntoa(client_addr.sin_addr); // 将客户端地址转换为字符串，并赋值给地址变量
        }
    }
    else if (sock == tcp_sock)
    {                                                  // 如果是TCP
        char buffer[2];                                // 创建一个缓冲区，大小为2字节
        int n = recv(sock, buffer, sizeof(buffer), 0); // 从TCPfd接收数据
        if (n == 2)
        {                                                // 如果接收成功
            uint16_t len = ntohs(*((uint16_t *)buffer)); // 将缓冲区中的数据转换为网络字节序，并赋值给长度变量
            data.resize(len);                            // 调整数据向量的大小为长度变量的值
            n = recv(sock, data.data(), len, 0);         // 从TCPfd接收数据
            if (n != len)
            {                 // 如果接收失败
                data.clear(); // 清空数据向量
            }
        }
    }
    else if (sock == dot_sock)
    {                                             // 如果是DoT
        SSL *ssl = SSL_new(create_ssl_context()); // 创建一个SSL对象，加密和解密数据
        SSL_set_fd(ssl, sock);                    // 将SSL对象和DoTfd关联起来
        SSL_accept(ssl);                          // 接受客户端的SSL连接请求

        char buffer[2];                                // 创建一个缓冲区，大小为2字节
        int n = SSL_read(ssl, buffer, sizeof(buffer)); // 从SSL对象读取数据，并获取读取到的字节数
        if (n == 2)
        {                                                // 如果读取成功
            uint16_t len = ntohs(*((uint16_t *)buffer)); // 将缓冲区中的数据转换为网络字节序，并赋值给长度变量
            data.resize(len);                            // 调整数据向量的大小为长度变量的值
            n = SSL_read(ssl, data.data(), len);         // 从SSL对象读取数据，并获取读取到的字节数
            if (n != len)
            {                 // 如果读取失败
                data.clear(); // 清空数据向量
            }
        }

        SSL_free(ssl); // 释放SSL对象
    }
    else
    {                                                  // 如果是上游DNS服务器fd
        char buffer[512];                              // 创建一个缓冲区
        int n = recv(sock, buffer, sizeof(buffer), 0); // 从上游DNS服务器fd接收数据
        if (n > 0)
        {
            data.assign(buffer, buffer + n); // 将缓冲区中的数据复制到数据向量中
        }
    }

    return std::make_pair(data, addr); // 返回读取到的数据和地址信息
}

bool Net::write_to_sock(int sock, const std::vector<char> &data, const std::string &addr)
{
    bool success = false;

    if (sock == udp_sock)
    {                                                                                                            // 如果是UDP
        struct sockaddr_in client_addr;                                                                          // 存储客户端地址
        client_addr.sin_family = AF_INET;                                                                        // 设置地址族为IPv4
        client_addr.sin_port = htons(53);                                                                        // 设置端口号
        client_addr.sin_addr.s_addr = inet_addr(addr.c_str());                                                   // 设置IP地址
        int n = sendto(sock, data.data(), data.size(), 0, (struct sockaddr *)&client_addr, sizeof(client_addr)); // 向UDP发送数据，并获取发送的字节数
        if (n == data.size())
        {
            success = true;
        }
    }
    else if (sock == tcp_sock)
    {                                                            // 如果是TCP
        uint16_t len = htons(data.size());                       //
        std::vector<char> buffer(2 + data.size());               // 创建一个缓冲区，大小为2加上数据向量的大小
        *((uint16_t *)buffer.data()) = len;                      // 将长度变量复制到缓冲区的前两个字节中
        std::copy(data.begin(), data.end(), buffer.begin() + 2); // 将数据向量复制到缓冲区的后面部分中
        int n = send(sock, buffer.data(), buffer.size(), 0);     // 向TCP发送数据
        if (n == buffer.size())
        {                   // 如果发送成功
            success = true; // 将成功变量设置为真
        }
    }
    else if (sock == dot_sock)
    {                                             // 如果是DoT
        SSL *ssl = SSL_new(create_ssl_context()); // 创建一个SSL对象，加密和解密数据
        SSL_set_fd(ssl, sock);                    // 将SSL对象和DoTfd关联起来
        SSL_accept(ssl);                          // 接受客户端的SSL连接请求

        uint16_t len = htons(data.size());                       // 将数据向量的大小转换为网络字节序，并赋值给长度变量
        std::vector<char> buffer(2 + data.size());               //
        *((uint16_t *)buffer.data()) = len;                      //
        std::copy(data.begin(), data.end(), buffer.begin() + 2); // 将数据向量复制到缓冲区的后面部分中
        int n = SSL_write(ssl, buffer.data(), buffer.size());    // 向SSL写入数据
        if (n == buffer.size())
        {
            success = true;
        }

        SSL_free(ssl); // 释放SSL对象
    }
    else
    {                                                    // 如果是上游DNS服务器
        int n = send(sock, data.data(), data.size(), 0); // 向上游DNS服务器fd发送数据，并获取发送的字节数
        if (n == data.size())
        {
            success = true;
        }
    }

    return success;
}

int create_upstream_socket(const std::string &server)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0); // 创建一个IPv4地址族和UDP协议类型的fd
    if (sock < 0)
    { // 如果创建失败，返回-1
        return -1;
    }

    struct sockaddr_in server_addr;                       // 存储服务器地址
    server_addr.sin_family = AF_INET;                     // 设置地址族为IPv4
    server_addr.sin_port = htons(53);                     // 设置端口
    struct hostent *host = gethostbyname(server.c_str()); // 根据服务器地址获取主机
    if (host == nullptr)
    { // 如果获取失败，关闭fd并返回-1
        close(sock);
        return -1;
    }

    server_addr.sin_addr.s_addr = *((unsigned long *)host->h_addr); //

    int n = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)); // 将fd连接到服务器地址
    if (n < 0)
    {
        close(sock);
        return -1;
    }

    return sock;
}

std::vector<char> Net::recv_data(int sock, sockaddr_in &cli_addr)
{
    std::vector<char> data; // 存储接收到的数据的向量
    // sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    char buffer[512];                  // 创建一个字符数组，存储临时的数据
    memset(buffer, 0, sizeof(buffer)); // 将字符数组清零

    ssize_t len = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr *)&cli_addr, &cli_len); // 从fd接收数据，并存储到字符数组中，并获取接收到的数据长度
    if (len < 0)
    {
        std::cerr << "Failed to receive data: " << strerror(errno) << "\n"; // 输出错误信息
        return data;
    }
    std::cout << "Received data from " << inet_ntoa(cli_addr.sin_addr) << ":" << ntohs(cli_addr.sin_port) << std::endl;
    data.resize(len);                 // 调整数据向量的大小为接收到的数据长度
    memcpy(data.data(), buffer, len); // 将字符数组中的数据复制到数据向量中

    return data; // 返回数据
}

int Net::create_upstream_socket(const std::string &ip)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0); // 创建一个IPv4的UDP
    if (sock < 0)
    {
        std::cerr << "Failed to create socket: " << strerror(errno) << "\n"; // 输出错误信息
        return -1;
    }

    struct sockaddr_in addr;                     // 上游DNS服务器
    memset(&addr, 0, sizeof(addr));              // 清零
    addr.sin_family = AF_INET;                   // IPv4
    addr.sin_port = htons(config.upstream_port); // 转换为网络字节序

    int ret = inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    if (ret <= 0)
    {
        std::cerr << "Invalid IP address: " << ip << "\n"; // 输出错误信息
        close(sock);
        return -1;
    }

    ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        std::cerr << "Failed to connect socket: " << strerror(errno) << "\n"; // 输出错误信息
        close(sock);
        return -1;
    }

    return sock; // 返回fd文件描述符，表示创建成功
}

int Net::create_local_socket()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0); // 创建一个IPv4的UDP
    if (sock < 0)
    {
        std::cerr << "Failed to create socket: " << strerror(errno) << "\n"; // 输出错误信息
        return -1;
    }
    // 设置socket地址可重用
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;                  // 本地DNS服务器
    memset(&addr, 0, sizeof(addr));           // 清零
    addr.sin_family = AF_INET;                // 将IPv4
    addr.sin_port = htons(config.local_port); // 将转换为网络字节序
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 转换为网络字节序

    int ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr)); // 将fd绑定到本地DNS服务器的地址上

    if (ret < 0)
    {
        std::cerr << "Failed to bind socket: " << strerror(errno) << "\n"; // 输出错误信息
        close(sock);
        return -1;
    }
    return sock;
}

void Net::send_data(int sock, const std::vector<char> &data, sockaddr_in &cli_addr)
{
    socklen_t cli_len = sizeof(cli_addr);
    ssize_t len = sendto(sock, data.data(), data.size(), 0, (sockaddr *)&cli_addr, cli_len); // 发送数据
    if (len < 0)
    {
        std::cerr << "Failed to send data: " << strerror(errno) << "\n"; // 输出错误信息
        return;
    }
}
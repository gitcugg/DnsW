#include "redis_cache.h"
#TODO
bool redisCache::redisInit()
{
    redisContext *pRedisContext = redisConnect("127.0.0.1", 6379);
    if (c->err)
    {
        redisFree(c);
        std::cout << "connect to redis fail" << std::endl;
        return 1;
    }
    std::cout << "connect to redis success" << std::endl;
    redisReply *r = (redisReply *)redisCommand(c, "get name");
    std::cout << r->str << std::endl;
    return true;
}
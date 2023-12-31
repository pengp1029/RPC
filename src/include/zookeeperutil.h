#pragma once

#include<semaphore.h>
#include<zookeeper/zookeeper.h>
#include<string>

// 封装的zookeeper客户端类
class ZooKeeperClient{
    public:
        ZooKeeperClient();
        ~ZooKeeperClient();

        // zkclient启动连接zkserver
        void Start();
        // 在zkserver上根据指定的path创建znode节点
        void Create(const char*path,const char*data,int datalen,int state=0);
        // 根据参数指定的znode节点路径获取znode节点的值
        std::string GetData(const char*path);
    private:
        // zookeeper客户端的句柄
        zhandle_t *m_zhandle;
};
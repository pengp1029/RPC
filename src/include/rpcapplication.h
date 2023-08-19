#pragma once

#include "rpcconfig.h"
#include"rpcchannel.h"
#include"rpccontroller.h"
// rpc框架基础类的初始化 单例模式
class RpcApplication{
    public:
        static void Init(int argc,char **argv);
        static RpcApplication& GetInstance();
        static RpcConfig& GetConfig();
    private:
        static RpcConfig m_config;
        RpcApplication(){};
        // 拷贝构造去掉
        RpcApplication(const RpcApplication&)=delete;
        RpcApplication(RpcApplication&&)=delete;
};
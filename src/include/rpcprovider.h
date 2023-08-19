#pragma once
#include "google/protobuf/service.h"
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/TcpConnection.h>
#include<muduo/base/Timestamp.h>
#include<unordered_map>
#include<iostream>
#include<google/protobuf/descriptor.h>
#include<string>

// 专门发布rpc服务的网络对象类
class RpcProvider{
    public:
        // rpc框架提供外部的可以发布rpc方法的函数接口
        void NotifyService(google::protobuf::Service *service);
        // 启动rpc服务节点，提供rpc远程网络调用服务
        void Run();
    private:
        // 组合eventloop
        muduo::net::EventLoop m_eventLoop;

        struct ServiceInfo{
            google::protobuf::Service *m_service; //服务对象
            std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_methodMap;//服务方法
        };
        // 存储注册成功的服务对象的名字和service服务对象与方法
        std::unordered_map<std::string,ServiceInfo> m_serviceMap;

        // 新的socket连接回调
        void OnConnection(const muduo::net::TcpConnectionPtr&);
        // 已建立连接用户的读写事件回调
        void OnMessage(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer* ,muduo::Timestamp);
        // 调用函数的回调操作，用于序列化rpc的响应和网络发送
        void SendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message*);
};
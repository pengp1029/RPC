#include<iostream>
#include"rpcapplication.h"
#include"user.pb.h"
#include"rpcchannel.h"
int main(int argc,char **argv){
    // rpc服务启动以后，利用rpc框架享受rpc服务调用，先调用框架的初始化函数
    RpcApplication::Init(argc,argv);

    // 调用远程发布的rpc方法
    fixbug::UserServiceRpc_Stub stub(new RpcChannel());
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    fixbug::LoginResponse response;
    // rpc调用 同步阻塞的rpc调用过程
    stub.Login(nullptr,&request,&response,nullptr); // RpcChannel->RpcChannel::callMethod 所有rpc方法调用的参数序列化和网络发送
    
    // 一次rpc调用完成，读调用的结果
    if(0 == response.result().errcode()){
        std::cout<<"rpc login response: "<<response.sucess()<<std::endl;
    }
    else{
        std::cout<<"rpc login response error: "<<response.result().ermsg()<<std::endl;
    }

    // 调用远程发布的rpc方法
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("zhang san");
    req.set_pwd("123456");

    fixbug::RegisterResponse rsp;
    // rpc调用 同步阻塞的rpc调用过程
    stub.Register(nullptr,&req,&rsp,nullptr); // RpcChannel->RpcChannel::callMethod 所有rpc方法调用的参数序列化和网络发送
    
    // 一次rpc调用完成，读调用的结果
    if(0 == rsp.result().errcode()){
        std::cout<<"rpc register response: "<<rsp.sucess()<<std::endl;
    }
    else{
        std::cout<<"rpc register response error: "<<rsp.result().ermsg()<<std::endl;
    }

    return 0;
}
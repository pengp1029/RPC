#include<iostream>
#include"rpcapplication.h"
#include"friend.pb.h"


int main(int argc,char **argv){
    // rpc服务启动以后，利用rpc框架享受rpc服务调用，先调用框架的初始化函数
    RpcApplication::Init(argc,argv);

    // 调用远程发布的rpc方法
    fixbug::FriendServiceRpc_Stub stub(new RpcChannel());
    fixbug::GetFriendListRequest request;
    request.set_userid(1000);

    fixbug::GetFriendListResponse response;

    // 初始化controller
    MyRpcController controller;

    // rpc调用 同步阻塞的rpc调用过程
    stub.GetFriendList(&controller,&request,&response,nullptr); // RpcChannel->RpcChannel::callMethod 所有rpc方法调用的参数序列化和网络发送
    
    if(controller.Failed()){
        std::cout<<controller.ErrorText()<<std::endl;
        return 0;
    }
    
    // 一次rpc调用完成，读调用的结果
    if(0 == response.result().errcode()){
        std::cout<<"rpc GetFriendList response success"<<std::endl;
        int friend_size = response.friends_size();
        for(int i=0;i<friend_size;++i){
            std::cout<<"index: "<<(i+1)<<"name: "<<response.friends(i)<<std::endl;
        }
    }
    else{
        std::cout<<"rpc GetFriendList response error: "<<response.result().ermsg()<<std::endl;
    }
    return 0;
}
#include<iostream>
#include<string>
#include"friend.pb.h"
#include"rpcapplication.h"
#include"rpcprovider.h"
#include"logger.h"

class FriendService:public fixbug::FriendServiceRpc{
    public:
        std::vector<std::string> GetFriendList(uint32_t userid){
            std::cout<<"do GetFirendList service!"<<std::endl<<"userid: "<<userid<<std::endl;
            std::vector<std::string> vec;
            vec.push_back("asdf");
            vec.push_back("qwer");
            vec.push_back("zxcv");
            return vec;
        }

        void GetFriendList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendListRequest* request,
                       ::fixbug::GetFriendListResponse* response,
                       ::google::protobuf::Closure* done){
            uint32_t userid = request->userid();
            std::vector<std::string> friendList = GetFriendList(userid);
            response->mutable_result()->set_errcode(0);
            response->mutable_result()->set_ermsg("");
            
            for(std::string &name:friendList){
                std::string *p = response->add_friends();
                *p = name;
            }
            done->Run();
        }

};
int main(int argc, char **argv){
    LOG_INFO("first log message! ");
    LOG_ERR("%s:%s:%d",__FILE__,__FUNCTION__,__LINE__);
    // 框架初始化
    RpcApplication::Init(argc,argv);

    // 把UserService发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点 进程阻塞，等待请求
    provider.Run();
    return 0;
}
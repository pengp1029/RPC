#include<iostream>
#include<string>
#include"user.pb.h"
#include"rpcapplication.h"
#include"rpcprovider.h"

/*
UserService 是本地服务，提供了两个进程内的本地方法，Login和GetFriendLists
UserServiceRpc ： rpc服务提供者 发布端
*/
class UserService:public fixbug::UserServiceRpc{ 
    public:
        bool Login(std::string name,std::string pwd){
            std::cout<<"doing local service: Login"<<std::endl;
            std::cout<<"name: "<<name<<std::endl<<"pwd: "<<pwd<<std::endl;
            return true;
        }

        bool Register(uint32_t id,std::string name,std::string pwd){
            std::cout<<"doing local service: Register"<<std::endl;
            std::cout<<"id: "<<id<<"name: "<<name<<std::endl<<"pwd: "<<pwd<<std::endl;
            return true;
        }

        /* 
        重写UserServiceRpc的虚函数 通过框架序列化
        1. caller ===> Login(LoginRequest) => muduo => callee
        2. callee ===> Login(LoginRequest) => 重写的Login方法
        */
        void Login(::google::protobuf::RpcController* controller,
                         const ::fixbug::LoginRequest* request,
                         ::fixbug::LoginResponse* response,
                         ::google::protobuf::Closure* done) {
            // protobuf反序列化，Login获取响应数据做本地业务
            std::string name = request->name();
            std::string pwd = request->pwd();

            // 调用本地业务
            bool login_result = Login(name,pwd);

            // 响应写入
            fixbug::ResultCode *code = response->mutable_result();
            code->set_errcode(0);
            code->set_ermsg("Login do error! ");
            response->set_sucess(login_result);

            // 回调 执行响应对象数据的序列化和网络发送（由protobuf实现）
            done->Run();
        }

        void Register(::google::protobuf::RpcController* controller,
                         const ::fixbug::RegisterRequest* request,
                         ::fixbug::RegisterResponse* response,
                         ::google::protobuf::Closure* done) {
            // protobuf反序列化，Login获取响应数据做本地业务
            uint32_t id = request->id();
            std::string name = request->name();
            std::string pwd = request->pwd();

            // 调用本地业务
            bool register_result = Register(id,name,pwd);

            // 响应写入
            fixbug::ResultCode *code = response->mutable_result();
            code->set_errcode(0);
            code->set_ermsg("Login do error! ");
            response->set_sucess(register_result);

            // 回调 执行响应对象数据的序列化和网络发送（由protobuf实现）
            done->Run();
        }
};

int main(int argc, char **argv){
    // 框架初始化
    RpcApplication::Init(argc,argv);

    // 把UserService发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // 启动一个rpc服务发布节点 进程阻塞，等待请求
    provider.Run();
    return 0;
}
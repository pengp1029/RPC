#include"rpcchannel.h"
#include"rpcheader.pb.h"
#include<sys/types.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include<netinet/in.h>
#include<error.h>
#include"rpcapplication.h"
#include<unistd.h>
/*
    header_size service_name method_name args_size args
*/
void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,google::protobuf::RpcController* controller, const google::protobuf::Message* request,google::protobuf::Message* response, google::protobuf::Closure* done){
    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    // 参数的序列化字符串长度
    uint32_t args_size = 0;
    std::string args_str;
    if(request->SerializeToString(&args_str)){
        args_size = args_str.size();
    }
    else{
        //std::cout<<"serialize request error : "<<std::endl;
        controller->SetFailed("serialize rpc request error!");
        return;
    }

    // 定义rpc的请求head
    rpc::RpcHeader rpcheader;
    rpcheader.set_service_name(service_name);
    rpcheader.set_method_name(method_name);
    rpcheader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if(rpcheader.SerializeToString(&rpc_header_str)){
        header_size = rpc_header_str.size();
    }
    else{
        //std::cout<<"serialize header error"<<std::endl;
        controller->SetFailed("serialize rpc header error! ");
        return;
    }

    // 组织待发送的rpc请求字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0,std::string((char*)&header_size,4));
    send_rpc_str += rpc_header_str; // header
    send_rpc_str += args_str; // args

    // 打印内容
    std::cout<<"header_size: "<<header_size<<std::endl;
    std::cout<<"rpc_header_str: "<<rpc_header_str<<std::endl;
    std::cout<<"service_name: "<<service_name<<std::endl;
    std::cout<<"method_name: "<<method_name<<std::endl;

    // 使用tcp编程，完成rpc方法的远程调用
    int clientfd = socket(AF_INET,SOCK_STREAM,0);
    if(-1==clientfd){
        //std::cout<<"create socket error! erron: "<<errno<<std::endl;
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt,"create socket error! erron: %d",errno);
        controller->SetFailed(errtxt);
        return;
    }


    /*// 读配置
    std::string ip=RpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(RpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());*/

    // 查询zk上该服务器所在的host信息
    ZooKeeperClient zkCli;
    zkCli.Start();
    std::string method_path = "/"+service_name+"/"+method_name;
    std::string host_data = zkCli.GetData(method_path.c_str());
    if(host_data==""){
        controller->SetFailed(method_path+" is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if(idx==-1){
        controller->SetFailed(method_path+" address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0,idx);
    uint16_t port = atoi(host_data.substr(idx+1,host_data.size()-idx-1).c_str());

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 发起rpc连接
    if(-1 == connect(clientfd,(struct sockaddr*)&server_addr,sizeof(server_addr))){
        //std::cout<<"connect error! errno:"<<errno<<std::endl;
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt,"connect error! errno: %d",errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 发送rpc请求
    if(-1 == send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0)){
        //std::cout<<"send error! errno:"<<errno<<std::endl;
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt,"send error! errno: %d",errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 接受rpc请求的响应值
    char recv_buf[1024]={0};
    int recv_size = recv(clientfd,recv_buf,1024,0);
    if(-1 == recv_size){
        //std::cout<<"recv error! error:"<<errno<<std::endl;
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt,"recv error! errno: %d",errno);
        controller->SetFailed(errtxt);
        return;
    }

    //std::string response_str(recv_buf,0,recv_size);  出现问题 如果recv_buf中遇到\0后面的数据就结束

    // 数据反序列化
    if(!response->ParseFromArray(recv_buf,recv_size)){
        //std::cout<<"parse error! response_str: "<<recv_buf<<std::endl;
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt,"parse error! response_str: %s",recv_buf);
        controller->SetFailed(errtxt);
        return;
    }

    close(clientfd);

}
#include"rpcprovider.h"
#include"rpcapplication.h"
#include "rpcheader.pb.h"
#include"logger.h"
#include"zookeeperutil.h"

/*
service_name => service描述
service *记录服务对象
method_name 对应method方法对象
*/
void RpcProvider::NotifyService(google::protobuf::Service *service){
    ServiceInfo service_info;
    
    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();

    // 获取服务的名字
    std::string service_name = pserviceDesc->name();
    // 获取service方法的数量
    int methodCount = pserviceDesc->method_count();

    //std::cout<<"service_name: "<<service_name <<std::endl;
    LOG_INFO("service_name: %s\n",service_name.c_str());
    for(int i=0;i<methodCount;i++){
        // 获取service指定下标服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name,pmethodDesc});
        //std::cout<<"method_name: "<<method_name <<std::endl;
        LOG_INFO("method_name: %s\n",method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name,service_info});
}

void RpcProvider::Run(){
    // 读取配置信息
    std::string ip=RpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(RpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip,port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");
    // 绑定连接回调和消息读写回调方法
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1 ));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    // 设置muduo库的线程数量
    server.setThreadNum(2);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    ZooKeeperClient zkCli;
    zkCli.Start();
    // service_name 为永久性节点 method_name为临时性节点ZOO_EPHEMERAL 存储当前rpc服务节点主机的ip和port
    for(auto &sp : m_serviceMap){
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(),nullptr,0);
        for(auto &mp:sp.second.m_methodMap){
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128]={0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }

    std::cout<<"RpcProvider start service at ip: "<<ip<<std::endl;

    //启动网络服务
    server.start();
    m_eventLoop.loop();


}

// 新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn){
    if(!conn->connected()){
        // 和rpc client的连接断开
        conn->shutdown();
    }
}

/*
    RpcProvicer和RpcConsumer协商好通信用的protobuf数据类型
    定义proto的message类型进行数据头的序列化和反序列化

    数据：service_name message_name args
    真实收到的：16UserServiceLoginzhang san123456
    header_size（4bytes） + header_str + args_size + args_str
    不用string存储，直接传输四个字节
    
    解决TCP粘包的问题:记录数据的长度 args_size
*/
// 远程有一个rpc服务的调用请求，OnMessage方法会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer* buffer,muduo::Timestamp){
    std::string recv_buf = buffer->retrieveAllAsString();

    // 数据流中读前四个字节
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size,4,0);

    //根据header_size读取数据头原始字符流，反序列化输得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4,header_size);
    rpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str)){
        // 反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else{
        // 反序列化失败
        std::cout<<"rpc_header_str: "<<rpc_header_str<<" parse error!"<<std::endl;
        return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4+header_size,args_size);

    // 打印调试信息
    std::cout<<"header_size: "<<header_size<<std::endl;
    std::cout<<"rpc_header_str: "<<rpc_header_str<<std::endl;
    std::cout<<"service_name: "<<service_name<<std::endl;
    std::cout<<"method_name: "<<method_name<<std::endl;

    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if(it==m_serviceMap.end()){
        std::cout<<service_name<<"is not exist!"<<std::endl;
    }

    google::protobuf::Service *service = it->second.m_service;

    auto mit = it->second.m_methodMap.find(method_name);
    if(mit==it->second.m_methodMap.end()){
        std::cout<<method_name<<"is not exist!"<<std::endl;
    }

    const google::protobuf::MethodDescriptor* method = mit->second;

    // 生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str)){
        std::cout<<"request parse error,content: "<<args_str<<std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给method的调用绑定一个回调函数
    google::protobuf::Closure*done = google::protobuf::NewCallback<RpcProvider,const muduo::net::TcpConnectionPtr&,google::protobuf::Message*>(this,&RpcProvider::SendRpcResponse,conn,response);
    
    // 调用当前rpc节点上发布的方法
    service->CallMethod(method,nullptr,request,response,done);
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn,google::protobuf::Message*response){
    std::string response_str;
    if(response->SerializeToString(&response_str)){ // 序列化
        // 序列化成功，通过网络把rpc执行的结果发送回调用方
        conn->send(response_str);
    }
    else{
        std::cout<<"serialize response_str error!";
    }
    conn->shutdown();//模拟http的短链接服务，由rpcprovider主动断开连接
}
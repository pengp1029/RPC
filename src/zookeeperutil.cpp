#include "zookeeperutil.h"
#include"rpcapplication.h"
#include<semaphore.h>
#include<iostream>

// 全局的watcher观察器 zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx){
    if(type==ZOO_SESSION_EVENT){ // 回调的消息类型是和会话相关的消息类型
        if(state==ZOO_CONNECTING_STATE){ // 连接成功
            sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }

}

ZooKeeperClient::ZooKeeperClient():m_zhandle(nullptr){}

ZooKeeperClient::~ZooKeeperClient(){
    if(m_zhandle!=nullptr){
        zookeeper_close(m_zhandle);
    }
}

// zkclient启动连接zkserver
void ZooKeeperClient::Start(){
    std::string host = RpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = RpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;
    /*
    zookeeper_mt: 多线程版本
    zookeeper提供了三个线程：
        API调用线程 zookeeper_init
        网络I/O线程 pthread_create poll
        watcher回调线程 pthread_create zkserver给zkclient发送消息
    异步建立，如果收到了ZOO_CONNECTING_STATE则建立成功
    */
    m_zhandle = zookeeper_init(connstr.c_str(),nullptr,30000,nullptr,nullptr,0);
    if(nullptr==m_zhandle){
        std::cout<<"zookeeper init error!"<<std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem,0,0);
    zoo_set_context(m_zhandle,&sem);

    sem_wait(&sem);
    std::cout<<"zookeeper_init sucess!"<<std::endl;
}

// 在zkserver上根据指定的path创建znode节点
void ZooKeeperClient::Create(const char*path,const char*data,int datalen,int state){
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag = zoo_exists(m_zhandle,path,0,nullptr);
    if(ZNONODE == flag){
        flag = zoo_create(m_zhandle,path,data,datalen,&ZOO_OPEN_ACL_UNSAFE,state,path_buffer,bufferlen);
        if(ZOK == flag){
            std::cout<<"znode create success... path: "<<path<<std::endl;
        }
        else{
            std::cout<<"flag: "<<flag<<std::endl;
            std::cout<<"znode create error ... path: "<<path<<std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

// 根据参数指定的znode节点路径获取znode节点的值
std::string ZooKeeperClient::GetData(const char*path){
    char buffer[64];
    int bufferlen = sizeof(buffer);
    int flag = zoo_get(m_zhandle,path,0,buffer,&bufferlen,nullptr);
    if(ZOK != flag){
        std::cout<<"get znode error... path: "<<path<<std::endl;
        return "";
    }
    else{
        return buffer;
    }
}
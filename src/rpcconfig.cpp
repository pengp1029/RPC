#include"rpcconfig.h"
#include<iostream>
#include<string>

void RpcConfig::LoadConfigFile(const char*config_file){
    FILE *pf = fopen(config_file,"r");
    if(nullptr==pf){
        std::cout<<config_file<<" is not exists!"<<std::endl;
        exit(EXIT_FAILURE);
    }
    // 1. 注释 2. 正确的配置项 3. 去掉空格
    while(!feof(pf)){
        char buf[512] = {0};
        fgets(buf,512,pf);

        std::string src_buf(buf);
        Trim(src_buf);

        //判断注释
        if(src_buf[0]=='#'||src_buf.empty())
            continue;
        
        // 解析配置项
        int idx = src_buf.find('=');
        if(idx==-1){
            // 不合法
            continue;
        }

        std::string key;
        std::string value;
        key = src_buf.substr(0,idx);
        Trim(key);

        int endidx = src_buf.find('\n',idx);
        value = src_buf.substr(idx+1,endidx-idx-1);
        Trim(value);
        m_configMap.insert({key,value});


    }
}

std::string RpcConfig::Load(const std::string &key){
    auto it = m_configMap.find(key);
    if(it==m_configMap.end()){
        return "";
    }
    return it->second;
}

void RpcConfig::Trim(std::string &src_buf){
    // 去掉前置空格
    int idx = src_buf.find_first_not_of(' ');
    if(idx!=-1){
        src_buf = src_buf.substr(idx,src_buf.size()-idx);
    }

    // 去掉后置空格
    idx = src_buf.find_last_not_of(' ');
    if(idx!=-1){
        src_buf = src_buf.substr(0,idx+1);
    }
}
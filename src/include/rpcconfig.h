#pragma once

#include<unordered_map>

//rpcserverip rpcserverport zookeeperip zookeeperport
// 读取配置文件
class RpcConfig{
    public:
        // 加载配置文件
        void LoadConfigFile(const char*config_file);
        // 查询配置信息
        std::string Load(const std::string &key);
    private:
        std::unordered_map<std::string,std::string> m_configMap;
        // 去掉前置后置空格
        void Trim(std::string &src_buf);
};
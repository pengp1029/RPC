#include "logger.h"
#include<time.h>
#include<iostream>

// 获取日志的单例
Logger& Logger::GetInstance(){
    static Logger logger;
    return logger;
}

Logger::Logger(){
    // 启动专门的写日志线程
    std::thread writeLogTask([&](){
        for(;;){
            // 获取日期，取日志信息写入相应的日志文件中
            time_t now = time(nullptr);
            tm *nowtime = localtime(&now);

            // 文件名
            char file_name[128];
            sprintf(file_name,"%d-%d-%d-log.txt",nowtime->tm_year+1900,nowtime->tm_mon+1,nowtime->tm_mday);

            FILE *pf = fopen(file_name,"a+");
            if(pf==nullptr){
                std::cout<<"logger_file: "<<file_name<<"open error! "<<std::endl;
                exit(EXIT_FAILURE);
            }
            
            std::string msg = m_lckQue.Pop();
            char buff[128]={0};
            sprintf(buff,"%d:%d:%d => [%s]",nowtime->tm_hour,nowtime->tm_min,nowtime->tm_sec,(m_loglevel==INFO)?"info":"error");
            msg.insert(0,buff);
            msg.append("\n");

            fputs(msg.c_str(),pf);
            fclose(pf);
        }
    });

    // 设置分离线程，守护线程
    writeLogTask.detach();
}

// 设置日志级别
void Logger::SetLogLevel(LogLevel level){
    m_loglevel = level;
}

// 写日志 把日志信息写入lockqueue缓冲区中
void Logger::Log(std::string msg){
    m_lckQue.Push(msg);
}
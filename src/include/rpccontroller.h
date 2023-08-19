#pragma once
#include<google/protobuf/service.h>
#include<string>
//#include<google/protobuf/stubs/callback.h>

class MyRpcController:public google::protobuf::RpcController{
    public:
        MyRpcController();
        void Reset();
        bool Failed()const;
        std::string ErrorText()const;
        void SetFailed(const std::string& reson);

        void StartCancel();
        bool IsCanceled() const;
        void NotifyOnCancel(google::protobuf::Closure* callback);

    private:
        bool m_failed;
        std::string m_errText;
};
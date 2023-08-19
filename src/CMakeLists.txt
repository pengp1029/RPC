aux_source_directory(. SRC_LIST)
#set(SRC_LIST rpcapplication.cpp rpcconfig.cpp rpcheader.pb.cc rpcprovider.cpp rpcchannel.cpp fpccontroller.cpp logger.cpp zookeeperutil.cpp)
add_library(rpc ${SRC_LIST})
target_link_libraries(rpc muduo_net muduo_base pthread zookeeper_mt)
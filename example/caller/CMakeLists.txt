#set(SRC_LIST calluserservice.cpp ../user.pb.cc)
set(SRC_LIST callfriendservier.cpp ../friend.pb.cc)
add_executable(consumer ${SRC_LIST})
target_link_libraries(consumer rpc protobuf)

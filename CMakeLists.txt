cmake_minimum_required(VERSION 3.24)
project(dd)

set(CMAKE_CXX_STANDARD 98)

add_executable(dd main.cpp Channel.cpp Database_channels.cpp Database_users.cpp KeventHandler.cpp Parser.cpp Receiver.cpp Sender.cpp Server.cpp User.cpp utils.cpp)

# Introduction
This C++ Message SDK is a industry control library base on ZeroMQ that hides most of the c-style interface core 0mq provides. 

A really basic Makefile is provided for this purpose and will generate shared libraries.




# Install

## prequest
libzmq  git://github.com/zeromq/libzmq.git  
cppzmq   https://github.com/zeromq/cppzmq
zmqpp  git://github.com/zeromq/zmqpp.git


1. make              build directory with bin,lib subdirectory
2. make install      cp lib/message_sdk.so to /usr/local/lib
3. run example       in ./build/bin directory
      rep_server
      req_client

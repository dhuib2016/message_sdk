# Introduction
This C++ Message SDK is a industry control library base on ZeroMQ that hides most of the c-style interface core 0mq provides. 

A really basic Makefile is provided for this purpose and will generate shared libraries.

# Install



1. make              build directory with bin,lib subdirectory
2. make install      cp lib/message_sdk.so to /usr/local/lib
3. run example       in ./build/bin directory
      rep_server
      req_client

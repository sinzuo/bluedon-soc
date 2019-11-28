#!/bin/bash

WORK_DIR="/root/SOC/soc5.1/src"
#echo $WORK_DIR

INC="-I. -I$WORK_DIR -I$WORK_DIR/Plugin/share -I$WORK_DIR/../lib_third/poco/Foundation/include/Poco -I/usr/local/ace/include -I/usr/local/log4cxx/include -I$WORK_DIR/../lib_third/curl/include"
#echo $INC
LIBPATH="-L. -L/root/SOC/soc5.1/lib_third/poco/lib/Linux/x86_64 -L/usr/local/ace/lib -L/root/SOC/soc5.1/lib_third/log4cxx/lib  -L/root/SOC/soc5.1/lib_third/curl/lib"
#echo $LIBPATH 

LIBS="-lACE -lPocoFoundation -lPocoUtil -lPocoXML -lprotobuf -llog4cxx -lzmq -lcurl  "

gcc -o test_main.bin main_test.cpp $INC -lbdinspecUrl $LIBS $LIBPATH
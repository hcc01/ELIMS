#ifndef _CELL_HPP_
#define _CELL_HPP_

//SOCKET
#ifdef _WIN32
	#define FD_SETSIZE      256
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include<windows.h>
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include<unistd.h> //uni std
	#include<arpa/inet.h>
	#include<string.h>
	#include<signal.h>
	#include<sys/socket.h>

	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif
//
#include"MessageHeader.h"
#include"CELLTimestamp.h"
#include"CELLTask.h"
#include"CELLLog.h"

//
#include<stdio.h>

//缓冲区最小单元大小
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 81920
#define SEND_BUFF_SZIE 102400
#endif // !RECV_BUFF_SZIE


enum Authority{
    ANALYSE=1,//分析
    ANALYSE_VERIFY=2,//分析审核
    Sampling=2,//采样
    ReportEdit=4,//报告

};






#endif // !_CELL_HPP_

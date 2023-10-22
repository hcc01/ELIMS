#ifndef _CELL_BUFFER_HPP_
#define _CELL_BUFFER_HPP_

#include"CELL.h"
#include"NetKits_global.h"
class NETKITS_EXPORT CELLBuffer
{
public:
	CELLBuffer(int nSize = 8192)
	{
		_nSize = nSize;
		_pBuff = new char[_nSize];
	}

	~CELLBuffer()
	{
		if (_pBuff)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}

	char* data()
	{
		return _pBuff;
	}

        bool push(const char* pData, int nLen);


        void pop(int nLen);


        int write2socket(SOCKET sockfd);


        int read4socket(SOCKET sockfd);

        bool hasMsg();


	bool needWrite()
	{
		return _nLast > 0;
	}
private:
	//第二缓冲区 发送缓冲区
	char* _pBuff = nullptr;
	//可以用链表或队列来管理缓冲数据块
	//list<char*> _pBuffList;
	//缓冲区的数据尾部位置，已有数据长度
	int _nLast = 0;
	//缓冲区总的空间大小，字节长度
	int _nSize = 0;
	//缓冲区写满次数计数
	int _fullCount = 0;
};

#endif // !_CELL_BUFFER_HPP_

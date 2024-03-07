
#include"CELLStream.h"

CELLStream::CELLStream(char *pData, int nSize, bool bDelete)
{
    CELLLog::Info(" CELLStream set nSize:%d",nSize);
    _nSize = nSize;
    _pBuff = pData;
    _bDelete = bDelete;
}

CELLStream::~CELLStream()
{
    if (_bDelete && _pBuff)
    {
        delete[] _pBuff;
        _pBuff = nullptr;
    }
}

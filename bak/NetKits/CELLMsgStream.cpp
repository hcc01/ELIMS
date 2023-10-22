#include"CELLMsgStream.h"

CELLReadStream::CELLReadStream(netmsg_DataHeader *header)
    :CELLReadStream((char*)header, header->dataLength)
{

}

CELLReadStream::CELLReadStream(char *pData, int nSize, bool bDelete)
    :CELLStream(pData, nSize, bDelete)
{
    push(nSize);
    ////预先读取消息长度
    //ReadInt16();
    ////预先读取消息命令
    //getNetCmd();
}

CELLWriteStream::CELLWriteStream(char *pData, int nSize, bool bDelete)
    :CELLStream(pData, nSize, bDelete)
{
    //预先占领消息长度所需空间
    Write<uint16_t>(0);
}

CELLWriteStream::CELLWriteStream(int nSize)
    :CELLStream(nSize)
{
    //预先占领消息长度所需空间
    Write<uint16_t>(0);
}

void CELLWriteStream::finsh()
{
    int pos = length();
    setWritePos(0);
    Write<uint16_t>(pos);
    setWritePos(pos);
}

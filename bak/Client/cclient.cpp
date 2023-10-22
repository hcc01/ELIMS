#include "cclient.h"

CClient::CClient()
{

}

void CClient::OnNetMsg(netmsg_DataHeader *header)
{
    emit netMsg(header);

}

void CClient::connectError(const char *error)
{
    emit onConnectError(error);
}

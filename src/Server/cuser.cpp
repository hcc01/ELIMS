#include "cuser.h"

CUser::CUser()
{

}

CUser::CUser(int ID, const QString &name):
    _id(ID),_name(name)
{

}

void CUser::OnDisconnect()
{
    delete this;
}

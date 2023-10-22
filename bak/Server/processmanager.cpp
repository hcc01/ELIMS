#include "processmanager.h"

ProcessManager::ProcessManager(QObject *parent) : QObject(parent)
{

}

void ProcessManager::onJsonCmd(const QJsonCmd &cmd)
{
    int type=cmd.jsCmd().value("tpye").toInt();
    switch (type) {
    case WF_NEW_PROCESS:
    {
        NewWorkFlowCMD wfCmd(cmd.jsCmd());

        break;
    }
    default:
        break;
    }
}

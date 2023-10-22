#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_

#include<thread>
#include<mutex>
#include<list>

#include<functional>

#include"CELLThread.h"

//执行任务的服务类型
#include"NetKits_global.h"
class NETKITS_EXPORT CELLTaskServer
{
public:
	//所属serverid
	int serverId = -1;
private:
	typedef std::function<void()> CELLTask;
private:
	//任务数据
	std::list<CELLTask> _tasks;
	//任务数据缓冲区
	std::list<CELLTask> _tasksBuf;
	//改变数据缓冲区时需要加锁
	std::mutex _mutex;
	//
	CELLThread _thread;
public:
	//添加任务
        void addTask(CELLTask task);
	//启动工作线程
        void Start();

        void Close();
protected:
	//工作函数
        void OnRun(CELLThread* pThread);
};
#endif // !_CELL_TASK_H_

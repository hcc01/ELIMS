#ifndef _CELL_SEMAPHORE_HPP_
#define _CELL_SEMAPHORE_HPP_

#include<chrono>
#include<thread>
#include<QDebug>
#include<condition_variable>
//信号量
class CELLSemaphore
{
public:
	//阻塞当前线程
	void wait()
	{
        qDebug()<<"wait"<<_wait<<_wakeup;
		std::unique_lock<std::mutex> lock(_mutex);
		if (--_wait < 0)
		{
			//阻塞等待
			_cv.wait(lock, [this]()->bool{
				return _wakeup > 0;
			});
			--_wakeup;
		}
        qDebug()<<"endwait"<<_wait<<_wakeup;
	}
	//唤醒当前线程
	void wakeup()
	{
        qDebug()<<"wakeup()"<<_wait<<_wakeup;
		std::lock_guard<std::mutex> lock(_mutex);
		if (++_wait <= 0)
		{
			++_wakeup;
			_cv.notify_one();
		}
        qDebug()<<"endwakeup()"<<_wait<<_wakeup;
	}

private:
	//改变数据缓冲区时需要加锁
	std::mutex _mutex;
	//阻塞等待-条件变量
	std::condition_variable _cv;
	//等待计数
	int _wait = 0;
	//唤醒计数
	int _wakeup = 0;
};

#endif // !_CELL_SEMAPHORE_HPP_

//虚假唤醒

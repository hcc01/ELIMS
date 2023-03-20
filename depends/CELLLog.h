#ifndef _CELL_LOG_HPP_
#define _CELL_LOG_HPP_

#include"CELL.h"
#include"CELLTask.h"
#include<ctime>
class CELLLog
{
	//Info
	//Debug
	//Warring
	//Error
private:
        CELLLog();

        ~CELLLog();
public:
	static CELLLog& Instance()
	{
		static  CELLLog sLog;
		return sLog;
	}

        void setLogPath(const char* logPath, const char* mode);

        static void Info(const char* pStr);

	template<typename ...Args>
        static void Info(const char* pformat, Args ... args)
        {
                CELLLog* pLog = &Instance();
                pLog->_taskServer.addTask([=]() {
                        if (pLog->_logFile)
                        {
                                auto t = system_clock::now();
                                auto tNow = system_clock::to_time_t(t);
                                //fprintf(pLog->_logFile, "%s", ctime(&tNow));
                                std::tm* now = std::gmtime(&tNow);
                                fprintf(pLog->_logFile, "%s", "Info ");
                                fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
                                fprintf(pLog->_logFile, pformat, args...);
                                fflush(pLog->_logFile);
                        }
                        printf(pformat, args...);
                });
        }


private:
	FILE* _logFile = nullptr;
	CELLTaskServer _taskServer;
};

#endif // !_CELL_LOG_HPP_

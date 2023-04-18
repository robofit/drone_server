#ifndef ASYNC_HANDLER_HPP
#define ASYNC_HANDLER_HPP

#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

template<class T>
class async_handler
{
    std::atomic<bool> run;
	std::thread* handler;
	std::queue<T> msg_q;
	std::mutex msg_qm;
    std::condition_variable cv;
public:
    void stop()
    {
        run = false;
        cv.notify_one();
        if (handler->joinable())
            handler->join();
    }
    virtual ~async_handler()
    {
        stop();
        delete handler;
    }
    void add(T j)
    {
        std::unique_lock<std::mutex> lock(msg_qm);
        msg_q.push(j);
        cv.notify_one();
    }
protected:
    virtual void handle(T) {}
    async_handler()
    : run(true)
    {
        handler = new std::thread([this]()
        {
            while (run)
            {
                std::unique_lock<std::mutex> lock(msg_qm);
                cv.wait(lock, [this]{ return !msg_q.empty() || !run; });

                if (!msg_q.empty())
                {
                    T j = msg_q.front();
                    msg_q.pop();

                    handle(j);
                }
            }
        });
    }
};

#endif
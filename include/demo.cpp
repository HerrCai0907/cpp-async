#include <chrono>
#include <functional>
#include <iostream>
#include <queue>
#include <thread>
#include <utility>
#include <vector>
#include "async/task_completion_source.h"

struct TaskQueue
{
    struct TimerCallback
    {
        int m_t;
        std::function<void()> m_cb;
        auto operator<=>(TimerCallback const& other) const { return this->m_t <=> other.m_t; }
    };

    void add(int t, std::function<void()> cb) { m_cbs.push(TimerCallback{ .m_t = t, .m_cb = std::move(cb) }); }

    bool consume()
    {
        if (m_cbs.empty())
        {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::seconds(m_cbs.top().m_t - m_current));
        std::function<void()> fn = std::move(m_cbs.top().m_cb);
        m_current = m_cbs.top().m_t;
        m_cbs.pop();
        fn();
        return true;
    }

    std::priority_queue<TimerCallback> m_cbs{};
    int m_current = 0;
};

TaskQueue queue{};

async::task<void> sleep(int t)
{
    auto promise{ std::make_shared<async::task_completion_source<void>>() };
    queue.add(t, [promise]() { promise->set_value(); });
    return promise->task();
}

inline async::task<void> do_async(int base)
{
    if (base == 5)
    {
        std::cout << "enter " << __PRETTY_FUNCTION__ << "\n";
        co_await sleep(base + 1);
    }
    std::cout << "middle " << __PRETTY_FUNCTION__ << "\n";
    co_await sleep(base + 2);
    std::cout << "end " << __PRETTY_FUNCTION__ << "\n";
    co_return;
}

inline async::task<void> app()
{
    co_await do_async(0);
    co_await do_async(5);
    co_await do_async(10);
    co_await do_async(15);
    co_return;
}

int main()
{
    app();
    while (queue.consume())
    {
    }
}

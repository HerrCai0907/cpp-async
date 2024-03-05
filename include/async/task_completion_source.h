// © Microsoft Corporation. All rights reserved.

#pragma once

#include <memory>
#include "task.h"

namespace async::details
{
    enum class task_completion_state
    {
        unset,
        setting,
        set
    };

    template <typename T>
    struct task_completion_source_core final
    {
        task_completion_source_core() :
            m_taskState{ task_state<T>::create_shared() }, m_completionState{ task_completion_state::unset }
        {
        }

        ::async::task<T> task() const noexcept { return ::async::task<T>{ m_taskState }; }

        template <typename... Args>
        void set_value(Args&&... args)
        {
            task_completion_state expected{ task_completion_state::unset };

            assert(m_completionState.compare_exchange_strong(expected, task_completion_state::setting));

            m_taskState->result.set_value(std::forward<T>(args)...);
            m_completionState = task_completion_state::set;
            try_complete();
        }

    private:
        void try_complete() noexcept
        {
            assert(m_completionState.load() == task_completion_state::set);
            const std::coroutine_handle<> possibleCompletion{ m_taskState->mark_ready() };
            if (possibleCompletion)
            {
                possibleCompletion();
            }
        }

        std::shared_ptr<task_state<T>> m_taskState;
        atomic_acq_rel<task_completion_state> m_completionState;
    };
}

namespace async
{
    template <typename T>
    struct task_completion_source final
    {
        ::async::task<T> task() const noexcept { return m_core.task(); }

        void set_value(T value) { m_core.set_value(value); }

    private:
        details::task_completion_source_core<T> m_core;
    };

    template <>
    struct task_completion_source<void> final
    {
        ::async::task<void> task() const noexcept { return m_core.task(); }

        void set_value() { m_core.set_value(); }

    private:
        details::task_completion_source_core<void> m_core;
    };
}

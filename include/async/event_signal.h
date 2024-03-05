// // © Microsoft Corporation. All rights reserved.

// #pragma once

// #include <chrono>
// #include <functional>
// #include "atomic_acq_rel.h"

// namespace async
// {
//     struct event_signal final
//     {
//         event_signal() : m_signaled{ false } {}

//         bool is_set() const { return m_signaled; }

//         void set() { m_signaled = true; }

//     private:
//         bool m_signaled;
//     };
// }

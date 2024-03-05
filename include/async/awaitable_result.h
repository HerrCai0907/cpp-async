// © Microsoft Corporation. All rights reserved.

#pragma once

#include <cassert>
#include <cstring>
#include <utility>

namespace async
{
    template <typename T>
    struct awaitable_result final
    {
        constexpr awaitable_result() noexcept : m_storageType{ result_union_type::unset }, m_storage{} {}

        awaitable_result(const awaitable_result&) = delete;

        awaitable_result(awaitable_result&& other) noexcept : m_storageType{ result_union_type::unset }, m_storage{}
        {
            switch (other.m_storageType)
            {
            case result_union_type::value:
                new (m_storage) possible_reference{ std::move(other.m_storage) };
                break;
            default:
                break;
            }

            m_storageType = other.m_storageType;
            other.m_storageType = result_union_type::unset;
        }

        ~awaitable_result() noexcept
        {
            switch (m_storageType)
            {
            case result_union_type::value:
                m_storage.~possible_reference();
                break;
            }
        }

        awaitable_result& operator=(const awaitable_result&) = delete;
        awaitable_result& operator=(awaitable_result&&) noexcept = delete;

        T operator()()
        {
            switch (m_storageType)
            {
            case result_union_type::value:
                return std::forward<T>(m_storage.value);
            case result_union_type::unset:
                assert(false && "Awaitable result is not yet available.");
            default:
                assert(false && "Invalid awaitable result state.");
            }
        }

        void set_value(T value) noexcept
        {
            new (std::addressof(m_storage)) possible_reference{ std::forward<T>(value) };
            m_storageType = result_union_type::value;
        }

    private:
        // A union (result_union below) may not contain a reference type.
        // Wrap the possible reference type in a struct, which may contain a reference type.
        struct possible_reference
        {
            T value;
        };

        enum class result_union_type
        {
            unset,
            value,
        };

        result_union_type m_storageType;
        possible_reference m_storage;
    };

    template <>
    struct awaitable_result<void> final
    {
        awaitable_result() noexcept {}

        awaitable_result(const awaitable_result&) = delete;
        awaitable_result(awaitable_result&&) noexcept = default;

        ~awaitable_result() noexcept = default;

        awaitable_result& operator=(const awaitable_result&) = delete;

        awaitable_result& operator=(awaitable_result&& other) noexcept = default;

        void operator()() const {}

        constexpr void set_value() const noexcept {};

    private:
    };
}

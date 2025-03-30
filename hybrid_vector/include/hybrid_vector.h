#pragma once
#include <cstddef>
#include <array>
#include <vector>
#include <type_traits>
#include <stdexcept>
#include <utility>

#include "hybrid_vector_def.h"

namespace hybrid_vector
{

template <typename T, size_t array_sz>
HybridVector<T, array_sz>::HybridVector()
    : m_used(0u)
    , m_arr{}
    , m_vec{}
{
}

template <typename T, size_t array_sz>
template <typename Src, typename can_enable>
HybridVector<T, array_sz>::HybridVector(const Src& src)
    : m_used(0u)
    , m_arr{}
    , m_vec{}
{
    for (auto it = src.begin(); it != src.end(); ++it)
    {
        this->push_back(*it);
    }
}

template <typename T, size_t array_sz>
template <typename Src, typename can_enable>
HybridVector<T, array_sz>::HybridVector(Src&& xsrc)
    : m_used(0u)
    , m_arr{}
    , m_vec{}
{
    for (auto it = xsrc.begin(); it != xsrc.end(); ++it)
    {
        this->push_back(std::move(*it));
    }
    xsrc.clear();
}

template <typename T, size_t array_sz>
template <class BeginIter, class EndIter, typename can_enable> 
HybridVector<T, array_sz>::HybridVector(BeginIter itBegin, EndIter itEnd)
    : m_used(0u)
    , m_arr{}
    , m_vec{}
{
    for (auto it = itBegin; it != itEnd; ++it)
    {
        this->push_back(*it);
    }
}

template <typename T, size_t array_sz>
template <typename U, size_t other_array_sz, typename can_enable>
HybridVector<T, array_sz>::HybridVector(const HybridVector<U, other_array_sz>& other)
    : m_used(other.m_used)
    , m_arr{}
    , m_vec{}
{
    const size_t self_arr_sz = self_type::array_size;
    const size_t copy_count = m_used;
    this->reserve(copy_count);
    for (size_t i = 0u; i < copy_count; ++i)
    {
        if (i < self_arr_sz)
        {
            m_arr[i] = other[i];
        }
        else
        {
            m_vec.emplace_back(other[i]);
        }
    }
}

template <typename T, size_t array_sz>
template <typename U, size_t other_array_sz, typename can_enable>
HybridVector<T, array_sz>::HybridVector(HybridVector<U, other_array_sz>&& other)
    : m_used(other.m_used)
    , m_arr{}
    , m_vec{}
{
    const size_t self_arr_sz = self_type::array_size;
    const size_t copy_count = m_used;
    this->reserve(copy_count);
    for (size_t i = 0u; i < copy_count; ++i)
    {
        if (i < self_arr_sz)
        {
            m_arr[i] = std::move(other[i]);
        }
        else
        {
            m_vec.emplace_back(std::move(other[i]));
        }
    }
    other.clear();
}

template <typename T, size_t array_sz>
size_t HybridVector<T, array_sz>::size() const
{
    return m_used;
}

template <typename T, size_t array_sz>
size_t HybridVector<T, array_sz>::capacity() const
{
    return self_type::array_size + m_vec.capacity();
}

template <typename T, size_t array_sz>
void HybridVector<T, array_sz>::reserve(size_t request)
{
    if (request <= self_type::array_size)
    {
        return;
    }
    request -= self_type::array_size;
    size_t capacity = m_vec.capacity();
    if (request > capacity)
    {
        m_vec.reserve(request);
    }
}

template <typename T, size_t array_sz>
void HybridVector<T, array_sz>::clear()
{
    m_used = 0u;
    for (size_t i = 0u; i < self_type::array_size; ++i)
    {
        m_arr[i] = value_type{};
    }
    m_vec.clear();
}

template <typename T, size_t array_sz>
auto HybridVector<T, array_sz>::operator[](size_t idx) const -> const value_type&
{
    this->validate_idx_else_throw(idx);
    if (idx < self_type::array_size)
    {
        return m_arr[idx];
    }
    else
    {
        return m_vec[idx - self_type::array_size];
    }
}

template <typename T, size_t array_sz>
auto HybridVector<T, array_sz>::operator[](size_t idx) -> value_type&
{
    this->validate_idx_else_throw(idx);
    if (idx < self_type::array_size)
    {
        return m_arr[idx];
    }
    else
    {
        return m_vec[idx - self_type::array_size];
    }
}

template <typename T, size_t array_sz>
void HybridVector<T, array_sz>::push_back(const value_type& t)
{
    if (m_used < self_type::array_size)
    {
        m_arr[m_used] = t;
    }
    else
    {
        m_vec.push_back(t);
    }
    ++m_used;
}

template <typename T, size_t array_sz>
void HybridVector<T, array_sz>::push_back(value_type&& t)
{
    if (m_used < self_type::array_size)
    {
        m_arr[m_used] = std::move(t);
    }
    else
    {
        m_vec.push_back(std::move(t));
    }
    ++m_used;
}

template <typename T, size_t array_sz>
template <typename... Args>
void HybridVector<T, array_sz>::emplace_back(Args&&... args)
{
    if (m_used < self_type::array_size)
    {
        m_arr[m_used] = value_type{std::forward<Args>(args)...};
    }
    else
    {
        m_vec.emplace_back(std::forward<Args>(args)...);
    }
    ++m_used;
}

template <typename T, size_t array_sz>
auto HybridVector<T, array_sz>::pop_back() -> value_type
{
    this->validate_nonempty_else_throw();
    --m_used;
    if (m_used < self_type::array_size)
    {
        /**
         * @note value_type can potentially be resource handles, therefore
         *       re-assigning the value to {} is necessary to avoid memory leak.
         */
        value_type t = std::move(m_arr[m_used]);
        m_arr[m_used] = {};
        return t;
    }
    else
    {
        value_type t = std::move(m_vec.back());
        m_vec.pop_back();
        return t;
    }
}

template <typename T, size_t array_sz>
void HybridVector<T, array_sz>::validate_idx_else_throw(size_t idx) const
{
    if (idx < m_used)
    {
        return;
    }
    //! @note customized error message to be more informative.
    char msg[144] = {};
    snprintf(msg, sizeof(msg), "index out of range: ((idx)%zu >= (size)%zu). (this)%p", idx, m_used, this);
    msg[sizeof(msg) - 1] = '\0';
    this->pre_throw();
    throw std::out_of_range(msg);
}

template <typename T, size_t array_sz>
void HybridVector<T, array_sz>::validate_nonempty_else_throw() const
{
    if (m_used > 0u)
    {
        return;
    }
    //! @note customized error message to be more informative.
    char msg[108] = {};
    snprintf(msg, sizeof(msg), "HybridVector is empty. (this)%p", this);
    msg[sizeof(msg) - 1] = '\0';
    this->pre_throw();
    throw std::out_of_range(msg);
}

template <typename T, size_t array_sz>
void HybridVector<T, array_sz>::pre_throw() const
{
    //! @todo Call debugger hook here, if available.
}

} // namespace hybrid_vector

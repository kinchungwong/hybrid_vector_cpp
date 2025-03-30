#pragma once
#include <cstddef>
#include <array>
#include <vector>
#include <type_traits>
#include <stdexcept>
#include <utility>

#include "hybrid_vector_fwd.h"
#include "detail/container_traits.h"

namespace hybrid_vector
{

/**
 * @brief Vector-like container that stores the first few elements in a fixed-size array,
 *        and the remainder in a vector.
 */
template <typename T, size_t array_sz>
class HybridVector
{
public:
    using self_type = HybridVector<T, array_sz>;
    using value_type = T;
    static constexpr const size_t array_size = array_sz;

private:
    size_t m_used;
    std::array<value_type, array_size> m_arr;
    std::vector<value_type> m_vec;

public:
    /**
     * @brief Default constructor.
     */
    explicit HybridVector();

    /**
     * @brief Constructor that takes a container of values that can be converted into T.
     */
    template <
        typename Src,
        typename can_enable = typename std::enable_if<container_traits::is_readable_container<Src, T>::value>::type
    >
    explicit HybridVector(const Src& src);

    /**
     * @brief Constructor that takes an expiring container of values that can be moved into T.
     */
    template <
        typename Src,
        typename can_enable = typename std::enable_if<container_traits::is_eraseable_container<Src, T>::value>::type
    >
    explicit HybridVector(Src&& xsrc);

    /**
     * @brief Constructor that takes a pair of iterators.
     */
    template <
        class BeginIter, 
        class EndIter,
        typename can_enable = typename std::enable_if<container_traits::is_readable_iter_pair<BeginIter, EndIter, T>::value>::type
    > 
    HybridVector(BeginIter itBegin, EndIter itEnd);

    /**
     * @brief Copy constructor.
     */
    template <
        typename U, 
        size_t other_array_sz,
        typename can_enable = typename std::enable_if<std::is_convertible<T, U>::value>::type
    >
    HybridVector(const HybridVector<U, other_array_sz>& other);

    /**
     * @brief Move constructor.
     */
    template <
        typename U, 
        size_t other_array_sz,
        typename can_enable = typename std::enable_if<std::is_convertible<T, U>::value>::type
    >
    HybridVector(HybridVector<U, other_array_sz>&& other);

    /**
     * @brief Destructor.
     */
    ~HybridVector() = default;

    //! @brief Number of items in the container.
    size_t size() const;

    //! @brief Number of items that can be stored in the container without any additional allocation.
    size_t capacity() const;

    /**
     * @brief Reserve space for at least the requested number of items.
     * @param request The number of items to reserve space for.
     */
    void reserve(size_t request);

    /**
     * @brief Clear the container.
     */
    void clear();

    /**
     * @brief Access an element by index by const-reference.
     * @exception std::out_of_range if the container is empty.
     */
    const value_type& operator[](size_t idx) const;

    /**
     * @brief Access an element by index by reference.
     * @exception std::out_of_range if the container is empty.
     */
    value_type& operator[](size_t idx);

    /**
     * @brief Adds an element to the end of the container.
     */
    void push_back(const value_type& t);

    /**
     * @brief Adds an element to the end of the container by moving.
     */
    void push_back(value_type&& t);

    /**
     * @brief Constructs an element in-place at the end of the container.
     */
    template <typename... Args> void emplace_back(Args&&... args);

    /**
     * @brief Removes the last element from the container and returns it.
     * @exception std::out_of_range if the container is empty.
     */
    value_type pop_back();

private:
    void validate_idx_else_throw(size_t idx) const;
    void validate_nonempty_else_throw() const;
    void pre_throw() const;
};

} // namespace hybrid_vector

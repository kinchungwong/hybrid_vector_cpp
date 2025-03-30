#pragma once

#include <type_traits>

namespace container_traits
{

namespace member_tests
{

/**
 * @brief Test if C has a member begin(), and is deferenceable.
 */
template <class C> struct has_begin
{
    template <typename C2>
    static auto test(int) -> decltype(std::declval<C2>().begin(), *std::declval<C2>().begin(), std::true_type{});
    template <typename C2>
    static auto test(...) -> std::false_type;
    static constexpr bool value = std::is_same<decltype(test<C>(0)), std::true_type>::value;
};

/**
 * @brief Test if C has a member end().
 * @note The end iterator is not required to be of same type as begin(), 
 *       and is not required to be dereferenceable.
 */
template <class C> struct has_end
{
    template <typename C2>
    static auto test(int) -> decltype(std::declval<C2>().end(), std::true_type{});
    template <typename C2>
    static auto test(...) -> std::false_type;
    static constexpr bool value = std::is_same<decltype(test<C>(0)), std::true_type>::value;
};

/**
 * @brief Test if C has a member size().
 */
template <class C> struct has_size
{
    template <typename C2>
    static auto test(int) -> decltype(std::declval<C2>().size(), std::true_type{});
    template <typename C2>
    static auto test(...) -> std::false_type;
    static constexpr bool value = std::is_same<decltype(test<C>(0)), std::true_type>::value;
};

/**
 * @brief Test if C has a member clear().
 * @note The presence of clear() is usually dependent on whether C is const-qualified.
 *       This, in turn, is used to check whether C can be allowed to be moved from.
 * @note The test for moveability is not complete with a template instantiation because
 *       the user may have incorrectly conveyed the type of C to the template.
 *       Therefore, the user must qualify its function argument as C&&, while using
 *       the traits tests to determine if that function should be instantiated.
 */
template <class C> struct has_clear
{
    template <typename C2>
    static auto test(int) -> decltype(std::declval<C2>().clear(), std::true_type{});
    template <typename C2>
    static auto test(...) -> std::false_type;
    static constexpr bool value = std::is_same<decltype(test<C>(0)), std::true_type>::value;
};

} // member_tests

namespace iter_tests
{

template <
    typename BeginIter,
    typename EndIter
> struct iter_pair_has_noteq
{
    template <typename B2, typename E2>
    static auto test(int) -> decltype(std::declval<B2>() != std::declval<E2>(), std::true_type{});
    template <typename B2, typename E2>
    static auto test(...) -> std::false_type;
    static constexpr bool value = std::is_same<decltype(test<BeginIter, EndIter>(0)), std::true_type>::value;
};

} // iter_tests

namespace detail {

template <typename ValueType, typename ContainerType> 
struct container_detail
{
    using begin_type = decltype(std::declval<ContainerType>().begin());
    using end_type = decltype(std::declval<ContainerType>().end());
    using deref_begin_type = decltype(*std::declval<begin_type>());
    using moving_deref_begin_type = decltype(std::move(*std::declval<begin_type>()));
    using begin_end_noteq_type = decltype(std::declval<begin_type>() != std::declval<end_type>());

    static constexpr bool has_clear =
        member_tests::has_clear<ContainerType>::value;

    static constexpr bool is_convertible_to_value = 
        std::is_convertible<deref_begin_type, ValueType>::value;

    static constexpr bool is_moveable_to_value = 
        std::is_convertible<moving_deref_begin_type, ValueType>::value;

    static constexpr bool is_begin_end_bool_comparable = 
        std::is_convertible<begin_end_noteq_type, bool>::value;
};

template <typename BeginType, typename EndType, typename ValueType>
struct iter_pair_detail
{
    using begin_type = BeginType;
    using end_type = EndType;
    using deref_begin_type = decltype(*std::declval<begin_type>());
    using moving_deref_begin_type = decltype(std::move(*std::declval<begin_type>()));
    using begin_end_noteq_type = decltype(std::declval<begin_type>() != std::declval<end_type>());

    static constexpr bool is_convertible_to_value = 
        std::is_convertible<deref_begin_type, ValueType>::value;

    static constexpr bool is_moveable_to_value = 
        std::is_convertible<moving_deref_begin_type, ValueType>::value;

    static constexpr bool is_begin_end_bool_comparable = 
        std::is_convertible<begin_end_noteq_type, bool>::value;
};

} // detail

/**
 * @brief Test if ContainerType is a container of values that can be
 *        converted into ValueType (by copying).
 */
template <
    typename ContainerType,
    typename ValueType,
    typename Detail = detail::container_detail<ValueType, ContainerType>,
    bool is_convertible_to_value = Detail::is_convertible_to_value,
    bool is_begin_end_bool_comparable = Detail::is_begin_end_bool_comparable>
struct is_readable_container
    : std::false_type
{};

template <typename ContainerType, typename ValueType, typename Detail>
struct is_readable_container<ContainerType, ValueType, Detail, true, true>
    : std::true_type, Detail
{};

/**
 * @brief Test if ContainerType is a container of values that can be
 *        converted into ValueType (by moving), and if the container
 *        can be erased (by clear()).
 */
template <
    typename ContainerType,
    typename ValueType,
    typename Detail = detail::container_detail<ValueType, ContainerType>,
    bool has_clear = Detail::has_clear,
    bool is_moveable_to_value = Detail::is_moveable_to_value,
    bool is_begin_end_bool_comparable = Detail::is_begin_end_bool_comparable>
struct is_eraseable_container
    : std::false_type
{};

template <typename ContainerType, typename ValueType, typename Detail>
struct is_eraseable_container<ContainerType, ValueType, Detail, true, true, true>
    : std::true_type, Detail
{};

/**
 * @brief Test if the template parameters specify a pair of iterators
 * that can act as begin and end iterators for a container, and that
 * the dereferenced iterator can be converted to ValueType.
 */
template <
    typename BeginType,
    typename EndType,
    typename ValueType,
    typename Detail = detail::iter_pair_detail<BeginType, EndType, ValueType>,
    bool is_convertible_to_value = Detail::is_convertible_to_value,
    bool is_begin_end_bool_comparable = Detail::is_begin_end_bool_comparable>
struct is_readable_iter_pair : std::false_type
{};

template <typename BeginType, typename EndType, typename ValueType, typename Detail>
struct is_readable_iter_pair<BeginType, EndType, ValueType, Detail, true, true>
    : std::true_type, Detail
{};

} // container_traits

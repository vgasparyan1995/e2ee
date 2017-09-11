#pragma once

#include <type_traits>

template <typename ContainerT>
struct has_begin
{
    using Yes = uint8_t;
    using No = uint16_t;

    template <typename T>
    static Yes test( decltype( std::declval<T>().begin() )* );
    template <typename T>
    static No test(...);

    static const bool value = (sizeof(test<ContainerT>(nullptr)) == sizeof(Yes));
};

template <typename ContainerT>
struct has_size
{
    using Yes = uint8_t;
    using No = uint16_t;

    template <typename T>
    static Yes test( decltype( std::declval<T>().size() )* );
    template <typename T>
    static No test(...);

    static const bool value = (sizeof(test<ContainerT>(nullptr)) == sizeof(Yes));
};

template <typename ContainerT>
struct is_stl_container
{
    static const bool value = has_begin<ContainerT>::value && has_size<ContainerT>::value;
};

using Byte = uint8_t;
using ByteArray = std::basic_string<Byte>;

template <typename NumericT>
ByteArray toByteArray(const NumericT value, typename std::enable_if<std::is_arithmetic<NumericT>::value>::type* = nullptr);
template <typename NumericT>
int fromByteArray(const Byte* data, NumericT& value, typename std::enable_if<std::is_arithmetic<NumericT>::value>::type* = nullptr);
template <typename ContainerT>
ByteArray toByteArray(const ContainerT& container, typename std::enable_if<is_stl_container<ContainerT>::value>::type* = nullptr);
template <typename ContainerT>
int fromByteArray(const Byte* data, ContainerT& container, typename std::enable_if<is_stl_container<ContainerT>::value>::type* = nullptr);
template <typename U, typename V>
ByteArray toByteArray(const std::pair<U, V>& p);
template <typename U, typename V>
int fromByteArray(const Byte* data, std::pair<U, V>& p);

template <typename NumericT>
ByteArray toByteArray(const NumericT value, typename std::enable_if<std::is_arithmetic<NumericT>::value>::type* /*= nullptr*/)
{
    ByteArray result;
    auto v = reinterpret_cast<const Byte*>(&value);
    for (unsigned i = 0; i < sizeof(NumericT); ++i) {
        result.push_back(v[i]);
    }
    return result;
}

template <typename NumericT>
int fromByteArray(const Byte* data, NumericT& value, typename std::enable_if<std::is_arithmetic<NumericT>::value>::type* /*= nullptr*/)
{
    int bytesRead = sizeof(NumericT);
    value = *reinterpret_cast<const NumericT*>(data);
    return bytesRead;
}

template <typename ContainerT>
ByteArray toByteArray(const ContainerT& container, typename std::enable_if<gix::utility::is_stl_container<ContainerT>::value>::type* /*= nullptr*/)
{
    ByteArray result;
    result += toByteArray(static_cast<uint32_t>(container.size()));
    for (const auto& item : container) {
        result += toByteArray(item);
    }
    return result;
}

template <typename ContainerT>
int fromByteArray(const Byte* data, ContainerT& container, typename std::enable_if<gix::utility::is_stl_container<ContainerT>::value>::type* /*= nullptr*/)
{
    int bytesRead = 0;
    uint32_t containerSize = 0;
    bytesRead += fromByteArray(data, containerSize);
    for (uint32_t i = 0; i < containerSize; ++i) {
        typename ContainerT::value_type value;
        bytesRead += fromByteArray(data + bytesRead, value);
        container.insert(container.end(), value); // A generic interface which provides almost every STL container
    }
    return bytesRead;
}

template <typename U, typename V>
ByteArray toByteArray(const std::pair<U, V>& p)
{
    ByteArray result;
    result += toByteArray(p.first);
    result += toByteArray(p.second);
    return result;
}

template <typename U, typename V>
int fromByteArray(const Byte* data, std::pair<U, V>& p)
{
    int bytesRead = 0;
    auto& first = const_cast<typename std::remove_const<U>::type&>(p.first);
    auto& second = const_cast<typename std::remove_const<V>::type&>(p.second);
    bytesRead += fromByteArray(data + bytesRead, first);
    bytesRead += fromByteArray(data + bytesRead, second);
    return bytesRead;
}

#include "BigInteger.h"

template <>
ByteArray toByteArray<BigInteger>(const BigInteger& value)
{
    return toByteArray(value.raw_data());
}

template <>
int fromByteArray(const Byte* data, BigInteger& value)
{

}

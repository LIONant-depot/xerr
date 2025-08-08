#ifndef XERROR_H
#define XERROR_H
#pragma once

#include <array>
#include <cassert>

namespace xerr_details
{
    template<std::size_t N>
    struct string_literal
    {
        std::array<char, N> m_Value;
        consteval string_literal(const char(&str)[N]) noexcept { for (std::size_t i = 0; i < N; ++i) m_Value[i] = str[i]; }
    };

    // Simple constexpr hash function for strings
    // djb2 hash algorithm
#if defined(_MSC_VER)
    template<typename T_STATE_ENUM>
    consteval static uint32_t create_uid(void) noexcept { const char* s = __FUNCSIG__;         uint32_t hash = 5381; for (int i = 0; (s[i]); ++i)  hash = ((hash << 5) + hash) + s[i]; return hash; }
#else
    template<typename T_STATE_ENUM>
    consteval static uint32_t create_uid(void) noexcept { const char* s = __PRETTY_FUNCTION__; uint32_t hash = 5381; for (int i = 0; (s[i]); ++i)  hash = ((hash << 5) + hash) + s[i]; return hash; }
#endif

    // Template to get a unique 32 bit ID form a type
    // Cross-compiler macro for type name
    template<typename T_STATE_ENUM>
    inline constexpr static uint32_t uid_v = create_uid<T_STATE_ENUM>();

    template< std::size_t T_SIZE_V >
    struct info_construct
    {
        std::uint32_t m_TypeGUID;
        char          m_State;
        char          m_Message[T_SIZE_V];
    };

    template <string_literal T_STR_V, auto T_STATE_V>
    inline constexpr static auto data_v = []() consteval noexcept
    {
        auto a = info_construct<T_STR_V.m_Value.size()>{};
        a.m_TypeGUID = uid_v<decltype(T_STATE_V)>;
        a.m_State    = static_cast<char>(T_STATE_V);
        for (std::size_t i = 0; (a.m_Message[i] = T_STR_V.m_Value[i]); ++i){}
        return a;
    }();
}

struct xerr
{
    // The default states for xerr. Please note that this could be customized per error class
    enum class default_states : std::uint8_t
    { OK        = 0
    , FAILURE   = 1
    // The user can customize their own error states by copying this enum and then
    // adding additional states below these two
    };

    constexpr               operator bool   (void)                  const   noexcept { return !!m_pMessage; }
    inline void             clear           (void)                          noexcept { m_pMessage = nullptr; }
    constexpr std::uint32_t getStateUID     (void)                  const   noexcept { return m_pMessage ? reinterpret_cast<const std::uint32_t&>(m_pMessage[-5]) : 0; }

    template< typename T_STATE_ENUM >
    constexpr bool          isState         (void)                  const   noexcept { return m_pMessage == nullptr || xerr_details::uid_v<T_STATE_ENUM> == getStateUID(); }

    template< typename T_STATE_ENUM >
    constexpr T_STATE_ENUM  getState        (void)                  const   noexcept { assert(isState<T_STATE_ENUM>() || m_pMessage[-1] == static_cast<char>(T_STATE_ENUM::FAILURE)); return m_pMessage ? static_cast<T_STATE_ENUM>(m_pMessage[-1]) : T_STATE_ENUM::OK; }

    template <auto T_STATE_V, xerr_details::string_literal T_STR_V>
    consteval static xerr   create          (void)                          noexcept requires (std::is_enum_v<decltype(T_STATE_V)>) { static_assert(sizeof(T_STATE_V) == 1); return { xerr_details::data_v<T_STR_V, T_STATE_V>.m_Message }; }

    template <xerr_details::string_literal T_STR_V>
    consteval static xerr   create_f        (void)                          noexcept { return { xerr_details::data_v<T_STR_V, default_states::FAILURE>.m_Message }; }

    const char* m_pMessage = nullptr;
};

#endif

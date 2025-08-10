namespace xerr_details
{
    // Simple constexpr hash function for strings
    // djb2 hash algorithm
#if defined(_MSC_VER)

    //------------------------------------------------------------------------------------

    template<typename T_STATE_ENUM>
    consteval static uint32_t create_uid(void) noexcept 
    { 
        const char*     s       = __FUNCSIG__;
        std::uint32_t   hash    = 5381; 
        for (int i = 0; (s[i]); ++i)  
            hash = ((hash << 5) + hash) + s[i]; 
        return hash; 
    }

    //------------------------------------------------------------------------------------

    namespace details
    {
        template<auto T>
        consteval auto getValueTypeName() noexcept
        {
            constexpr const char* sig = __FUNCSIG__;
            constexpr std::size_t len = [&] {
                std::size_t i = 0;
                while (sig[i] != '\0') ++i;
                return i;
                }();

            constexpr const char* prefix = "getValueTypeName<";
            constexpr std::size_t prefix_len = [&] {
                std::size_t i = 0;
                while (prefix[i] != '\0') ++i;
                return i;
                }();

            std::size_t start = 0;
            for (std::size_t i = 0; i < len - prefix_len; ++i)
            {
                bool match = true;
                for (std::size_t j = 0; j < prefix_len; ++j)
                {
                    if (sig[i + j] != prefix[j])
                    {
                        match = false;
                        break;
                    }
                }
                if (match)
                {
                    start = i + prefix_len;
                    break;
                }
            }

            std::size_t end = start;
            while (sig[end] != '>' && sig[end] != '\0') ++end;

            std::array<char, 128> result = {};
            for (std::size_t i = start; i < end && i - start < result.size(); ++i)
            {
                result[i - start] = sig[i];
            }
            result[end - start] = '\0';

            return result;
        }
    }
#else

    //------------------------------------------------------------------------------------

    template<typename T_STATE_ENUM>
    consteval static uint32_t create_uid(void) noexcept
    {
        const char*     s       = __PRETTY_FUNCTION__;
        std::uint32_t   hash    = 5381;
        for (int i = 0; (s[i]); ++i)  
            hash = ((hash << 5) + hash) + s[i];
        return hash;
    }

    //------------------------------------------------------------------------------------

    namespace details
    {
        template<auto T>
        consteval auto getValueTypeName() noexcept
        {
            constexpr const char* sig = __PRETTY_FUNCTION__;
            constexpr std::size_t len = [] { std::size_t i = 0; while (sig[i] != '\0') ++i; return i; }();
            constexpr const char* prefix = "T = ";
            constexpr std::size_t prefix_len = [] { std::size_t i = 0; while (prefix[i] != '\0') ++i; return i; }();

            std::size_t start = 0;
            for (std::size_t i = 0; i < len - prefix_len; ++i)
            {
                bool match = true;
                for (std::size_t j = 0; j < prefix_len; ++j)
                {
                    if (sig[i + j] != prefix[j]) { match = false; break; }
                }
                if (match) { start = i + prefix_len; break; }
            }
            std::size_t end = start;
            while (sig[end] != ']' && sig[end] != ';' && sig[end] != '\0') ++end;
            std::array<char, 128> result = {};

            for (std::size_t i = start; i < end && i - start < result.size(); ++i)
            {
                result[i - start] = sig[i];
            }
            result[end - start] = '\0';
            return result;
        }
    }
#endif

    //------------------------------------------------------------------------------------

    template<auto T>
    inline static constexpr auto value_type_name_v = details::getValueTypeName<T>();

    //------------------------------------------------------------------------------------

    // Template to get a unique 32 bit ID form a type
    // Cross-compiler macro for type name
    template<typename T_STATE_ENUM>
    inline constexpr static uint32_t uid_v = create_uid<T_STATE_ENUM>();

    //------------------------------------------------------------------------------------

    template< std::size_t T_SIZE_V >
    struct info_construct
    {
        std::uint32_t m_TypeGUID;
        char          m_State;
        char          m_Message[T_SIZE_V];
    };

    //------------------------------------------------------------------------------------

    template <string_literal T_STR_V, auto T_STATE_V>
    inline constexpr static auto data_v = []() consteval noexcept
    {
        auto a = info_construct<T_STR_V.m_Value.size()>{};
        a.m_TypeGUID = uid_v<decltype(T_STATE_V)>;
        a.m_State = static_cast<char>(T_STATE_V);
        for (std::size_t i = 0; (a.m_Message[i] = T_STR_V.m_Value[i]); ++i) {}
        return a;
    }();

    //------------------------------------------------------------------------------------

    inline chain_pool::chain_pool(void) noexcept
    {
        for (std::int16_t i = 0; i < m_Pool.size(); ++i) m_Pool[i].m_iNext = i + 1;
        m_Pool[m_Pool.size() - 1].m_iNext = -1;
        m_iEmpty.store(0, std::memory_order_relaxed);
    }

    //------------------------------------------------------------------------------------
    // Lockless allocator
    inline std::int16_t chain_pool::Alloc(void) noexcept
    {
        // Pop from free list
        std::int16_t idx = m_iEmpty.load(std::memory_order_acquire);
        while (idx != -1)
        {
            std::int16_t next = m_Pool[idx].m_iNext;
            if (m_iEmpty.compare_exchange_weak(idx, next, std::memory_order_acq_rel))
            {
                return idx;
            }
        }

        // Fallback: use index 0 if pool exhausted
        assert(false);
        return 0;
    }

    //------------------------------------------------------------------------------------

    inline void chain_pool::Free(std::int16_t& iHead, std::int16_t& iTail) noexcept
    {
        if (iHead == -1)
        {
            assert(iTail == -1);
            return;
        }

        // Atomically push chain to free list
        std::int16_t old_head = m_iEmpty.load(std::memory_order_relaxed);
        do
        {
            m_Pool[iTail].m_iNext = old_head;

        } while (!m_iEmpty.compare_exchange_weak(old_head, iHead, std::memory_order_acq_rel));


        iHead = iTail = -1;
    }

    //------------------------------------------------------------------------------------

    thread_local inline static std::int16_t g_iCurChain = -1; // -1 means no chain
    thread_local inline static std::int16_t g_iCurTail  = -1; // -1 means no chain

    //------------------------------------------------------------------------------------

    inline auto& CreateEntry(void) noexcept
    {
        auto  iNewIndex = xerr::m_ChainPool.Alloc();
        auto& Entry     = xerr::m_ChainPool.m_Pool[iNewIndex];
        Entry.m_iNext   = xerr_details::g_iCurChain;
        Entry.m_iPrev   = -1;

        if (xerr_details::g_iCurChain != -1) xerr::m_ChainPool.m_Pool[xerr_details::g_iCurChain].m_iPrev = iNewIndex;
        xerr_details::g_iCurChain = iNewIndex;
        return Entry;
    };
}

//------------------------------------------------------------------------------------

constexpr
xerr::operator bool(void) const noexcept
{
    return !!m_pMessage;
}

//------------------------------------------------------------------------------------

inline
void xerr::clear(void) noexcept
{
    m_pMessage = nullptr;
    xerr_details::g_iCurChain = -1;
}

//------------------------------------------------------------------------------------

constexpr
std::uint32_t xerr::getStateUID(void) const noexcept
{
    return m_pMessage ? reinterpret_cast<const std::uint32_t&>(m_pMessage[-5]) : 0;
}

//------------------------------------------------------------------------------------

template< typename T_STATE_ENUM > consteval
std::uint32_t xerr::fromStateUID(void) noexcept requires (std::is_enum_v<T_STATE_ENUM>)
{
    return xerr_details::uid_v<T_STATE_ENUM>;
}

//------------------------------------------------------------------------------------

template< typename T_STATE_ENUM > constexpr
bool xerr::isState(void) const noexcept requires (std::is_enum_v<T_STATE_ENUM>)
{
    return m_pMessage == nullptr || xerr_details::uid_v<T_STATE_ENUM> == getStateUID();
}

//------------------------------------------------------------------------------------

constexpr
bool xerr::hasChain(void) const noexcept
{
    return m_pMessage != nullptr && xerr_details::g_iCurChain != -1;
}

//------------------------------------------------------------------------------------

inline
std::string_view xerr::getMessage(void) const noexcept
{
    if (m_pMessage == nullptr) return {};

    int i = 0;
    while (m_pMessage[i] && m_pMessage[i] != '|') i++;

    return { m_pMessage, m_pMessage + i };
}

//------------------------------------------------------------------------------------

inline
std::string_view xerr::getHint(void) const noexcept
{
    if (m_pMessage == nullptr) return {};
    int iStart = 0;
    while (m_pMessage[iStart] && m_pMessage[iStart] != '|') iStart++;
    if (m_pMessage[iStart] != '|') return {};
    ++iStart;
    int iEnd = iStart + 1;
    while (m_pMessage[iEnd]) iEnd++;
    return { m_pMessage + iStart, m_pMessage + iEnd };
}

//------------------------------------------------------------------------------------

template< typename T_CALLBACK> inline
void xerr::ForEachInChain(T_CALLBACK&& Callback) const noexcept requires std::invocable<T_CALLBACK, xerr>
{
    if( m_pMessage == nullptr ) return;

    if(xerr_details::g_iCurChain == -1)
    {
        Callback(*this);
    }
    else
    {
        for (auto i = xerr_details::g_iCurTail; i != -1; i = m_ChainPool.m_Pool[i].m_iPrev)
            Callback(xerr{ m_ChainPool.m_Pool[i].m_pError });
    }
}

//------------------------------------------------------------------------------------

template< typename T_CALLBACK> inline
void xerr::ForEachInChainBackwards(T_CALLBACK&& Callback) const noexcept requires std::invocable<T_CALLBACK, xerr>
{
    if (m_pMessage == nullptr) return;

    if (xerr_details::g_iCurChain == -1)
    {
        Callback(*this);
    }
    else
    {
        for (auto i = xerr_details::g_iCurChain; i != -1; i = m_ChainPool.m_Pool[i].m_iNext)
            Callback(xerr{ m_ChainPool.m_Pool[i].m_pError });
    }
}

//------------------------------------------------------------------------------------

template< typename T_STATE_ENUM > constexpr
T_STATE_ENUM xerr::getState(void) const noexcept requires (std::is_enum_v<T_STATE_ENUM>)
{
    assert(isState<T_STATE_ENUM>() || m_pMessage[-1] == static_cast<char>(T_STATE_ENUM::FAILURE));
    return m_pMessage ? static_cast<T_STATE_ENUM>(m_pMessage[-1]) : T_STATE_ENUM::OK;
}

//------------------------------------------------------------------------------------

template <auto T_STATE_V, xerr_details::string_literal T_STR_V>  constexpr
 xerr xerr::create(void) noexcept requires (std::is_enum_v<decltype(T_STATE_V)>)
{
    static_assert(sizeof(T_STATE_V) == 1);
    if (xerr_details::g_iCurChain != -1) m_ChainPool.Free(xerr_details::g_iCurChain, xerr_details::g_iCurTail);
    if (m_pCallback) m_pCallback(xerr_details::value_type_name_v<T_STATE_V>.data(), static_cast<std::uint8_t>(T_STATE_V), T_STR_V.m_Value.data());
    return { xerr_details::data_v<T_STR_V, T_STATE_V>.m_Message };
}

//------------------------------------------------------------------------------------

template <auto T_STATE_V, xerr_details::string_literal T_STR_V> constexpr
 xerr xerr::create(const xerr PrevError) noexcept requires (std::is_enum_v<decltype(T_STATE_V)>) 
{
    static_assert(sizeof(T_STATE_V) == 1);

    if (xerr_details::g_iCurChain == -1)
    {
        xerr_details::CreateEntry().m_pError = PrevError.m_pMessage;
        xerr_details::g_iCurTail = xerr_details::g_iCurChain;
    }

    auto Err = create<T_STATE_V, T_STR_V>();
    xerr_details::CreateEntry().m_pError = Err.m_pMessage;
    if (m_pCallback) m_pCallback(xerr_details::value_type_name_v<T_STATE_V>.data(), static_cast<std::uint8_t>(T_STATE_V), T_STR_V.m_Value.data());
    return Err;
}

//------------------------------------------------------------------------------------

template <typename T_STATE_ENUM, xerr_details::string_literal T_STR_V> constexpr
xerr xerr::create_f(void) noexcept requires (std::is_enum_v<T_STATE_ENUM>)
{
    return create<T_STATE_ENUM::FAILURE, T_STR_V>();
}

//------------------------------------------------------------------------------------

template <typename T_STATE_ENUM, xerr_details::string_literal T_STR_V> constexpr
xerr xerr::create_f(const xerr PrevError) noexcept requires (std::is_enum_v<T_STATE_ENUM>)
{
    return create<T_STATE_ENUM::FAILURE, T_STR_V>(PrevError);
}

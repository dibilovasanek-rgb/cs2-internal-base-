#pragma once
#include <cstdint>
constexpr auto INVALID_EHANDLE_INDEX = 0xffffffff;
constexpr auto ENT_ENTRY_MASK = 0x7fff;
constexpr auto NUM_SERIAL_NUM_SHIFT_BITS = 15;

class c_base_entity;
class c_base_handle
{
public:
    c_base_handle() : m_index(INVALID_EHANDLE_INDEX) {}
    c_base_handle(uint32_t value) : m_index(value) {}
    c_base_handle(int entry, int serial_number) { init(entry, serial_number); }

    void init(int entry, int serial_number);

    bool is_valid() const;

    int get_entry_index() const;
    int get_serial_number() const;

    int to_int() const;
    bool operator==(c_base_handle rhs) const;

    c_base_entity* get() const;
protected:
    uint32_t m_index;
};

inline void c_base_handle::init(int entry, int serial_number)
{
    m_index = entry | (serial_number << NUM_SERIAL_NUM_SHIFT_BITS);
}

inline bool c_base_handle::is_valid() const
{
    return m_index != INVALID_EHANDLE_INDEX;
}

inline int c_base_handle::get_entry_index() const
{
    return is_valid() ? m_index & ENT_ENTRY_MASK : ENT_ENTRY_MASK;
}

inline int c_base_handle::get_serial_number() const
{
    return m_index >> NUM_SERIAL_NUM_SHIFT_BITS;
}

inline int c_base_handle::to_int() const
{
    return static_cast<int>(m_index);
}

inline bool c_base_handle::operator==(c_base_handle rhs) const
{
    return m_index == rhs.m_index;
}



template <typename T>
class c_handle : public c_base_handle
{
public:
    auto get() const
    {
        return reinterpret_cast<T*>(c_base_handle::get());
    }
};

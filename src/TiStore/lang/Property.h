#pragma once

#include "TiStore/basic/cstdint"

// Define offsetof macro
#if defined(_MSC_VER) && !defined(_CRT_USE_BUILTIN_OFFSETOF)
    #ifdef __cplusplus
        #define class_offset_of(host, member_name) ((size_t)&reinterpret_cast<char const volatile &>((((host *)0)->member_name)))
    #else
        #define class_offset_of(host, member_name) ((size_t)&(((host *)0)->member_name))
    #endif
#else
    #define class_offset_of(host, member_name) __builtin_offsetof(host, member_name)
#endif

#define DOWN_CONST_CAST_THIS(this_type, This)   (reinterpret_cast<char *>(const_cast<this_type *>(This)))
#define DOWN_CAST_THIS(this_type, This)         (reinterpret_cast<char *>(reinterpret_cast<this_type *>(This)))

namespace TiStore {

template <typename T, typename Host = void,
          const T (Host::*Getter)() const = nullptr,
          void (Host::*Setter)(const T & value) = nullptr>
class Property {
public:
    typedef T value_type;
    typedef Property<T, Host, Getter, Setter> this_type;

    static intptr_t property_offset;

private:
    value_type value_;

public:
    Property() : value_(0) {}
    Property(Host * host, value_type value) : value_(value) {
        setter(value);
    }
    ~Property() {}

    intptr_t get_offset() const {
        return property_offset;
    }

    const value_type getter() const {
        if (Getter == nullptr)
            return value_;
        else
            return ((reinterpret_cast<Host *>(DOWN_CONST_CAST_THIS(this_type, this) - property_offset))->*Getter)();
    }

    void setter(const value_type & value) {
        if (Setter == nullptr)
            value_ = value;
        else
            ((reinterpret_cast<Host *>(DOWN_CAST_THIS(this_type, this) - property_offset))->*Setter)(value);
    }

    this_type & setter_ref(const value_type & value) {
        setter(value);
        return *this;
    }

    operator const value_type () {
        return getter();
    }

    this_type & operator = (const value_type & value) {
        return setter_ref(value);
    } 
};

template <typename T, typename Host,
          const T (Host::*Getter)() const,
          void (Host::*Setter)(const T & value)>
intptr_t Property<T, Host, Getter, Setter>::property_offset = 0; //offsetof(Host, MemberName);

} // namespace TiStore

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

//
// CppCon2015: Effective C++ Implementation of Class Properties
// See: https://github.com/CppCon/CppCon2015/tree/master/Tutorials/Effective%20C%2B%2B%20Implementation%20of%20Class%20Properties
//
// Github See: https://github.com/bitekas/properties
//

template <typename Host, typename MemberPropertyType>
constexpr std::size_t class_offsetof(MemberPropertyType MemberPropertyPtr) {
    return reinterpret_cast<std::size_t>(&((Host *)0->*MemberPropertyPtr));
};

template <typename T>
class Property {
public:
    typedef T value_type;
    typedef Property<T> this_type;

private:
    value_type value_;

public:
    Property() : value_(0) {}
    Property(value_type value) : value_(value) {
        setter(value);
    }
    ~Property() {}

    const value_type getter() const {
        return value_;
    }

    void setter(const value_type & value) {
        value_ = value;
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

template <typename T>
class PropertyWithStorage {
public:
    typedef T value_type;
    typedef PropertyWithStorage<T> this_type;

private:
    value_type value_;
};

template <typename T, typename Host,
          typename Host::member_property_ptr (*GetPropertyOffsetFunc)(),
          const T (Host::*Getter)() const,
          void (Host::*Setter)(T const & value)>
class PropertyWithGetSet {
public:
    typedef T value_type;
    typedef PropertyWithGetSet<T, Host, GetPropertyOffsetFunc, Getter, Setter> this_type;

    static intptr_t s_property_offset;

public:
    PropertyWithGetSet() {
        setter(0);
    }
    PropertyWithGetSet(value_type value) {
        setter(value);
    }
    ~PropertyWithGetSet() {}

    static intptr_t get_offset() {
        assert(GetPropertyOffsetFunc != nullptr);
        const intptr_t property_offset = reinterpret_cast<intptr_t>(&((Host *)0->*((*GetPropertyOffsetFunc)())));
        return property_offset;
    }

    const value_type getter() const {
        assert(Getter != nullptr);
        assert(GetPropertyOffsetFunc != nullptr);
        const intptr_t property_offset = reinterpret_cast<intptr_t>(&((Host *)0->*((*GetPropertyOffsetFunc)())));
        return ((reinterpret_cast<Host *>(DOWN_CONST_CAST_THIS(this_type, this) - property_offset))->*Getter)();
    }

    void setter(const value_type & value) {
        assert(Setter != nullptr);
        assert(GetPropertyOffsetFunc != nullptr);
        const intptr_t property_offset = reinterpret_cast<intptr_t>(&((Host *)0->*((*GetPropertyOffsetFunc)())));
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
          typename Host::member_property_ptr (*GetPropertyOffsetFunc)(),
          const T (Host::*Getter)() const,
          void (Host::*Setter)(const T & value)>
intptr_t PropertyWithGetSet<T, Host, GetPropertyOffsetFunc, Getter, Setter>::s_property_offset = 0;

template <typename T, typename Host = void,
          //void * Host::*MemberPropertyPtr = nullptr,
          //typename Host::member_ptr (Host::*MemberPropertyPtr)() = nullptr,
          typename Host::member_property_ptr (*GetPropertyOffsetFunc)() = nullptr,
          const T (Host::*Getter)() const = nullptr,
          void (Host::*Setter)(const T & value) = nullptr>
class PropertyAdvance {
public:
    typedef T value_type;
    typedef PropertyAdvance<T, Host, GetPropertyOffsetFunc, Getter, Setter> this_type;
    typedef typename Host::member_ptr member_ptr;

    static intptr_t property_offset;
    //static const intptr_t offset = class_offsetof<Host, decltype(MemberPropertyPtr)>(MemberPropertyPtr);
    //static const intptr_t offset = class_offsetof<Host>(MemberPropertyPtr);
    //static const intptr_t offset2 = (intptr_t)reinterpret_cast<std::size_t>(&((Host *)0->*MemberPropertyPtr));
    //static const intptr_t member_offset = reinterpret_cast<std::size_t>(&((Host *)0->*((*MemberPropertyPtr)())));
    //static const intptr_t member_offset2 = reinterpret_cast<std::size_t>(&((Host *)0->*(Host::get_property_offset())));
    //static const member_ptr mem_ptr = Host::get_property_offset();

private:
    value_type value_;

public:
    PropertyAdvance() : value_(0) {}
    PropertyAdvance(Host * host, value_type value) : value_(value) {
        setter(value);
    }
    ~PropertyAdvance() {}

    intptr_t get_offset() const {
        //intptr_t member_offset = reinterpret_cast<std::intptr_t>(&((Host *)0->*(Host::get_property_offset())));
        const intptr_t member_offset = reinterpret_cast<std::intptr_t>(&((Host *)0->*((*GetPropertyOffsetFunc)())));
        printf("This              = 0x%p\n", this);
        printf("MemberPropertyPtr = 0x%p\n", MemberPropertyPtr);
        printf("member_offset     = %zd\n", member_offset);
        printf("(Host::*MemberPropertyPtr)() = 0x%p\n",
            //((reinterpret_cast<Host *>(DOWN_CONST_CAST_THIS(this_type, this) - property_offset))->*MemberPropertyPtr)()
            (*MemberPropertyPtr)()
            //Host::get_property_offset()
            );
        return property_offset;
    }

    const value_type getter() const {
        if (Getter == nullptr) {
            return value_;
        }
        else {
            const intptr_t member_offset = reinterpret_cast<std::intptr_t>(&((Host *)0->*((*GetPropertyOffsetFunc)())));
            return ((reinterpret_cast<Host *>(DOWN_CONST_CAST_THIS(this_type, this) - member_offset))->*Getter)();
        }
    }

    void setter(const value_type & value) {
        if (Setter == nullptr) {
            value_ = value;
        }
        else {
            const intptr_t member_offset = reinterpret_cast<std::intptr_t>(&((Host *)0->*((*GetPropertyOffsetFunc)())));
            ((reinterpret_cast<Host *>(DOWN_CAST_THIS(this_type, this) - member_offset))->*Setter)(value);
        }
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
          //void * Host::*MemberPropertyPtr,
          //typename Host::member_ptr (Host::*MemberPropertyPtr)(),
          typename Host::member_property_ptr (*GetPropertyOffsetFunc)(),
          const T (Host::*Getter)() const,
          void (Host::*Setter)(const T & value)>
intptr_t PropertyAdvance<T, Host, GetPropertyOffsetFunc, Getter, Setter>::property_offset = 0; //offsetof(Host, MemberName);

} // namespace TiStore

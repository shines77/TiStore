
#include "test.h"

#include "TiStore/lang/TypeInfo.h"
#include "TiStore/kv/BloomFilter.h"
#include "TiStore/kv/BloomFilterFixed.h"
#include "TiStore/traits.h"

#include <stdio.h>

#include <cstdio>
#include <iostream>
#include <type_traits>
#include <typeinfo>

using namespace TiStore;

void test_typeinfo_module()
{
    printf("test_typeinfo_module()\n");
    printf("\n");

    printf("TypeInfo<char>::type_id()                           = %11u\n", TiStore::TypeInfo<char>::type_id());
    printf("TypeInfo<int>::type_id()                            = %11u\n", TiStore::TypeInfo<int>::type_id());
    printf("TypeInfo<StandardBloomFilter<128, 2>>::type_id()    = %11u\n", TiStore::TypeInfo<StandardBloomFilterFixed<128, 2>>::type_id());
    printf("TypeInfo<StandardBloomFilter<128, 2>>::type_id()    = %11u\n", TiStore::TypeInfo<StandardBloomFilterFixed<128, 2>>::type_id());
    printf("TypeInfo<StandardBloomFilter<128, 4>>::type_id()    = %11u\n", TiStore::TypeInfo<StandardBloomFilterFixed<128, 4>>::type_id());
    printf("TypeInfo<char>::type_id()                           = %11u\n", TiStore::TypeInfo<char>::type_id());
    printf("\n");

    printf("TypeInfo<char>::hash_code()                         = 0x%08X\n", TiStore::TypeInfo<char>::hash_code());
    printf("TypeInfo<int>::hash_code()                          = 0x%08X\n", TiStore::TypeInfo<int>::hash_code());
    printf("TypeInfo<StandardBloomFilter<128, 2>>::hash_code()  = 0x%08X\n", TiStore::TypeInfo<StandardBloomFilterFixed<128, 2>>::hash_code());
    printf("TypeInfo<StandardBloomFilter<128, 2>>::hash_code()  = 0x%08X\n", TiStore::TypeInfo<StandardBloomFilterFixed<128, 2>>::hash_code());
    printf("TypeInfo<StandardBloomFilter<128, 4>>::hash_code()  = 0x%08X\n", TiStore::TypeInfo<StandardBloomFilterFixed<128, 4>>::hash_code());
    printf("TypeInfo<char>::hash_code()                         = 0x%08X\n", TiStore::TypeInfo<char>::hash_code());
    printf("\n");
}

#define REMOVE_CONST_TEST(test_name, test_type, verify_type) \
    std::cout << #test_name " " \
              << (traits::is_same<test_type, verify_type>::value ? "passed" : "failed") << ", " \
              << "typeid("#test_type").name() = " << typeid(test_type).name() << std::endl

class foo {};

void test_std_remove_const_traist()
{
    std::cout << "test_std_remove_const_traist()" << std::endl;
    std::cout << std::endl;

    REMOVE_CONST_TEST(test1, std::remove_const<foo>::type, foo);
    REMOVE_CONST_TEST(test2, std::remove_const<foo &>::type, foo &);
    REMOVE_CONST_TEST(test3, std::remove_const<const foo>::type, foo);
    REMOVE_CONST_TEST(test4, std::remove_const<const foo &>::type, const foo &);
    REMOVE_CONST_TEST(test5, std::remove_const<foo *>::type, foo *);
    REMOVE_CONST_TEST(test6, std::remove_const<const foo *>::type, foo const *);
    REMOVE_CONST_TEST(test7, std::remove_const<foo * const>::type, foo *);
    REMOVE_CONST_TEST(test8, std::remove_const<const foo * const>::type, foo const *);
    std::cout << std::endl;
}

void test_remove_const_traist()
{
    std::cout << "test_remove_const_traist()" << std::endl;
    std::cout << std::endl;

    REMOVE_CONST_TEST(test1, traits::remove_const<foo>::type, foo);
    REMOVE_CONST_TEST(test2, traits::remove_const<foo &>::type, foo &);
    REMOVE_CONST_TEST(test3, traits::remove_const<const foo>::type, foo);
    REMOVE_CONST_TEST(test4, traits::remove_const<const foo &>::type, const foo &);
    REMOVE_CONST_TEST(test5, traits::remove_const<foo *>::type, foo *);
    REMOVE_CONST_TEST(test6, traits::remove_const<const foo *>::type, foo const *);
    REMOVE_CONST_TEST(test7, traits::remove_const<foo * const>::type, foo *);
    REMOVE_CONST_TEST(test8, traits::remove_const<const foo * const>::type, foo const *);
    std::cout << std::endl;
}

void test_std_remove_volatile()
{
    typedef std::remove_volatile<const int>::type type1;
    typedef std::remove_volatile<volatile int>::type type2;
    typedef std::remove_volatile<const volatile int>::type type3;
    typedef std::remove_volatile<const volatile int *>::type type4;
    typedef std::remove_volatile<int * const volatile>::type type5;

    std::cout << "test_std_remove_volatile()" << std::endl;
    std::cout << std::endl;
 
    std::cout << "test1 " << (std::is_same<type1, const int>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test2 " << (std::is_same<type2, int>::value
        ? "passed" : "failed") << std::endl;;
    std::cout << "test3 " << (std::is_same<type3, const int>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test4 " << (std::is_same<type4, const volatile int *>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test5 " << (std::is_same<type5, int * const>::value
        ? "passed" : "failed") << std::endl;
    std::cout << std::endl;
}

void test_remove_volatile()
{
    typedef traits::remove_volatile<const int>::type type1;
    typedef traits::remove_volatile<volatile int>::type type2;
    typedef traits::remove_volatile<const volatile int>::type type3;
    typedef traits::remove_volatile<const volatile int *>::type type4;
    typedef traits::remove_volatile<int * const volatile>::type type5;

    std::cout << "test_remove_volatile()" << std::endl;
    std::cout << std::endl;
 
    std::cout << "test1 " << (std::is_same<type1, const int>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test2 " << (std::is_same<type2, int>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test3 " << (std::is_same<type3, const int>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test4 " << (std::is_same<type4, const volatile int *>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test5 " << (std::is_same<type5, int * const>::value
        ? "passed" : "failed") << std::endl;
    std::cout << std::endl;
}

void test_std_remove_cv()
{
    typedef std::remove_cv<const int>::type type1;
    typedef std::remove_cv<volatile int>::type type2;
    typedef std::remove_cv<const volatile int>::type type3;
    typedef std::remove_cv<const volatile int *>::type type4;
    typedef std::remove_cv<int * const volatile>::type type5;

    std::cout << "test_std_remove_cv()" << std::endl;
    std::cout << std::endl;
 
    std::cout << "test1 " << (std::is_same<type1, int>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test2 " << (std::is_same<type2, int>::value
        ? "passed" : "failed") << std::endl;;
    std::cout << "test3 " << (std::is_same<type3, int>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test4 " << (std::is_same<type4, const volatile int *>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test5 " << (std::is_same<type5, int *>::value
        ? "passed" : "failed") << std::endl;
    std::cout << std::endl;
}

void test_remove_cv()
{
    typedef traits::remove_cv<const int>::type type1;
    typedef traits::remove_cv<volatile int>::type type2;
    typedef traits::remove_cv<const volatile int>::type type3;
    typedef traits::remove_cv<const volatile int *>::type type4;
    typedef traits::remove_cv<int * const volatile>::type type5;

    std::cout << "test_remove_cv()" << std::endl;
    std::cout << std::endl;
 
    std::cout << "test1 " << (std::is_same<type1, int>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test2 " << (std::is_same<type2, int>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test3 " << (std::is_same<type3, int>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test4 " << (std::is_same<type4, const volatile int *>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test5 " << (std::is_same<type5, int *>::value
        ? "passed" : "failed") << std::endl;
    std::cout << std::endl;
}

void test_traist()
{
    test_std_remove_const_traist();
    test_remove_const_traist();

    test_std_remove_volatile();
    test_remove_volatile();

    test_std_remove_cv();
    test_remove_cv();
}

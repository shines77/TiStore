
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <type_traits>
#include <typeinfo>

#include "TiStore/TiFS.h"
#include "TiStore/TiStore.h"
#include "TiStore/fs/Initor.h"
#include "TiStore/kv/BloomFilter.h"
#include "TiStore/kv/SkipList.h"
#include "TiStore/lang/TypeInfo.h"

#include "stop_watch.h"
#include "test.h"

using namespace TiStore;

static const int kVerbose = 1;
static const int kIterators = 10000000;

#define TEST_VERIFY_BOOL(value, format, var) \
    printf(format, var, bool_to_string(!!(value)).c_str());

static inline std::string bool_to_string(bool value)
{
    if (value)
        return "true";
    else
        return "false";
}

void test_bloomfilter_hash_impl(const char key[])
{
    StopWatch sw;
    /**/ volatile /**/ uint32_t hash;
    StandardBloomFilter sbf(8192, 10);
    Slice skey(key);
    printf("key = %s\n\n", skey.toString().c_str());

    hash = 0;
    sw.start();
    for (int i = 0; i < kIterators; ++i) {
        hash += sbf.maybe_match(skey);
    }
    sw.stop();
    printf("sbf.maybe_match()         time spent: %8.3f ms, hash: 0x%08X\n", sw.getElapsedMillisec(), hash);

    hash = 0;
    sw.start();
    for (int i = 0; i < kIterators; ++i) {
        hash += sbf.rocksdb_maybe_match(skey);
    }
    sw.stop();
    printf("sbf.rocksdb_maybe_match() time spent: %8.3f ms, hash: 0x%08X\n", sw.getElapsedMillisec(), hash);

    hash = 0;
    sw.start();
    for (int i = 0; i < kIterators; ++i) {
        hash += sbf.maybe_match2(skey);
    }
    sw.stop();
    printf("sbf.maybe_match2()        time spent: %8.3f ms, hash: 0x%08X\n", sw.getElapsedMillisec(), hash);

    hash = 0;
    sw.start();
    for (int i = 0; i < kIterators; ++i) {
        hash += sbf.maybe_match_openssl(skey);
    }
    sw.stop();
    printf("sbf.maybe_match_openssl() time spent: %8.3f ms, hash: 0x%08X\n\n", sw.getElapsedMillisec(), hash);
}

void test_bloomfilter_hash()
{
    test_bloomfilter_hash_impl("This is a hash test.");
    test_bloomfilter_hash_impl("This is a hash test...");
}

void test_bloomfilter_impl()
{
    StandardBloomFilter sbf(8192, 10);
    bool isMatch;

    Slice skey("abc");
    sbf.setVerbose(true);

    sbf.addKey(skey);
    sbf.addKey("skyinno");
    sbf.addKey("bigdata");
    printf("\n");

    isMatch = sbf.maybeMatch(skey);
    TEST_VERIFY_BOOL(isMatch, "maybeMatch(\"%s\"): %s\n", skey.data());

    isMatch = sbf.maybeMatch("skyinno");
    TEST_VERIFY_BOOL(isMatch, "maybeMatch(\"%s\"): %s\n", "skyinno");

    isMatch = sbf.maybeMatch("bigdata");
    TEST_VERIFY_BOOL(isMatch, "maybeMatch(\"%s\"): %s\n", "bigdata");
    printf("\n");

    isMatch = sbf.maybeMatch("ambari");
    TEST_VERIFY_BOOL(isMatch, "maybeMatch(\"%s\"): %s\n", "ambari");

    isMatch = sbf.maybeMatch("Karazhan");
    TEST_VERIFY_BOOL(isMatch, "maybeMatch(\"%s\"): %s\n", "Karazhan");

    isMatch = sbf.maybeMatch("pokemon go");
    TEST_VERIFY_BOOL(isMatch, "maybeMatch(\"%s\"): %s\n", "pokemon go");

    printf("\n");
}

static Slice MemIntegerKey(int i, char * buffer) {
    ::memcpy(buffer, &i, sizeof(i));
    return Slice(buffer, sizeof(i));
}

static int NextLength(int length) {
    if (length < 10) {
        length += 1;
    }
    else if (length < 100) {
        length += 10;
    }
    else if (length < 1000) {
        length += 100;
    }
    else if (length < 10000) {
        length += 1000;
    }
    else {
        length += 5000;
    }
    return length;
}

template <typename T>
double getFalsePositiveRate(T const & bloom_filter, int length) {
    char buffer[sizeof(int)];
    int result = 0;
    //int max_length = (length <= 10000) ? 10000 : length;
    int max_length = length;
    for (int i = 0; i < 10000; ++i) {
        if (bloom_filter.maybeMatch(MemIntegerKey(i + 1000000000, buffer))) {
            result++;
        }
    }
    return (double)result / 10000.0;
}

void test_bloomfilter_standard_false_positive_rate()
{
    char buffer[sizeof(int)];

    // Count number of filters that significantly exceed the false positive rate
    int mediocre_filters = 0;
    int good_filters = 0;
    bool isMatch;

    std::cout << "----------------------------------" << std::endl;
    std::cout << "StandardBloomFilter Test" << std::endl;
    std::cout << "----------------------------------" << std::endl;
    std::cout << std::endl;

    StandardBloomFilter bloomfilter(10000, 14, true);

    for (int length = 1; length <= 10000; length = NextLength(length)) {
        bloomfilter.setOption(length, 14, false);
        bloomfilter.reset();
        for (int i = 0; i < length; i++) {
            bloomfilter.addKey(MemIntegerKey(i, buffer));
        }

        // All added keys must match
        for (int i = 0; i < length; ++i) {
            isMatch = bloomfilter.maybeMatch(MemIntegerKey(i, buffer));
            if (!isMatch) {
                std::cout << "[Not Match], Length = " << length << "; key = " << i << std::endl;
            }
        }

        // Check false positive rate
        double rate = getFalsePositiveRate(bloomfilter, length);
        if (kVerbose >= 1) {
            fprintf(stderr, "False positive rates: %5.2f%% @ length = %6d ; bytes = %6d ; used_bits = %d\n",
                rate * 100.0, length, (int)bloomfilter.getFilterSize(), (int)bloomfilter.getUsedBits());
        }
        if (rate > 0.0125)
            mediocre_filters++;  // Allowed, but not too often
        else
            good_filters++;
    }
    if (kVerbose >= 1) {
        fprintf(stderr, "\n");
        fprintf(stderr, "Filters: %d good, %d mediocre\n",
            good_filters, mediocre_filters);
    }
    //TEST_ASSERT_LE(mediocre_filters, good_filters / 5);

    fprintf(stderr, "\n");
}

void test_bloomfilter_full_false_positive_rate()
{
    char buffer[sizeof(int)];

    // Count number of filters that significantly exceed the false positive rate
    int mediocre_filters = 0;
    int good_filters = 0;
    bool isMatch;

    std::cout << "----------------------------------" << std::endl;
    std::cout << "FullBloomFilter Test" << std::endl;
    std::cout << "----------------------------------" << std::endl;
    std::cout << std::endl;

    FullBloomFilter bloomfilter(10000, 10, true);

    for (int length = 1; length <= 10000; length = NextLength(length)) {
        bloomfilter.setOption(length, 10, false);
        bloomfilter.reset();
        for (int i = 0; i < length; i++) {
            bloomfilter.addKey(MemIntegerKey(i, buffer));
        }

        // All added keys must match
        for (int i = 0; i < length; ++i) {
            isMatch = bloomfilter.maybeMatch(MemIntegerKey(i, buffer));
            if (!isMatch) {
                std::cout << "[Not Match], Length = " << length << "; key = " << i << std::endl;
            }
        }

        // Check false positive rate
        double rate = getFalsePositiveRate(bloomfilter, length);
        if (kVerbose >= 1) {
            fprintf(stderr, "False positive rates: %5.2f%% @ length = %6d ; bytes = %6d ; used_bits = %d\n",
                rate * 100.0, length, (int)bloomfilter.getFilterSize(), (int)bloomfilter.getUsedBits());
        }
        if (rate > 0.0125)
            mediocre_filters++;  // Allowed, but not too often
        else
            good_filters++;
    }
    if (kVerbose >= 1) {
        fprintf(stderr, "\n");
        fprintf(stderr, "Filters: %d good, %d mediocre\n",
            good_filters, mediocre_filters);
    }
    //TEST_ASSERT_LE(mediocre_filters, good_filters / 5);

    fprintf(stderr, "\n");
}

void test_bloomfilter()
{
    test_bloomfilter_impl();
    test_bloomfilter_hash();

    stop_watch sw;
    sw.start();
    test_bloomfilter_standard_false_positive_rate();
    sw.stop();

    printf("time spent: %0.3f ms.\n\n", sw.getElapsedMillisec());

    sw.start();
    test_bloomfilter_full_false_positive_rate();
    sw.stop();

    printf("time spent: %0.3f ms.\n\n", sw.getElapsedMillisec());
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
    typedef std::remove_cv<const int>::type type1;
    typedef std::remove_cv<volatile int>::type type2;
    typedef std::remove_cv<const volatile int>::type type3;
    typedef std::remove_cv<const volatile int*>::type type4;
    typedef std::remove_cv<int * const volatile>::type type5;

    std::cout << "test_std_remove_volatile()" << std::endl;
    std::cout << std::endl;
 
    std::cout << "test1 " << (std::is_same<int, type1>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test2 " << (std::is_same<int, type2>::value
        ? "passed" : "failed") << std::endl;;
    std::cout << "test3 " << (std::is_same<int, type3>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test4 " << (std::is_same<const volatile int*, type4>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test5 " << (std::is_same<int*, type5>::value
        ? "passed" : "failed") << std::endl;
    std::cout << std::endl;
}

void test_remove_volatile()
{
    typedef traits::remove_cv<const int>::type type1;
    typedef traits::remove_cv<volatile int>::type type2;
    typedef traits::remove_cv<const volatile int>::type type3;
    typedef traits::remove_cv<const volatile int*>::type type4;
    typedef traits::remove_cv<int * const volatile>::type type5;

    std::cout << "test_remove_volatile()" << std::endl;
    std::cout << std::endl;
 
    std::cout << "test1 " << (std::is_same<int, type1>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test2 " << (std::is_same<int, type2>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test3 " << (std::is_same<int, type3>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test4 " << (std::is_same<const volatile int*, type4>::value
        ? "passed" : "failed") << std::endl;
    std::cout << "test5 " << (std::is_same<int*, type5>::value
        ? "passed" : "failed") << std::endl;
    std::cout << std::endl;
}

void test_traist()
{
    test_std_remove_const_traist();
    test_remove_const_traist();

    test_std_remove_volatile();
    test_remove_volatile();
}

int main(int argc, char * argv[])
{
    printf("\n");

    TiStore::fs::Initor initor;

    TiStore::fs::File file;
    file.open("C:\\test.bin");
    if (file.is_open()) {
        file.close();
    }

    TiStore::fs::File file1("C:\\test.bin");
    file1.close();

    SkipList<Slice &, 16> skipList;
    skipList.build();

    test_traist();

    //test_bloomfilter();
    //test_typeinfo_module();

    //printf("\n");

#ifdef _WIN32
    ::system("pause");
#endif
    return 0;
}


#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>

#include "TiStore/TiFS.h"
#include "TiStore/TiStore.h"
#include "TiStore/fs/Initor.h"
#include "TiStore/kv/BloomFilter.h"
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
    StandardBloomFilter<1024, 10, 2> sbf;
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
    //StandardBloomFilter<1024, 10, 2> sbf;
    StandardBloomFilter<4096 * 8, 10, 3> sbf;
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
    for (int i = 0; i < max_length; ++i) {
        if (bloom_filter.maybeMatch(MemIntegerKey(i + 1000000000, buffer))) {
            result++;
        }
    }
    return (double)result / 10000.0;
}

void test_standard_bloomfilter_false_positive_rate()
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

    StandardBloomFilter<1980 * 8, 10, 6> bloomfilter(true);

    for (int length = 1; length <= 50000; length = NextLength(length)) {
        bloomfilter.reset();
        bloomfilter.setVerbose(false);
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
            fprintf(stderr, "False positive rates: %5.2f%% @ length = %6d ; bytes = %6d\n",
                rate * 100.0, length, (int)bloomfilter.getFilterSize());
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

void test_full_bloomfilter_false_positive_rate()
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

    FullBloomFilter<1980 * 8, 10, 6> bloomfilter(true);

    for (int length = 1; length <= 50000; length = NextLength(length)) {
        bloomfilter.reset();
        bloomfilter.setVerbose(false);
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
            fprintf(stderr, "False positive rates: %5.2f%% @ length = %6d ; bytes = %6d\n",
                rate * 100.0, length, (int)bloomfilter.getFilterSize());
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

    test_standard_bloomfilter_false_positive_rate();
    test_full_bloomfilter_false_positive_rate();
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

    test_bloomfilter();
    //test_typeinfo_module();

    //printf("\n");

#ifdef _WIN32
    ::system("pause");
#endif
    return 0;
}


#include <stdlib.h>
#include <stdio.h>
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

    isMatch = sbf.maybeMatch("karazanh");
    TEST_VERIFY_BOOL(isMatch, "maybeMatch(\"%s\"): %s\n", "karazanh");

    isMatch = sbf.maybeMatch("pokemon go");
    TEST_VERIFY_BOOL(isMatch, "maybeMatch(\"%s\"): %s\n", "pokemon go");

    printf("\n");
}

void test_bloomfilter()
{
    test_bloomfilter_impl();
    test_bloomfilter_hash();
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

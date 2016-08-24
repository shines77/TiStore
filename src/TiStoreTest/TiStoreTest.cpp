
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

void test_bloomfilter_hash_impl(const char key[])
{
    StopWatch sw;
    /**/ volatile /**/ uint32_t hash;
    StandardBloomFilter<128, 2> sbf;
    Slice skey(key);
    printf("key = %s\n\n", skey.toString().c_str());

    hash = 0;
    sw.start();
    for (int i = 0; i < kIterators; ++i) {
        hash += sbf.maybe_match(skey);
    }
    sw.stop();
    printf("sbf.maybe_match()         time spent: %8.3f ms, hash: 0x%08X\n\n", sw.getElapsedMillisec(), hash);

    hash = 0;
    sw.start();
    for (int i = 0; i < kIterators; ++i) {
        hash += sbf.rocksdb_maybe_match(skey);
    }
    sw.stop();
    printf("sbf.rocksdb_maybe_match() time spent: %8.3f ms, hash: 0x%08X\n\n", sw.getElapsedMillisec(), hash);

    hash = 0;
    sw.start();
    for (int i = 0; i < kIterators; ++i) {
        hash += sbf.maybe_match2(skey);
    }
    sw.stop();
    printf("sbf.maybe_match2()        time spent: %8.3f ms, hash: 0x%08X\n\n", sw.getElapsedMillisec(), hash);

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

    printf("TypeInfo<char>::type_id()                           = %11u\n", TiStore::TypeInfo<char>::type_id());
    printf("TypeInfo<int>::type_id()                            = %11u\n", TiStore::TypeInfo<int>::type_id());
    printf("TypeInfo<StandardBloomFilter<128,2>>::type_id()     = %11u\n", TiStore::TypeInfo<StandardBloomFilter<128,2>>::type_id());
    printf("TypeInfo<StandardBloomFilter<128,2>>::type_id()     = %11u\n", TiStore::TypeInfo<StandardBloomFilter<128,2>>::type_id());
    printf("TypeInfo<StandardBloomFilter<128,4>>::type_id()     = %11u\n", TiStore::TypeInfo<StandardBloomFilter<128,4>>::type_id());
    printf("TypeInfo<char>::type_id()                           = %11u\n", TiStore::TypeInfo<char>::type_id());
    printf("\n");

    printf("TypeInfo<char>::hash_code()                         = 0x%08X\n", TiStore::TypeInfo<char>::hash_code());
    printf("TypeInfo<int>::hash_code()                          = 0x%08X\n", TiStore::TypeInfo<int>::hash_code());
    printf("TypeInfo<StandardBloomFilter<128,2>>::hash_code()   = 0x%08X\n", TiStore::TypeInfo<StandardBloomFilter<128,2>>::hash_code());
    printf("TypeInfo<StandardBloomFilter<128,2>>::hash_code()   = 0x%08X\n", TiStore::TypeInfo<StandardBloomFilter<128,2>>::hash_code());
    printf("TypeInfo<StandardBloomFilter<128,4>>::hash_code()   = 0x%08X\n", TiStore::TypeInfo<StandardBloomFilter<128,4>>::hash_code());
    printf("TypeInfo<char>::hash_code()                         = 0x%08X\n", TiStore::TypeInfo<char>::hash_code());
    printf("\n");
}

void test_bloomfilter()
{
    test_bloomfilter_hash();

    BloomFilter bf;
    bf.mount();
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

    test_typeinfo_module();

    //printf("\n");

#ifdef _WIN32
    ::system("pause");
#endif
    return 0;
}


#include "test.h"

#include "TiStore/lang/TypeInfo.h"
#include "TiStore/kv/BloomFilter.h"
#include "TiStore/kv/BloomFilterFixed.h"

#include <stdio.h>

using namespace TiStore;

void test_typeinfo_module()
{
    printf("test_typeinfo_module()\n");
    printf("\n");

    printf("TypeInfo<char>::type_id()                           = %11u\n", TiStore::TypeInfo<char>::type_id());
    printf("TypeInfo<int>::type_id()                            = %11u\n", TiStore::TypeInfo<int>::type_id());
    printf("TypeInfo<StandardBloomFilter<128,2>>::type_id()     = %11u\n", TiStore::TypeInfo<StandardBloomFilterFixed<128,2>>::type_id());
    printf("TypeInfo<StandardBloomFilter<128,2>>::type_id()     = %11u\n", TiStore::TypeInfo<StandardBloomFilterFixed<128,2>>::type_id());
    printf("TypeInfo<StandardBloomFilter<128,4>>::type_id()     = %11u\n", TiStore::TypeInfo<StandardBloomFilterFixed<128,4>>::type_id());
    printf("TypeInfo<char>::type_id()                           = %11u\n", TiStore::TypeInfo<char>::type_id());
    printf("\n");

    printf("TypeInfo<char>::hash_code()                         = 0x%08X\n", TiStore::TypeInfo<char>::hash_code());
    printf("TypeInfo<int>::hash_code()                          = 0x%08X\n", TiStore::TypeInfo<int>::hash_code());
    printf("TypeInfo<StandardBloomFilter<128,2>>::hash_code()   = 0x%08X\n", TiStore::TypeInfo<StandardBloomFilterFixed<128,2>>::hash_code());
    printf("TypeInfo<StandardBloomFilter<128,2>>::hash_code()   = 0x%08X\n", TiStore::TypeInfo<StandardBloomFilterFixed<128,2>>::hash_code());
    printf("TypeInfo<StandardBloomFilter<128,4>>::hash_code()   = 0x%08X\n", TiStore::TypeInfo<StandardBloomFilterFixed<128,4>>::hash_code());
    printf("TypeInfo<char>::hash_code()                         = 0x%08X\n", TiStore::TypeInfo<char>::hash_code());
    printf("\n");
}

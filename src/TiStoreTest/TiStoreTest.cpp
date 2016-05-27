
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>

#include "TiStore/TiFS.h"
#include "TiStore/TiStore.h"
#include "TiStore/fs/Initor.h"

using namespace timax;

int main(int argc, char * argv[])
{
    timax::fs::Initor initor;

    timax::fs::File file;
    file.open("C:\\test.bin");
    if (file.is_open()) {
        file.close();
    }

    timax::fs::File file1("C:\\test.bin");
    file1.close();

    return 0;
}

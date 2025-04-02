#include <iostream>
#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <cstring>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <sys/mman.h>
#include <cstdarg>
#include <exception>
#include "addresses.h"

#ifndef TAG
#define TAG "addresses"
#endif

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)



uintptr_t FindLibrary(const char* library)
{
    char filename[0xFF] = {0},
    buffer[2048] = {0};
    FILE *fp = 0;
    uintptr_t address = 0;

    sprintf( filename, "/proc/%d/maps", getpid() );

    fp = fopen( filename, "rt" );
    if(fp == 0)
    {
        LOGE("ERROR: can't open file %s", filename);
        goto done;
    }

    while(fgets(buffer, sizeof(buffer), fp))
    {
        if( strstr( buffer, library ) )
        {
            address = (uintptr_t)strtoul( buffer, 0, 16 );
            break;
        }
    }

    done:

    if(fp)
      fclose(fp);

    return address;
}

size_t GetLibrarySize(const char* lib_name) {
    char filename[0xFF] = {0};
    char buffer[2048] = {0};
    FILE *fp = nullptr;
    size_t lib_size = 0;
    uintptr_t min_addr = 0xFFFFFFFF;
    uintptr_t max_addr = 0;

    sprintf(filename, "/proc/%d/maps", getpid());
    
    fp = fopen(filename, "rt");
    if (fp == nullptr) {
        LOGE("ERROR: can't open file %s", filename);
        return 0;
    }

    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strstr(buffer, lib_name)) {
            uintptr_t start_addr, end_addr;
            LOGI("Found library entry: %s", buffer);
            if (sscanf(buffer, "%x-%x", &start_addr, &end_addr) == 2) {
                if (start_addr < min_addr) min_addr = start_addr;
                if (end_addr > max_addr) max_addr = end_addr;
            } else {
                LOGE("Failed to parse addresses from: %s", buffer);
            }
        }
    }

    fclose(fp);

    if (min_addr != 0xFFFFFFFF && max_addr != 0) {
        lib_size = max_addr - min_addr;
    } else {
        LOGE("ERROR: library size not found for %s", lib_name);
    }

    return lib_size;
}
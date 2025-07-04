#include <jni.h>
#include <android/log.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>

#include "utils/logging.h"
#include "utils/armhook.h"
#include "utils/addresses.h"


#define HOOK_LIBRARY "libsamp.so"
#define HOOK_PATTERN "\x48\x1C\x04\xBF"


unsigned char sampEncrTable[256] = {
    0x27, 0x69, 0xFD, 0x87, 0x60, 0x7D, 0x83, 0x02, 0xF2, 0x3F, 0x71, 0x99, 0xA3, 0x7C, 0x1B, 0x9D,
    0x76, 0x30, 0x23, 0x25, 0xC5, 0x82, 0x9B, 0xEB, 0x1E, 0xFA, 0x46, 0x4F, 0x98, 0xC9, 0x37, 0x88,
    0x18, 0xA2, 0x68, 0xD6, 0xD7, 0x22, 0xD1, 0x74, 0x7A, 0x79, 0x2E, 0xD2, 0x6D, 0x48, 0x0F, 0xB1,
    0x62, 0x97, 0xBC, 0x8B, 0x59, 0x7F, 0x29, 0xB6, 0xB9, 0x61, 0xBE, 0xC8, 0xC1, 0xC6, 0x40, 0xEF,
    0x11, 0x6A, 0xA5, 0xC7, 0x3A, 0xF4, 0x4C, 0x13, 0x6C, 0x2B, 0x1C, 0x54, 0x56, 0x55, 0x53, 0xA8,
    0xDC, 0x9C, 0x9A, 0x16, 0xDD, 0xB0, 0xF5, 0x2D, 0xFF, 0xDE, 0x8A, 0x90, 0xFC, 0x95, 0xEC, 0x31,
    0x85, 0xC2, 0x01, 0x06, 0xDB, 0x28, 0xD8, 0xEA, 0xA0, 0xDA, 0x10, 0x0E, 0xF0, 0x2A, 0x6B, 0x21,
    0xF1, 0x86, 0xFB, 0x65, 0xE1, 0x6F, 0xF6, 0x26, 0x33, 0x39, 0xAE, 0xBF, 0xD4, 0xE4, 0xE9, 0x44,
    0x75, 0x3D, 0x63, 0xBD, 0xC0, 0x7B, 0x9E, 0xA6, 0x5C, 0x1F, 0xB2, 0xA4, 0xC4, 0x8D, 0xB3, 0xFE,
    0x8F, 0x19, 0x8C, 0x4D, 0x5E, 0x34, 0xCC, 0xF9, 0xB5, 0xF3, 0xF8, 0xA1, 0x50, 0x04, 0x93, 0x73,
    0xE0, 0xBA, 0xCB, 0x45, 0x35, 0x1A, 0x49, 0x47, 0x6E, 0x2F, 0x51, 0x12, 0xE2, 0x4A, 0x72, 0x05,
    0x66, 0x70, 0xB8, 0xCD, 0x00, 0xE5, 0xBB, 0x24, 0x58, 0xEE, 0xB4, 0x80, 0x81, 0x36, 0xA9, 0x67,
    0x5A, 0x4B, 0xE8, 0xCA, 0xCF, 0x9F, 0xE3, 0xAC, 0xAA, 0x14, 0x5B, 0x5F, 0x0A, 0x3B, 0x77, 0x92,
    0x09, 0x15, 0x4E, 0x94, 0xAD, 0x17, 0x64, 0x52, 0xD3, 0x38, 0x43, 0x0D, 0x0C, 0x07, 0x3C, 0x1D,
    0xAF, 0xED, 0xE7, 0x08, 0xB7, 0x03, 0xE6, 0x8E, 0xAB, 0x91, 0x89, 0x3E, 0x2C, 0x96, 0x42, 0xD9,
    0x78, 0xDF, 0xD0, 0x57, 0x5D, 0x84, 0x41, 0x7E, 0xCE, 0xF7, 0x32, 0xC3, 0xD5, 0x20, 0x0B, 0xA7
};

unsigned char encrBuffer[4092];

void kyretardizeDatagram(unsigned char *buf, int len, int port, int unk) {
    unsigned char bChecksum = 0;
    for(int i = 0; i < len; i++) {
        unsigned char bData = buf[i];
        bChecksum ^= bData & 0xAA;
    }
    encrBuffer[0] = bChecksum;

    unsigned char *buf_nocrc = &encrBuffer[1];
    memcpy(buf_nocrc, buf, len);

    for(int i = 0; i < len; i++) {
        buf_nocrc[i] = sampEncrTable[buf_nocrc[i]];
        if (unk)
            buf_nocrc[i] ^= ((unsigned char*)&port)[0] ^ 0xCC;
        unk ^= 1u;
    }
}

signed int (*SocketLayer__SendTo)(int socket, int sockfd, int buffer, int length, int ip_addr, unsigned int port) = nullptr;
signed int SocketLayer__SendTo_Hook([[maybe_unused]] int socket, int sockfd, int buffer, int length, int ip_addr, unsigned int port) {
    if (sockfd == -1) {
        LOGI("Invalid socket descriptor");
        return -1;
    }

    kyretardizeDatagram((unsigned char*)buffer, length, port, 0);
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ip_addr;

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);
    
    int result = sendto(sockfd, encrBuffer, length + 1, 0, (struct sockaddr*)&addr, sizeof(addr));
    
    if (result == -1) {
        LOGI("sendto failed with error: %s (errno: %d)", strerror(errno), errno);
    }
    
    return result;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    (void)reserved;
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    
    
    LOGI("ARZCryptHook - packet hook loaded | Build time: %s", __DATE__ " " __TIME__);
    LOGI("Source project on GitHub: https://github.com/idmkdev/arzcrypthook");
    char libName[256] = {0};
    uintptr_t libHandle = FindLibrary(HOOK_LIBRARY);
    if(libHandle == 0)
    {
        char prefix[256] = {0};
        strncpy(prefix, HOOK_LIBRARY, sizeof(prefix) - 1);
        char* dot = strrchr(prefix, '.');
        if(dot) *dot = '\0';
        LOGI("Not found %s, trying to find by prefix %s", HOOK_LIBRARY, prefix);
        LibraryInfo libInfo = FindLibraryByPrefix(prefix);
        if(libInfo.address == 0 || libInfo.name[0] == '\0')
        {
            Log("Not found %s or any library starting with %s", HOOK_LIBRARY, prefix);
            pid_t pid = getpid();
            kill(pid, SIGKILL);
        }
        LOGI("Found library by prefix at address: %x with name: %s", libInfo.address, libInfo.name);
        libHandle = libInfo.address;
        strncpy(libName, libInfo.name, sizeof(libName) - 1);
        InitHookStuff(libName);
    }
    else
    {
        strncpy(libName, HOOK_LIBRARY, sizeof(libName) - 1);
        InitHookStuff(libName);
    } 


    void* func_addr = FindPattern(HOOK_PATTERN, libHandle, GetLibrarySize(libName));   
    if(func_addr)
    {
        SetUpHook(reinterpret_cast<uintptr_t>(func_addr), reinterpret_cast<uintptr_t>(SocketLayer__SendTo_Hook), reinterpret_cast<uintptr_t*>(&SocketLayer__SendTo));
        LOGI("Hooks installed successfully, address: %x", libHandle-(uintptr_t)func_addr);
    } else {
        LOGE("Can't find offset from pattern");
        pid_t pid = getpid();
        kill(pid, SIGKILL);
    }
    
    return JNI_VERSION_1_6;
} 

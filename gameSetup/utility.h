#pragma once
#include "../windowsPlatformLayer/buildConfig.h"

//todo windows assert
#if ENABLE_ASSERT

#define assert(x) if(!(x)){/*std::cout<< __LINE__ << " " << GetLastError(); Sleep(100000);*/*(char*)0 = 0;}

#else

#define assert(x)

#endif


#define KB(x) ((x)*1024LL)
#define MB(x) (KB(x)*1024LL)
#define GB(x) (MB(x)*1024LL)
#define TB(x) (GB(x)*1024LL)

void clearMemory(void* p, size_t size);
#pragma once

//todo windows assert
#define assert(x) if(!(x)){/*std::cout<< __LINE__ << " " << GetLastError(); Sleep(100000);*/*(char*)0 = 0;}

void clearMemory(void* p, size_t size);
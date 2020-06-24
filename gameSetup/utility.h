#pragma once

#define assert(x) if(!x){std::cout<< __LINE__ << " " << GetLastError(); Sleep(100000);*(char*)0 = 0;}
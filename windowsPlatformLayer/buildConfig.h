#pragma once

#define INTERNAL_BUILD 1	//this includes things like debugg checking replay and hot code reloading
							//when this is 0 the code will build for shipping mode, it will disable
							//debugging tools like input record, custom memory allocation adress space
							//and the option to ignore an assert

#define ENABLE_CONSOLE 1	//this is the default windows console

#define ALLOW_ONLY_ONE_INSTANCE 1

#define NOT_RECORD_DELTATIME 0

#define USE_GPU_ENGINE 1


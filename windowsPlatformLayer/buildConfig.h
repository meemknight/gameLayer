#pragma once

#define INTERNAL_BUILD 1	//this includes things like debugg checking replay and hot code reloading
							//when this is 0 the code will build for shipping mode: 
							// it will disable:
							//debugging tools like input record, 
							//custom memory allocation adress space (some internal stuff)
							//and the option to ignore an assert

#define ENABLE_CONSOLE 1	//this is the default windows console

#define ENABLE_CONSOLE 1	//this is the internal console that you can open with alt + `

#define ALLOW_ONLY_ONE_INSTANCE 1	//allows only one instance open of this program. You need it if you use hot code reloading
#define UNIQUE_NAME "example"		//set this to your program name or sthing unique if you use ALLOW_ONLY_ONE_INSTANCE


#define NOT_RECORD_DELTATIME 0	//when recording and playing back the input you have the option to also record deltaTime values
								//you might want to record it to have a persistant playback for debugging purposes.
								//keep this flag at 0 if you want to record it.

#define USE_GPU_ENGINE 0		//enables the gpu if present.


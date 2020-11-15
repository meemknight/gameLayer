#pragma once
#include "../windowsPlatformLayer/buildConfig.h"




#define KB(x) ((x)*1024LL)
#define MB(x) (KB(x)*1024LL)
#define GB(x) (MB(x)*1024LL)
#define TB(x) (GB(x)*1024LL)

void clearMemory(void* p, size_t size);


#pragma region serialized variable

enum type
{
	int_type = 0,
	float_type = 0,

};

struct SerializedVariable
{
	int type;
	void* ptr;
	const char* name;
};

struct SerializedVariabels
{
	constexpr static int MAX_VAR_SIZE = 100;
	SerializedVariable var[MAX_VAR_SIZE] = {};
	int pos = 0;
};

struct SerializedVariableInstance
{
	SerializedVariableInstance(SerializedVariabels& vars, int type, void* ptr, const char* name)
	{
		winAssertComment(vars.pos < vars.MAX_VAR_SIZE, "Serialized Variables limit reached");
		vars.var[vars.pos] = { type, ptr, name };
		vars.pos++;
	};
};

#define SERIALIZE(t, x, val, name) t x = val; private:\
 SerializedVariableInstance x ## _serialize = {serializedVariables, \
type:: ## t ## _type, (void*)& x, name}; public:


#pragma endregion

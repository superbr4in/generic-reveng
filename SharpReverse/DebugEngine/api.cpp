#include "stdafx.h"

#include "debugger.h"

#include "bin_dump.h"

#define API extern "C" __declspec(dllexport)

API debugger* open(const char* bytes, const size_t size)
{
    return new debugger(std::vector<char>(bytes, bytes + size));
}
API debugger* open_file(const char* file_name)
{
    return new debugger(create_dump(file_name));
}

API void close(debugger* handle)
{
    handle->close();
    free(handle); // TODO: Necessary?
}

API void debug(debugger* handle, instruction_info& instruction)
{
    instruction = handle->debug();
}

API void get_register_state(debugger* handle, register_info& register_state)
{
    register_state = handle->inspect_registers();
}

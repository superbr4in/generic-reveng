// wrapper.cpp

#include "stdafx.h"

#include "X86Disassembler.h"

#define API extern "C" __declspec(dllexport)

/*struct asm_instr
{
	uint64_t address;
	std::string mnemonic;
	std::string op_str;
};*/

API int disasm(const uint8_t* bytes, cs_insn s)
{
	csh handle;
	cs_insn* insn;

	if (cs_open(CS_ARCH_X86, CS_MODE_32, &handle) != CS_ERR_OK)
		return -1;

	const auto count = cs_disasm(handle, bytes, sizeof(bytes) - 1, 0x1000, 0, &insn);

	if (count > 0)
	{
		s = insn[0];
	}
	else return -1;

	cs_close(&handle);

	return 0;
}

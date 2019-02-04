#pragma once

#include <memory>

#include <capstone/capstone.h>

namespace cs
{
    struct free
    {
        void operator()(cs_insn* instruction) const;
    };

    std::unique_ptr<cs_insn, free> malloc(csh handle);

    bool disasm_iter(csh handle, std::basic_string_view<std::byte>* code, std::uint_fast64_t* address, cs_insn* instruction);
}
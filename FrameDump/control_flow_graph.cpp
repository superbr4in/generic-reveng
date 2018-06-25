#include "stdafx.h"

#include "control_flow_graph.h"
#include "display.h"

static std::vector<uint8_t> assemble_x86(const uint64_t address, const std::string& string)
{
    const std::string var_code = "v1";
    const std::string var_length = "v2";

    auto s_python =
        "from keystone import *\n"
        "ks = Ks(KS_ARCH_X86, KS_MODE_64)\n"
        + var_code + ", count = ks.asm(b\"" + string + "\", " + std::to_string(address) + ")\n"
        + var_length + " = len(" + var_code + ")";

    Py_Initialize();
    PyRun_SimpleString(s_python.c_str());

    const auto main = PyImport_AddModule("__main__");

    const auto p_code = PyObject_GetAttrString(main, var_code.c_str());
    const auto length = _PyInt_AsInt(PyObject_GetAttrString(main, var_length.c_str()));

    std::vector<uint8_t> code;
    for (auto i = 0; i < length; ++i)
        code.push_back(_PyInt_AsInt(PyList_GetItem(p_code, i)));

    Py_Finalize();

    return code;
}

control_flow_graph_x86::control_flow_graph_x86(const std::shared_ptr<debugger>& debugger, const uint64_t root_address)
{
    const auto root_instruction = debugger->disassemble_at(root_address);
    if (root_instruction.str_mnemonic != "push")
    {
        std::cout << "Unexpected root" << std::endl;

        root_ = nullptr;
        return;
    }

    root_ = build(debugger, root_address, assemble_x86(0, "pop " + root_instruction.str_operands), map_);
}

void control_flow_graph_x86::draw() const
{
    std::map<block, char> block_map;

    auto id = 'A';
    for (const auto m : map_)
    {
        const auto [it, b] = block_map.try_emplace(*m.second.first, id);
        if (b) ++id;
    }

    const std::string l = "| ";
    const std::string r = " |";

    const auto h = '-';

    const auto eu = '.';
    const auto ed = '\'';

    std::map<char, std::string> string_map;
    for (const auto& [block, block_id] : block_map)
    {
        std::ostringstream ss;

        const auto last = block.instructions.back();
        const auto last_string = last.to_string(last.is_conditional || last.is_volatile);

        const auto width = last_string.size();

        ss << block_id << std::setfill(h) << std::setw(width + l.size() + r.size() - 2) << std::left << "(" + std::to_string(block.instructions.size()) + ")" << eu;
        for (const auto p : block.previous)
            ss << ' ' << block_map.at(*p);
        ss << std::endl;

        ss << std::setfill(' ');

        if (block.instructions.size() > 1)
            ss << l << std::setw(width) << std::left << block.instructions.front().to_string(false) << r << std::endl;
        if (block.instructions.size() > 2)
            ss << l << std::setw(width) << std::left << ':' << r << std::endl;
        ss << l << std::setw(width) << std::left << last_string << r << std::endl;

        ss << ed << std::string(width + 2, '-') << ed;
        for (const auto n : block.next)
            ss << ' ' << block_map.at(*n);
        ss << std::endl << std::endl;

        string_map.emplace(block_id, ss.str());
    }

    for (const auto& [block_id, string] : string_map)
    {
        if (block_id == block_map.at(*root_))
            std::cout << dsp::colorize(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << string << dsp::decolorize;
    }
}

control_flow_graph_x86::block* control_flow_graph_x86::build(const std::shared_ptr<debugger>& debugger, uint64_t address,
    const std::vector<uint8_t>& stop, std::map<uint64_t, std::pair<block*, size_t>>& map)
{
    // New (current) block
    const auto cur = new block;

    // Appends an existing block at the specified address as successor
    const std::function<bool(uint64_t)> success = [cur, &map](const uint64_t next_address)
    {
        const auto map_it = map.find(next_address);
        if (map_it == map.end())
        {
            // No block exists at this address
            return false;
        }

        const auto [orig, index] = map_it->second;

        if (index == 0)
        {
            // Block does not have to be split
            cur->next.insert(orig);
            orig->previous.insert(cur);
            return true;
        }

        const auto begin = orig->instructions.begin() + index;
        const auto end = orig->instructions.end();

        const auto next = new block;

        // Copy tail
        next->instructions = std::vector<instruction_x86>(begin, end);

        // Update map
        // TODO: Inefficient with large blocks
        for (auto j = 0; j < end - begin; ++j)
            map[(begin + j)->address] = std::make_pair(next, j);

        // Truncate tail
        orig->instructions.erase(begin, end);

        // Update successor information
        cur->next.insert(next);
        next->next = orig->next;
        orig->next = { next };

        // Update predecessor information
        next->previous.insert(orig);
        for (const auto nn : next->next)
        {
            nn->previous.erase(orig);
            nn->previous.insert(next);
        }
        next->previous.insert(cur);

        return true;
    };

    // Repeat until successors are set
    while (cur->next.empty())
    {
        // Map address to block and index
        map.emplace(address, std::make_pair(cur, cur->instructions.size()));

        debugger->jump_to(address);

        const auto instruction = debugger->next_instruction();

        // Append instruction
        cur->instructions.push_back(instruction);

        // Emulate instruction
        if (debugger->step_into() != UC_ERR_OK)
            std::cout << "FAIL: " << instruction.to_string(true) << std::endl;

        if (instruction.code == stop)
        {
            // Reached final instruction, stop without successor
            break;
        }

        if (instruction.type == ins_jump && instruction.is_conditional)
        {
            std::vector<uint64_t> next_addresses;

            // Consider both jump results
            next_addresses.push_back(address + instruction.code.size());
            next_addresses.push_back(std::get<op_immediate>(instruction.operands.at(0).value));

            // Save current emulation state
            const auto snapshot = debugger->take_snapshot();

            for (const auto next_address : next_addresses)
            {
                if (!success(next_address))
                {
                    // Recursively create a new successor
                    const auto next = build(debugger, next_address, stop, map);
                    cur->next.insert(next);
                    next->previous.insert(cur);
                }

                // Reset to original state
                debugger->reset(snapshot);
            }
        }
        else
        {
            const auto next_address = debugger->next_instruction().address;

            if (!success(next_address))
            {
                // Advanced address and continue
                address = next_address;
            }
        }
    }

    return cur;
}

bool operator<(const control_flow_graph_x86::block& block1, const control_flow_graph_x86::block& block2)
{
    return block1.instructions.front().address < block2.instructions.front().address;
}

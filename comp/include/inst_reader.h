#pragma once

#include "bytefile.h"
#include "instruction.h"
#include "opcode.h"

#include <glog/logging.h>
#include <cstring>
#include <memory>
#include <ranges>
#include <unordered_map>

namespace lama {

class InstReader {
public:
    InstReader(bytefile const* file)
        : file(file)
        , ip(file->code_ptr)
        , function_index{} {
        for (auto i : std::views::iota(0, file->public_symbols_number)) {
            function_names[get_public_offset(file, i)] = get_public_name(file, i);
        }
    }

    inline int get_offset() const noexcept {
        return (int)(ip - file->code_ptr);
    }

private:
    bytefile const* file;
    char const* ip;
    int function_index;
    std::unordered_map<int, std::string> function_names;

    inline void assert_can_read(int bytes) {
        CHECK_LE(file->code_ptr, ip) << "ip is out of code section";
        CHECK_LE(ip + bytes, (char const*)file + file->size) << "ip is out of code section";
    }

    inline char read_byte() {
        assert_can_read(1);
        return (*(ip++));
    }

    inline int read_int() {
        assert_can_read(4);
        int result;
        std::memcpy(&result, ip, sizeof(int));
        ip += sizeof(int);
        return result;
    }

    inline char const* read_string() {
        return get_string(file, read_int());
    }

    inline LocationEntry read_loc() {
        Location kind = (Location)read_byte();
        int index = read_int();
        return LocationEntry{.kind = kind, .index = index};
    }

public:
    std::unique_ptr<Instruction> read_inst();
};

}  // namespace lama

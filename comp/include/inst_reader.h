#pragma once

#include "bytefile.h"
#include "instructions.h"
#include "opcode.h"

#include <glog/logging.h>
#include <cstring>
#include <memory>
#include <ranges>
#include <unordered_map>
#include <vector>

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
    inline std::unique_ptr<Instruction> read_inst() {
        unsigned char x = read_byte(), h = (x & 0xF0) >> 4, l = x & 0x0F;

        switch (x) {
        case Opcode_Const: {
            return std::make_unique<Const>(read_int());
        }

        case Opcode_String: {
            return std::make_unique<String>(read_string());
        }

        case Opcode_SExp: {
            return std::make_unique<SExpression>(read_string(), read_int());
        }

        case Opcode_StI: {
            return std::make_unique<StoreStack>();
        }

        case Opcode_StA: {
            return std::make_unique<StoreArray>();
        }

        case Opcode_Jmp: {
            return std::make_unique<Jump>(read_int());
        }

        case Opcode_End: {
            return std::make_unique<End>();
        }

        case Opcode_Ret: {
            return std::make_unique<Return>();
        }

        case Opcode_Drop: {
            return std::make_unique<Drop>();
        }

        case Opcode_Dup: {
            return std::make_unique<Duplicate>();
        }

        case Opcode_Swap: {
            return std::make_unique<Swap>();
        }

        case Opcode_Elem: {
            return std::make_unique<Elem>();
        }

        case Opcode_CJmpZ: {
            return std::make_unique<ConditionalJump>(read_int(), true);
        }

        case Opcode_CJmpNZ: {
            return std::make_unique<ConditionalJump>(read_int(), false);
        }

        case Opcode_Begin: {
            // fprintf(stderr, "Reading function %s at offset %d, pub_offset: %d\n", get_public_name(file,
            // function_index), get_offset(), get_public_offset(file, function_index));
            DCHECK_EQ(get_public_offset(file, function_index), get_offset() - 1) << "function offset mismatch";
            return std::make_unique<Begin>(
                std::string{get_public_name(file, function_index++)}, read_int(), read_int()
            );
            break;
        }

        case Opcode_CBegin: {
            return std::make_unique<Begin>("closure", read_int(), read_int());
            break;
        }

        case Opcode_Closure: {
            int entry = read_int();
            std::vector<LocationEntry> captured;
            {
                int n = read_int();
                for (auto _ : std::views::iota(0, n)) {
                    captured.push_back(read_loc());
                }
            }
            return std::make_unique<Closure>(entry, std::move(captured));
        }

        case Opcode_CallC: {
            return std::make_unique<CallClosure>(read_int());
        }

        case Opcode_Call: {
            DCHECK(function_names.contains(read_int())) << "unknown function called";
            return std::make_unique<Call>(function_names[read_int()], read_int());
        }

        case Opcode_Tag: {
            return std::make_unique<Tag>(read_string(), read_int());
        }

        case Opcode_Array: {
            return std::make_unique<Array>(read_int());
        }

        case Opcode_Fail: {
            return std::make_unique<Fail>(read_int(), read_int());
        }

        case Opcode_Line: {
            return std::make_unique<Line>(read_int());
        }

        default:

            switch (h) {
            case HOpcode_Binop: {
                return std::make_unique<Binop>(l);
            }

            case HOpcode_Ld: {
                return std::make_unique<Load>(LocationEntry{.kind = static_cast<Location>(l), .index = read_int()});
            }
            case HOpcode_LdA: {
                return std::make_unique<LoadArray>(read_int(), l);
            }
            case HOpcode_St: {
                return std::make_unique<Store>(LocationEntry{.kind = static_cast<Location>(l), .index = read_int()});
            }

            case HOpcode_Patt: {
                return std::make_unique<PatternInst>(l);
            }

            case HOpcode_LCall: {
                switch (l) {
                case LCall_Lread: {
                    return std::make_unique<BuiltinRead>();
                }
                case LCall_Lwrite: {
                    return std::make_unique<BuiltinWrite>();
                }
                case LCall_Llength: {
                    return std::make_unique<BuiltinLength>();
                }
                case LCall_Lstring: {
                    return std::make_unique<BuiltinString>();
                }
                case LCall_Barray: {
                    return std::make_unique<BuiltinArray>();
                }
                default:
                    LOG(FATAL) << std::format("Unknown LCall {:d}", l);
                }
                break;
            }
            case HOpcode_Stop: {
                return nullptr;
            }
            default:
                LOG(FATAL) << std::format("Unknown opcode {:d}", x);
            }
        }
    }
};

}  // namespace lama

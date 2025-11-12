#include <glog/logging.h>
#include <memory>
#include "inst_reader.h"
#include "instructions.h"

std::unique_ptr<lama::Instruction> lama::InstReader::read_inst() {
    unsigned char x = read_byte(), h = (x & 0xF0) >> 4, l = x & 0x0F;

    switch (static_cast<Opcode>(x)) {
    case Opcode::Const: {
        return std::make_unique<Const>(read_int());
    }

    case Opcode::String: {
        return std::make_unique<String>(read_string());
    }

    case Opcode::SExp: {
        return std::make_unique<SExpression>(read_string(), read_int());
    }

    case Opcode::StI: {
        return std::make_unique<StoreStack>();
    }

    case Opcode::StA: {
        return std::make_unique<StoreArray>();
    }

    case Opcode::Jmp: {
        return std::make_unique<Jump>(read_int());
    }

    case Opcode::End: {
        return std::make_unique<End>();
    }

    case Opcode::Ret: {
        return std::make_unique<Return>();
    }

    case Opcode::Drop: {
        return std::make_unique<Drop>();
    }

    case Opcode::Dup: {
        return std::make_unique<Duplicate>();
    }

    case Opcode::Swap: {
        return std::make_unique<Swap>();
    }

    case Opcode::Elem: {
        return std::make_unique<Elem>();
    }

    case Opcode::CJmpZ: {
        return std::make_unique<ConditionalJump>(read_int(), true);
    }

    case Opcode::CJmpNZ: {
        return std::make_unique<ConditionalJump>(read_int(), false);
    }

    case Opcode::Begin: {
        auto const index = function_index++;
        auto const is_public = index < file->public_symbols_number;
        size_t const offset = get_offset() - 1;
        auto const argc = read_int();
        auto const locc = read_int();
        if (is_public) {
            DCHECK_EQ(get_public_offset(file, index), offset) << "function offset mismatch";
            return std::make_unique<Begin>(std::string{get_public_name(file, index)}, argc, locc);
        } else {
            return std::make_unique<Begin>(offset, argc, locc);
        }
    }

    case Opcode::CBegin: {
        return std::make_unique<Begin>("closure", read_int(), read_int());
    }

    case Opcode::Closure: {
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

    case Opcode::CallC: {
        return std::make_unique<CallClosure>(read_int());
    }

    case Opcode::Call: {
        auto const callee = read_int();
        auto const argc = read_int();
        return std::make_unique<Call>(callee, argc);
    }

    case Opcode::Tag: {
        return std::make_unique<Tag>(read_string(), read_int());
    }

    case Opcode::Array: {
        return std::make_unique<Array>(read_int());
    }

    case Opcode::Fail: {
        return std::make_unique<Fail>(read_int(), read_int());
    }

    case Opcode::Line: {
        return std::make_unique<Line>(read_int());
    }

    default:

        switch (static_cast<HOpcode>(h)) {
        case HOpcode::Binop: {
            return std::make_unique<Binop>(static_cast<BinopKind>(l));
        }

        case HOpcode::Ld: {
            return std::make_unique<Load>(LocationEntry{.kind = static_cast<Location>(l), .index = read_int()});
        }
        case HOpcode::LdA: {
            return std::make_unique<LoadArray>(read_int(), l);
        }
        case HOpcode::St: {
            return std::make_unique<Store>(LocationEntry{.kind = static_cast<Location>(l), .index = read_int()});
        }

        case HOpcode::Patt: {
            return std::make_unique<PatternInst>(l);
        }

        case HOpcode::LCall: {
            switch (static_cast<LCall>(l)) {
            case LCall::Lread: {
                return std::make_unique<BuiltinRead>();
            }
            case LCall::Lwrite: {
                return std::make_unique<BuiltinWrite>();
            }
            case LCall::Llength: {
                return std::make_unique<BuiltinLength>();
            }
            case LCall::Lstring: {
                return std::make_unique<BuiltinString>();
            }
            case LCall::Barray: {
                return std::make_unique<BuiltinArray>();
            }
            default:
                LOG(FATAL) << std::format("Unknown LCall {:d}", l);
                return nullptr;
            }
        }
        case HOpcode::Stop: {
            return nullptr;
        }
        default:
            LOG(FATAL) << std::format("Unknown opcode {:d}", x);
            return nullptr;
        }
    }
}

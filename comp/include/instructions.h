#pragma once
#include "code_buffer.h"
#include "instruction.h"
#include "cpp23.h"
#include "opcode.h"
#include "register.h"
#include <cstdint>
#include <ranges>

namespace lama {

#define INSTRUCTIONS(MACRO) \
    MACRO(Const)            \
    MACRO(String)           \
    MACRO(SExpression)      \
    MACRO(StoreStack)       \
    MACRO(StoreArray)       \
    MACRO(Jump)             \
    MACRO(ConditionalJump)  \
    MACRO(Return)           \
    MACRO(Drop)             \
    MACRO(Duplicate)        \
    MACRO(Swap)             \
    MACRO(Elem)             \
    MACRO(Closure)          \
    MACRO(Begin)            \
    MACRO(End)              \
    MACRO(CallClosure)      \
    MACRO(Call)             \
    MACRO(Tag)              \
    MACRO(Array)            \
    MACRO(Fail)             \
    MACRO(Line)             \
    MACRO(Binop)            \
    MACRO(Load)             \
    MACRO(LoadArray)        \
    MACRO(Store)            \
    MACRO(PatternInst)      \
    MACRO(BuiltinRead)      \
    MACRO(BuiltinWrite)     \
    MACRO(BuiltinLength)    \
    MACRO(BuiltinString)    \
    MACRO(BuiltinArray)


#define BASE(class_name, super_class_name)       \
  class class_name: public super_class_name {    \


#define LEAF(class_name, super_class_name) \
  BASE(class_name, super_class_name)       \
   public:                                 \
    const char* name() const override {    \
        return #class_name;                \
    }

using SymbolicLocationType = rv::SymbolicStack::LocType;

LEAF(Const, Instruction)
private:
    int _value;

public:
    Const(int value) : _value(2 * value + 1) {}

    inline int value() { return _value; }

    IsTerminator emit_code(rv::Compiler *c) const override {
        auto loc = c->st.alloc();
        switch (loc.type) {
            case SymbolicLocationType::Memory: {
                c->cb.emit_li(rv::Register::temp1(), _value);
                c->cb.emit_sd(rv::Register::temp1(), rv::Register::fp(), -loc.number * rv::WORD_SIZE);
                break;
            }
            case SymbolicLocationType::Register: {
                c->cb.emit_li({(uint8_t) loc.number}, _value);
                break;
            }
        }
        return IsTerminator::No;
    }
};

LEAF(String, Instruction)
private:
    const char *_str;
public:
    String(const char *str) : _str(str) {}

    inline const char* str() { return _str; }

    IsTerminator emit_code(rv::Compiler *c) const override {
        c->strs.emplace_back(_str);
        // call BString
        return IsTerminator::No;
    }
};

LEAF(SExpression, Instruction)
private:
    const char* _name;
    size_t _size;
public:
    SExpression(const char* name, int size) : _name(name), _size(size) {}

    inline const char* tag() { return _name; }

    inline size_t size() { return _size; }
};

LEAF(StoreStack, Instruction)
public:
    IsTerminator emit_code(rv::Compiler *c) const override {
        auto value_loc = c->st.pop();
        auto ptr_loc = c->st.pop();
        c->st.push(value_loc);
        c->cb.symb_emit_sd(value_loc, ptr_loc, 0);
        return IsTerminator::No;
    }
};
LEAF(StoreArray, Instruction) };
LEAF(End, Instruction)
    IsTerminator emit_code(rv::Compiler *c) const override {
        DCHECK(c->current_frame.has_value()) << "no current frame in End instruction";
        size_t _locc = c->current_frame->locals_count;
        c->cb.symb_emit_mv(rv::Register::arg(0), c->st.pop());
        // Restore sp
        c->cb.emit_mv(rv::Register::sp(), rv::Register::fp());
        // Restore callee-saved registers (fp is included)
        rv::Register::saved_apply([c, _locc](const rv::Register& r, int i) {
            c->cb.emit_ld(r, rv::Register::sp(), -(i + _locc) * rv::WORD_SIZE);
        });
        // Return
        c->cb.emit_ret();
        return IsTerminator::Yes;
    }
};
LEAF(Return, Instruction) };
LEAF(Duplicate, Instruction) };
LEAF(Drop, Instruction)
    public:
    IsTerminator emit_code(rv::Compiler *c) const override {
        c->st.pop();
        return IsTerminator::No;
    }
};
LEAF(Swap, Instruction) };
LEAF(Elem, Instruction) };
LEAF(Jump, Instruction)
private:
    size_t _target;

public:
    Jump(int target) : _target(target) {}
    IsTerminator emit_code(rv::Compiler *c) const override {
        c->cb.emit_j(c->cb.label_for_ip(_target));
        c->bb_begin(_target, c->st.top);
        return IsTerminator::Yes;
    }
};
LEAF(ConditionalJump, Instruction)
private:
    size_t _target;
    bool _zero;

public:
    ConditionalJump(int target, bool zero) : _target(target), _zero(zero) {}

    IsTerminator emit_code(rv::Compiler *c) const override {
        auto const reg = c->cb.to_reg(c->st.pop(), rv::Register::temp1());
        c->cb.emit_srai(reg, reg, 1);
        c->cb.emit_cj(
            _zero,
            reg,
            rv::Register::zero(),
            c->cb.label_for_ip(_target)
        );
        c->bb_begin(_target, c->st.top);
        return IsTerminator::No;
    }
};
LEAF(Begin, Instruction)
private:
    std::string _function_name;
    size_t _argc, _locc;
public:
    Begin(const std::string& function_name, int argc, int locc) : _function_name(function_name), _argc(argc), _locc(locc) {}

    IsTerminator emit_code(rv::Compiler *c) const override {
        c->cb.emit_label(_function_name);
        if (_function_name == "main") {
            c->cb.emit(c->premain());
        }
        c->current_frame = rv::FrameInfo{.function_name=_function_name, .locals_count=_locc, .args_count=_argc};
        // Save callee-saved registers (fp is included)
        rv::Register::saved_apply([c, this](const rv::Register& r, int i) {
            c->cb.emit_sd(r, rv::Register::sp(), -(i + _locc) * rv::WORD_SIZE);
        });
        // Set new frame pointer
        c->cb.emit_mv(rv::Register::fp(), rv::Register::sp());
        // Save sp
        // c->cb.emit_sd(rv::Register::sp(), rv::Register::sp(), -(_locc + 13) * rv::WORD_SIZE);
        c->cb.emit_addi(rv::Register::sp(), rv::Register::sp(), -(_locc + 12) * rv::WORD_SIZE);
        return IsTerminator::No;
    }
};

LEAF(Closure, Instruction)
private:
    size_t _offset;
    std::vector<LocationEntry> _entries;
public:
    Closure(int offset, std::vector<LocationEntry>&& entries) : _offset(offset), _entries(entries) {}
};

LEAF(CallClosure, Instruction)
private:
    size_t _argc;
public:
    CallClosure(int argc) : _argc(argc) {}
};

LEAF(Call, Instruction)
private:
    std::string _function_name;
    size_t  _argc;
public:
    Call(std::string function_name, int argc) : _function_name(function_name), _argc(argc) {}

    IsTerminator emit_code(rv::Compiler *c) const override {
        size_t alignment = (c->current_frame->locals_count + c->st.spilled_count() + _argc) & 1;
        // Skip spilled registers
        c->cb.emit_comment(std::format("Skip spilled registers {}", c->st.spilled_count()));
        c->cb.emit_addi(rv::Register::sp(), rv::Register::sp(), -c->st.spilled_count() * rv::WORD_SIZE);
        // Save ra
        c->cb.emit_sd(rv::Register::ra(), rv::Register::sp(), -rv::WORD_SIZE);
        c->cb.emit_addi(rv::Register::sp(), rv::Register::sp(), -rv::WORD_SIZE);
        // Save temp registers
        rv::Register::temp_apply([c](const rv::Register& r, int) {
            c->cb.emit_sd(r, rv::Register::sp(), -rv::WORD_SIZE);
            c->cb.emit_addi(rv::Register::sp(), rv::Register::sp(), -rv::WORD_SIZE);
        });
        // Save arguments
        rv::Register::arg_apply([c](const rv::Register& r, int) {
            c->cb.emit_sd(r, rv::Register::sp(), -rv::WORD_SIZE);
            c->cb.emit_addi(rv::Register::sp(), rv::Register::sp(), -rv::WORD_SIZE);
        });
        for (auto i : std::views::iota(0ul, _argc) | std::views::take(8)) {
            c->cb.symb_emit_mv(rv::Register::arg(i), c->st.pop());
        }
        // Align sp to 16 bytes
        if (alignment) {
            c->cb.emit_addi(rv::Register::sp(), rv::Register::sp(), -rv::WORD_SIZE);
        }
        // Store extra arguments on stack
        for (auto _ : std::views::iota(0ul, _argc) | std::views::drop(8) | std::views::reverse) {
            c->cb.emit_sd(c->cb.to_reg(c->st.pop(), rv::Register::temp1()), rv::Register::sp(), -rv::WORD_SIZE);
            c->cb.emit_addi(rv::Register::sp(), rv::Register::sp(), -rv::WORD_SIZE);
        }
        // Call function
        c->cb.emit_call(_function_name);
        // Drop extra arguments from stack
        if (_argc > 8) {
            c->cb.emit_addi(rv::Register::sp(), rv::Register::sp(), (_argc - 8) * rv::WORD_SIZE);
        }
        if (alignment) {
            c->cb.emit_addi(rv::Register::sp(), rv::Register::sp(), rv::WORD_SIZE);
        }
        c->cb.symb_emit_mv(c->st.alloc(), rv::Register::arg(0));
        // Restore arguments
        for (auto i : std::views::iota(0ul, 8ul) | std::views::reverse) {
            c->cb.emit_ld(rv::Register::arg(i), rv::Register::sp(), 0);
            c->cb.emit_addi(rv::Register::sp(), rv::Register::sp(), rv::WORD_SIZE);
        };
        // Restore temp registers
        c->cb.emit_addi(rv::Register::sp(), rv::Register::sp(), 8 * rv::WORD_SIZE);
        rv::Register::temp_apply([c](const rv::Register& r, int i) {
            c->cb.emit_ld(r, rv::Register::sp(), -(i + 1)*rv::WORD_SIZE);
        });
        // Restore ra
        c->cb.emit_ld(rv::Register::ra(), rv::Register::sp(), -rv::WORD_SIZE);
        c->cb.emit_addi(rv::Register::sp(), rv::Register::sp(), c->st.spilled_count() * rv::WORD_SIZE);
        return IsTerminator::No;
    }
};

LEAF(Tag, Instruction)
private:
    const char * _tag;
    size_t _size;
public:
    Tag(const char *tag, int size) : _tag(tag), _size(size) {}
};

LEAF(Array, Instruction)
private:
    size_t _size;
public:
    Array(int size) : _size(size) {}
};

LEAF(Fail, Instruction)
private:
    size_t _line, _col;
public:
    Fail(int line, int col) : _line(line), _col(col) {}
};

LEAF(Line, Instruction)
private:
    size_t _line;
public:
    Line(int line) : _line(line) {}
    IsTerminator emit_code(rv::Compiler *) const override {
        // do nothing
        return IsTerminator::No;
    }
};

LEAF(Binop, Instruction)
private:
    size_t _op;
public:
    Binop(int op) : _op(op) {}

    IsTerminator emit_code(rv::Compiler *c) const override {
        auto second_loc = c->st.pop();
        auto first_loc = c->st.pop();
        auto dest_loc = c->st.alloc();

        c->cb.symb_emit_srai(first_loc, first_loc, 1);
        c->cb.symb_emit_srai(second_loc, second_loc, 1);
#define EMIT_BINOP(code, symb) \
        case code: { \
            c->cb.symb_emit_##symb(dest_loc, first_loc, second_loc); \
            break; \
        }

        switch (_op) {
            BINOPS(EMIT_BINOP);
        }
        c->cb.symb_emit_slli(dest_loc, dest_loc, 1);
        c->cb.symb_emit_addi(dest_loc, dest_loc, 1);
        return IsTerminator::No;
    }
};

LEAF(Load, Instruction)
private:
    LocationEntry _loc;
public:
    Load(LocationEntry loc) : _loc(loc) {}

    IsTerminator emit_code(rv::Compiler *c) const override {
        switch (_loc.kind) {

        case Location_Global:{
            DCHECK_LT(_loc.index , c->globals_count) << "global index out of bounds";
            c->cb.symb_emit_ld(c->st.alloc(), {SymbolicLocationType::Register, rv::Register::gp().regno}, _loc.index * rv::WORD_SIZE);
            break;
        };

        case Location_Local:
        case Location_Arg:
        case Location_Captured:
            LOG(FATAL) << std::format("Unimplemented {:s} (loc.kind = {:d})", name(), to_underlying(_loc.kind));
            break;
        }
        return IsTerminator::No;
    }
};

LEAF(LoadArray, Instruction)
private:
    size_t _index;
    Location _loc;
public:
    LoadArray(int index, int location) : _index(index), _loc((Location)location) {}
};

LEAF(Store, Instruction)
private:
    LocationEntry _loc;
public:
    Store(LocationEntry loc) : _loc(loc) {}

    IsTerminator emit_code(rv::Compiler *c) const override {
        auto value = c->st.peek();
        switch (_loc.kind) {
        case Location_Global:{
            c->cb.symb_emit_sd(value, {SymbolicLocationType::Register, rv::Register::gp().regno}, _loc.index * rv::WORD_SIZE);
            break;
        }
        case Location_Local:
        case Location_Arg:
        case Location_Captured:
            LOG(FATAL) << std::format("Unimplemented {:s} (loc.kind = {:d})", name(), to_underlying(_loc.kind));
        }
        return IsTerminator::No;
    }
};

LEAF(PatternInst, Instruction)
private:
    Pattern _type;
public:
    PatternInst(int type) : _type(Pattern(type)) {}
};

LEAF(BuiltinRead, Instruction)
public:
    IsTerminator emit_code(rv::Compiler *c) const override {
        Call("Lread", 0).emit_code(c);
        return IsTerminator::No;
    }
};
LEAF(BuiltinWrite, Instruction)
    IsTerminator emit_code(rv::Compiler *c) const override {
        Call("Lwrite", 1).emit_code(c);
        return IsTerminator::No;
    }
};
LEAF(BuiltinLength, Instruction) };
LEAF(BuiltinString, Instruction) };
LEAF(BuiltinArray, Instruction) };

}
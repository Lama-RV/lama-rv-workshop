#pragma once
#include "code_buffer.h"
#include "instruction.h"
#include "opcode.h"
#include "register.h"
#include <cassert>
#include <cstdint>

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


#define LEAF(class_name, super_class_name)       \
  BASE(class_name, super_class_name)             \
   public:                                       \
    virtual const char* name() const             { return #class_name; }       \

using SymbolicLocationType = rv::SymbolicStack::LocType;

LEAF(Const, Instruction)
private:
    int _value;

public:
    Const(int value) : _value(value) {}

    inline int value() { return _value; }

    void emit_code(rv::Compiler *c) override {
        auto loc = c->st.alloc(); 
        switch (loc.type) {
            case SymbolicLocationType::Memory: {
                c->cb.emit_li(rv::Register::temp1(), _value);
                c->cb.emit_sd(rv::Register::temp1(), rv::Register::fp(), -loc.number * rv::WORD_SIZE);
                return;
            }
            case SymbolicLocationType::Register: {
                c->cb.emit_li({(uint8_t) loc.number}, _value);
                return;
            }
        }
    }
};

LEAF(String, Instruction)
private:
    const char *_str;
public:
    String(const char *str) : _str(str) {}

    inline const char* str() { return _str; }

    void emit_code(rv::Compiler *c) override {
        c->strs.emplace_back(_str);
        // call BString
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
    void emit_code(rv::Compiler *c) override {
        auto value_loc = c->st.pop();
        auto ptr_loc = c->st.pop();
        c->st.push(value_loc);
        c->cb.symb_emit_sd(value_loc, ptr_loc, 0);
    }
};
LEAF(StoreArray, Instruction) };
LEAF(End, Instruction) };
LEAF(Return, Instruction) };
LEAF(Duplicate, Instruction) };
LEAF(Drop, Instruction) 
    public:
    void emit_code(rv::Compiler *c) override {
        c->st.pop();
    }
};
LEAF(Swap, Instruction) };
LEAF(Elem, Instruction) };
LEAF(Jump, Instruction) 
private:
    size_t _offset;

public:
    Jump(int offset) : _offset(offset) {}
};
LEAF(ConditionalJump, Instruction) 
private:
    size_t _offset;
    bool _zero;

public:
    ConditionalJump(int offset, bool zero) : _offset(offset), _zero(zero) {}
};
LEAF(Begin, Instruction)
private:
    std::string _function_name;
    size_t _argc, _locc;
public:
    Begin(const std::string& function_name, int argc, int locc) : _function_name(function_name), _argc(argc), _locc(locc) {}

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
};

LEAF(Binop, Instruction)
private:
    size_t _op;
public:
    Binop(int op) : _op(op) {}

    void emit_code(rv::Compiler *c) override {
        auto first_loc = c->st.pop();
        auto second_loc = c->st.pop();
        auto dest_loc = c->st.alloc();

#define EMIT_BINOP(code, symb) \
        case code: { \
            c->cb.symb_emit_##symb(dest_loc, first_loc, second_loc); \
            break; \
        }

        switch (_op) {
            BINOPS(EMIT_BINOP);
        }
    }
};

LEAF(Load, Instruction)
private:
    LocationEntry _loc;
public:
    Load(LocationEntry loc) : _loc(loc) {}

    void emit_code(rv::Compiler *c) override {
        switch (_loc.kind) {

        case Location_Global:
            assert(_loc.index < c->globals_count && "global index out of bounds");
            c->cb.symb_emit_ld(c->st.alloc(), {SymbolicLocationType::Register, rv::Register::gp().regno}, _loc.index * rv::WORD_SIZE);
            return;
    
        case Location_Local:
        case Location_Arg:
        case Location_Captured:
            assert(0 && "unimplemented");
            break;
        }
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

    void emit_code(rv::Compiler *c) override {
        auto value = c->st.peek();
        switch (_loc.kind) {
        case Location_Global:
            c->cb.symb_emit_sd(value, {SymbolicLocationType::Register, rv::Register::gp().regno}, _loc.index * rv::WORD_SIZE);
            return;
        case Location_Local:
        case Location_Arg:
        case Location_Captured:
            assert(0 && "unimplemented");
        }
    }
};

LEAF(PatternInst, Instruction)
private:
    Pattern _type;
public:
    PatternInst(int type) : _type(Pattern(type)) {}
};

LEAF(BuiltinRead, Instruction) };
LEAF(BuiltinWrite, Instruction) };
LEAF(BuiltinLength, Instruction) };
LEAF(BuiltinString, Instruction) };
LEAF(BuiltinArray, Instruction) };

}
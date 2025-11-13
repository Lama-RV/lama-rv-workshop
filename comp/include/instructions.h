#pragma once

#include "instruction.h"
#include "opcode.h"
#include "runtime.h"

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

using SymbolicLocationType = rv::SymbolicStack::LocType;

class Const : public Instruction {
private:
    int _value;

public:
    Const(int value)
        : _value(BOX(value)) {}

    inline int value() {
        return _value;
    }

    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class String : public Instruction {
private:
    size_t _ind;
    std::string_view _str;

public:
    String(size_t index, std::string_view str)
        : _ind(index), _str(str) {}

    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class SExpression : public Instruction {
private:
    char const* _name;
    size_t _size;

public:
    SExpression(char const* name, int size)
        : _name(name)
        , _size(size) {}

    inline char const* tag() {
        return _name;
    }

    inline size_t size() {
        return _size;
    }

    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class StoreStack : public Instruction {
public:
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class StoreArray : public Instruction {
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class End : public Instruction {
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
    bool is_terminator() const override {
        return true;
    }
};

class Return : public Instruction {
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class Duplicate : public Instruction {
public:
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override {
        c->cb.symb_emit_mv(c->st.alloc(), c->st.peek());
    }
};

class Drop : public Instruction {
public:
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override {
        c->st.pop();
    }
};

class Swap : public Instruction {
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class Elem : public Instruction {
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class Jump : public Instruction {
private:
    size_t _target;

public:
    Jump(int target)
        : _target(target) {}

    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
    bool is_terminator() const override {
        return true;
    }
};

class ConditionalJump : public Instruction {
private:
    size_t _target;
    bool _zero;

public:
    ConditionalJump(int target, bool zero)
        : _target(target)
        , _zero(zero) {}

    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class Begin : public Instruction {
private:
    std::variant<std::string, size_t> _id;
    size_t _argc, _locc;

public:
    Begin(std::string function_name, int argc, int locc)
        : _id(std::move(function_name))
        , _argc(argc)
        , _locc(locc) {}
    Begin(size_t offset, int argc, int locc)
        : _id(offset)
        , _argc(argc)
        , _locc(locc) {}

    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class Closure : public Instruction {
private:
    size_t _offset;
    std::vector<LocationEntry> _entries;

public:
    Closure(int offset, std::vector<LocationEntry>&& entries)
        : _offset(offset)
        , _entries(entries) {}
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class CallClosure : public Instruction {
private:
    size_t _argc;

public:
    CallClosure(int argc)
        : _argc(argc) {}

    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class Call : public Instruction {
private:
    std::variant<std::string, size_t> _callee;
    size_t _argc;

public:
    Call(std::string function_name, int argc)
        : _callee(std::move(function_name))
        , _argc(argc) {}
    Call(size_t offset, int argc)
        : _callee(offset)
        , _argc(argc) {}

    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class Tag : public Instruction {
private:
    const char * _tag;
    size_t _size;

public:
    Tag(char const* tag, int size)
        : _tag(tag)
        , _size(size) {}

    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class Array : public Instruction {
private:
    size_t _size;

public:
    Array(int size)
        : _size(size) {}

    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class Fail : public Instruction {
private:
    size_t _line, _col;

public:
    Fail(int line, int col)
        : _line(line)
        , _col(col) {}

    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
    bool is_terminator() const override { return true; }
};

class Line : public Instruction {
private:
    size_t _line;

public:
    Line(int line)
        : _line(line) {}
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override {
        c->cb.emit_comment(std::format("LINE {:d}", _line));
    }
};

class Binop : public Instruction {
private:
    BinopKind _op;

public:
    Binop(::BinopKind op)
        : _op(op) {}
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class Load : public Instruction {
private:
    LocationEntry _loc;

public:
    Load(LocationEntry loc)
        : _loc(loc) {}

    void print(std::ostream&) const override;

    void emit_code(rv::Compiler* c) const override;
};

class LoadArray : public Instruction {
private:
    size_t _index;
    Location _loc;

public:
    LoadArray(int index, int location)
        : _index(index)
        , _loc((Location)location) {}

    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class Store : public Instruction {
private:
    LocationEntry _loc;

public:
    Store(LocationEntry loc)
        : _loc(loc) {}

    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class PatternInst : public Instruction {
private:
    Pattern _type;

public:
    PatternInst(int type)
        : _type(Pattern(type)) {}
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class BuiltinRead : public Instruction {
public:
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override {
        c->compile_call("Lread", 0);
    }
};

class BuiltinWrite : public Instruction {
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override {
        c->compile_call("Lwrite", 1);
    }
};

class BuiltinLength : public Instruction {
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class BuiltinString : public Instruction {
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

class BuiltinArray : public Instruction {
    size_t _len;

public:
    BuiltinArray(size_t len)
        : _len(len) {}
    void print(std::ostream&) const override;
    void emit_code(rv::Compiler* c) const override;
};

}  // namespace lama

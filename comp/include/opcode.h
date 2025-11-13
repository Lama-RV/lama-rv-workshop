#pragma once

#include <ostream>

enum class HOpcode {
    Binop = 0,
    Ld = 2,
    LdA = 3,
    St = 4,
    Patt = 6,
    LCall = 7,
    Stop = 15,
};

enum class BinopKind {
    Add = 1,
    Sub = 2,
    Mul = 3,
    Div = 4,
    Rem = 5,
    LessThan = 6,
    LessEqual = 7,
    GreaterThan = 8,
    GreaterEqual = 9,
    Equal = 10,
    NotEqual = 11,
    And = 12,
    Or = 13,
};

enum class Opcode {
    Const = 16,
    String = 17,
    SExp = 18,
    StI = 19,
    StA = 20,
    Jmp = 21,
    End = 22,
    Ret = 23,
    Drop = 24,
    Dup = 25,
    Swap = 26,
    Elem = 27,

    CJmpZ = 80,
    CJmpNZ = 81,
    Begin = 82,
    CBegin = 83,
    Closure = 84,
    CallC = 85,
    Call = 86,
    Tag = 87,
    Array = 88,
    Fail = 89,
    Line = 90,
};

enum class LCall {
    Lread = 0,
    Lwrite = 1,
    Llength = 2,
    Lstring = 3,
    Barray = 4,
};

enum class Location {
    Global = 0,
    Local = 1,
    Arg = 2,
    Captured = 3,
};

enum class Pattern {
    //
    String = 0,
    StringTag = 1,
    ArrayTag = 2,
    SExpTag = 3,
    Boxed = 4,
    Unboxed = 5,
    ClosureTag = 6
};

#define BINOPS(MACRO)                   \
    MACRO(BinopKind::Add, add)          \
    MACRO(BinopKind::Sub, sub)          \
    MACRO(BinopKind::Mul, mul)          \
    MACRO(BinopKind::Div, div)          \
    MACRO(BinopKind::LessThan, slt)     \
    MACRO(BinopKind::LessEqual, sle)    \
    MACRO(BinopKind::And, and)          \
    MACRO(BinopKind::Or, or)            \
    MACRO(BinopKind::Rem, rem)          \
    MACRO(BinopKind::GreaterThan, sgt)  \
    MACRO(BinopKind::GreaterEqual, sge) \
    MACRO(BinopKind::Equal, eq)         \
    MACRO(BinopKind::NotEqual, neq)

#define LOCATIONS(hi, MACRO)         \
    MACRO(hi, Location::Global, "G") \
    MACRO(hi, Location::Local, "L")  \
    MACRO(hi, Location::Arg, "A")    \
    MACRO(hi, Location::Captured, "C")

struct LocationEntry {
    Location kind;
    int index;
};

#define LCALLS(MACRO)                \
    MACRO(LCall::Lread, "Lread")     \
    MACRO(LCall::Lwrite, "Lwrite")   \
    MACRO(LCall::Llength, "Llength") \
    MACRO(LCall::Lstring, "Lstring") \
    MACRO(LCall::Barray, "Barray")

#define PATTERNS(MACRO)                  \
    MACRO(Pattern::String, "=str")       \
    MACRO(Pattern::StringTag, "#string") \
    MACRO(Pattern::ArrayTag, "#array")   \
    MACRO(Pattern::SExpTag, "#sexp")     \
    MACRO(Pattern::Boxed, "#ref")        \
    MACRO(Pattern::Unboxed, "#val")      \
    MACRO(Pattern::ClosureTag, "#fun")

#define SINGLE(code) ((unsigned char)(code))
#define COMPOSED(hi, lo) ((unsigned char)(((hi) << 4) | (lo)))

std::ostream& operator<<(std::ostream&, BinopKind const&);
std::ostream& operator<<(std::ostream&, Location const&);
std::ostream& operator<<(std::ostream&, LocationEntry const&);

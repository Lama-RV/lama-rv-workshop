#include <glog/logging.h>
#include <cstddef>
#include <variant>
#include "instructions.h"
#include "opcode.h"

#define TODO() LOG(FATAL) << "Not implemented yet"

namespace {

std::string ip_to_string(size_t ip) {
    return std::format("{:#010x}", ip);
}

}  // namespace

namespace lama {

void Const::print(std::ostream& os) const {
    os << "CONST\t" << _value;
}

void String::print(std::ostream& os) const {
    os << "STRING\t" << this->_str;
}

void SExpression::print(std::ostream& os) const {
    os << "SEXP\t" << _name << " " << _size;
}

void StoreStack::print(std::ostream&) const {
    TODO();
}

void StoreArray::print(std::ostream& os) const {
    os << "STA";
}

void Jump::print(std::ostream& os) const {
    os << "JMP\t" << std::format("{:#010x}", _target);
}

void ConditionalJump::print(std::ostream& os) const {
    os << "CJMP" << (_zero ? "z" : "nz") << "\t" << std::format("{:#010x}", _target);
}

void Return::print(std::ostream&) const {
    TODO();
}

void Drop::print(std::ostream& os) const {
    os << "DROP";
}

void Duplicate::print(std::ostream& os) const {
    os << "DUP";
}

void Swap::print(std::ostream&) const {
    TODO();
}

void Elem::print(std::ostream& os) const {
    os << "ELEM";
}

void Closure::print(std::ostream& os) const {
    os << "CLOSURE\t" << ip_to_string(_offset);
    for (auto const& loc : _entries) {
        os << loc;
    }
}

void CBegin::print(std::ostream& os) const {
    os << "CBEGIN\t" << _argc << " " << _locc;
}

void Begin::print(std::ostream& os) const {
    os << "BEGIN\t" << _argc << " " << _locc;
    if (std::holds_alternative<std::string>(_id)) {
        os << "\t# " << std::get<std::string>(_id);
    }
}

void End::print(std::ostream& os) const {
    os << "END";
}

void CallClosure::print(std::ostream& os) const {
    os << "CALLC\t" << _argc;
}

void Tag::print(std::ostream& os) const {
    os << "TAG\t" << _tag << " " << _size;
}

void Array::print(std::ostream& os) const {
    os << "ARRAY\t" << _size;
}

void Fail::print(std::ostream& os) const {
    os << "FAIL\t" << _line << ":" << _col;
}

void Line::print(std::ostream& os) const {
    os << "LINE\t" << _line;
}

void Load::print(std::ostream& os) const {
    os << "LD\t" << _loc;
}

void LoadArray::print(std::ostream&) const {
    TODO();
}

void Store::print(std::ostream& os) const {
    os << "ST\t" << _loc;
}

void PatternInst::print(std::ostream& os) const {
    os << "PATT\t" << _type;
}

void BuiltinRead::print(std::ostream& os) const {
    os << "CALL\tLread";
}

void BuiltinWrite::print(std::ostream& os) const {
    os << "CALL\tLwrite";
}

void BuiltinLength::print(std::ostream& os) const {
    os << "CALL\tLlength";
}

void BuiltinString::print(std::ostream& os) const {
    os << "CALL\tLstring";
}

void BuiltinArray::print(std::ostream& os) const {
    os << "CALL\tBarray\t" << _len;
}

void Call::print(std::ostream& os) const {
    os << "CALL\t" << ip_to_string(_callee) << " " << _argc;
}

void BuiltinCall::print(std::ostream& os) const {
    os << "CALL\t" << _callee << " " << _argc;
}

void Binop::print(std::ostream& os) const {
    os << "BINOP\t" << _op;
}

}  // namespace lama

std::ostream& operator<<(std::ostream& os, BinopKind const& op) {
    switch (op) {
    case BinopKind::Add:
        return os << "+";
    case BinopKind::Sub:
        return os << "-";
    case BinopKind::Mul:
        return os << "*";
    case BinopKind::Div:
        return os << "/";
    case BinopKind::Rem:
        return os << "%";
    case BinopKind::LessThan:
        return os << "<";
    case BinopKind::LessEqual:
        return os << "<=";
    case BinopKind::GreaterThan:
        return os << ">";
    case BinopKind::GreaterEqual:
        return os << ">=";
    case BinopKind::Equal:
        return os << "==";
    case BinopKind::NotEqual:
        return os << "!=";
    case BinopKind::And:
        return os << "&&";
    case BinopKind::Or:
        return os << "||";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Pattern const& patt) {
#define PRINT_PATT(p, name) \
    case p:                 \
        return os << name;
    switch (patt) { PATTERNS(PRINT_PATT); }
    return os;
};

#define PRINT_LOC(os, location, name) \
    case location:                    \
        return os << name;

std::ostream& operator<<(std::ostream& os, Location const& loc) {
    switch (loc) { LOCATIONS(os, PRINT_LOC); }
    return os;
};

std::ostream& operator<<(std::ostream& os, LocationEntry const& loc) {
    return os << loc.kind << "(" << loc.index << ")";
};

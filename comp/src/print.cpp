#include <glog/logging.h>
#include <cstddef>
#include <variant>
#include "cpp.h"
#include "instructions.h"
#include "opcode.h"

#define TODO() LOG(FATAL) << "Not implemented yet"

namespace {

std::string ip_to_string(size_t ip) {
    return std::format("{:#010x}", ip);
}

std::string loc_to_string(std::variant<std::string, size_t> const& loc) {
    return std::visit(
        overloads{
            [](size_t ip) { return ip_to_string(ip); },
            [](std::string name) { return name; },
        },
        loc
    );
}
}  // namespace

namespace lama {

void Const::print(std::ostream& os) const {
    DCHECK(_value & 1);
    os << "CONST\t" << (_value >> 1);
}

void String::print(std::ostream& os) const {
    os << "STRING\t" << this->_str;
}

void SExpression::print(std::ostream& os) const {
    TODO();
}

void StoreStack::print(std::ostream& os) const {
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

void Return::print(std::ostream& os) const {
    TODO();
}

void Drop::print(std::ostream& os) const {
    os << "DROP";
}

void Duplicate::print(std::ostream& os) const {
    TODO();
}

void Swap::print(std::ostream& os) const {
    TODO();
}

void Elem::print(std::ostream& os) const {
    os << "ELEM";
}

void Closure::print(std::ostream& os) const {
    TODO();
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
    TODO();
}

void Tag::print(std::ostream& os) const {
    TODO();
}

void Array::print(std::ostream& os) const {
    TODO();
}

void Fail::print(std::ostream& os) const {
    TODO();
}

void Line::print(std::ostream& os) const {
    os << "LINE\t" << _line;
}

void Load::print(std::ostream& os) const {
    os << "LD\t" << _loc;
}

void LoadArray::print(std::ostream& os) const {
    TODO();
}

void Store::print(std::ostream& os) const {
    os << "ST\t" << _loc;
}

void PatternInst::print(std::ostream& os) const {
    TODO();
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
    os << "CALL\t" << loc_to_string(_callee) << " " << _argc;
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

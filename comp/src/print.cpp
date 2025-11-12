#include "instructions.h"
#include <glog/logging.h>
#include <variant>
#include "compiler.h"
#include "cpp.h"

namespace lama {

#define TODO() LOG(FATAL) << "Not implemented yet"

void Const::print(std::ostream&) const {
    TODO();
}

void String::print(std::ostream&) const {
    TODO();
}

void SExpression::print(std::ostream&) const {
    TODO();
}

void StoreStack::print(std::ostream&) const {
    TODO();
}

void StoreArray::print(std::ostream&) const {
    TODO();
}

void Jump::print(std::ostream&) const {
    TODO();
}

void ConditionalJump::print(std::ostream&) const {
    TODO();
}

void Return::print(std::ostream&) const {
    TODO();
}

void Drop::print(std::ostream&) const {
    TODO();
}

void Duplicate::print(std::ostream&) const {
    TODO();
}

void Swap::print(std::ostream&) const {
    TODO();
}

void Elem::print(std::ostream&) const {
    TODO();
}

void Closure::print(std::ostream&) const {
    TODO();
}

void Begin::print(std::ostream& os) const {
    os << "BEGIN "
       << std::visit(
              overloads{
                  [](size_t ip) { return rv::Compiler::label_for_ip(ip); },
                  [](std::string name) { return name; },
              },
              _id
          )
       << "\t" << _argc << "\t" << _locc;
}

void End::print(std::ostream&) const {
    TODO();
}

void CallClosure::print(std::ostream&) const {
    TODO();
}

void Tag::print(std::ostream&) const {
    TODO();
}

void Array::print(std::ostream&) const {
    TODO();
}

void Fail::print(std::ostream&) const {
    TODO();
}

void Line::print(std::ostream&) const {
    TODO();
}

void Load::print(std::ostream&) const {
    TODO();
}

void LoadArray::print(std::ostream&) const {
    TODO();
}

void Store::print(std::ostream&) const {
    TODO();
}

void PatternInst::print(std::ostream&) const {
    TODO();
}

void BuiltinRead::print(std::ostream&) const {
    TODO();
}

void BuiltinWrite::print(std::ostream&) const {
    TODO();
}

void BuiltinLength::print(std::ostream&) const {
    TODO();
}

void BuiltinString::print(std::ostream&) const {
    TODO();
}

void BuiltinArray::print(std::ostream&) const {
    TODO();
}
void Call::print(std::ostream&) const {
    TODO();
}

void Binop::print(std::ostream&) const {
    TODO();
}

}  // namespace lama

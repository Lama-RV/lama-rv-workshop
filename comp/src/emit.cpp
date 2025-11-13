#include <glog/logging.h>
#include <variant>
#include "compiler.h"
#include "cpp.h"
#include "instructions.h"
#include "runtime.h"

namespace lama {

#define TODO() LOG(FATAL) << "Not implemented yet\t"

void Const::emit_code(rv::Compiler* c) const {
    auto loc = c->st.alloc();
    switch (loc.type) {
    case SymbolicLocationType::Memory: {
        c->cb.emit_li(rv::Register::temp1(), _value);
        c->cb.emit_sd(rv::Register::temp1(), rv::Register::fp(), -loc.number * rv::WORD_SIZE);
        break;
    }
    case SymbolicLocationType::Register: {
        c->cb.emit_li({(uint8_t)loc.number}, _value);
        break;
    }
    }
}

void String::emit_code(rv::Compiler* c) const {
    c->cb.symb_emit_la(c->st.alloc(), std::format("string_{}", _ind));
    c->compile_call("RVBstring", 1);
}

void SExpression::emit_code(rv::Compiler* c) const {
    c->cb.symb_emit_li(c->st.alloc(), lama::LtagHash(const_cast<char*>(_name)));
    c->compile_call("RVBsexp", _size + 1, BOX(_size + 1));
}

void StoreStack::emit_code(rv::Compiler* c) const {
    auto value_loc = c->st.pop();
    auto ptr_loc = c->st.pop();
    c->st.push(value_loc);
    c->cb.symb_emit_sd(value_loc, ptr_loc, 0);
}

void StoreArray::emit_code(rv::Compiler* c) const {
    c->compile_call("Bsta", 3);
}

void Jump::emit_code(rv::Compiler* c) const {
    c->cb.emit_j(c->label_for_ip(_target));
    c->add_jump_target(_target, c->st.top);
}

void ConditionalJump::emit_code(rv::Compiler* c) const {
    auto const reg = c->cb.to_reg(c->st.pop(), rv::Register::temp1());
    c->cb.emit_srai(reg, reg, 1);
    c->cb.emit_cj(_zero, reg, rv::Register::zero(), c->label_for_ip(_target));
    c->add_jump_target(_target, c->st.top);
}

void Return::emit_code(rv::Compiler*) const {
    TODO();
}

void Swap::emit_code(rv::Compiler*) const {
    TODO();
}

void Elem::emit_code(rv::Compiler* c) const {
    c->compile_call("Belem", 2);
}

void Closure::emit_code(rv::Compiler*) const {
    TODO();
}

void CBegin::emit_code(rv::Compiler* c) const {
    Begin(std::format("closure@{:#010x}", offset), _argc, _locc).emit_code(c);
}

void Begin::emit_code(rv::Compiler* c) const {
    std::string name = std::visit(
        overloads{
            [c](size_t offset) { return c->label_for_ip(offset); },
            [c](std::string name) {
                c->cb.emit_label(name);
                return name;
            },
        },
        _id
    );
    c->current_frame = rv::FrameInfo{.function_name = name, .locals_count = _locc, .args_count = _argc};
    if (name == "main") {
        c->cb.emit(c->premain());
        c->compile_call("__init", 0);
    }
    // Save callee-saved registers (fp is included)
    rv::Register::saved_apply([c, this](rv::Register const& r, int i) {
        c->cb.emit_sd(r, rv::Register::sp(), -(i + _locc) * rv::WORD_SIZE);
    });
    // Set new frame pointer
    c->cb.emit_mv(rv::Register::fp(), rv::Register::sp());
    // Save sp
    // c->cb.emit_sd(rv::Register::sp(), rv::Register::sp(), -(_locc + 13) * rv::WORD_SIZE);
    c->cb.emit_addi(rv::Register::sp(), rv::Register::sp(), -(_locc + 12) * rv::WORD_SIZE);
}

void End::emit_code(rv::Compiler* c) const {
    DCHECK(c->current_frame.has_value()) << "no current frame in End instruction";
    size_t _locc = c->current_frame->locals_count;
    c->cb.symb_emit_mv(rv::Register::arg(0), c->st.pop());
    // Restore sp
    c->cb.emit_mv(rv::Register::sp(), rv::Register::fp());
    // Restore callee-saved registers (fp is included)
    rv::Register::saved_apply([c, _locc](rv::Register const& r, int i) {
        c->cb.emit_ld(r, rv::Register::sp(), -(i + _locc) * rv::WORD_SIZE);
    });
    if (c->current_frame->function_name == "main") {
        c->cb.emit(c->postmain());
    }
    // Return
    c->cb.emit_ret();
}

void CallClosure::emit_code(rv::Compiler*) const {
    TODO();
}

void Tag::emit_code(rv::Compiler* c) const {
    c->cb.symb_emit_li(c->st.alloc(), LtagHash(_tag));
    c->cb.symb_emit_li(c->st.alloc(), BOX(_size));
    c->compile_call("Btag", 3);
}

void Array::emit_code(rv::Compiler*) const {
    TODO();
}

void Fail::emit_code(rv::Compiler* c) const {
    c->cb.symb_emit_la(c->st.alloc(), "fname");
    c->cb.symb_emit_li(c->st.alloc(), BOX(_line));
    c->cb.symb_emit_li(c->st.alloc(), BOX(_col));
    c->compile_call("Bmatch_failure", 4);
}

void Load::emit_code(rv::Compiler* c) const {
    switch (_loc.kind) {
    case Location::Global: {
        DCHECK_LT(_loc.index, c->globals_count) << "global index out of bounds";
        c->cb.symb_emit_ld(
            c->st.alloc(), {SymbolicLocationType::Register, rv::Register::gp().regno}, _loc.index * rv::WORD_SIZE
        );
        break;
    };

    case Location::Local: {
        DCHECK_LT(_loc.index, c->current_frame->locals_count) << "local index out of bounds";
        c->cb.symb_emit_ld(
            c->st.alloc(), {SymbolicLocationType::Register, rv::Register::fp().regno}, -_loc.index * rv::WORD_SIZE
        );
        break;
    };

    case Location::Arg: {
        DCHECK_LT(_loc.index, c->current_frame->args_count) << "arg index out of bounds";
        if (_loc.index < 8) {
            c->cb.symb_emit_mv(c->st.alloc(), rv::Register::arg(_loc.index));
        } else {
            c->cb.symb_emit_ld(
                c->st.alloc(), {SymbolicLocationType::Register, rv::Register::fp().regno},
                static_cast<size_t>(_loc.index) - 8
            );
        }
        break;
    }

    case Location::Captured:
        TODO() << *this;
        break;
    }
}

void LoadArray::emit_code(rv::Compiler*) const {
    TODO();
}

void Store::emit_code(rv::Compiler* c) const {
    auto value = c->st.peek();
    switch (_loc.kind) {
    case Location::Global: {
        c->cb.symb_emit_sd(
            value, {SymbolicLocationType::Register, rv::Register::gp().regno}, _loc.index * rv::WORD_SIZE
        );
        break;
    }
    case Location::Local:
        c->cb.symb_emit_sd(
            value, {SymbolicLocationType::Register, rv::Register::fp().regno}, -_loc.index * rv::WORD_SIZE
        );
        break;
    case Location::Arg:
        if (_loc.index < 8) {
            c->cb.symb_emit_mv(value, rv::Register::arg(_loc.index));
        } else {
            c->cb.symb_emit_sd(
                value, {SymbolicLocationType::Register, rv::Register::fp().regno}, (_loc.index - 8) * rv::WORD_SIZE
            );
        }
        break;
    case Location::Captured:
        TODO() << *this;
    }
}

void PatternInst::emit_code(rv::Compiler*) const {
    TODO();
}

void BuiltinLength::emit_code(rv::Compiler* c) const {
    c->compile_call("Llength", 1);
}

void BuiltinString::emit_code(rv::Compiler* c) const {
    c->compile_call("RVLstring", 1);
}

void BuiltinArray::emit_code(rv::Compiler* c) const {
    c->compile_call("RVBarray", _len, BOX(_len));
}

void Call::emit_code(rv::Compiler* c) const {
    c->compile_call(_callee, _argc);
}

void Binop::emit_code(rv::Compiler* c) const {
    auto second_loc = c->st.pop();
    auto first_loc = c->st.pop();
    auto dest_loc = c->st.alloc();

    c->cb.symb_emit_srai(first_loc, first_loc, 1);
    c->cb.symb_emit_srai(second_loc, second_loc, 1);
#define EMIT_BINOP(code, symb)                                   \
    case code: {                                                 \
        c->cb.symb_emit_##symb(dest_loc, first_loc, second_loc); \
        break;                                                   \
    }

    switch (_op) { BINOPS(EMIT_BINOP); }
    c->cb.symb_emit_slli(dest_loc, dest_loc, 1);
    c->cb.symb_emit_addi(dest_loc, dest_loc, 1);
}

}  // namespace lama

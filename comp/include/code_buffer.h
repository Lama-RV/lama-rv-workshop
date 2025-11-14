#pragma once

#include <format>
#include <ostream>
#include <string>
#include <string_view>

#include "register.h"
#include "symb_stack.h"

namespace lama::rv {
static size_t temp_reg_num = 30;

class CodeBuffer {
private:
    std::ostream& out_;

public:
    CodeBuffer(std::ostream& os)
        : out_(os) {};

    using SymbolicLocation = SymbolicStack::Loc;

// insn reg_dest, reg_src1, reg_src1
#define R_TYPE(rv_insn)                                                                         \
    void emit_##rv_insn(const Register& dst, const Register& src1, const Register& src2) {      \
        emit_r_type(#rv_insn, dst, src1, src2);                                                 \
    }                                                                                           \
    void symb_emit_##rv_insn(                                                                   \
        const SymbolicLocation& dst, const SymbolicLocation& src1, const SymbolicLocation& src2 \
    ) {                                                                                         \
        symb_emit_r_type(#rv_insn, dst, src1, src2);                                            \
    }

// insn reg_dest, reg_src, immediate
#define I_TYPE(rv_insn)                                                                           \
    void emit_##rv_insn(const Register& dst, const Register& src, int imm) {                      \
        emit_i_type(#rv_insn, dst, src, imm);                                                     \
    }                                                                                             \
    void symb_emit_##rv_insn(const SymbolicLocation& dst, const SymbolicLocation& src, int imm) { \
        symb_emit_i_type(#rv_insn, dst, src, imm);                                                \
    }

// insn reg_dest, base_reg(immediate)
#define S_TYPE(rv_insn)                                                                            \
    void emit_##rv_insn(const Register& dst, const Register& base, int off) {                      \
        emit_s_type(#rv_insn, dst, base, off);                                                     \
    }                                                                                              \
    void symb_emit_##rv_insn(const SymbolicLocation& dst, const SymbolicLocation& base, int off) { \
        symb_emit_s_type(#rv_insn, dst, base, off);                                                \
    }

// insn reg_dest, immediate
#define U_TYPE(rv_insn)                                              \
    void emit_##rv_insn(const Register& dst, int imm) {              \
        emit_u_type(#rv_insn, dst, imm);                             \
    }                                                                \
    void symb_emit_##rv_insn(const SymbolicLocation& dst, int imm) { \
        symb_emit_u_type(#rv_insn, dst, imm);                        \
    }

#define PSEUDO_TYPE(rv_insn)                                                             \
    void emit_##rv_insn(const Register& dst, const Register& src) {                      \
        emit_pseudo_type(#rv_insn, dst, src);                                            \
    }                                                                                    \
    void symb_emit_##rv_insn(const SymbolicLocation& dst, const SymbolicLocation& src) { \
        symb_emit_pseudo_type(#rv_insn, dst, src);                                       \
    }

    R_TYPE(add);
    R_TYPE(sub);
    R_TYPE(slt);
    R_TYPE(sgt);

    template <typename T = std::false_type, typename F>
    void apply(SymbolicStack::Loc const& loc, F&& f) {
        if (loc.type == SymbolicStack::LocType::Memory) {
            CHECK_LT(temp_reg_num, 32) << "No more temporary registers";
            Register temp_reg = Register{temp_reg_num++};
            if (!T::value) {
                emit_ld(temp_reg, rv::Register::sp(), -loc.number * WORD_SIZE);
            }
            f(temp_reg);
            if (T::value) {
                emit_sd(temp_reg, rv::Register::sp(), -loc.number * WORD_SIZE);
            }
            --temp_reg_num;
        } else {
            f(Register{loc.number});
        }
    }

    void emit_eq(Register const& dst, Register const& src1, Register const& src2) {
        emit_sub(dst, src2, src1);
        emit_seqz(dst, dst);
    }

    void symb_emit_eq(SymbolicLocation const& dst, SymbolicLocation const& src1, SymbolicLocation const& src2) {
        symb_emit_sub(dst, src2, src1);
        symb_emit_seqz(dst, dst);
    }

    void emit_sle(Register const& dst, Register const& src1, Register const& src2) {
        emit_sgt(dst, src1, src2);
        emit_xori(dst, dst, 1);
    }

    void symb_emit_sle(SymbolicLocation const& dst, SymbolicLocation const& src1, SymbolicLocation const& src2) {
        symb_emit_sgt(dst, src1, src2);
        symb_emit_xori(dst, dst, 1);
    }

    // sltu rd, x0, rs
    void emit_neq(Register const& dst, Register const& src1, Register const& src2) {
        emit_sub(dst, src2, src1);
        emit_snez(dst, dst);
    }

    void symb_emit_neq(SymbolicLocation const& dst, SymbolicLocation const& src1, SymbolicLocation const& src2) {
        symb_emit_sub(dst, src2, src1);
        symb_emit_snez(dst, dst);
    }

    void emit_sge(Register const& dst, Register const& src1, Register const& src2) {
        emit_slt(dst, src1, src2);
        emit_xori(dst, dst, 1);
    }

    void symb_emit_sge(SymbolicLocation const& dst, SymbolicLocation const& src1, SymbolicLocation const& src2) {
        symb_emit_slt(dst, src1, src2);
        symb_emit_xori(dst, dst, 1);
    }

    R_TYPE(or);
    R_TYPE(and);
    R_TYPE(srl);  // logical
    R_TYPE(sra);  // arithmetical
    R_TYPE(sll);
    R_TYPE(mul);
    R_TYPE(div);
    R_TYPE(rem);

    I_TYPE(addi);
    I_TYPE(slti);
    I_TYPE(srai);
    I_TYPE(slli);
    I_TYPE(ori);
    I_TYPE(xori);
    I_TYPE(andi);

    S_TYPE(ld);
    S_TYPE(sd);

    U_TYPE(li);

    PSEUDO_TYPE(seqz);
    PSEUDO_TYPE(snez);

    void emit_mv(Register const& dst, Register const& src) {
        emit_addi(dst, src, 0);
    }

    void symb_emit_mv(Register const& dst_reg, SymbolicLocation const& src) {
        apply(src, [&](Register const& src_reg) { emit(std::format("mv\t{},\t{}", dst_reg, src_reg)); });
    }

    void symb_emit_mv(SymbolicLocation const& dst, Register const& src_reg) {
        apply<std::true_type>(dst, [&](Register const& dst_reg) { emit(std::format("mv\t{},\t{}", dst_reg, src_reg)); });
    }

    void symb_emit_mv(SymbolicLocation const& dst, SymbolicLocation const& src) {
        apply(src, [&](Register const& src_reg) {
            apply<std::true_type>(dst, [&](Register const& dst_reg) { emit(std::format("mv\t{},\t{}", dst_reg, src_reg)); });
        });
    }

    void symb_emit_la(SymbolicLocation const& dst, std::string_view label) {
        apply<std::true_type>(dst, [&](Register const& dst_reg) { emit(std::format("la\t{},\t{}", dst_reg, label)); });
    }

    void emit_call(std::string_view label) {
        emit(std::format("call\t{}", label));
    }

    void emit_jalr(Register const& reg) {
        emit(std::format("jalr\t{}", reg));
    }

    void emit_ret() {
        emit("ret");
    }

    void emit_label(std::string_view label) {
        emit(std::format("{}:", label));
    }

    void emit_comment(std::string_view comment) {
        emit(std::format("# {}", comment));
    }

    void emit_j(std::string_view target_label) {
        emit(std::format("j {}", target_label));
    }

    void emit_cj(bool on_eq, Register const& r1, Register const& r2, std::string_view target_label) {
        emit(std::format("{:s}\t{},\t{},\t{}", on_eq ? "beq" : "bne", r1, r2, target_label));
    }

    void emit(std::string_view str) {
        out_ << str << std::endl;
    }

private:
    void emit_pseudo_type(std::string const& insn, Register const& dst, Register const& src) {
        emit(std::format("{}\t{},\t{}", insn, dst, src));
    }

    void emit_r_type(std::string const& insn, Register const& dst, Register const& src1, Register const& src2) {
        emit(std::format("{}\t{},\t{},\t{}", insn, dst, src1, src2));
    }

    void emit_i_type(std::string const& insn, Register const& dst, Register const& src, int imm) {
        emit(std::format("{}\t{},\t{},\t{}", insn, dst, src, imm));
    }

    void emit_u_type(std::string const& insn, Register const& dst, int imm) {
        emit(std::format("{}\t{},\t{}", insn, dst, imm));
    }

    void emit_s_type(std::string const& insn, Register const& dst, Register const& base, int off) {
        emit(std::format("{}\t{},\t{}({})", insn, dst, off, base));
    }

    void symb_emit_r_type(
        std::string const& insn,
        SymbolicLocation const& dst,
        SymbolicLocation const& src1,
        SymbolicLocation const& src2
    ) {
        apply(src1, [&](Register const& src1_reg) {
            apply(src2, [&](Register const& src2_reg) {
                switch (dst.type) {
                case SymbolicStack::LocType::Register:
                    emit_r_type(insn, Register{dst.number}, src1_reg, src2_reg);
                    return;
                case SymbolicStack::LocType::Memory:
                    emit_r_type(insn, src1_reg, src1_reg, src2_reg);
                    emit_sd(src1_reg, rv::Register::sp(), -dst.number * WORD_SIZE);
                    return;
                }
            });
        });
    }

    void symb_emit_i_type(std::string const& insn, SymbolicLocation const& dst, SymbolicLocation const& src, int imm) {
        apply(src, [&](Register const& src_reg) {
            apply<std::true_type>(dst, [&](Register const& dst_reg) { emit_i_type(insn, dst_reg, src_reg, imm); });
        });
    }

    void symb_emit_u_type(std::string const& insn, SymbolicLocation const& dst, int imm) {
        apply<std::true_type>(dst, [&](Register const& dst_reg) { emit_u_type(insn, dst_reg, imm); });
    }

    void symb_emit_s_type(std::string const& insn, SymbolicLocation const& dst, SymbolicLocation const& base, int off) {
        apply(base, [&](Register const& base_reg) {
            apply<std::true_type>(dst, [&](Register const& dst_reg) { emit_s_type(insn, dst_reg, base_reg, off); });
        });
    }

    void symb_emit_pseudo_type(std::string const& insn, SymbolicLocation const& dst, SymbolicLocation const& src) {
        apply(src, [&](Register const& src_reg) {
            apply<std::true_type>(dst, [&](Register const& dst_reg) { emit_pseudo_type(insn, dst_reg, src_reg); });
        });
    }
};

}  // namespace lama::rv

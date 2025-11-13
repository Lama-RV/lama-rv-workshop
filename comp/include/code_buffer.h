#pragma once
#include <format>
#include <string>
#include <string_view>
#include <vector>

#include "symb_stack.h"
#include "register.h"

#define DEBUG_COMMENTS 1

namespace lama::rv {

    class CodeBuffer {
        private:
        std::vector<std::string> _code;
        public:
        CodeBuffer() = default;

        std::string dump_asm() {
            std::string s;
            s.reserve(1 << 14);
            for (const std::string& str : _code) {
                s += str + "\n";
            }
            return s;
        }

        using SymbolicLocation = SymbolicStack::Loc;

        // insn reg_dest, reg_src1, reg_src1
        #define R_TYPE(rv_insn) \
            void emit_ ## rv_insn (const Register& dst, const Register& src1, const Register& src2) { \
                emit_r_type(#rv_insn, dst, src1, src2); \
            } \
            void symb_emit_ ## rv_insn (const SymbolicLocation& dst, const SymbolicLocation& src1, const SymbolicLocation& src2) { \
                symb_emit_r_type(#rv_insn, dst, src1, src2); \
            } \

        // insn reg_dest, reg_src, immediate
        #define I_TYPE(rv_insn) \
            void emit_ ## rv_insn (const Register& dst, const Register& src, int imm) { \
                emit_i_type(#rv_insn, dst, src, imm); \
            } \
            void symb_emit_ ## rv_insn (const SymbolicLocation& dst, const SymbolicLocation& src, int imm) { \
                symb_emit_i_type(#rv_insn, dst, src, imm); \
            }

        // insn reg_dest, base_reg(immediate)
        #define S_TYPE(rv_insn) \
            void emit_ ## rv_insn (const Register& dst, const Register& base, int off) { \
                emit_s_type(#rv_insn, dst, base, off); \
            } \
            void symb_emit_ ## rv_insn (const SymbolicLocation& dst, const SymbolicLocation& base, int off) { \
                symb_emit_s_type(#rv_insn, dst, base, off); \
            }

        // insn reg_dest, immediate
        #define U_TYPE(rv_insn) \
            void emit_ ## rv_insn (const Register& dst, int imm) { \
                emit_u_type(#rv_insn, dst, imm); \
            } \
            void symb_emit_ ## rv_insn (const SymbolicLocation& dst, int imm) { \
                symb_emit_u_type(#rv_insn, dst, imm); \
            }

        #define PSEUDO_TYPE(rv_insn) \
            void emit_ ## rv_insn (const Register& dst, const Register& src) { \
                emit_pseudo_type(#rv_insn, dst, src); \
            } \
            void symb_emit_ ## rv_insn (const SymbolicLocation& dst, const SymbolicLocation& src) { \
                symb_emit_pseudo_type(#rv_insn, dst, src); \
            }


        R_TYPE(add);
        R_TYPE(sub);
        R_TYPE(slt);
        R_TYPE(sgt);

        void emit_eq(const Register &dst, const Register &src1, const Register &src2) {
            emit_sub(dst, src2, src1);
            emit_seqz(dst, dst);
        }

        void symb_emit_eq(const SymbolicLocation &dst, const SymbolicLocation &src1,
                        const SymbolicLocation &src2) {
            symb_emit_sub(dst, src2, src1);
            symb_emit_seqz(dst, dst);
        }

        void emit_sle(const Register &dst, const Register &src1, const Register &src2) {
            emit_sgt(dst, src1, src2);
            emit_xori(dst, dst, 1);
        }

        void symb_emit_sle(const SymbolicLocation &dst, const SymbolicLocation &src1,
                        const SymbolicLocation &src2) {
            symb_emit_sgt(dst, src1, src2);
            symb_emit_xori(dst, dst, 1);
        }

        // sltu rd, x0, rs
        void emit_neq(const Register &dst, const Register &src1, const Register &src2) {
            emit_sub(dst, src2, src1);
            emit_snez(dst, dst);
        }

        void symb_emit_neq(const SymbolicLocation &dst, const SymbolicLocation &src1,
                        const SymbolicLocation &src2) {
            symb_emit_sub(dst, src2, src1);
            symb_emit_snez(dst, dst);
        }

        void emit_sge(const Register &dst, const Register &src1, const Register &src2) {
            emit_slt(dst, src1, src2);
            emit_xori(dst, dst, 1);
        }

        void symb_emit_sge(const SymbolicLocation &dst, const SymbolicLocation &src1,
                        const SymbolicLocation &src2) {
            symb_emit_slt(dst, src1, src2);
            symb_emit_xori(dst, dst, 1);
        }


        R_TYPE(or);
        R_TYPE(and);
        R_TYPE(srl); // logical
        R_TYPE(sra); // arithmetical
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

        void emit_mv(const Register& dst, const Register& src) {
            emit_addi(dst, src, 0);
        }

        void symb_emit_mv(const Register& dst_reg, const SymbolicLocation& src) {
            auto src_reg = to_reg(src, rv::Register::temp2());
            emit(std::format("mv\t{},\t{}", dst_reg, src_reg));
        }

        void symb_emit_mv(const SymbolicLocation& dst, const Register& src_reg) {
            auto dst_reg = to_reg(dst, rv::Register::temp2());
            emit(std::format("mv\t{},\t{}", dst_reg, src_reg));
        }

        void emit_call(std::string label) {
            emit(std::format("call\t{}", label));
        }

        void emit_ret() {
            emit("ret");
        }

        void emit_label(const std::string& label) {
            emit(std::format("{}:", label));
        }

        void emit_comment([[maybe_unused]] const std::string& comment) {
#if DEBUG_COMMENTS
            emit(std::format("# {}", comment));
#endif
        }

        void emit_j(std::string_view target_label) {
            emit(std::format("j {}", target_label));
        }

        void emit_cj(bool on_eq, Register const& r1, Register const& r2, std::string_view target_label){
            emit(
                std::format(
                    "{:s}\t{},\t{},\t{}",
                    on_eq?"beq":"bne",
                    r1,
                    r2,
                    target_label
                )
            );
        }

        inline Register to_reg(const SymbolicLocation& loc, const Register& temp) {
            return loc.type == SymbolicStack::LocType::Memory
                ? emit_ld(temp, rv::Register::sp(), -loc.number * WORD_SIZE), temp
                : Register{loc.number};
        }

        void emit(std::string str) {
            _code.emplace_back(str);
        }

        private:

        void emit_pseudo_type(const std::string& insn, const Register& dst, const Register& src) {
            emit(std::format("{}\t{},\t{}", insn, dst, src));
        }

        void emit_r_type(const std::string& insn, const Register& dst, const Register& src1, const Register& src2) {
            emit(std::format("{}\t{},\t{},\t{}", insn, dst, src1, src2));
        }

        void emit_i_type(const std::string& insn, const Register& dst, const Register& src, int imm) {
            emit(std::format("{}\t{},\t{},\t{}", insn, dst, src, imm));
        }

        void emit_u_type(const std::string& insn, const Register& dst, int imm) {
            emit(std::format("{}\t{},\t{}", insn, dst, imm));
        }

        void emit_s_type(const std::string& insn, const Register& dst, const Register& base, int off) {
            emit(std::format("{}\t{},\t{}({})", insn, dst, off, base));
        }

        void symb_emit_r_type(const std::string& insn, const SymbolicLocation& dst, const SymbolicLocation& src1, const SymbolicLocation& src2) {
            auto src1_reg = to_reg(src1, rv::Register::temp1());
            auto src2_reg = to_reg(src2, rv::Register::temp2());

            switch (dst.type) {
            case SymbolicStack::LocType::Register:
                emit_r_type(insn, Register{dst.number}, src1_reg, src2_reg);
                return;
            case SymbolicStack::LocType::Memory:
                emit_r_type(insn, src1_reg, src1_reg, src2_reg);
                emit_sd(src1_reg, rv::Register::sp(), -dst.number * WORD_SIZE);
                return;
            }
        }

        void symb_emit_i_type(const std::string& insn, const SymbolicLocation& dst, const SymbolicLocation& src, int imm) {
            auto dst_reg = to_reg(dst, rv::Register::temp1());
            auto src_reg = to_reg(src, rv::Register::temp2());
            emit_i_type(insn, dst_reg, src_reg, imm);
        }

        void symb_emit_u_type(const std::string& insn, const SymbolicLocation& dst, int imm) {
            auto dst_reg = to_reg(dst, rv::Register::temp1());
            emit_u_type(insn, dst_reg, imm);
        }

        void symb_emit_s_type(const std::string& insn, const SymbolicLocation& dst, const SymbolicLocation& base, int off) {
            auto dst_reg = to_reg(dst, rv::Register::temp1());
            auto base_reg = to_reg(base, rv::Register::temp2());
            emit_s_type(insn, dst_reg, base_reg, off);
        }

        void symb_emit_pseudo_type(const std::string& insn, const SymbolicLocation& dst, const SymbolicLocation& src) {
            auto dst_reg = to_reg(dst, rv::Register::temp1());
            auto src_reg = to_reg(src, rv::Register::temp2());
            emit_pseudo_type(insn, dst_reg, src_reg);
        }
    };

}
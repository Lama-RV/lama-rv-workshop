#include <glog/logging.h>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include "bytefile.h"
#include "inst_reader.h"
#include "instruction.h"

std::string emit(std::map<size_t, std::unique_ptr<lama::Instruction>> const& instructions, size_t n_globals) {
    lama::rv::Compiler c{n_globals};
    CHECK(!instructions.empty());
    c.add_jump_target(instructions.begin()->first, 0);
    while (true) {
        auto todo_it = c.todo.begin();
        if (todo_it == c.todo.end()) {
            break;
        }
        size_t const start_offset = todo_it->first;
        c.st.top = todo_it->second;
        c.todo.erase(todo_it);
        if (!c.should_emit(start_offset)) {
            continue;
        }
        for (auto it = instructions.find(start_offset); it != instructions.end(); ++it) {
            auto const& [offset, inst] = *it;
            c.inst_begin(offset);
            {
                std::ostringstream disasm;
                disasm << "-> " << *inst;
                c.cb.emit_comment(disasm.view());
            }
            c.debug_stack_height();
            inst->emit_code(&c);
            c.debug_stack_height();
            if (inst->is_terminator()) {
                c.cb.emit_comment("============");
                break;
            }
        }
    }
    return c.dump_asm();
}

int main(int argc, char const* argv[]) {
    FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);

    CHECK_EQ(argc, 2);
    bytefile* file = read_file(argv[1]);
    lama::InstReader reader{file};
    std::map<size_t, std::unique_ptr<lama::Instruction>> instructions;
    while (true) {
        auto const offset = reader.get_offset();
        auto inst = reader.read_inst();
        if (!inst) {
            break;
        }
        auto [_pos, inserted] = instructions.emplace(offset, std::move(inst));
        DCHECK(inserted) << std::format("{:#x}", offset);
    }
    std::cout << emit(instructions, file->global_area_size) << std::endl;
    close_file(file);
}

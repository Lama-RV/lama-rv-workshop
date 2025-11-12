#include <glog/logging.h>
#include <iostream>
#include "bytefile.h"
#include "inst_reader.h"
#include "instruction.h"

int main(int argc, char const* argv[]) {
    FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);

    CHECK_EQ(argc, 2);
    bytefile* file = read_file(argv[1]);
    lama::InstReader reader{file};

    lama::rv::Compiler c{(size_t)file->global_area_size};

    while (true) {
        auto const offset = reader.get_offset();
        c.cb.set_ip(offset);
        auto const stack_height =c.st.top;
        c.stack_hieghts.actual[offset] = stack_height;
        c.cb.emit_comment(std::format("Stack height = {:d}", stack_height));
        auto const inst = reader.read_inst();
        if (!inst) {
            break;
        }
        inst->emit_code(&c);
    }

    std::cout << c.dump_asm() << std::endl;

    close_file(file);

    for (auto const& [offset, expected] : c.stack_hieghts.expected) {
        DCHECK_EQ(expected, c.stack_hieghts.actual.at(offset)) << std::format("Offset = {:#x}", offset);
    }
}

#include "bytefile.h"
#include "inst_reader.h"
#include "instruction.h"
#include <iostream>

int main(int argc, const char* argv[]) {
    bytefile *file = read_file(argv[1]);
    lama::InstReader reader{file};

    lama::rv::Compiler c{(size_t)file->global_area_size};

    for (auto inst = reader.read_inst(); inst; inst = reader.read_inst()) {
        inst->emit_code(&c);
    }
    
    std::cout << c.dump_asm();

    close_file(file);
}
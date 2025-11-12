#include <glog/logging.h>
#include <format>
#include <iostream>
#include <ostream>
#include "bytefile.h"
#include "inst_reader.h"
#include "instruction.h"

void dump(bytefile* bf, std::ostream& out) {
    out << std::format(
             "String table size       : {:d}\n"
             "Global area size        : {:d}\n"
             "Number of public symbols: {:d}\n"
             "Public symbols          :",
             bf->stringtab_size, bf->global_area_size, bf->public_symbols_number
         )
      << std::endl;

    for (size_t i = 0; i < bf->public_symbols_number; i++) {
        out << std::format("   {:#010x}: {:s}", get_public_offset(bf, i), get_public_name(bf, i)) << std::endl;
    }

    out << "Code:" << std::endl;

    lama::InstReader reader{bf};
    while (true) {
        auto const offset = reader.get_offset();
        auto const inst = reader.read_inst();
        if (!inst) {
            std::cout << std::format("{:#010x}:\t<end>", offset) << std::endl;
            break;
        }
        std::cout << std::format("{:#010x}:\t", offset) << *inst << std::endl;
    }
}

int main(int argc, char const* argv[]) {
    FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);
    CHECK_EQ(argc, 2);
    bytefile* file = read_file(argv[1]);
    dump(file, std::cout);
    close_file(file);
}

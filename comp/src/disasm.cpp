#include <glog/logging.h>
#include <cstdio>
#include <cstring>
#include <format>
#include <iostream>
#include "bytefile.h"

void disassemble(std::ostream& f, bytefile* bf) {
    char* ip = bf->code_ptr;
    char const* ops[] = {"+", "-", "*", "/", "%", "<", "<=", ">", ">=", "==", "!=", "&&", "!!"};
    char const* pats[] = {"=str", "#string", "#array", "#sexp", "#ref", "#val", "#fun"};
    char const* lds[] = {"LD", "LDA", "ST"};

    auto INT = [&ip]() -> int {
        int result;
        std::memcpy(&result, ip, sizeof(int));
        ip += sizeof(int);
        return result;
    };
    auto BYTE = [&ip]() -> char { return *(ip++); };
    auto STRING = [bf, &INT] { return get_string(bf, INT()); };

    while (true) {
        char const x = BYTE();
        char const h = (x & 0xF0) >> 4;
        char const l = x & 0x0F;

        auto FAIL = [h, l] { std::cerr << std::format("ERROR: invalid opcode {:d}-{:d}", h, l) << std::endl; };

        f << std::format("{:#010x}:\t", ip - bf->code_ptr - 1);

        switch (h) {
        case 15:
            goto stop;

        /* BINOP */
        case 0:
            f << std::format("BINOP\t{:s}", ops[l - 1]);
            break;

        case 1:
            switch (l) {
            case 0:
                f << std::format("CONST\t{:d}", INT());
                break;

            case 1:
                f << std::format("STRING\t{:s}", STRING());
                break;

            case 2:
                f << std::format("SEXP\t{:s} ", STRING());
                f << std::format("{:d}", INT());
                break;

            case 3:
                f << "STI";
                break;

            case 4:
                f << "STA";
                break;

            case 5:
                f << std::format("JMP\t{:#010x}", INT());
                break;

            case 6:
                f << "END";
                break;

            case 7:
                f << "RET";
                break;

            case 8:
                f << "DROP";
                break;

            case 9:
                f << "DUP";
                break;

            case 10:
                f << "SWAP";
                break;

            case 11:
                f << "ELEM";
                break;

            default:
                FAIL();
            }
            break;

        case 2:
        case 3:
        case 4:
            f << std::format("{:s}\t", lds[h - 2]);
            switch (l) {
            case 0:
                f << std::format("G({:d})", INT());
                break;
            case 1:
                f << std::format("L({:d})", INT());
                break;
            case 2:
                f << std::format("A({:d})", INT());
                break;
            case 3:
                f << std::format("C({:d})", INT());
                break;
            default:
                FAIL();
            }
            break;

        case 5:
            switch (l) {
            case 0:
                f << std::format("CJMPz\t{:#010x}", INT());
                break;

            case 1:
                f << std::format("CJMPnz\t{:#010x}", INT());
                break;

            case 2: {
                {
                    auto const offset = (ip - bf->code_ptr) - 1;
                    f << std::format("BEGIN\t{:d} ", INT());
                    f << std::format("{:d}", INT());
                    for (size_t i = 0; i < bf->public_symbols_number; ++i) {
                        if (get_public_offset(bf, i) == offset) {
                            f << "\t# " << get_public_name(bf, i);
                        }
                    }
                }
            } break;

            case 3:
                f << std::format("CBEGIN\t{:d} ", INT());
                f << std::format("{:d}", INT());
                break;

            case 4:
                f << std::format("CLOSURE\t{:#010x}", INT());
                {
                    int n = INT();
                    for (int i = 0; i < n; i++) {
                        switch (BYTE()) {
                        case 0:
                            f << std::format("G({:d})", INT());
                            break;
                        case 1:
                            f << std::format("L({:d})", INT());
                            break;
                        case 2:
                            f << std::format("A({:d})", INT());
                            break;
                        case 3:
                            f << std::format("C({:d})", INT());
                            break;
                        default:
                            FAIL();
                        }
                    }
                };
                break;

            case 5:
                f << std::format("CALLC\t{:d}", INT());
                break;

            case 6:
                f << std::format("CALL\t{:#010x} ", INT());
                f << std::format("{:d}", INT());
                break;

            case 7:
                f << std::format("TAG\t{:s} ", STRING());
                f << std::format("{:d}", INT());
                break;

            case 8:
                f << std::format("ARRAY\t{:d}", INT());
                break;

            case 9:
                f << std::format("FAIL\t{:d}", INT());
                f << std::format("{:d}", INT());
                break;

            case 10:
                f << std::format("LINE\t{:d}", INT());
                break;

            default:
                FAIL();
            }
            break;

        case 6:
            f << std::format("PATT\t{:s}", pats[l]);
            break;

        case 7: {
            switch (l) {
            case 0:
                f << "CALL\tLread";
                break;

            case 1:
                f << "CALL\tLwrite";
                break;

            case 2:
                f << "CALL\tLlength";
                break;

            case 3:
                f << "CALL\tLstring";
                break;

            case 4:
                f << std::format("CALL\tBarray\t{:d}", INT());
                break;

            default:
                FAIL();
            }
        } break;

        default:
            FAIL();
        }

        f << std::endl;
    }
stop:
    f << "<end>" << std::endl;
}

/* Dumps the contents of the file */
void dump_file(std::ostream& f, bytefile* bf) {
    int i;

    f << std::format(
             "String table size       : {:d}\n"
             "Global area size        : {:d}\n"
             "Number of public symbols: {:d}\n"
             "Public symbols          :",
             bf->stringtab_size, bf->global_area_size, bf->public_symbols_number
         )
      << std::endl;

    for (i = 0; i < bf->public_symbols_number; i++) {
        f << std::format("   {:#010x}: {:s}", get_public_offset(bf, i), get_public_name(bf, i)) << std::endl;
    }

    f << "Code:" << std::endl;
    disassemble(f, bf);
}

int main(int argc, char* argv[]) {
    FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);
    CHECK_EQ(argc, 2) << "Please, provide input file name";
    bytefile* f = read_file(argv[1]);
    dump_file(std::cout, f);
    close_file(f);
    return 0;
}

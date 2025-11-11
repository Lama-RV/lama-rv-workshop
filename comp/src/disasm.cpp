#include <fmt/format.h>
#include <glog/logging.h>
#include <cstdio>
#include <cstring>
#include "bytefile.h"

void disassemble(FILE* f, bytefile* bf) {
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

        auto FAIL = [h, l] { return fmt::println(stderr, "ERROR: invalid opcode {:d}-{:d}", h, l); };

        fmt::print(f, "{:#010x}:\t", ip - bf->code_ptr - 1);

        switch (h) {
        case 15:
            goto stop;

        /* BINOP */
        case 0:
            fmt::print(f, "BINOP\t{:s}", ops[l - 1]);
            break;

        case 1:
            switch (l) {
            case 0:
                fmt::print(f, "CONST\t{:d}", INT());
                break;

            case 1:
                fmt::print(f, "STRING\t{:s}", STRING());
                break;

            case 2:
                fmt::print(f, "SEXP\t{:s} ", STRING());
                fmt::print(f, "{:d}", INT());
                break;

            case 3:
                fmt::print(f, "STI");
                break;

            case 4:
                fmt::print(f, "STA");
                break;

            case 5:
                fmt::print(f, "JMP\t{:#010x}", INT());
                break;

            case 6:
                fmt::print(f, "END");
                break;

            case 7:
                fmt::print(f, "RET");
                break;

            case 8:
                fmt::print(f, "DROP");
                break;

            case 9:
                fmt::print(f, "DUP");
                break;

            case 10:
                fmt::print(f, "SWAP");
                break;

            case 11:
                fmt::print(f, "ELEM");
                break;

            default:
                FAIL();
            }
            break;

        case 2:
        case 3:
        case 4:
            fmt::print(f, "{:s}\t", lds[h - 2]);
            switch (l) {
            case 0:
                fmt::print(f, "G({:d})", INT());
                break;
            case 1:
                fmt::print(f, "L({:d})", INT());
                break;
            case 2:
                fmt::print(f, "A({:d})", INT());
                break;
            case 3:
                fmt::print(f, "C({:d})", INT());
                break;
            default:
                FAIL();
            }
            break;

        case 5:
            switch (l) {
            case 0:
                fmt::print(f, "CJMPz\t{:#010x}", INT());
                break;

            case 1:
                fmt::print(f, "CJMPnz\t{:#010x}", INT());
                break;

            case 2:
                fmt::print(f, "BEGIN\t{:d} ", INT());
                fmt::print(f, "{:d}", INT());
                break;

            case 3:
                fmt::print(f, "CBEGIN\t{:d} ", INT());
                fmt::print(f, "{:d}", INT());
                break;

            case 4:
                fmt::print(f, "CLOSURE\t{:#010x}", INT());
                {
                    int n = INT();
                    for (int i = 0; i < n; i++) {
                        switch (BYTE()) {
                        case 0:
                            fmt::print(f, "G({:d})", INT());
                            break;
                        case 1:
                            fmt::print(f, "L({:d})", INT());
                            break;
                        case 2:
                            fmt::print(f, "A({:d})", INT());
                            break;
                        case 3:
                            fmt::print(f, "C({:d})", INT());
                            break;
                        default:
                            FAIL();
                        }
                    }
                };
                break;

            case 5:
                fmt::print(f, "CALLC\t{:d}", INT());
                break;

            case 6:
                fmt::print(f, "CALL\t{:#010x} ", INT());
                fmt::print(f, "{:d}", INT());
                break;

            case 7:
                fmt::print(f, "TAG\t{:s} ", STRING());
                fmt::print(f, "{:d}", INT());
                break;

            case 8:
                fmt::print(f, "ARRAY\t{:d}", INT());
                break;

            case 9:
                fmt::print(f, "FAIL\t{:d}", INT());
                fmt::print(f, "{:d}", INT());
                break;

            case 10:
                fmt::print(f, "LINE\t{:d}", INT());
                break;

            default:
                FAIL();
            }
            break;

        case 6:
            fmt::print(f, "PATT\t{:s}", pats[l]);
            break;

        case 7: {
            switch (l) {
            case 0:
                fmt::print(f, "CALL\tLread");
                break;

            case 1:
                fmt::print(f, "CALL\tLwrite");
                break;

            case 2:
                fmt::print(f, "CALL\tLlength");
                break;

            case 3:
                fmt::print(f, "CALL\tLstring");
                break;

            case 4:
                fmt::print(f, "CALL\tBarray\t{:d}", INT());
                break;

            default:
                FAIL();
            }
        } break;

        default:
            FAIL();
        }

        fmt::println(f, "");
    }
stop:
    fmt::print(f, "<end>\n");
}

/* Dumps the contents of the file */
void dump_file(FILE* f, bytefile* bf) {
    int i;

    fmt::print(f, "String table size       : {:d}\n", bf->stringtab_size);
    fmt::print(f, "Global area size        : {:d}\n", bf->global_area_size);
    fmt::print(f, "Number of public symbols: {:d}\n", bf->public_symbols_number);
    fmt::print(f, "Public symbols          :\n");

    for (i = 0; i < bf->public_symbols_number; i++)
        fmt::print(f, "   {:#010x}: {:s}\n", get_public_offset(bf, i), get_public_name(bf, i));

    fmt::print(f, "Code:\n");
    disassemble(f, bf);
}

int main(int argc, char* argv[]) {
    FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);
    CHECK_EQ(argc, 2) << "Please, provide input file name";
    bytefile* f = read_file(argv[1]);
    dump_file(stdout, f);
    close_file(f);
    return 0;
}

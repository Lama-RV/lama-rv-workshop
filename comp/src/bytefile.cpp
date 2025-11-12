#include <glog/logging.h>
#include <cerrno>
#include <cstddef>
#include <cstring>

#include "bytefile.h"

bytefile* read_file(char const* fname) {
    auto f = fopen(fname, "rb");

    PCHECK(f != nullptr);

    PCHECK(fseek(f, 0, SEEK_END) != -1);

    auto const size = ftell(f);
    size_t const total_size = offsetof(bytefile, stringtab_size) + size;
    auto file = (bytefile*)malloc(total_size);
    CHECK_NOTNULL(file)->size = total_size;

    rewind(f);
    if (errno) {
        LOG(FATAL) << "Error rewinding " << fname;
    }

    PCHECK(fread(&file->stringtab_size, 1, size, f) == size);

    fclose(f);

    CHECK_GE(file->stringtab_size, 0) << "Negative string section size";
    CHECK_GE(file->public_symbols_number, 0) << "Negative public symbols number";
    CHECK_GE(file->global_area_size, 0) << "Negative global area size";

    int total_sections_size =
        file->stringtab_size + file->public_symbols_number * 2 * sizeof(int) + file->global_area_size * sizeof(int);

    CHECK_LE(total_sections_size + sizeof(bytefile), file->size) << "Invalid sections layout";

    file->string_ptr = &file->buffer[file->public_symbols_number * 2 * sizeof(int)];
    file->public_ptr = (int*)file->buffer;
    file->code_ptr = &file->string_ptr[file->stringtab_size];
    file->global_ptr = (int*)malloc(file->global_area_size * sizeof(int));

    return file;
}

char const* get_string(bytefile const* f, int pos) {
    CHECK_GE(pos, 0) << "Negative string index";
    CHECK_LT(pos, f->stringtab_size) << "Index out of bounds";
    return &f->string_ptr[pos];
}

char const* get_public_name(bytefile const* f, int i) {
    CHECK_GE(i, 0) << "Negative public symbol index";
    CHECK_LT(i, f->public_symbols_number) << "Index out of bounds";
    return get_string(f, f->public_ptr[i * 2]);
}

int get_public_offset(bytefile const* f, int i) {
    CHECK_GE(i, 0) << "Negative public symbol index";
    CHECK_LT(i, f->public_symbols_number) << "Index out of bounds";
    int offset = f->public_ptr[i * 2 + 1];
    CHECK_LE(offset, f->size) << "Invalid public offset";
    return offset;
}

int get_code_size(bytefile const* f) {
    return f->size - (f->code_ptr - (char const*)f);
}

void close_file(bytefile* f) {
    free(f->global_ptr);
    free(f);
}

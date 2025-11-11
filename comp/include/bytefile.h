#pragma once

/* The unpacked representation of bytecode file */
struct bytefile {
  int size;
  char *string_ptr;              /* A pointer to the beginning of the string table */
  int  *public_ptr;              /* A pointer to the beginning of publics table    */
  char *code_ptr;                /* A pointer to the bytecode itself               */
  int  *global_ptr;              /* A pointer to the global area                   */
  int   stringtab_size;          /* The size (in bytes) of the string table        */
  int   global_area_size;        /* The size (in words) of global area             */
  int   public_symbols_number;   /* The number of public symbols                   */
  char  buffer[0];
};

/* Gets a string from a string table by an index */
const char* get_string (const bytefile *f, int pos);

/* Gets a name for a public symbol */
const char* get_public_name (const bytefile *f, int i);

/* Reads a binary bytecode file by name and unpacks it */
bytefile *read_file (const char *fname);

/* Gets an offset for a public symbol */
int get_public_offset(const bytefile *f, int i);

void close_file(bytefile *f);

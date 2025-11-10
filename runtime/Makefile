PREFIX := riscv64-unknown-linux-gnu
CC := $(PREFIX)-gcc
AR := $(PREFIX)-ar

DISABLE_WARNINGS=-Wno-shift-negative-value
COMMON_FLAGS=$(DISABLE_WARNINGS) -g $(ARCH) --std=c11
PROD_FLAGS=$(COMMON_FLAGS) -DLAMA_ENV
TEST_FLAGS=$(COMMON_FLAGS) -DDEBUG_VERSION
UNIT_TESTS_FLAGS=$(TEST_FLAGS)
INVARIANTS_CHECK_FLAGS=$(TEST_FLAGS) -DFULL_INVARIANT_CHECKS

all build: gc.o runtime.o
	$(AR) rc runtime.a runtime.o gc.o

gc.o: gc.c gc.h
	$(CC) $(PROD_FLAGS) -c gc.c -o gc.o

runtime.o: runtime.c runtime.h
	$(CC) -O2 $(PROD_FLAGS) -c runtime.c -o runtime.o

clean:
	$(RM) *.a *.o *~ negative_scenarios/*.err


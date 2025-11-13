TESTS=$(sort $(basename $(wildcard *.lama)))

LAMAC=lamac
LAMA_RV_BACKEND?=../comp/build/lama-rv
RV_TRIPLET=riscv64-unknown-linux-gnu
SIM=qemu-riscv64 -L /usr/$(RV_TRIPLET) -plugin ./libinsn.so,inline=on -d plugin
RV_AS=$(RV_TRIPLET)-as
RV_GCC=$(RV_TRIPLET)-gcc

.PHONY: check $(TESTS)

check: $(TESTS)

$(TESTS): %: %.lama ../runtime/runtime.a
	@echo "Running benchmark $@"
	@$(LAMAC) -b $< # produce bytecode file
	@$(LAMA_RV_BACKEND) $@.bc > $@.S # produce asm backend output
	@$(RV_AS) $@.S -o $@.o # produce object file
	@$(RV_GCC) $@.o ../runtime/runtime.a -o $@.elf # linux object file with runtime & C standart library
	@$(SIM) $@.elf > /dev/null # run elf in functional simulator of RISC-V hart

../runtime/runtime.a:
	+make -C ../runtime
clean:
	rm -rf *.o *.S *.i *.bc *.elf

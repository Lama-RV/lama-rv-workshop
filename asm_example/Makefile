TESTS=$(sort $(basename $(wildcard *.S)))

LAMAC=lamac
RV_TRIPLET=riscv64-unknown-linux-gnu
SIM=qemu-riscv64 -L /usr/$(RV_TRIPLET)
RV_AS=$(RV_TRIPLET)-as
RV_GCC=$(RV_TRIPLET)-gcc

$(TESTS): %: %.S ../runtime/runtime.a
	@echo "Running test $@"
	$(RV_AS) $@.S -o $@.o # produce object file
	$(RV_GCC) $@.o ../runtime/runtime.a -o $@.elf # linux object file with runtime & C standart library
	$(SIM) $@.elf # run elf in functional simulator of RISC-V hart

../runtime/runtime.a:
	+make -C ../runtime
clean:
	rm -rf *.bc *.elf

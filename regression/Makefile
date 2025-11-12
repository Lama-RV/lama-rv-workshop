TESTS=$(sort $(basename $(wildcard test*.lama)))

LAMA_RV_BACKEND?=../comp/build/lama-rv
LAMAC=lamac
RV_TRIPLET=riscv64-unknown-linux-gnu
SIM=qemu-riscv64 -L /usr/$(RV_TRIPLET)
RV_AS=$(RV_TRIPLET)-as
RV_GCC=$(RV_TRIPLET)-gcc

check: $(TESTS)

$(TESTS): %: %.lama
	$(if $(value LAMA_RV_BACKEND),,$(error LAMA_RV_BACKEND is undefined))
	@echo "Running test $@"
	$(LAMAC) -b $< # produce bytecode file
	$(LAMA_RV_BACKEND) $@.bc > $@.S # produce asm backend output
	$(RV_AS) $@.S -o $@.o # produce object file
	$(RV_GCC) $@.o ../runtime/runtime.a -o $@.elf # linux object file with runtime & C standart library
	$(SIM) $@.elf < $@.input > $@.output # run elf in functional simulator of RISC-V hart
	diff --suppress-common-lines -y $@.ref $@.output # compare output with reference (testxxx.ref file)

clean:
	rm -rf *.bc *.elf *.S *.o *.output

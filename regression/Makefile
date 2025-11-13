TESTS=$(sort $(basename $(wildcard test*.lama)))

LAMA_RV_BACKEND?=../comp/build/lama-rv
DISASM?=../comp/build/disasm
BCDUMP?=../comp/build/bcdump
LAMAC=lamac
RV_TRIPLET=riscv64-unknown-linux-gnu
SIM=qemu-riscv64 -L /usr/$(RV_TRIPLET)
RV_AS=$(RV_TRIPLET)-as
RV_GCC=$(RV_TRIPLET)-gcc

check: $(TESTS)

$(TESTS): %: %.lama
	$(if $(value LAMA_RV_BACKEND),,$(error LAMA_RV_BACKEND is undefined))
	# Running test $@
	$(LAMAC) -b $<
	$(DISASM) $@.bc > $@-disasm.output
	$(BCDUMP) $@.bc > $@-bcdump.output
	diff --suppress-common-lines -y $@-disasm.output $@-bcdump.output
	$(LAMA_RV_BACKEND) $@.bc > $@.S
	$(RV_AS) $@.S -o $@.o
	$(RV_GCC) $@.o ../runtime/runtime.a -o $@.elf
	$(SIM) $@.elf < $@.input > $@.output
	diff --suppress-common-lines -y $@.ref $@.output

clean:
	rm -rf *.bc *.elf *.S *.o *.output

TESTS=$(sort $(basename $(wildcard *.lama)))

LAMAC=lamac

.PHONY: check $(TESTS)

check: $(TESTS)

$(TESTS): %: %.lama
	@echo $@
	@$(LAMAC) -b $@.lama && `which time` -f "$@ iter\t%U" $(BYTERUN) $@.bc
	@cat Sort.input | LAMA=../runtime `which time` -f "$@ (-i) rec \t%U" $(LAMAC) -i $<
	@cat Sort.input | LAMA=../runtime `which time` -f "$@ (-s) rec \t%U" $(LAMAC) -s $<

clean:
	$(RM) test*.log *.s *~ $(TESTS) *.i *.bc

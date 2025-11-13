LAMA_RV_BUILD_DIR=$(PWD)/comp/build
LAMA_RV=$(LAMA_RV_BUILD_DIR)/lama-rv

build: runtime-rv lama-rv

$(LAMA_RV_BUILD_DIR):
	mkdir -p $(LAMA_RV_BUILD_DIR)
	cmake -DCMAKE_BUILD_TYPE=Debug -S comp -B $(LAMA_RV_BUILD_DIR)

lama-rv: | $(LAMA_RV_BUILD_DIR)
	cmake --build $(LAMA_RV_BUILD_DIR) --parallel --target lama-rv

bcdump: | $(LAMA_RV_BUILD_DIR)
	cmake --build $(LAMA_RV_BUILD_DIR) --target bcdump

disasm: | $(LAMA_RV_BUILD_DIR)
	cmake --build $(LAMA_RV_BUILD_DIR) --parallel --target disasm

runtime-rv:
	$(MAKE) -C runtime build

regression: build
	$(MAKE) -C regression $(if $(value TEST),test$(TEST),check) LAMA_RV_BACKEND=$(LAMA_RV)

clean:
	rm -rf $(LAMA_RV_BUILD_DIR)
	$(MAKE) clean -C runtime
	$(MAKE) clean -C regression
	$(MAKE) clean -C performance

.PHONY: all $(LAMA_RV_BUILD_DIR) build runtime-rv regression clean lama-rv bcdump disasm

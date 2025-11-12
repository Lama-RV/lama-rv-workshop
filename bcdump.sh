#!/usr/bin/env bash

set -euxo pipefail

cd "$(dirname "$0")"

COMP_SRC=comp/
COMP_DST=$COMP_SRC/build/

cmake -S ${COMP_SRC} -B ${COMP_DST}
cmake --build ${COMP_DST} --parallel -- bcdump disasm

for file in regression/*.bc; do
	delta <($COMP_DST/disasm "$file") <($COMP_DST/bcdump "$file")
done

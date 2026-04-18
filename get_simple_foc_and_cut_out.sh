#!/bin/bash
set -euo pipefail

wget -O Arduino-FOC-2.4.0.zip https://codeload.github.com/simplefoc/Arduino-FOC/zip/refs/tags/v2.4.0
unzip Arduino-FOC-2.4.0.zip
rm Arduino-FOC-2.4.0.zip

pushd Arduino-FOC-2.4.0 > /dev/null
rm -rf \
    .github examples CITATION.cff CNAME CODE_OF_CONDUCT.md Doxyfile keywords.txt \
    library.json library.properties

pushd src > /dev/null

pushd current_sense > /dev/null
pushd hardware_specific > /dev/null
ls | grep -Ev '(generic_mcu.cpp)' | xargs rm -rf
popd > /dev/null
popd > /dev/null

pushd drivers > /dev/null
pushd hardware_specific > /dev/null
ls | grep -Ev '(generic_mcu.cpp)' | xargs rm -rf
popd > /dev/null
popd > /dev/null

popd > /dev/null

popd > /dev/null

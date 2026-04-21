#!/bin/bash
set -euo pipefail

wget -O Arduino-FOC-2.4.0.zip https://codeload.github.com/simplefoc/Arduino-FOC/zip/refs/tags/v2.4.0
unzip Arduino-FOC-2.4.0.zip
rm Arduino-FOC-2.4.0.zip

pushd Arduino-FOC-2.4.0
rm -rf \
    .github examples CITATION.cff CNAME CODE_OF_CONDUCT.md Doxyfile keywords.txt \
    library.json library.properties

pushd src

pushd current_sense
pushd hardware_specific
ls | grep -Ev '(generic_mcu.cpp)' | xargs rm -rf
popd
popd

pushd drivers
pushd hardware_specific
ls | grep -Ev '(generic_mcu.cpp)' | xargs rm -rf
popd
popd

popd

popd

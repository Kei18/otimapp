#!/bin/bash
set -e
bash `dirname $0`/../../third_party/openFrameworks/scripts/osx/download_libs.sh
cd ./visualizer
make build
cd ..
chmod +x ./visualize.sh

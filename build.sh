#!/bin/sh

cd Lib
python3 upd_linux_libs.py
cd ..

cmake CMakeLists.txt
make -j 4

echo "Build completed"

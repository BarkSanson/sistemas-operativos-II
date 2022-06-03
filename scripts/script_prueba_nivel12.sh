cd ..
make clean
make
cd build
./mi_mkfs disco 100000
time ./simulacion disco
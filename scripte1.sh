make clean
make
echo "$ rm disco"
rm disco
echo "$ ./mi_mkfs disco 100000"
build/mi_mkfs disco 100000
echo "################################################################################"
echo "$ ./leer_sf disco"
build/leer_sf disco
echo "################################################################################"
echo "$./escribir"
build/escribir
echo "################################################################################"
echo "$ ./escribir disco "$(cat texto2.txt)" 0"
echo "#escribimos el texto2.txt en los offsets  9000, 209000, 30725000, "
echo "#409605000 y 480000000 de un mismo inodo"
build/escribir disco "$(cat texto2.txt)" 0
echo "################################################################################"
echo "$ ./permitir disco 1 0"
echo "#cambiamos permisos del inodo 1 a 0"
#build/permitir disco 1 0
echo "################################################################################"
echo "$ ./leer disco 1"
echo "#intentamos leer inodo 1 con permisos=0"
build/leer disco 1
echo "################################################################################"
echo "$ ./permitir disco 1 6"
echo "#cambiamos permisos del inodo 1 a 6"
build/permitir disco 1 6
echo "################################################################################"
echo "$ ./leer disco 1 > ext1.txt"
build/leer disco 1 > ext1.txt
echo "################################################################################"
echo "$ ls -l ext1.txt"
ls -l ext1.txt
echo "################################################################################"
echo "$ ./leer_sf disco"
build/leer_sf disco
echo "################################################################################"
echo "$ ./truncar disco 1 409605001"
build/truncar disco 1 409605001
echo "################################################################################"
echo "$ ./leer_sf disco"
build/leer_sf disco
echo "################################################################################"
echo "$ ./truncar disco 1 30725003"
build/truncar disco 1 30725003
echo "################################################################################"
echo "$ ./leer_sf disco"
build/leer_sf disco
echo "################################################################################"
echo "$ ./truncar disco 1 209008"
build/truncar disco 1 209008
echo "################################################################################"
echo "$ ./leer_sf disco"
build/leer_sf disco
echo "################################################################################"
echo "$ ./truncar disco 1 9005"
build/truncar disco 1 9005
echo "################################################################################"
echo "$ ./leer disco 1 > ext1.txt"
build/leer disco 1 > ext1.txt
echo "################################################################################"
echo "$ ls -l ext1.txt"
ls -l ext1.txt
echo "################################################################################"
echo "$ cat ext1.txt"
cat ext1.txt
echo ""
echo "################################################################################"
echo "$ ./leer_sf disco"
build/leer_sf disco
echo "################################################################################"
echo "$ ./truncar disco 1 0"
build/truncar disco 1 0
echo "################################################################################"
echo "$ ./leer_sf disco"
build/leer_sf disco
echo "################################################################################"
make clean
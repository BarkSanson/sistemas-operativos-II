echo "################################################################################"
echo "$ rm disco"
rm disco
echo "$ ../build/mi_mkfs disco 100000"
../build/mi_mkfs disco 100000
echo "################################################################################"
echo "$ ../build/escribir disco "$(cat texto2.txt)" 1"
echo "#escribimos el texto contenido en text2.txt en los offsets  9000, 209000, 30725000, "
echo "#409605000 y 480000000 de inodos diferentes"
../build/escribir disco "$(cat texto2.txt)" 1
echo "###############################################################################"
echo "$ ../build/leer disco 2 > ext4.txt"
echo "#leemos el contenido del inodo 2 (escrito en el offset 209000) y lo direccionamos"
echo "#al fichero externo ext4.txt"
../build/leer disco 2 > ext4.txt
echo "################################################################################"
echo "$ ls -l ext4.txt"
echo "#comprobamos cuánto ocupa el fichero externo ext4.txt"
echo "#(ha de coincidir con el tamaño en bytes lógico del inodo 2 y con total_leidos)"
ls -l ext4.txt
echo "################################################################################"
echo "$ cat ext4.txt"
echo "#usamos el cat del sistema para leer el contenido de nuestro fichero direccionado"
echo "#No hay que mostrar basura"
cat ext4.txt
echo "################################################################################"
echo "$ ../build/permitir"
echo "#mostramos sintaxis de permitir"
../build/permitir
echo "################################################################################"
echo "$ ../build/permitir disco 2 0"
echo "#cambiamos permisos del inodo 2 a 0"
../build/permitir disco 2 0
echo "################################################################################"
echo "$ ../build/leer disco 2"
echo "#intentamos leer inodo 2 con permisos=0"
../build/leer disco 2
bison -d cemenos.y
flex cemenos.l
gcc *.c -o cll -ly -lfl

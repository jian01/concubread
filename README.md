# concubread

Compilacion:
`gcc -Wall -pedantic -g -D_POSIX_SOURCE -D_XOPEN_SOURCE=500 -std=c99 -o concubread *.c -lm`

Uso:
```
./concubread <recepcionistas> <pizzeros> <panaderos> <input_filename> <optinal_debug_filename>
```

Ejemplo de corrida con valgrind:
```
valgrind --leak-check=full --show-leak-kinds=all --undef-value-errors=no ./concubread 4 4 4 input debug
```

# concubread

Compilacion:
`gcc -Wall -pedantic -g -D_POSIX_SOURCE -std=c11 -o concubread *.c`

Uso:
```
./concubread <recepcionistas> <pizzeros> <panaderos> <input_filename> <optinal_debug_filename>
```

Ejemplo de corrida con valgrind:
```
valgrind --leak-check=full --show-leak-kinds=all ./concubread 4 4 4 input debug
```

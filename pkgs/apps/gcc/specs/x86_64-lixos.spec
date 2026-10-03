*target:
x86_64-lixos

*link:
-m elf_x86_64 -dynamic-linker /lib/libc.so -rpath /lib:/usr/lib

*lib:
-lc -lm

*cpp:
-isystem /include -isystem /usr/include -D__lixos__ -D__LIXOS__

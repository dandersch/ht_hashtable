#!/bin/bash

g++ -g test.cpp -o hash_test     && ./hash_test
gcc -g test.c -o hash_test       && ./hash_test
clang++ -g test.cpp -o hash_test && ./hash_test
clang -g test.c -o hash_test     && ./hash_test
tcc -g test.c -o hash_test       && ./hash_test
cl.exe /Zi /std:c11 test.c /link /DEBUG:FULL /OUT:hash_test.exe /SUBSYSTEM:CONSOLE && WINEDEBUG=-all wine ./hash_test.exe
cl.exe /Zi test.cpp /link /DEBUG:FULL /OUT:hash_test.exe /SUBSYSTEM:CONSOLE        && WINEDEBUG=-all wine ./hash_test.exe

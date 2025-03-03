#!/bin/bash

file_all=$(ls *.c *.h)
# file_all=$(find . -type f -name '*.c' -o -name '*.h')
format="clang-format.exe -style=file:C:/Users/hao/.clang-format -i "

for f in ${file_all}; do
    cmd=${format}${f}
    ${cmd}

    echo ${f} "done..."
done

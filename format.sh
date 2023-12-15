#!/usr/bin/env bash

find engine -regex '.*\.\(cpp\|hpp\|h\|cc\|cxx\)' -exec clang-format -style=file -i {} \;
find sandbox -regex '.*\.\(cpp\|hpp\|h\|cc\|cxx\)' -exec clang-format -style=file -i {} \;


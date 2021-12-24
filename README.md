# OASM Scripting language

Compile instructions on Windows

**Step 1:**
download MSYS2 from https://www.msys2.org/ and install it

**Step 2:**
run MSYS2, and in the terminal run:
```
pacman -Syuu
pacman -S mingw-w64-x86_64-toolchain base-devel
```

**Step 3:**
then put {install-dir-of-msys}\mingw64\bin in your Path variable

**Step 4:**
download CMake from https://cmake.org/download/ and install it

**Step 5:**
put {install-dir-of-cmake}\bin in your path variable

**Step 6:**
Reboot your system

**Step 7:**
open a command prompt inside the root directory of the project

**Step 8:**
execute 'run-cmake.bat'

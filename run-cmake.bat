cmake -B bin -S src -G "MinGW Makefiles"
cd bin
mingw32-make install
cd ..

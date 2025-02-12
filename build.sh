set -xe

CPPC=i686-w64-mingw32-c++
$CPPC -g -o api_enumerate.o -c api_enumerate.cpp 
$CPPC -static -v -o api_enumerate.exe api_enumerate.o -lntdll -ldbghelp

CPPC=x86_64-w64-mingw32-c++
$CPPC -g -o api_enumerate.o -c api_enumerate.cpp 
$CPPC -static -v -o api_enumerate64.exe api_enumerate.o -lntdll -ldbghelp

### C api relay builder

#### Usage
`api_enumerate.exe <target dll>`

It will generate `<target.dll>_relay.h` which can be included into builds, uses `LOG(...)` to report errors, and a `load_syms_<dll name>` init function

eg.

```
...
#define LOG(...) {printf(__VA_ARGS__)}
#include <my_lib.dll_relay.h>

int main (){
	load_syms_my_lib()
	...
}
...

```

#### Build

0. on a linux/cygwin environment, install i686-w64-mingw32-c++ and x86_64-w64-mingw32-c++
1. run build.sh

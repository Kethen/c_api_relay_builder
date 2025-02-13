#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include <dbghelp.h>

#include <vector>
#include <string>

static FILE* log_file = NULL;
static char log_file_name[512];
#define LOG(...){ \
	if(log_file == NULL){ \
		log_file = fopen(log_file_name, "w"); \
	} \
	if(log_file != NULL){ \
		fprintf(log_file, __VA_ARGS__); \
	} \
}

BOOL CALLBACK EnumSymProc( 
    PSYMBOL_INFO pSymInfo,   
    ULONG SymbolSize,      
    PVOID UserContext)
{
	if((pSymInfo->Flags & SYMFLAG_EXPORT) == 0 || strcmp("EntryPoint", pSymInfo->Name) == 0){
		return TRUE;
	}

	std::vector<std::string> &syms = *(std::vector<std::string> *)(UserContext);
	syms.push_back(std::string(pSymInfo->Name));
	return TRUE;
}

int main(int c, char **argv){
	char mod_name[512];
	if(c != 2){
		fprintf(stderr, "usage: %s <target dll>\n", argv[0]);
		exit(1);
	}

	strcpy(mod_name, argv[1]);
	sprintf(log_file_name, "%s_relay.h", mod_name);

	char mod_path[1024];
	sprintf(mod_path, "./%s", mod_name);
	HMODULE lib = LoadLibraryA(mod_path);

	if(lib == NULL){
		LOG("failed opening %s\n", mod_path);
		exit(1);
	}

	// ref https://learn.microsoft.com/en-us/windows/win32/debug/enumerating-symbols
	HANDLE hProcess = GetCurrentProcess();
	DWORD64 BaseOfDll;
	const char *Mask = "*";
	BOOL status;

	status = SymInitialize(hProcess, NULL, FALSE);
	if(status == FALSE){
		LOG("SymInitialize failed\n");
		exit(1);
	}

	BaseOfDll = SymLoadModuleEx(hProcess, NULL, mod_name, NULL, 0, 0, NULL, 0);
	if(BaseOfDll == 0){
		LOG("SymLoadModuleEx failed\n");
		exit(1);
	}

	std::vector<std::string> syms;

	BOOL ret = SymEnumSymbols(hProcess, BaseOfDll, Mask, EnumSymProc, &syms);
	if(!ret){
		LOG("SymEnumSymbols failed\n");
		exit(1);
	}

	for(auto itr = syms.begin(); itr != syms.end(); itr++){
		LOG("static void (*%s_)() = NULL;\n", itr->c_str());
	}

	LOG("#ifdef __cplusplus\n");
	LOG("extern \"C\"{\n");
	LOG("#endif\n");
	for(auto itr = syms.begin(); itr != syms.end(); itr++){
		LOG("	void %s(void){%s_();}\n", itr->c_str(), itr->c_str());
	}
	LOG("#ifdef __cplusplus\n");
	LOG("}\n");
	LOG("#endif\n");

	char new_name[512] = {0};
	memcpy(new_name, mod_name, strlen(mod_name) - 4);

	LOG("static void load_syms_%s(){\n"
	"	HMODULE lib = LoadLibraryA(\"_%s\");\n"
	"	if(lib == NULL){\n"
	"		LOG(\"failed to load original dll _%s\\n\")\n"
	"		exit(1);\n"
	"	}\n"
	, new_name, mod_name, mod_name);

	for(auto itr = syms.begin(); itr != syms.end(); itr++){
		LOG("	%s_ = (void (*)())GetProcAddress(lib, \"%s\");\n", itr->c_str(), itr->c_str());
		LOG("	if(%s_ == NULL){\n", itr->c_str());
		LOG("		LOG(\"failed to load %s\\n\");\n", itr->c_str());
		LOG("		exit(1);\n");
		LOG("	}\n");
	}
	LOG("}")
}

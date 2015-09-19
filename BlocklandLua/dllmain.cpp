#include <cstdlib>
#include "Torque.h"
#include "lua.hpp"

lua_State *gL;

bool istrue(const char *arg)
{
	return !_stricmp(arg, "true") || !_stricmp(arg, "1") || (0 != atoi(arg));
}

static bool luaEval(DWORD* obj, int argc, const char** argv)
{
	if (luaL_loadbuffer(gL, argv[1], strlen(argv[1]), "input") || lua_pcall(gL, 0, 0, 0))
	{
		if (argc < 3 || !istrue(argv[2]))
			Printf("Lua error: %s", lua_tostring(gL, -1));
		else
			lua_pop(gL, -1);

		return false;
	}

	return true;
}

static bool luaExec(DWORD *obj, int argc, const char** argv)
{
	// warning: does not use Blockland file system atm, allows execution outside game folder
	
	if (argc < 3 || !istrue(argv[2]))
		Printf("Executing %s.", argv[1]);

	if (luaL_loadfile(gL, argv[1]) || lua_pcall(gL, 0, 0, 0))
	{
		if (argc < 3 || !istrue(argv[2]))
			Printf("Lua error: %s", lua_tostring(gL, -1));
		else
			lua_pop(gL, -1);

		return false;
	}

	return true;
}

static int tsEval(lua_State *L)
{
	lua_pushstring(L, Eval(luaL_checkstring(L, 1)));
	return 1;
}

static int tsEcho(lua_State *L)
{
	Printf("%s", luaL_checkstring(L, -1));
	return 0;
}

DWORD WINAPI Init(LPVOID args)
{
	if (!InitTorqueStuff())
		return 0;

	lua_State *L = luaL_newstate();
	gL = L;

	if (L == NULL)
	{
		Printf("Failed to initialize Lua");
		return 1;
	}

	luaL_openlibs(L);

	lua_newtable(L);
	lua_pushstring(L, "eval");
	lua_pushcfunction(L, tsEval);
	lua_pushvalue(L, -1);
	lua_setglobal(L, "eval");
	lua_rawset(L, -3);
	lua_pushstring(L, "echo");
	lua_pushcfunction(L, tsEcho);
	lua_pushvalue(L, -1);
	lua_setglobal(L, "echo");
	lua_rawset(L, -3);
	lua_setglobal(L, "ts");

	ConsoleFunction(NULL, "luaEval", luaEval, "luaEval(string code, bool silent=false) - Execute a chunk of code as Lua.", 2, 3);
	ConsoleFunction(NULL, "luaExec", luaExec, "luaExec(string filename, bool silent=false) - Execute a Lua code file.", 2, 3);

	return 0;
}

int WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Init, NULL, 0, NULL);

	return true;
}
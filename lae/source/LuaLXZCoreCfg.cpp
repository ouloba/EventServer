extern "C" 
{
#include "lua.hpp"
#include "tolua++.h"
}

#include "LXZCoreCfg.h"

static int LuaLXZCoreCfg_new(lua_State* pState)
{
	LXZCoreCfg* cfg = new LXZCoreCfg();
	tolua_pushusertype(pState, cfg, "ILXZCoreCfg");
	return 1;
}

static int LuaLXZCoreCfg_new_local(lua_State* pState)
{
	LXZCoreCfg* cfg = new LXZCoreCfg();
	tolua_pushusertype_and_takeownership(pState, cfg, "ILXZCoreCfg");
	return 1;
}


static int LuaLXZCoreCfg_Ref(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	const char* alias = tolua_tostring(pState, 3, NULL);
	int ref = cfg->Ref(name, alias);
	tolua_pushnumber(pState, ref);
	return 1;
}

static int LuaLXZCoreCfg_GetObj(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	const char* alias = tolua_tostring(pState, 3, NULL);
	int ref = (int)tolua_tonumber(pState, 4, -1);
	void* obj = cfg->GetObj(name,&ref,alias);
	tolua_pushuserdata(pState, obj);
	return 1;
}

static int LuaLXZCoreCfg_SetObj(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	int ref = (int)tolua_tonumber(pState, 3, -1);
	void* obj = tolua_touserdata(pState, 4, NULL);
	void* old = cfg->SetObj(name, &ref, obj);
	tolua_pushuserdata(pState, old);
	return 1;
}

static int LuaLXZCoreCfg_GetBool(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	const char* alias = tolua_tostring(pState, 3, NULL);
	int ref = (int)tolua_tonumber(pState, 4, -1);
	bool b = cfg->GetBool(name, &ref,alias);
	tolua_pushboolean(pState, b);
	return 1;
}

static int LuaLXZCoreCfg_GetSaveFlag(lua_State* pState){
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	int ref = (int)tolua_tonumber(pState, 2, -1);
	bool ret = cfg->GetSaveFlag(ref);
	tolua_pushboolean(pState, ret?1:0);
	return 1;
}

static int LuaLXZCoreCfg_SetSaveFlag(lua_State* pState){
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	int ref = (int)tolua_tonumber(pState, 2, -1);
	int flag = (int)tolua_toboolean(pState, 3, 0);
	cfg->SetSaveFlag(ref, flag==1);	
	return 0;
}

static int LuaLXZCoreCfg_GetFloat(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	const char* alias = tolua_tostring(pState, 3, NULL);
	int ref = (int)tolua_tonumber(pState, 4, -1);
	float b = (float)cfg->GetFloat(name, &ref, alias);
	tolua_pushnumber(pState, b);
	return 1;
}

static int LuaLXZCoreCfg_GetInt(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	const char* alias = tolua_tostring(pState, 3, NULL);
	int ref = (int)tolua_tonumber(pState, 4, -1);
	int b = (int)cfg->GetInt(name, &ref, alias);
	tolua_pushnumber(pState, b);
	return 1;
}


static int LuaLXZCoreCfg_GetUint(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	const char* alias = tolua_tostring(pState, 3, NULL);
	int ref = (int)tolua_tonumber(pState, 4, -1);
	LXZuint32 b = (LXZuint32)cfg->GetUint(name, &ref, alias);
	tolua_pushnumber(pState, b);
	return 1;
}

static int LuaLXZCoreCfg_GetCString(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	const char* alias = tolua_tostring(pState, 3, NULL);
	int ref = (int)tolua_tonumber(pState, 4, -1);
	const char* b = (const char*)cfg->GetCString(name, &ref, alias);
	tolua_pushstring(pState, b);
	return 1;
}

static int LuaLXZCoreCfg_SetUint(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	int ref = (int)tolua_tonumber(pState, 3, -1);
	LXZuint32 obj =(LXZuint32) tolua_tonumber(pState, 4, 0);
	LXZuint32 old = cfg->SetUint(name, &ref, obj);
	tolua_pushnumber(pState, old);
	return 1;
}

static int LuaLXZCoreCfg_SetFloat(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	int ref = (int)tolua_tonumber(pState, 3, -1);
	float obj = (float)tolua_tonumber(pState, 4, 0);
	float old = cfg->SetFloat(name, &ref, obj);
	tolua_pushnumber(pState, old);
	return 1;
}

static int LuaLXZCoreCfg_SetInt(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	int ref = (int)tolua_tonumber(pState, 3, -1);
	int obj = (int)tolua_tonumber(pState, 4, 0);
	int old = cfg->SetInt(name, &ref, obj);
	tolua_pushnumber(pState, old);
	return 1;
}

static int LuaLXZCoreCfg_SetBool(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	int ref = (int)tolua_tonumber(pState, 3, -1);
	int obj = (int)tolua_toboolean(pState, 4, 0);
	bool old = cfg->SetBool(name, &ref, obj==1?true:false);
	tolua_pushnumber(pState, old);
	return 1;
}

static int LuaLXZCoreCfg_SetCString(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	int ref = (int)tolua_tonumber(pState, 3, -1);
	const char* obj = (const char*)tolua_tostring(pState, 4, NULL);
	const char* old = cfg->SetCString(name, &ref, obj);
	tolua_pushstring(pState, old);
	return 1;
}

static int LuaLXZCoreCfg_destroy(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	cfg->destroy();
	return 0;
}

static int LuaLXZCoreCfg_release(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	cfg->release();
	return 0;
}

static int LuaLXZCoreCfg_save(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	cfg->save(name);
	return 0;
}

static int LuaLXZCoreCfg_load(lua_State* pState)
{
	ILXZCoreCfg* cfg = (ILXZCoreCfg*)tolua_tousertype(pState, 1, NULL);
	const char* name = tolua_tostring(pState, 2, "");
	bool b = cfg->load(name);
	tolua_pushboolean(pState, b ? 1 : 0);
	return 1;
}


int LuaLXZCoreCfgInitial(lua_State* pState)
{
	tolua_open(pState);
	tolua_module(pState, NULL, 0);
	tolua_beginmodule(pState, NULL);

	tolua_usertype(pState, "ILXZCoreCfg");
	tolua_cclass(pState, "ILXZCoreCfg", "ILXZCoreCfg", "", LuaLXZCoreCfg_destroy);

	tolua_beginmodule(pState, "ILXZCoreCfg");	
	tolua_function(pState, "new", LuaLXZCoreCfg_new);
	tolua_function(pState, "delete", LuaLXZCoreCfg_destroy);
	tolua_function(pState, "new_local", LuaLXZCoreCfg_new_local);
	tolua_function(pState, "release", LuaLXZCoreCfg_release);

	tolua_function(pState, "load", LuaLXZCoreCfg_load);
	tolua_function(pState, "save", LuaLXZCoreCfg_save);

	tolua_function(pState, "GetBool", LuaLXZCoreCfg_GetBool);
	tolua_function(pState, "SetBool", LuaLXZCoreCfg_SetBool);
	tolua_function(pState, "Ref", LuaLXZCoreCfg_Ref);

	tolua_function(pState, "GetInt", LuaLXZCoreCfg_GetInt);
	tolua_function(pState, "SetInt", LuaLXZCoreCfg_SetInt);

	tolua_function(pState, "GetUint", LuaLXZCoreCfg_GetUint);
	tolua_function(pState, "SetUint", LuaLXZCoreCfg_SetUint);

	tolua_function(pState, "GetObj", LuaLXZCoreCfg_GetObj);
	tolua_function(pState, "SetObj", LuaLXZCoreCfg_SetObj);


	tolua_function(pState, "GetCString", LuaLXZCoreCfg_GetCString);
	tolua_function(pState, "SetCString", LuaLXZCoreCfg_SetCString);

	tolua_function(pState, "GetFloat", LuaLXZCoreCfg_GetFloat);
	tolua_function(pState, "SetFloat", LuaLXZCoreCfg_SetFloat);
	
	tolua_function(pState, "GetSaveFlag", LuaLXZCoreCfg_GetSaveFlag);
	tolua_function(pState, "SetSaveFlag", LuaLXZCoreCfg_SetSaveFlag);

	tolua_endmodule(pState);

	tolua_endmodule(pState);

	return 0;
}
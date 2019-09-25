#include<stdlib.h>
#include "EventServer.h"
#include "LuaThread.h"
#include "LuaThreadMgr.h"
#include <time.h>

extern CEventServer* g_server;

extern "C" void lae_log(const char* fmt, ...);
extern "C" int luaopen_lfs(lua_State *L);
extern int LuaLXZMessageInitial(lua_State* pState);
extern int LuaLXZCoreCfgInitial(lua_State* pState);
extern "C" int luaopen_cjson(lua_State *l);
extern "C" int luaopen_mime_core(lua_State *L);
#ifndef WIN32
extern unsigned long GetTickCount();
#endif

static int LLuaPrintToConsole(lua_State* pState)
{
	const char* str = tolua_tostring(pState, 1, "");
	printf("%s\n", str);
	return 0;
}

static int __GetCurrentLuaTask(lua_State* pState)
{
	return 1;
}

static int __SetUserResult(lua_State* pState)
{
	CLuaThread* pTask = (CLuaThread*)tolua_tousertype(pState, 1, NULL);
	size_t count = 0;
	const char* code = (const char*)lua_tolstring(pState, 2, &count);
	_LuaTask* pTaskCmd = pTask->GetNowTask();
	if (pTaskCmd) {
		pTaskCmd->retbuf.grow(count);
		memcpy(pTaskCmd->retbuf.buf(), code, count);
		pTaskCmd->user_ret = true;
	}
	return 0;
}

static int lua_tousertype1(lua_State* L)
{
	void *req = (void *)lua_topointer(L, 1);
	const char* szType = (const char*)tolua_tostring(L, 2, "");
	tolua_pushusertype(L, req, szType);
	return 1;
}


extern "C"
{
#ifdef WIN32
#define	LAE_API __declspec(dllexport)
#else
#define	LAE_API
#endif
	
	LAE_API size_t curlreceive_data(void *buffer, size_t size, size_t nmemb, void* usermsg)
	{
		CLXZMessage* msg = (CLXZMessage*)usermsg;
		msg->Write((const char*)buffer, size*nmemb);
		return size*nmemb;
	}
}




CLuaThread::CLuaThread(){

	_max_tasks = 10000;
	_id = -1;
	_state = lua_open();
	luaL_openlibs(_state);
	LuaLXZMessageInitial(_state);
	LuaLXZCoreCfgInitial(_state);
	luaopen_lfs(_state);
	luaopen_mime_core(_state);
	luaopen_cjson(_state);

	tolua_open(_state);
	tolua_module(_state, NULL, 0);
	tolua_beginmodule(_state, NULL);	
	tolua_function(_state, "luaopen_cjson", luaopen_cjson);	
	tolua_function(_state, "PrintToConsole", LLuaPrintToConsole);
	tolua_function(_state, "tousertype", lua_tousertype1);
		
	
	lua_State* L = _state;
	tolua_usertype(L, "CLuaThread");
	tolua_cclass(L, "CLuaThread", "CLuaThread", "", NULL);
	tolua_beginmodule(L, "CLuaThread");
	tolua_function(L, "SetUserResult", __SetUserResult);
	tolua_endmodule(L);

	tolua_pushusertype(L, this, "CLuaThread");
	lua_setglobal(L, "CurrentTask");


	tolua_endmodule(_state);
}

CLuaThread::~CLuaThread(){
	lua_close(_state);
}

size_t CLuaThread::SetMaxTask(size_t max_tasks) {
	size_t tasks = _max_tasks;
	_max_tasks = max_tasks;
	return tasks;
}

size_t CLuaThread::GetTaskCount() {
	size_t size_tasks = 0;
	_lock.Linux_Win_Locked();
	size_tasks = _tasks.size();
	_lock.Linux_Win_UnLocked();
	return size_tasks;
}

int CLuaThread::PostTask(const char* code, size_t size, lua_State *co, lua_State *main){	
	size_t size_tasks = GetTaskCount();
	if (size_tasks>= _max_tasks) {
		lae_log("PostTask element overlow, size_tasks:%u _max_tasks:%u\r\n", size_tasks, _max_tasks);
		return size_tasks;
	}

	_LuaTask* pCmd = new _LuaTask;
	pCmd->cmdbuf.grow(size + 1);
	memcpy(pCmd->cmdbuf.buf(), code, size);
	pCmd->cmdbuf.setcplen(size);
	pCmd->co = co;
	pCmd->ok = false;
	pCmd->ref = -1;
	pCmd->time = GetTickCount();
	pCmd->main = main;
	if (co){
		lua_pushthread(co);
		pCmd->ref = lua_ref(co, LUA_REGISTRYINDEX);
	}

	_lock.Linux_Win_Locked();
	_tasks.push_back(pCmd);
	//printf("CLuaThread::PostTask, *****************_threadid1:%u size:%u\r\n", _threadid, _tasks.size());
	_lock.Linux_Win_UnLocked();
	_event.SetEvent();
	//Sleep(1);
	//printf("CLuaThread::PostTask, *****************_threadid2:%u size:%u\r\n", _threadid, _tasks.size());
	

	return size_tasks;
}

static int ldocall(lua_State *L, int narg, int clear)
{
	int status;
	int base = lua_gettop(L) - narg;  /* function index */
	status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), 0);
	//lua_remove(L, base);  /* remove traceback function */
	/* force a complete garbage collection in case of errors */
	if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
	return status;
}

void CLuaThread::WorkProc()
{	
	srand(time(NULL));
	char EventName[256] = { 0 };
	sprintf(EventName, "%u%u", _id, rand());
	_event.Initialize(EventName);
	printf("EventName:%s\r\n", EventName);

	_LuaTask* pCmd = NULL;
	while (true)
	{
		// Wait for http request tasks from main thread
		//printf("*");
		pCmd = NULL;
		_lock.Linux_Win_Locked();
		if (0 != _tasks.size())
		{
			pCmd = _tasks.front();
			_tasks.pop_front();
		}
		_lock.Linux_Win_UnLocked();

		if (pCmd == NULL){
			int semWaitRet = _event.Wait(5);
			if (semWaitRet < 0) {
				//CCLog("HttpRequest async thread semaphore error: %s\n", strerror(errno));
				printf("WorkProc async thread semaphore error: %s _threadid:%u\n", strerror(errno), _id);
				break;
			}
		}		

		
		if (NULL == pCmd){
			//_event.Wait(10);
			continue;
			//return 0;
		}		

		//printf("WorkProc :%u _threadid:%u\r\n", GetTickCount() - pCmd->time, _threadid);
		pCmd->user_ret = false;
		_pCmd = pCmd;
		int status = luaL_loadbuffer(_state,
			pCmd->cmdbuf.buf(),
			pCmd->cmdbuf.cplen(),
			"") || ldocall(_state, 0, (pCmd->co != NULL) ? 0 : 1);

		if (pCmd->user_ret) {
			pCmd->ok = false;
		}
		else if (status != 0) { //error	
			size_t len = 0;
			const char* szRet = luaL_checklstring(_state, -1, &len);
			pCmd->retbuf.grow(len + 1);
			memcpy(pCmd->retbuf.buf(), szRet, len);
			pCmd->retbuf.buf()[len] = 0x00;
			pCmd->retbuf.setcplen(len);
			pCmd->ok = false;
			lua_pop(_state, 1);
			lae_log("Lua execute error:%s\r\n", szRet);
		}
		else { // ok
			pCmd->ok = true;
			if (pCmd->co&&lua_isstring(_state, -1)) {//need result.			
				size_t len = 0;
				const char* szRet = luaL_checklstring(_state, -1, &len);
				pCmd->retbuf.grow(len + 1);
				memcpy(pCmd->retbuf.buf(), szRet, len);
				pCmd->retbuf.buf()[len] = 0x00;
				pCmd->retbuf.setcplen(len);
				lua_pop(_state, 1);
			}
		}

		//printf(".");
		if (pCmd->co)
		{
			CLuaThreadMgr::Instance().PostTaskReturn(pCmd);
		}
		else {
			delete pCmd;
			pCmd = NULL;
		}

		if (g_server) {
			g_server->user_trigger();
		}
		//CLuaThreadMgr::Instance().Update(_state);
	}
}

#include "LuaThreadMgr.h"

#ifdef WIN32
#include <process.h>
#include <io.h>
#else
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#endif

#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

CLuaThreadMgr::CLuaThreadMgr(){

}

CLuaThreadMgr::~CLuaThreadMgr(){
	std::list<CLuaThread*>::iterator p = _pools.begin();
	for (; p != _pools.end(); p++){
		delete *p;
	}
	_pools.clear();
}

CLuaThreadMgr& CLuaThreadMgr::Instance(){
	static CLuaThreadMgr instance;
	return instance;
}


// Worker thread
static void* LuaWorkThread(void *data)
{
	CLuaThread* pThread = (CLuaThread*)data;
	pThread->WorkProc();

#ifndef WIN32
	pthread_exit(NULL);
#endif
}

static void WinLuaWorkThread(void *data)
{
	CLuaThread* pThread = (CLuaThread*)data;
	pThread->WorkProc();
}

CLuaThread* CLuaThreadMgr::GetThread(size_t threadid, bool create){
	_lock.Linux_Win_Locked();
	std::list<CLuaThread*>::iterator p = _pools.begin();
	for (; p != _pools.end(); p++){
		if ((*p)->GetID() == threadid){
			CLuaThread* pLuaThread=(*p);
			_lock.Linux_Win_UnLocked();
			return pLuaThread;
		}
	}

	if (!create) {
		_lock.Linux_Win_UnLocked();
		return NULL;
	}

	CLuaThread* pLuaThread = new CLuaThread();
	pLuaThread->SetID(threadid);
	_pools.push_back(pLuaThread);
	_lock.Linux_Win_UnLocked();

#ifdef WIN32
	uintptr_t s_networkThread = _beginthread(WinLuaWorkThread, 0, pLuaThread);
#else
	pthread_t s_networkThread=0;
	pthread_create(&s_networkThread, NULL, LuaWorkThread, pLuaThread);
	pthread_detach(s_networkThread);
#endif


	return pLuaThread;
}

void CLuaThreadMgr::PostTaskReturn(_LuaTask* pRet){
	//printf("PostTaskReturn1:%u\r\n", GetTickCount() - pRet->time);
	_lockRet.Linux_Win_Locked();
	_poolsRet.push_back(pRet);
	_lockRet.Linux_Win_UnLocked();
	//printf("PostTaskReturn2:%u\r\n", GetTickCount()-pRet->time);
}

void CLuaThreadMgr::Update(lua_State *  co){
	std::list<_LuaTask*> lstRet;
	//printf(".");

	//filter
	_lockRet.Linux_Win_Locked();	
	lstRet.swap(_poolsRet);
	_lockRet.Linux_Win_UnLocked();

	// work
	//printf("CLuaThreadMgr :%u size:%u", _threadid, _poolsRet.size());
	std::list<_LuaTask*>::iterator p = lstRet.begin();
	while (p != lstRet.end()){
		_LuaTask* pCmd = *p;

		//
		if (pCmd->co) {
			if (lua_status(pCmd->co) == LUA_YIELD) {
				lua_pushboolean(pCmd->co, pCmd->ok ? 1 : 0);
				lua_pushlstring(pCmd->co, pCmd->retbuf.buf(), pCmd->retbuf.cplen());
				//printf("Update 1:%u _threadid:%u\r\n", GetTickCount() - pCmd->time, _threadid);
				int ret = lua_resume(pCmd->co, 2);
				//printf("Update 2:%u _threadid:%u\r\n", GetTickCount() - pCmd->time, _threadid);
				if (ret != LUA_YIELD&&ret != 0) {
					char buf[256];
					sprintf(buf, "ERROR: %s", lua_tostring(pCmd->co, -1));
					lua_pop(pCmd->co, -1);
#ifdef WIN32
					MessageBox(NULL, buf, "LuaResume", MB_OK);
#endif
				}

			}

			lua_unref(pCmd->co, pCmd->ref);
		}

		//printf("Update 3:%u _threadid:%u\r\n", GetTickCount() - pCmd->time, _threadid);

		// clear;
		delete pCmd;

		//
		p++;		
	}

	
}

#ifndef _LUA_THREAD_MGR_H
#define _LUA_THREAD_MGR_H

#include "LuaThread.h"
#include <list>

class CLuaThreadMgr{
public:
	CLuaThreadMgr();
	~CLuaThreadMgr();

	static CLuaThreadMgr& Instance();
	CLuaThread* GetThread(size_t threadid,bool create=true);

	void PostTaskReturn(_LuaTask* pRet);
	void Update(lua_State *  co);

private:
	Linux_Win_Lock       _lock;
	std::list<CLuaThread*> _pools;

	Linux_Win_Lock         _lockRet;
	std::list<_LuaTask*>   _poolsRet;
	
};

#endif
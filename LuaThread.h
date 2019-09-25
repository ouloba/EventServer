#ifndef _LUA_THREAD_H
#define _LUA_THREAD_H


extern "C"
{
#include "lua.hpp"
#include "tolua++.h"
}

#include "LXZMessage.h"
#include "LXZLock.h"
#include <list>

struct _LuaTask{
	lua_State *  main;
	FastBufAlloc<char, 128> cmdbuf;
	FastBufAlloc<char, 128> retbuf;
	lua_State *  co;
	LXZuint32    time;
	int          ref;
	bool         ok;
	bool         user_ret;
};

class  CLuaThread
{
public:
	CLuaThread();
	~CLuaThread();

	int PostTask(const char* code, size_t size, lua_State *co, lua_State *main=NULL);
	void WorkProc();

	size_t GetID() { return _id; }
	void   SetID(size_t id) { _id = id; }

	size_t GetTaskCount();
	size_t SetMaxTask(size_t max_tasks);
	_LuaTask* GetNowTask() { return _pCmd; }
private:
	lua_State* _state;
	size_t     _id;
	size_t    _max_tasks;

	_LuaTask* _pCmd;

	Linux_Win_Event      _event;
	Linux_Win_Lock       _lock;
	std::list<_LuaTask*> _tasks;
};






#endif


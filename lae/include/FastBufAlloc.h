
#ifndef _FASTBUFALLOC_
#define _FASTBUFALLOC_

#ifndef ANDROID
#include <memory.h>
#else
#include <string.h>
#endif

#include "ICGui.h"

template<typename T, size_t maxcachesize>
class FastBufAlloc
{
public:
	FastBufAlloc()
	{		
		memset(arrbuf, 0x00, sizeof(arrbuf));
		heapbuf = NULL;
		cp_len = 0;
		alloc_len = maxcachesize;
	}

	FastBufAlloc(FastBufAlloc& buf)
	{
		memcpy(arrbuf, buf.arrbuf, sizeof(arrbuf));
		if(buf.heapbuf)
		{
			heapbuf = new T[buf.alloclen()+1];
			memset(heapbuf, 0x00, sizeof(T)*(buf.alloclen() + 1));
			memcpy(heapbuf, buf.heapbuf, sizeof(T)*(buf.alloclen()));
			alloc_len = buf.alloclen();
		}

		cp_len = buf.cplen();
	}

	~FastBufAlloc()
	{
		release();
	}

	void operator =(FastBufAlloc<T,maxcachesize>& buf)
	{
		memcpy(arrbuf, buf.arrbuf, sizeof(arrbuf));
		if(buf.heapbuf)
		{
			heapbuf = new T[buf.alloclen()+1];
			memset(heapbuf, 0x00, sizeof(T)*(buf.alloclen() + 1));
			memcpy(heapbuf, buf.heapbuf, sizeof(T)*(buf.alloclen()));
			alloc_len = buf.alloclen();
		}

		cp_len = buf.cplen();	
	}

	void release()
	{
		memset(&arrbuf[0], 0x00, sizeof(arrbuf));
		//arrbuf[0] = 0;
		if(heapbuf != NULL)
		{
			delete []heapbuf;
		}

		heapbuf = NULL;		
		cp_len = 0;
		alloc_len = maxcachesize;
	}

	void append(T* arr, size_t size)
	{
		grow(cp_len+size);
		memcpy(&buf()[cp_len], arr, size*sizeof(T));
		setcplen(cp_len+size);
	}

	T* grow(size_t newsize)
	{		
		if(newsize<=alloc_len)
			return buf();
		
		if(heapbuf)
		{
			T* b = new T[newsize+1];
			memset(b, 0x00, sizeof(T)*(newsize+1));
			memcpy(b, heapbuf, sizeof(T)*(alloc_len));
			delete []heapbuf;
			heapbuf = b;
			alloc_len = newsize;
		}
		else
		{
			T* b = new T[newsize+1];
			memset(b, 0x00, sizeof(T)*(newsize+1));
			memcpy(b, buf(), sizeof(T)*(cp_len));			
			heapbuf = b;
			alloc_len = newsize;
		}

		return heapbuf;
	}

	T* buf() 
	{
		if(alloc_len>maxcachesize)
			return heapbuf;
		return arrbuf;
	}

	size_t cplen() { return cp_len; }
	void setcplen(size_t len)	{	cp_len = len; memset(&buf()[len], 0x00, sizeof(T)); }
	size_t alloclen() { return alloc_len; }	
private:
	T*  heapbuf;
	T   arrbuf[maxcachesize+1];
	size_t cp_len;
	size_t alloc_len;
};


#endif

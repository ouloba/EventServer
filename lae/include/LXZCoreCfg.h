
#ifndef _LXZCORE_CFG_H
#define _LXZCORE_CFG_H

#include "ILXZCoreCfg.h"
#include "vvector.h"
#include "FastBufAlloc.h"

class LXZCoreCfg :public ILXZCoreCfg
{
	enum{
		cfg_null=-1,
		cfg_i,
		cfg_f,
		cfg_cstr,
		cfg_p,
		cfg_b,
		cfg_u,
	};

	struct _cfg_value{
		union
		{
			unsigned int u;
			bool   b;
			int    i;
			float  f;
			void*  p;
			char*  cstr;
		};

		char* alias;
		char* name;
		LXZuint8 type;
		LXZuint8 flag;

		_cfg_value(){
			alias = NULL;
			name = NULL;
			type = cfg_null;
			flag = 0;
		}
	};
public:
	LXZCoreCfg();
	~LXZCoreCfg();

	bool GetSaveFlag(int ref);
	void SetSaveFlag(int ref, bool IsCanBeSaved);

	char* tolower_(const char* name, char* buf, int buf_len);
	char* cpstr(const char*);
	int   Ref(const char* name,const char* alias);
	void* GetObj(const char*name, int* ref, const char* alias = NULL, void* def = NULL);
	void* SetObj(const char*name, int* ref,void* v);
	float GetFloat(const char*name, int* ref, const char* alias = NULL, float def = 0.0f);
	int   GetInt(const char*name, int* ref, const char* alias = NULL, int def = 0);
	bool  GetBool(const char* name, int* ref, const char* alias = NULL, bool def = false);
	const char*  GetCString(const char* name, int* ref, const char* alias = NULL, const char* def = "");	
	float SetFloat(const char*name, int* ref, float v);
	int   SetInt(const char*name, int* ref, int v);
	bool  SetBool(const char* name, int* ref, bool v);
	const char*  SetCString(const char* name, int* ref, const char* v);
	unsigned int  GetUint(const char*name, int* ref, const char* alias = NULL, unsigned int def = 0);
	unsigned int  SetUint(const char*name, int* ref, unsigned int v);
	void destroy(){ delete this; }
	void release();
	void save(const char* filename);
	bool load(const char* filename);
private:
	const char* convert(const char* s, char* buf);
	FastBufAlloc<char, 16>   m_tmpAlloc;
	vvector<_cfg_value*>     m_vecCfg;
};

#endif
#include <stdio.h>
#include<stdlib.h>
#include "LXZCoreCfg.h"
#include <ctype.h>

LXZCoreCfg::LXZCoreCfg(){
	m_vecCfg.grown(2);
}

LXZCoreCfg::~LXZCoreCfg(){
	release();
}

void LXZCoreCfg::release(){
	for (int i = 0; i < m_vecCfg.size(); i++){
		delete m_vecCfg[i]->name;
		if (m_vecCfg[i]->type == cfg_cstr)
			delete m_vecCfg[i]->cstr;
	}
	m_vecCfg.clear();
}

char* LXZCoreCfg::tolower_(const char* name, char* buf, int buf_len){
	int len = strlen(name);
	for (int i = 0; i < len&&i<(buf_len-1); i++){
		buf[i] = tolower(name[i]);
	}

	if (len>(buf_len-1)){
		buf[buf_len-1] = 0x00;
	}
	else{
		buf[len] = 0x00;
	}

	return buf;
}

int   LXZCoreCfg::Ref(const char* name, const char* alias)
{
	char buf1[512] = {0};
	char buf2[512] = {0};

	int ref = -1;
	for (int i = 0; i < m_vecCfg.size(); i++){
		_cfg_value* cfg = m_vecCfg[i];
		char*  _name1 = tolower_(cfg->name, buf1, 512);
		char*  _name2 = tolower_(name, buf2, 512);
		if (strcmp(_name1,_name2) == 0){
			return i;
		}

		if (alias&&strcmp(_name1, alias) == 0){
			return i;
		}

		//	if (!match && (strstr(_name1, _name2) != NULL || strstr(_name2, _name1) != NULL)){
		//		ref = i;
		//	}
	}

	return  ref;
}

void* LXZCoreCfg::GetObj(const char*name, int* ref_, const char* alias,void* def)
{
	int ref = -1;
	if (ref_) ref = *ref_;

	if (ref&&ref == -1){
		ref = Ref(name, alias);
		if (ref_)*ref_ = ref;
	}

	if (ref == -1||m_vecCfg[ref]->type!=cfg_p){
		SetObj(name, ref_, def);
		return def;
	}

	return m_vecCfg[ref]->p;
}

void* LXZCoreCfg::SetObj(const char*name, int* ref_,void* v)
{
	int ref = -1;
	if (ref_) ref = *ref_;

	if (ref&&ref == -1){
		ref = Ref(name,NULL);
		if (ref_)*ref_ = ref;
	}

	if (ref == -1){
		_cfg_value* vv = new _cfg_value;
		vv->p = v;
		vv->type = cfg_p;
		vv->name = cpstr(name);
		m_vecCfg.push_back(vv);
		ref = m_vecCfg.size() - 1;
		if (ref_)*ref_ = ref;
		return v;
	}

	void* r = m_vecCfg[ref]->p;
	m_vecCfg[ref]->p = v;
	return r;
}

unsigned int  LXZCoreCfg::GetUint(const char*name, int* ref_, const char* alias,unsigned int def){
	int ref = -1;
	if (ref_) ref = *ref_;

	if (ref&&ref == -1){
		ref = Ref(name, alias);
		if (ref_)*ref_ = ref;
	}

	if (ref == -1||m_vecCfg[ref]->type != cfg_u){
		SetUint(name, ref_, def);
		return def;
	}

	return m_vecCfg[ref]->u;
}

unsigned int  LXZCoreCfg::SetUint(const char*name, int* ref_, unsigned int v){
	int ref = -1;
	if (ref_) ref = *ref_;

	if (ref&&ref == -1){
		ref = Ref(name,NULL);
		if (ref_)*ref_ = ref;
	}

	if (ref == -1){
		_cfg_value* vv = new _cfg_value;
		vv->u = v;
		vv->type = cfg_u;
		vv->name = cpstr(name);
		m_vecCfg.push_back(vv);
		ref = m_vecCfg.size() - 1;
		if (ref_)*ref_ = ref;
		return v;
	}

	unsigned int r = m_vecCfg[ref]->u;
	m_vecCfg[ref]->u = v;
	return r;
}

float LXZCoreCfg::GetFloat(const char*name, int* ref_, const char* alias,float def)
{
	int ref = -1;
	if (ref_) ref = *ref_;

	if (ref&&ref == -1){
		ref = Ref(name, alias);
		if (ref_)*ref_ = ref;
	}

	if (ref == -1 || m_vecCfg[ref]->type != cfg_f){
		SetFloat(name, ref_, def);
		return def;
	}

	return m_vecCfg[ref]->f;
}

int   LXZCoreCfg::GetInt(const char*name, int* ref_, const char* alias,int def)
{
	int ref = -1;
	if (ref_) ref = *ref_;

	if (ref&&ref == -1){
		ref = Ref(name, alias);
		if (ref_)*ref_ = ref;
	}

	if (ref == -1 || m_vecCfg[ref]->type != cfg_i){
		SetInt(name, ref_, def);
		return def;
	}

	return m_vecCfg[ref]->i;
}

bool  LXZCoreCfg::GetBool(const char* name, int* ref_, const char* alias,bool def)
{
	int ref = -1;
	if (ref_) ref = *ref_;

	if (ref&&ref == -1){
		ref = Ref(name, alias);
		if (ref_)*ref_ = ref;
	}

	if (ref == -1 || m_vecCfg[ref]->type != cfg_b){
		SetBool(name, ref_, def);
		return def;
	}

	return m_vecCfg[ref]->b;
}

const char*  LXZCoreCfg::GetCString(const char* name, int* ref_, const char* alias, const char* def)
{
	int ref = -1;
	if (ref_) ref = *ref_;

	if (ref&&ref == -1){
		ref = Ref(name, alias);
		if (ref_)*ref_ = ref;
	}

	if (ref == -1 || m_vecCfg[ref]->type != cfg_cstr){
		SetCString(name, ref_, def);
		return def;
	}

	return m_vecCfg[ref]->cstr;
}

float LXZCoreCfg::SetFloat(const char*name, int* ref_, float v)
{
	int ref = -1;
	if (ref_) ref = *ref_;

	if (ref&&ref == -1){
		ref = Ref(name,NULL);
		if (ref_)*ref_ = ref;
	}

	if (ref == -1){
		_cfg_value* vv = new _cfg_value;
		vv->f = v;
		vv->type = cfg_f;
		vv->name = cpstr(name);
		m_vecCfg.push_back(vv);
		ref = m_vecCfg.size() - 1;
		if (ref_)*ref_ = ref;
		return v;
	}

	float r = m_vecCfg[ref]->f;
	m_vecCfg[ref]->f = v;
	return r;
}

int   LXZCoreCfg::SetInt(const char*name, int* ref_, int v)
{
	int ref = -1;
	if (ref_) ref = *ref_;

	if (ref&&ref == -1){
		ref = Ref(name,NULL);
		if (ref_)*ref_ = ref;
	}

	if (ref == -1){
		_cfg_value* vv = new _cfg_value;
		vv->i = v;
		vv->type = cfg_i;
		vv->name = cpstr(name);
		m_vecCfg.push_back(vv);
		ref = m_vecCfg.size() - 1;
		if (ref_)*ref_ = ref;
		return v;
	}

	int r = m_vecCfg[ref]->i;
	m_vecCfg[ref]->i = v;
	return r;
}

bool  LXZCoreCfg::SetBool(const char* name, int* ref_, bool v){
	int ref = -1;
	if (ref_) ref = *ref_;

	if (ref&&ref == -1){
		ref = Ref(name,NULL);
		if (ref_)*ref_ = ref;
	}

	if (ref == -1){
		_cfg_value* vv = new _cfg_value;
		vv->i = v;
		vv->type = cfg_b;
		vv->name = cpstr(name);
		m_vecCfg.push_back(vv);
		ref = m_vecCfg.size() - 1;
		if (ref_)*ref_ = ref;
		return v;
	}

	bool r = m_vecCfg[ref]->b;
	m_vecCfg[ref]->b = v;
	return r;
}

char* LXZCoreCfg::cpstr(const char*s)
{
	char* p = new char[strlen(s)+1];
	strcpy(p, s);
	return p;
}

static bool _isnumber(const char* v){

	int d = 0;
	int c = 0;
	const char* p = v;
	if (*p == 0) return false;

	while (*p != 0){
		if (*p == '.'){
			c++;
			if (c >= 2) return false;
			p++;
			continue;
		}

		if (*p >= '0'&&*p <= '9'){
			p++;
			continue;
		}

		if(*p=='-'||*p=='+'){
			if(d>=1){
				return false;
			}

			d=d+1;
			p++;
			continue;
		}

		return false;
	}
	return true;
}


class CStringParsing{
public:
	CStringParsing(const char* string_, size_t length){
		_length = length;
		_string = string_;
		_position = 0;
		_isoverlow = false;
	}

	~CStringParsing(){
	}

	void  Skip(size_t offset){
		_position += offset;
	}

	float GetFloat(const char* split){
		char buf[64]={0};
		_position+=_substring(buf, 32,split);
		if(!IsOverlow()&&split) _position += strlen(split);
		return (float)atof(buf);
	}

	int   GetInt(const char* split){
		char buf[64]={0};
		_position+=_substring(buf, 32,split);
		if(!IsOverlow()&&split) _position += strlen(split);
		return atoi(buf);
	}

	const char* GetString(char* buf, size_t lenght, const char* split){
		_position+=_substring(buf, lenght,split);
		if(!IsOverlow()&&split) _position += strlen(split);
		return buf;
	}

	bool IsOverlow(){return _isoverlow;}

private:
	size_t _min(size_t x, size_t y){ if (x<y) return x; else return y; }
	size_t _substring(char *buf_, size_t len_, const char* split_){
		if(_position>=_length){
			_isoverlow = true;;
			return 0;
		}

		if(split_==NULL){
			int p=_min(len_-1,_length-_position);
			memcpy(buf_, _string+_position, p);
			buf_[p]=0;
			return p;
		}

		const char* pp = strstr(_string+_position, split_);
		if(pp==NULL){
			int p=_min(len_-1,_length-_position);
			memcpy(buf_, _string+_position, p);
			buf_[p]=0;
			return p;
		}

		size_t p = _min(len_ - 1, pp - (_string + _position));
		memcpy(buf_, _string+_position, p);
		buf_[p]=0;
		return p;
	}

private:
	size_t         _length;
	const char*    _string;
	size_t         _position;
	bool           _isoverlow;
};

bool LXZCoreCfg::load(const char* filename){
	FILE* file = fopen(filename, "rb");
	if (file){
		const int _len = 1024;
		char buf[_len * 2] = {0};
		int offset = 0;

		while (true){
			size_t ret = fread(&buf[offset],1, _len-offset,file);
			if (ret <= 0){
				ret = ferror(file);
				fclose(file);
				file = NULL;
				break;
			}

			printf("%s\n",buf);

			buf[offset+ret] = 0;
			if(((offset+ret+2)<1024)&&buf[offset+ret-1]!='\n'){
				buf[offset+ret] = '\r';
				buf[offset+ret+1] = '\n';
			}
			bool is_unix=false;
			const char* pp = strstr(buf, "\r\n");
			if(pp==NULL){
				pp=strstr(buf,"\n");
				is_unix=true;
			}

			const char* prev = buf;
			while (pp != NULL){
				//
				char name[256] = {0};
				char value[256] = { 0 };
				char line[1024 * 2] = {0};
				CStringParsing parsing(prev, strlen(prev));


				parsing.GetString(line,1023,is_unix?"\n":"\r\n");

				//sscanf(prev, "%s\n", line);

				// skip space.
				char* p=value;
				while(*p!=0&&*p!='\r'&&*p!='\t') p++;

				//
				char* _eq = strstr(line, "=");
				if (_eq != NULL){
					memcpy(name, line, _eq-line);
					memcpy(value, _eq + 1, strlen(_eq + 1));
					char* name_=name;
					while(*name_!=0&&(*name_=='\n'||*name_=='\r'||*name_=='\t')) name_++;


					if (strcmp(p, "true") == 0){
						SetBool(name_, NULL, true);
					}
					else if (strcmp(p, "false") == 0){
						SetBool(name_, NULL, false);
					}
					else if (_isnumber(p)){
						if (strstr(p, ".") != NULL){
							SetFloat(name_, NULL, atof(p));
						}
						else{
							SetInt(name_, NULL, atoi(p));
						}
					}else if(*p=='\"'){
						p++; // skip
						int i=0;
						char* pe=p;
						char v[512]={0};
						while(*pe!=0){
							if(*pe=='\\'&&*(pe+1)=='\"'&&*(pe+1)!=0){
								v[i++]='\"';pe++;
							}
							else if(*pe=='\"'){
								break;
							}
							else{
								v[i++]=*pe;
							}

							pe++;
						}
						*pe=0;
						SetCString(name_, NULL, v);
					}
					else if(*p=='f'&&_isnumber(p+1)){
						p++;
						SetFloat(name_, NULL, atof(p));
					}
					else if(*p=='i'&&_isnumber(p+1)){
						p++;
						SetInt(name_, NULL, atoi(p));
					}
					else if(*p=='u'&&_isnumber(p+1)){
						p++;
						SetInt(name_, NULL, atol(p));
					}else{
						SetCString(name_, NULL, p);
					}
				}

				//
				prev = pp+(is_unix?1:2);

				//
				pp = strstr(prev, is_unix?"\n":"\r\n");
				if (pp == NULL){
					int len = strlen(prev);
					memmove(buf, prev, len);
					offset = len;
					buf[len] = 0;
					break;
				}
				else{							
					offset = prev-buf;
				}
			}				
		}

		//fclose(file);
		//delete file;
		if (file){
			fclose(file);			
		}
		return true;
	}

	return false;
}

const char* LXZCoreCfg::convert(const char* s, char* buf){
	const char* p=s;
	char* d = buf;
	while(*p!=0){
		if(*p=='\"'){
			*d++='\\';
			*d++='\"';
		}else{
			*d++=*p;
		}

		p++;
	}

	*d=0;
	return buf;
}

void LXZCoreCfg::save(const char* filename){
	FILE* file = fopen(filename, "wb");
	if (file){
		char buf[1024] = {0};

		for (int i = 0; i < m_vecCfg.size(); i++){
			_cfg_value* cfg = m_vecCfg[i];
			if (cfg->type == cfg_cstr){
				m_tmpAlloc.grow(512);
				sprintf(buf, "%s=\"%s\"\r\n", cfg->name, convert(cfg->cstr,m_tmpAlloc.buf()));
			}
			else if (cfg->type==cfg_b){
				sprintf(buf, "%s=%s\r\n", cfg->name, cfg->b?"true":"false");
			}
			else if (cfg->type == cfg_f){
				sprintf(buf, "%s=f%f\r\n", cfg->name, cfg->f);
			}
			else if (cfg->type == cfg_i){
				sprintf(buf, "%s=i%d\r\n", cfg->name, cfg->i);
			}
			else if (cfg->type == cfg_u){
				sprintf(buf, "%s=u%u\r\n", cfg->name, cfg->u);
			}

			fwrite(buf, strlen(buf), 1, file);
		}

		fclose(file);
	}
}

bool  LXZCoreCfg::GetSaveFlag(int ref){
	if(ref<0||ref>=m_vecCfg.size()) return false;
	return (m_vecCfg[ref]->flag==0x01);
}

void  LXZCoreCfg::SetSaveFlag(int ref,bool IsCanBeSaved){
	if(ref<0||ref>=m_vecCfg.size()) return;
	m_vecCfg[ref]->flag=IsCanBeSaved?0x01:0x00;
}

const char*  LXZCoreCfg::SetCString(const char* name, int* ref_, const char* v)
{
	int ref = -1;
	if (ref_) ref = *ref_;

	if (ref&&ref == -1){
		ref = Ref(name,NULL);
		if (ref_)*ref_ = ref;
	}

	if (ref == -1){
		_cfg_value* vv = new _cfg_value;
		vv->cstr = cpstr(v);
		vv->type = cfg_cstr;
		vv->name = cpstr(name);
		vv->flag = 0;
		m_vecCfg.push_back(vv);
		ref = m_vecCfg.size() - 1;
		if (ref_)*ref_ = ref;
		return v;
	}

	_cfg_value* cf = m_vecCfg[ref];
	if(strcmp(cf->name,name)!=0&&cf->alias&&strcmp(cf->alias,name)!=0){
		return "";
	}

	char* r = cf->cstr;
	cf->cstr = cpstr(v);
	if (r){
		m_tmpAlloc.grow(strlen(r)+10);		
		strcpy(m_tmpAlloc.buf(), r);
		m_tmpAlloc.setcplen(strlen(r));
		delete []r;
		return m_tmpAlloc.buf();
	}

	return "";
}

extern "C" ILXZCoreCfg* LXZAPI_CreateLXZCfg() {
	LXZCoreCfg* ret = new LXZCoreCfg();
	return ret;
}


#include "LXZMessage.h"
#include <string.h>
#include <stdlib.h>

#ifndef smax
#define smax(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef smin
#define smin(a,b)            (((a) < (b)) ? (a) : (b))
#endif

extern "C" 
{
#include "lua.hpp"
#include "tolua++.h"
}
#include "LXZLock.h"
extern "C" void lae_log(const char* fmt,...);

#ifdef DEBUG
#ifndef __APPLE__
#define CHECK_CLASS(n,name) \
	if(1){tolua_Error tolua_err;\
	if (!tolua_isusertable(pState,n,name,0,&tolua_err))\
{\
	tolua_error(pState,"#ferror in function 'func'.",&tolua_err);\
	return 0;\
}\
	}

#define CHECK_USERTYPE(n,name) \
	if(1){tolua_Error tolua_err;\
	if (!tolua_isusertype(pState,n,name,0,&tolua_err))\
{\
	tolua_error(pState,"#ferror in function 'func'.",&tolua_err);\
	return 0;\
}\
	}

#define CHECK_STRING(n)\
	if(1){tolua_Error tolua_err;\
	if (!tolua_isstring(pState,n,0,&tolua_err))\
{\
	tolua_error(pState,"#ferror in function 'func'.",&tolua_err);\
	return 0;\
}\
	}

#define CHECK_NUMBER(n)\
	if(1){tolua_Error tolua_err;\
	if (!tolua_isnumber(pState,n,0,&tolua_err))\
{\
	tolua_error(pState,"#ferror in function 'func'.",&tolua_err);\
	return 0;\
}\
	}

#define CHECK_BOOL(n)\
	if(1){tolua_Error tolua_err;\
	if (!tolua_isboolean(pState,n,0,&tolua_err))\
{\
	tolua_error(pState,"#ferror in function 'func'.",&tolua_err);\
	return 0;\
}\
	}

#define CHECK_USERDATA(n) \
	if(1){tolua_Error tolua_err;\
	if (!tolua_isuserdata(pState,n,0,&tolua_err))\
{\
	tolua_error(pState,"#ferror in function 'func'.",&tolua_err);\
	return 0;\
}\
	}
#else
#define CHECK_CLASS
#define CHECK_USERTYPE(n,name)
#define CHECK_STRING
#define CHECK_NUMBER
#define CHECK_BOOL
#define CHECK_USERDATA
#endif

#else
#define CHECK_CLASS
#define CHECK_USERTYPE(n,name)
#define CHECK_STRING
#define CHECK_NUMBER
#define CHECK_BOOL
#define CHECK_USERDATA
#endif

//
#define FUNCTION_NAME(b,a) tolua_variable(pState, #b, tolua_get_##a##b,   tolua_set_##a##b);
#define FUNCTION_NEW(a) tolua_new_##a
#define FUNCTION_DEL(a) tolua_delete_##a
#define FUNCTION_GET_STRUCT(a,b)	tolua_variable(pState, #b, tolua_get_##a##b,   tolua_set_##a##b);

#define PROPTY_INT(b, c, d) \
	static int tolua_set_##d##b(lua_State* pState) \
{ \
	CHECK_USERTYPE(1, #d);\
	CHECK_NUMBER(2);\
	d* pEntity = (d*)tolua_tousertype(pState,1,0); \
	pEntity->b = ((c) tolua_tonumber(pState,2,0)); \
	return 0; \
} \
	static int tolua_get_##d##b(lua_State* pState) \
{ \
	CHECK_USERTYPE(1, #d);\
	d* pEntity = (d*)tolua_tousertype(pState,1,0); \
	tolua_pushnumber(pState, (double)pEntity->b);\
	return 1; \
} 

#define PROPTY_INTARRAY(b, c, d) \
	static int tolua_set_##d##b(lua_State* pState) \
{ \
	CHECK_USERTYPE(1, #d);\
	CHECK_NUMBER(2);\
	CHECK_NUMBER(3);\
	d* pEntity = (d*)tolua_tousertype(pState,1,0); \
	int nIndex = (int)tolua_tousertype(pState,2,0);\
	pEntity->b[nIndex] = ((c) tolua_tonumber(pState,3,0)); \
	return 0; \
} \
	static int tolua_get_##d##b(lua_State* pState) \
{ \
	CHECK_USERTYPE(1, #d);\
	CHECK_NUMBER(2);\
	d* pEntity = (d*)tolua_tousertype(pState,1,0); \
	int nIndex = (int)tolua_tousertype(pState,2,0);\
	tolua_pushnumber(pState, (double)pEntity->##b[nIndex]);\
	return 1; \
} 

#define PROPTY_STRING(b, d) \
	static int tolua_set_##d##b(lua_State* pState) \
{ \
	CHECK_USERTYPE(1, #d);\
	CHECK_STRING(2);\
	d* pEntity = (d*)tolua_tousertype(pState,1,0); \
	pEntity->b = ((char* )tolua_tostring(pState,2,0)); \
	return 0; \
} \
	static int tolua_get_##d##b(lua_State* pState) \
{ \
	CHECK_USERTYPE(1, #d);\
	d* pEntity = (d*)tolua_tousertype(pState,1,0); \
	tolua_pushstring(pState, (const char*)pEntity->##b.c_str());\
	return 1; \
} 

#define PROPTY_TYPE(b, c, d) \
	static int tolua_set_##d##b(lua_State* pState) \
{ \
	return 0; \
} \
	static int tolua_get_##d##b(lua_State* pState) \
{ \
	CHECK_USERTYPE(1, #d);\
	d* pD = (d*)tolua_tousertype(pState,1,0); \
	tolua_pushusertype(pState, &(pD->##b), #c);\
	return 1; \
} 

#define PROPTY_BOOL(b, d) \
	static int tolua_set_##d##b(lua_State* pState) \
{ \
	CHECK_USERTYPE(1, #d);\
	CHECK_BOOL(2);\
	d* pD = (d*)tolua_tousertype(pState,1,0); \
	pD->b = (tolua_toboolean(pState,2,0)==1)?true:false; \
	return 0; \
} \
	static int tolua_get_##d##b(lua_State* pState) \
{ \
	CHECK_USERTYPE(1, #d);\
	d* pD = (d*)tolua_tousertype(pState,1,0); \
	tolua_pushboolean(pState, pD->b?1:0);\
	return 1; \
} 

#define STRUCT_NEW(a) \
	static int tolua_new_##a(lua_State* pState) \
{ \
	a* pStruct = new a(); \
	tolua_pushusertype(pState, pStruct, #a); \
	return 1; \
}

#define STRUCT_DELETE(a) \
	static int tolua_delete_##a(lua_State* pState) \
{ \
	tolua_Error tolua_err; \
	if (!tolua_isusertype(pState, 1, #a, 0, &tolua_err)) \
{ \
	tolua_error(pState,"#ferror in function '##a'.",&tolua_err); \
	printf(NULL, "#ferror in function '##a'.", "DEBUG", MB_OK); \
	return 0; \
} \
	a* pStruct = (a*)tolua_tousertype(pState, 1, 0); \
	delete pStruct; \
	pStruct = NULL;\
	return 0; \
}

/* CRC 高位字节值表*/
static unsigned char auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;
/* CRC 低位字节值表*/
static unsigned char auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
0x43, 0x83, 0x41, 0x81, 0x80, 0x40
} ;
/*
CRC 添加到消息中时，低字节先加入，然后高字节。
CRC 简单函数如下：
*/
unsigned short CRC16(unsigned char *puchMsg,unsigned short  usDataLen)
//unsigned char *puchMsg ; /* 要进行CRC 校验的消息*/
//unsigned short usDataLen ; /* 消息中字节数*/  
{

    unsigned char uchCRCHi = 0xFF ; /* 高CRC 字节初始化*/
    unsigned char uchCRCLo = 0xFF ; /* 低CRC 字节初始化*/
    unsigned char uIndex ; /* CRC 循环中的索引*/
    while (usDataLen--) /* 传输消息缓冲区*/
    {
        uIndex = uchCRCHi ^ *puchMsg++ ; /* 计算CRC */
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
        uchCRCLo = auchCRCLo[uIndex] ;
    }
    //return (uchCRCHi << 8 | uchCRCLo) ;
return (uchCRCLo << 8 | uchCRCHi) ;	//修正
}

//发送映射
static unsigned char g_SendByteMap[256] =
{
	0x70,0x2F,0x40,0x5F,0x44,0x8E,0x6E,0x45,0x7E,0xAB,0x2C,0x1F,0xB4,0xAC,0x9D,0x91,
	0x0D,0x36,0x9B,0x0B,0xD4,0xC4,0x39,0x74,0xBF,0x23,0x16,0x14,0x06,0xEB,0x04,0x3E,
	0x12,0x5C,0x8B,0xBC,0x61,0x63,0xF6,0xA5,0xE1,0x65,0xD8,0xF5,0x5A,0x07,0xF0,0x13,
	0xF2,0x20,0x6B,0x4A,0x24,0x59,0x89,0x64,0xD7,0x42,0x6A,0x5E,0x3D,0x0A,0x77,0xE0,
	0x80,0x27,0xB8,0xC5,0x8C,0x0E,0xFA,0x8A,0xD5,0x29,0x56,0x57,0x6C,0x53,0x67,0x41,
	0xE8,0x00,0x1A,0xCE,0x86,0x83,0xB0,0x22,0x28,0x4D,0x3F,0x26,0x46,0x4F,0x6F,0x2B,
	0x72,0x3A,0xF1,0x8D,0x97,0x95,0x49,0x84,0xE5,0xE3,0x79,0x8F,0x51,0x10,0xA8,0x82,
	0xC6,0xDD,0xFF,0xFC,0xE4,0xCF,0xB3,0x09,0x5D,0xEA,0x9C,0x34,0xF9,0x17,0x9F,0xDA,
	0x87,0xF8,0x15,0x05,0x3C,0xD3,0xA4,0x85,0x2E,0xFB,0xEE,0x47,0x3B,0xEF,0x37,0x7F,
	0x93,0xAF,0x69,0x0C,0x71,0x31,0xDE,0x21,0x75,0xA0,0xAA,0xBA,0x7C,0x38,0x02,0xB7,
	0x81,0x01,0xFD,0xE7,0x1D,0xCC,0xCD,0xBD,0x1B,0x7A,0x2A,0xAD,0x66,0xBE,0x55,0x33,
	0x03,0xDB,0x88,0xB2,0x1E,0x4E,0xB9,0xE6,0xC2,0xF7,0xCB,0x7D,0xC9,0x62,0xC3,0xA6,
	0xDC,0xA7,0x50,0xB5,0x4B,0x94,0xC0,0x92,0x4C,0x11,0x5B,0x78,0xD9,0xB1,0xED,0x19,
	0xE9,0xA1,0x1C,0xB6,0x32,0x99,0xA3,0x76,0x9E,0x7B,0x6D,0x9A,0x30,0xD6,0xA9,0x25,
	0xC7,0xAE,0x96,0x35,0xD0,0xBB,0xD2,0xC8,0xA2,0x08,0xF3,0xD1,0x73,0xF4,0x48,0x2D,
	0x90,0xCA,0xE2,0x58,0xC1,0x18,0x52,0xFE,0xDF,0x68,0x98,0x54,0xEC,0x60,0x43,0x0F
};

//接收映射
static unsigned char g_RecvByteMap[256] =
{
	0x51,0xA1,0x9E,0xB0,0x1E,0x83,0x1C,0x2D,0xE9,0x77,0x3D,0x13,0x93,0x10,0x45,0xFF,
	0x6D,0xC9,0x20,0x2F,0x1B,0x82,0x1A,0x7D,0xF5,0xCF,0x52,0xA8,0xD2,0xA4,0xB4,0x0B,
	0x31,0x97,0x57,0x19,0x34,0xDF,0x5B,0x41,0x58,0x49,0xAA,0x5F,0x0A,0xEF,0x88,0x01,
	0xDC,0x95,0xD4,0xAF,0x7B,0xE3,0x11,0x8E,0x9D,0x16,0x61,0x8C,0x84,0x3C,0x1F,0x5A,
	0x02,0x4F,0x39,0xFE,0x04,0x07,0x5C,0x8B,0xEE,0x66,0x33,0xC4,0xC8,0x59,0xB5,0x5D,
	0xC2,0x6C,0xF6,0x4D,0xFB,0xAE,0x4A,0x4B,0xF3,0x35,0x2C,0xCA,0x21,0x78,0x3B,0x03,
	0xFD,0x24,0xBD,0x25,0x37,0x29,0xAC,0x4E,0xF9,0x92,0x3A,0x32,0x4C,0xDA,0x06,0x5E,
	0x00,0x94,0x60,0xEC,0x17,0x98,0xD7,0x3E,0xCB,0x6A,0xA9,0xD9,0x9C,0xBB,0x08,0x8F,
	0x40,0xA0,0x6F,0x55,0x67,0x87,0x54,0x80,0xB2,0x36,0x47,0x22,0x44,0x63,0x05,0x6B,
	0xF0,0x0F,0xC7,0x90,0xC5,0x65,0xE2,0x64,0xFA,0xD5,0xDB,0x12,0x7A,0x0E,0xD8,0x7E,
	0x99,0xD1,0xE8,0xD6,0x86,0x27,0xBF,0xC1,0x6E,0xDE,0x9A,0x09,0x0D,0xAB,0xE1,0x91,
	0x56,0xCD,0xB3,0x76,0x0C,0xC3,0xD3,0x9F,0x42,0xB6,0x9B,0xE5,0x23,0xA7,0xAD,0x18,
	0xC6,0xF4,0xB8,0xBE,0x15,0x43,0x70,0xE0,0xE7,0xBC,0xF1,0xBA,0xA5,0xA6,0x53,0x75,
	0xE4,0xEB,0xE6,0x85,0x14,0x48,0xDD,0x38,0x2A,0xCC,0x7F,0xB1,0xC0,0x71,0x96,0xF8,
	0x3F,0x28,0xF2,0x69,0x74,0x68,0xB7,0xA3,0x50,0xD0,0x79,0x1D,0xFC,0xCE,0x8A,0x8D,
	0x2E,0x62,0x30,0xEA,0xED,0x2B,0x26,0xB9,0x81,0x7C,0x46,0x89,0x73,0xA2,0xF7,0x72
};


static int LuaMessage_IsReading(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	uiboolean IsRead = pMsg->IsReading();
	tolua_pushboolean(pState, IsRead);
	return 1;
}

static int LuaMessage_IsWriting(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	uiboolean IsWrite = pMsg->IsWriting();
	tolua_pushboolean(pState, IsWrite);
	return 1;
}

static int LuaMessage_Serial_uiboolean(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if(pMsg->IsReading()==uifalse)
	{
		CHECK_NUMBER(2);
		int v = tolua_tonumber(pState, 2, 0);
		uiboolean b = (uiboolean)v;
		pMsg->Serial(b);
		tolua_pushnumber(pState, b);
	}
	else
	{
		uiboolean v;
		pMsg->Serial(v);
		tolua_pushnumber(pState, v);
	}	

	return 1;	
}

static int LuaMessage_Serial_bool(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if(pMsg->IsReading()==uifalse)
	{
		CHECK_NUMBER(2);
		int v = tolua_toboolean(pState, 2, 0);
		bool b = (v==1);
		pMsg->Serial(b);
		tolua_pushboolean(pState, v);
	}
	else
	{
		bool v;
		pMsg->Serial(v);
		tolua_pushboolean(pState, v?1:0);
	}	

	return 1;	
}

static int LuaMessage_Serial_int(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if(pMsg->IsReading()==uifalse)
	{
		CHECK_NUMBER(2);
		int v = tolua_tonumber(pState, 2, 0);
		pMsg->Serial(v);
		tolua_pushnumber(pState, v);
	}
	else
	{
		int v;
		pMsg->Serial(v);
		tolua_pushnumber(pState, v);
	}	

	return 1;	
}

/*
static int LuaMessage_Serial_long(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if(pMsg->IsReading()==uifalse)
	{
		CHECK_NUMBER(2);
		long v = (long)tolua_tonumber(pState, 2, 0);		
		pMsg->Serial(v);
		tolua_pushnumber(pState, v);
	}
	else
	{
		long v;
		pMsg->Serial(v);
		tolua_pushnumber(pState, v);
	}	

	return 1;	
}*/

static int LuaMessage_Serial_sint8(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if(pMsg->IsReading()==uifalse)
	{
		CHECK_NUMBER(2);
		LXZsint8 v = (LXZsint8)tolua_tonumber(pState, 2, 0);		
		pMsg->Serial(v);
		tolua_pushnumber(pState, (int)v);
	}
	else
	{
		LXZsint8 v;
		pMsg->Serial(v);
		tolua_pushnumber(pState, (int)v);
	}	

	return 1;	
}

static int LuaMessage_Serial_uint8(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if(pMsg->IsReading()==uifalse)
	{
		CHECK_NUMBER(2);
		LXZuint8 v = (LXZuint8)tolua_tonumber(pState, 2, 0);		
		pMsg->Serial(v);
		tolua_pushnumber(pState, (int)v);
	}
	else
	{
		LXZuint8 v;
		pMsg->Serial(v);
		tolua_pushnumber(pState, (int)v);
	}	

	return 1;	
}

static int LuaMessage_Serial_sint16(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if(pMsg->IsReading()==uifalse)
	{
		CHECK_NUMBER(2);
		LXZsint16 v = (LXZsint16)tolua_tonumber(pState, 2, 0);		
		pMsg->Serial(v);
		tolua_pushnumber(pState, (int)v);
	}
	else
	{
		LXZsint16 v;
		pMsg->Serial(v);
		tolua_pushnumber(pState, (int)v);
	}	

	return 1;	
}

static int LuaMessage_Serial_uint16(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if(pMsg->IsReading()==uifalse)
	{
		CHECK_NUMBER(2);
		LXZuint16 v = (LXZuint16)tolua_tonumber(pState, 2, 0);		
		pMsg->Serial(v);
		tolua_pushnumber(pState, (int)v);
	}
	else
	{
		LXZuint16 v;
		pMsg->Serial(v);
		tolua_pushnumber(pState, (int)v);
	}	

	return 1;	
}

static int LuaMessage_Serial_uint32(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if(pMsg->IsReading()==uifalse)
	{
		CHECK_NUMBER(2);
		LXZuint32 v = (LXZuint32)tolua_tonumber(pState, 2, 0);		
		pMsg->Serial(v);
		tolua_pushnumber(pState, (int)v);
	}
	else
	{
		LXZuint32 v;
		pMsg->Serial(v);
		tolua_pushnumber(pState, (int)v);
	}	

	return 1;	
}


static int LuaMessage_Serial_uint64(lua_State* pState)
{
	CHECK_USERTYPE(1, "CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if (pMsg->IsReading() == uifalse)
	{		
		const char* vs = (const char*)tolua_tostring(pState, 2, "");
		LXZuint64 v = atoll(vs);
		pMsg->Serial(v);
		return 0;
	}
	else
	{
		LXZuint64 v;
		pMsg->Serial(v);
		char buf[64];
		sprintf(buf, "%I64u", v);
		tolua_pushstring(pState, buf);
	}

	return 1;
}


static int LuaMessage_Serial_int64(lua_State* pState)
{
	CHECK_USERTYPE(1, "CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if (pMsg->IsReading() == uifalse)
	{
		const char* vs = (const char*)tolua_tostring(pState, 2, "");
		LXZuint64 v = atoll(vs);
		pMsg->Serial(v);
		return 0;
	}
	else
	{
		LXZuint64 v;
		pMsg->Serial(v);
		char buf[64];
		sprintf(buf, "%I64d", v);
		tolua_pushstring(pState, buf);
	}

	return 1;
}

static int LuaMessage_Serial_float(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if(pMsg->IsReading()==uifalse)
	{
		CHECK_NUMBER(2);
		float v = (float)tolua_tonumber(pState, 2, 0);		
		pMsg->Serial(v);
		tolua_pushnumber(pState, v);
	}
	else
	{
		float v;
		pMsg->Serial(v);
		tolua_pushnumber(pState, v);
	}	

	return 1;	
}



static int LuaMessage_Serial_cstring(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if(pMsg->IsReading()==uifalse)
	{
		CHECK_STRING(2);
		char* v = (char*)tolua_tostring(pState, 2, 0);	
		int len = strlen(v);
		if (len > 255) len = 255;
		pMsg->SerialStr(v, len);
	}
	else
	{
		char buf[256];
		const char* str = pMsg->SerialStr(buf, 255);
		tolua_pushstring(pState, str);
		return 1;
	}	

	return 0;	
}


static int LuaMessage_Serial_cstring_len(lua_State* pState)
{
	CHECK_USERTYPE(1, "CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if (pMsg->IsReading() == uifalse)
	{
		CHECK_STRING(2);
		size_t count = 0;
		const char* v = (const char*)luaL_checklstring(pState, 2, &count);
		size_t size = (size_t)tolua_tonumber(pState, 3, 0);

		char* buf = new char[size];
		memset(buf, 0, size);
		memcpy(buf, v, smin(count, size));
		pMsg->Serial(buf, size);
		delete[]buf;
		return 0;
	}
	else
	{
		size_t size = (size_t)tolua_tonumber(pState, 2, 0);
		char* buf = new char[size + 1];
		memset(buf, 0, size + 1);
		pMsg->Serial(buf, size);
		tolua_pushstring(pState, buf);
		delete[]buf;
	}

	return 1;
}


static int LuaMessage_Serial_string(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if(pMsg->IsReading()==uifalse)
	{
		CHECK_STRING(2);
		const char* v = (const char*)tolua_tostring(pState, 2, 0);	
		int len = strlen(v);

		char* buf = new char[len+1];
		memset(buf, 0, len+1);
		strcpy(buf, v);

		pMsg->Serial(len);
		pMsg->Serial(buf, len, len);
		delete []buf;
		return 0;	
	}
	else
	{
		int len;
		pMsg->Serial(len);

		char* buf = new char[len+1];
		memset(buf, 0, len+1);				
		pMsg->Serial(buf, len, len);

		tolua_pushstring(pState, buf);

		delete []buf;
	}	

	return 1;	
}

//only for write
static int LuaMessage_Serial_Write(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	if(pMsg->IsReading()==uifalse){
		size_t count=0;		
		tolua_Error tolua_err;
		if (tolua_isnumber(pState, 3, 0,&tolua_err)){
			const char* v = (const char*)luaL_checklstring(pState, 2, &count);	
			int len = (int)tolua_tonumber(pState, 3, 0);
			pMsg->Write(v, len);				
		}else{
			const char* v = (const char*)luaL_checklstring(pState, 2, &count);	
			int len = count;
			pMsg->Write(v, len);				
		}
	}

	return 0;
}

static int LuaMessage_Serial_lstring(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if(pMsg->IsReading()==uifalse)
	{
		CHECK_STRING(2);
		CHECK_NUMBER(3);
		size_t count=0;		
		const char* v = (const char*)lua_tolstring(pState, 2, &count);	
		int type = tolua_tonumber(pState, 3, 1);
		char *buf = new char[count+1];
		memcpy(buf, v, count);
		buf[count] = 0x00;
		int len = count;
		if(type==1){
			LXZuint8 u8=len;
			pMsg->Serial(u8);			
		}else if(type==2){
			LXZuint16 u16=len;
			pMsg->Serial(u16);			
		}else{
		//	LXZuint32 u32=len;
		//	pMsg->Serial(u32);
			len = type;
		}

		pMsg->Serial(buf, len);		
		delete []buf;
		return 0;
	}
	else
	{
		CHECK_NUMBER(2);
		int type = tolua_tonumber(pState, 2, 1);
		int len;
		//pMsg->Serial(len);
		if(type==1){
			LXZuint8 u8;
			pMsg->Serial(u8);
			len = u8;
		}else if(type==2){
			LXZuint16 u16;
			pMsg->Serial(u16);
			len = u16;
		}else{
		//	LXZuint32 u32;
		//	pMsg->Serial(u32);
		//	len = u32;
			len = type;
		}

		char* buf = new char[len+1];
		memset(buf, 0, len+1);				
		pMsg->Serial(buf, len);
		lua_pushlstring(pState, buf, len);
		delete []buf;
		return 1;
	}	

	return 0;	
}

static int LuaMessage_Serial_Ptr(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);

	if(pMsg->IsReading()==uifalse)
	{
		CHECK_STRING(2);
		const char* name = tolua_tostring(pState, 2, "");

		CHECK_USERTYPE(3, name);
		void* pObj = tolua_tousertype(pState, 3, 0);
		pMsg->SerialPtr(pObj, name);	
		tolua_pushusertype(pState, pObj, name);
	}
	else
	{
		CHECK_STRING(2);
		const char* name = tolua_tostring(pState, 2, "");

		void* pObj = NULL;
		pMsg->SerialPtr(pObj, name);

		tolua_pushusertype(pState, pObj, name);
	}	

	return 1;		
}

static int LuaMessage_getExecuteCount(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	tolua_pushnumber(pState, pMsg->getExecuteCount());
	return 1;
}

static int LuaMessage_ReleaseMemory(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	pMsg->ReleaseMemory();
	return 0;
}

static int LuaMessage_removeHeadBuffer(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	size_t size = (size_t)tolua_tonumber(pState, 2, 0);	
	pMsg->removeHeadBuffer(size);	
	return 0;
}


static int LuaMessage_getMode(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	tolua_pushnumber(pState, pMsg->getMode());

	return 1;
}

static int LuaMessage_setMode(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	CHECK_NUMBER(2);
	int mode = tolua_tonumber(pState, 2, 0);
	pMsg->setMode(mode);
	return 0;
}

static int LuaMessage_setLittleEndian(lua_State* pState)
{
	CHECK_USERTYPE(1, "CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	CHECK_NUMBER(2);
	int mode = tolua_toboolean(pState, 2, 0);
	pMsg->SetLittleEndian(mode);
	return 0;
}

static int LuaMessage_getSystemData(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	lua_pushnumber(pState, pMsg->getSystemData());

	return 1;
}

static int LuaMessage_getData(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	tolua_pushnumber(pState, pMsg->getData());

	return 1;
}

static int LuaMessage_getAppData(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	lua_pushlightuserdata(pState, pMsg->getAppData());

	return 1;
}

static int LuaMessage_getMsgPtr(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	lua_pushlstring(pState, (const char*)pMsg->getMsgPtr(), pMsg->getMsgSize());
	return 1;
}

static int LuaMessage_setResult(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	CLXZMessage* pResult = (CLXZMessage*)tolua_tousertype(pState, 2, 0);
	pMsg->setResult(pResult);
	return 0;
}

static int LuaMessage_getResult(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	CLXZMessage* pResult = (CLXZMessage*)pMsg->getResult();
	tolua_pushusertype(pState, pResult, "CLXZMessage");	
	return 1;
}

static int LuaMessage_getIndex(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	int index = pMsg->getIndex();
	tolua_pushnumber(pState, index);
	return 1;
}



static int LuaMessage_getMsgSize(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	tolua_pushnumber(pState, pMsg->getMsgSize());
	return 1;
}

static int LuaMessage_getMsgPos(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	tolua_pushnumber(pState, pMsg->getPos());
	return 1;
}

static int LuaMessage_setMsgPos(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CHECK_NUMBER(2);
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);	
	LXZuint32 pos = (LXZuint32)tolua_tonumber(pState, 2, 0);
	pMsg->setPos(pos);
	return 0;
}

static int LuaMessage_SetWebSocketFrame(lua_State* pState)
{
	CHECK_USERTYPE(1, "CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	LXZuint8 frameType = (LXZuint8)tolua_tonumber(pState, 2, 0);
	size_t count = 0;
	const unsigned char* s = (const unsigned char*)luaL_checklstring(pState, 3, &count);
	pMsg->SetWebSocketFrame((WebSocketFrameType)frameType, s, count);
	return 0;
}

static int LuaMessage_GetWebSocketFrame(lua_State* pState)
{
	CHECK_USERTYPE(1, "CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);	
	size_t count = 0;
	const unsigned char* s = (const unsigned char*)luaL_checklstring(pState, 2, &count);
	int out_len = 0;
	LXZuint32 ret = (LXZuint32)pMsg->GetWebSocketFrame(s, count, out_len);
	tolua_pushnumber(pState, ret);
	tolua_pushnumber(pState, out_len);
	return 2;
}

static int LuaMessage_CRC16(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CHECK_NUMBER(2);
	CHECK_NUMBER(2);
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);	
	LXZuint32 from    = (LXZuint32)tolua_tonumber(pState, 2, 0);
	LXZuint32 len     = (LXZuint32)tolua_tonumber(pState, 3, 0);	
	LXZuint16 sum = CRC16((unsigned char*)pMsg->getMsgPtr()+from, len);
	tolua_pushnumber(pState, sum);
	return 1;
}

static int LuaMessage_bitop(lua_State* pState)
{
	CHECK_USERTYPE(1, "CLXZMessage");
	CHECK_NUMBER(2);
	CHECK_NUMBER(3);
	CHECK_NUMBER(4);
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	LXZuint8 src  = (LXZuint32)tolua_tonumber(pState, 2, 0);
	LXZuint8 mask = (LXZuint32)tolua_tonumber(pState, 3, 0);
	const char* op = tolua_tostring(pState, 4, "A");
	if(op[0]== '^')	tolua_pushnumber(pState, (LXZuint8)(src^mask));
	if (op[0] == '&')	tolua_pushnumber(pState, (LXZuint8)(src&mask));
	if (op[0] == '~')	tolua_pushnumber(pState, (LXZuint8)(~src));
	if (op[0] == '|')	tolua_pushnumber(pState, (LXZuint8)(src|mask));
	return 1;
}

static int LuaMessage_MapKind(lua_State* pState)
{
	CHECK_USERTYPE(1, "CLXZMessage");
	CHECK_NUMBER(2);
	CHECK_NUMBER(3);
	CHECK_BOOL(4);
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	LXZuint32 from = (LXZuint32)tolua_tonumber(pState, 2, 0);
	LXZuint32 len = (LXZuint32)tolua_tonumber(pState, 3, 0);
	LXZuint32 mode = (LXZuint32)tolua_toboolean(pState, 4, 0);

	unsigned char* pMap = (mode == 0) ? g_SendByteMap : g_RecvByteMap;
	LXZuint8* pBuffer = pMsg->getMsgPtr();
	for (size_t i = 0; i < len; i++) {
		pBuffer[i+from] = pMap[pBuffer[i+from]];
	}
		
	return 0;
}

static int LuaMessage_setData(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	CHECK_NUMBER(2);
	LXZuint32 mode = (LXZuint32)tolua_tonumber(pState, 2, 0);
	pMsg->setData(mode);
	return 0;
}

static int LuaMessage_setIndexPos(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	CHECK_NUMBER(2);
	int index = (int)tolua_tonumber(pState, 2, 0);
	pMsg->setIndexPos(index);
	return 0;
}


static int LuaMessage_PushAddress(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	pMsg->PushAddress();	
	return 0;
}

static int LuaMessage_ResetAddress(lua_State* pState)
{
	CHECK_USERTYPE(1,"CLXZMessage");
	CLXZMessage* pMsg = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	pMsg->ResetAddress();	
	return 0;
}

#include<list>
static std::list<CLXZMessage*> s_pool;
static Linux_Win_Lock s_pool_lock;
static int LuaMessage_Serial_new(lua_State* pState)
{
	s_pool_lock.Linux_Win_Locked();
	if (s_pool.size() == 0){
		CLXZMessage* pMsg = new CLXZMessage();
		s_pool.push_back(pMsg);
	}

	CLXZMessage* pMsg = s_pool.front();
	s_pool.pop_front();
	s_pool_lock.Linux_Win_UnLocked();

	pMsg->ReleaseMemory();
	pMsg->setMode(SM_WRITE);
	tolua_pushusertype(pState, pMsg, "CLXZMessage");
	return 1;
}


static int LuaMessage_Serial_new_local(lua_State* pState)
{
	s_pool_lock.Linux_Win_Locked();
	if (s_pool.size() == 0){
		CLXZMessage* pMsg = new CLXZMessage();
		s_pool.push_back(pMsg);
	}

	CLXZMessage* pMsg = s_pool.front();
	s_pool.pop_front();
	s_pool_lock.Linux_Win_UnLocked();

	pMsg->ReleaseMemory();
	pMsg->setMode(SM_WRITE);
	tolua_pushusertype_and_takeownership(pState, pMsg, "CLXZMessage");
	return 1;
}

static int LuaLXZCLXZMessage_collect(lua_State* pState)
{
	CHECK_USERTYPE(1, "CLXZMessage");
	CLXZMessage* pt1 = (CLXZMessage*) tolua_tousertype(pState, 1, 0);
	//lae_log("message destroy:%u\n", pt1->getTime());
	//delete pt1;
	//pt1->ReleaseMemory();
	s_pool_lock.Linux_Win_Locked();
	s_pool.push_back(pt1);	
	if (s_pool.size() >= 10) {
		CLXZMessage* p= s_pool.front();
		s_pool.pop_front();
		delete p;
	}
	s_pool_lock.Linux_Win_UnLocked();
	return 0;
}

static int LuaMessage_grown(lua_State* pState)
{
	CLXZMessage* pt1 = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	size_t size = (size_t)tolua_tonumber(pState, 2, 0);
	pt1->grown(size);
	return 0;
}

static int LuaMessage_topointer(lua_State* L)
{
	CLXZMessage* pt1 = (CLXZMessage*)tolua_tousertype(L, 1, 0);
	lua_pushlightuserdata(L, pt1);
	return 1;
}

static int LuaMessage_tohex(lua_State* pState)
{
	CLXZMessage* pt1 = (CLXZMessage*)tolua_tousertype(pState, 1, 0);
	FastBufAlloc<char, 128> buf;
	buf.grow(pt1->getMsgSize() * 2 + 1);
	lua_pushlstring(pState, pt1->tohex(buf.buf(), pt1->getMsgSize()*2), pt1->getMsgSize() * 2);
	return 1;
}

int LuaLXZMessageInitial(lua_State* pState)
{
	tolua_open(pState);
    tolua_module(pState,NULL,0);
    tolua_beginmodule(pState,NULL);
	
	tolua_constant(pState, "SM_READ", SM_READ);
	tolua_constant(pState, "SM_WRITE", SM_WRITE);
	
	tolua_usertype(pState, "CLXZMessage");
	tolua_cclass(pState, "CLXZMessage", "CLXZMessage", "", LuaLXZCLXZMessage_collect);
	tolua_beginmodule(pState, "CLXZMessage");
	tolua_function(pState, "delete",         LuaLXZCLXZMessage_collect);
	tolua_function(pState, "new",         LuaMessage_Serial_new);
	tolua_function(pState, "new_local",   LuaMessage_Serial_new_local);
	tolua_function(pState, "uiboolean", LuaMessage_Serial_uiboolean);
	tolua_function(pState, "bool", LuaMessage_Serial_bool);
	tolua_function(pState, "int", LuaMessage_Serial_int);
	//tolua_function(pState, "long", LuaMessage_Serial_long);
	tolua_function(pState, "sint8", LuaMessage_Serial_sint8);
	tolua_function(pState, "uint8", LuaMessage_Serial_uint8);
	tolua_function(pState, "sint16", LuaMessage_Serial_sint16);
	tolua_function(pState, "uint16", LuaMessage_Serial_uint16);
	tolua_function(pState, "uint32", LuaMessage_Serial_uint32);
	tolua_function(pState, "uint64", LuaMessage_Serial_uint64);	
	tolua_function(pState, "int64", LuaMessage_Serial_int64);
	tolua_function(pState, "float", LuaMessage_Serial_float);
	tolua_function(pState, "string", LuaMessage_Serial_string);
	tolua_function(pState, "lstring", LuaMessage_Serial_lstring);
	tolua_function(pState, "cstring", LuaMessage_Serial_cstring);
	tolua_function(pState, "cstring_len", LuaMessage_Serial_cstring_len);	
	tolua_function(pState, "getMode", LuaMessage_getMode);
	tolua_function(pState, "setMode", LuaMessage_setMode);;
	tolua_function(pState, "Write",       LuaMessage_Serial_Write);
	tolua_function(pState, "Ptr",         LuaMessage_Serial_Ptr);
	//tolua_function(pState, "getMode",     LuaMessage_getMode);
	tolua_function(pState, "setLittleEndian",     LuaMessage_setLittleEndian);
	tolua_function(pState, "PushAddress",   LuaMessage_PushAddress);
	tolua_function(pState, "ResetAddress",  LuaMessage_ResetAddress);	
	tolua_function(pState, "getSystemData",     LuaMessage_getSystemData);
	tolua_function(pState, "getData",     LuaMessage_getData);
	tolua_function(pState, "setData",     LuaMessage_setData);	
	tolua_function(pState, "getMsgPtr",   LuaMessage_getMsgPtr);
	tolua_function(pState, "getMsgSize",   LuaMessage_getMsgSize);
	tolua_function(pState, "getMsgPos",   LuaMessage_getMsgPos);
	tolua_function(pState, "setMsgPos",   LuaMessage_setMsgPos);
	tolua_function(pState, "CRC16",       LuaMessage_CRC16);	
	tolua_function(pState, "getResult",       LuaMessage_getResult);		
	tolua_function(pState, "getIndex",       LuaMessage_getIndex);
	tolua_function(pState, "setIndexPos",       LuaMessage_setIndexPos);
	tolua_function(pState, "setResult",       LuaMessage_setResult);	
	tolua_function(pState, "getExecuteCount",       LuaMessage_getExecuteCount);
	tolua_function(pState, "ReleaseMemory",       LuaMessage_ReleaseMemory);
	tolua_function(pState, "removeHeadBuffer",       LuaMessage_removeHeadBuffer);
	tolua_function(pState, "MapKind", LuaMessage_MapKind);	
	tolua_function(pState, "bitop", LuaMessage_bitop);	
	tolua_function(pState, "SetWebSocketFrame", LuaMessage_SetWebSocketFrame);
	tolua_function(pState, "GetWebSocketFrame", LuaMessage_GetWebSocketFrame);	
	tolua_function(pState, "grown", LuaMessage_grown);
	tolua_function(pState, "tohex", LuaMessage_tohex);
	tolua_function(pState, "topointer", LuaMessage_topointer);
	tolua_endmodule(pState);


	tolua_endmodule(pState);

	return 0;
}
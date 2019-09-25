
#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "IArchive.h"
#include "IMemFile.h"

#define LXZSERIAL_STRUCT(ar) Serial(CIArchive& ar)\
{\
if(ar.IsReading())\
{\
	CLXZMessage msg(512);\
	msg.SerialMsg(ar);msg.setMode(SM_READ);\
	HelpSerial(msg);\
}\
else\
{\
	CLXZMessage msg(512);		\
	HelpSerial(msg);\
	msg.SerialMsg(ar);\
}\
}


enum WebSocketFrameType {
	ERROR_FRAME = 0xFF00,
	INCOMPLETE_FRAME = 0xFE00,

	OPENING_FRAME = 0x3300,
	CLOSING_FRAME = 0x3400,

	INCOMPLETE_TEXT_FRAME = 0x01,
	INCOMPLETE_BINARY_FRAME = 0x02,

	TEXT_FRAME = 0x81,
	BINARY_FRAME = 0x82,

	PING_FRAME = 0x19,
	PONG_FRAME = 0x1A
};

class DLL_CLS CLXZMessage : public CIArchive
{
public:
	enum
	{
		EVENT_CONNECT = 0,
		EVENT_DISCONNECT,
		EVENT_READ,
		EVENT_WRITE,
	};

	CLXZMessage(int size = 16);
	CLXZMessage(const char* strHead);

	~CLXZMessage();
	
	CLXZMessage& operator= (CLXZMessage &other);
		
	void SerialMsg(CIArchive& ar, bool bEncrypt = false);

	void setBufferSize(int nSize);
	int  getBufferSize();

	int   getMsgSize();
		
	unsigned char* getMsgPtr();
	void  setMode(int nMode);
	
	//void Overlow(int nSize);
	const unsigned char*  grown(size_t size);
	const char* tohex(char* buf, size_t buf_size);

	void   setData(LXZuint32 u4Data);
	LXZuint32 getData();

	void setTime(LXZuint32 timer) { m_u4Timer = timer; }
	LXZuint32 getTime()           { return m_u4Timer; }

	void setEvent(int nEvent)      { m_nEvent = nEvent; }
	int  getEvent()                { return m_nEvent;   }

	void setSockId(int SockId)      { m_SockId = SockId; }
	int  getSockId()                { return m_SockId;   }

	void   setAppData(void* pAppData)      { m_pAppData = pAppData; }
	void*  getAppData()                { return m_pAppData;   }

	void   setAppData1(void* pAppData)      { m_pAppData1 = pAppData; }
	void*  getAppData1()                { return m_pAppData1; }

	void rollback(int offset) { m_IMemStream.SetPos(m_IMemStream.GetPos()-offset);}

	//
	void ReleaseMemory(int nReverSize=16) { m_IMemStream.Release(nReverSize); }
	void removeHeadBuffer(size_t size){ m_IMemStream.removeHeadBuffer(size);}

	//
	int GetLuaRef() { return m_nLuaRef; }
	void SetLuaRef(int ref) { m_nLuaRef = ref;}

	//
	void setSystemData(LXZuint32 data) { m_u4SysData = data; }
	LXZuint32 getSystemData() { return m_u4SysData; }



	//websocket
	int SetWebSocketFrame(WebSocketFrameType frame_type, const unsigned char* msg, int msg_len);
	WebSocketFrameType GetWebSocketFrame(const unsigned char* in_buffer, int in_length, int& out_len);

public:
	static LXZuint32 msg_id;
private:	
	LXZuint32 m_u4Test;
	LXZuint32 m_u4Data;    // user data
	LXZuint32 m_u4SysData; // engine data, for timer handle;
	LXZuint32 m_u4Timer;
	int       m_nEvent;
	LXZuint32 m_SockId;
	void*     m_pAppData;	
	void*     m_pAppData1;
	CIMemFile  m_IMemStream;
	int m_nLuaRef;
};

#endif

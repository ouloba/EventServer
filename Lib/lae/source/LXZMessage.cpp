




#include "LXZMessage.h"
#include <string.h>
extern "C" void lae_log(const char* fmt,...);
LXZuint32 CLXZMessage::msg_id=0;
CLXZMessage::CLXZMessage(int size/* = 512*/)
{
	m_u4Test=123456;
	m_u4Timer = msg_id++;
	m_nEvent = -1;
	m_SockId = 0;
	m_pAppData = NULL;	
	m_u4Data = 0;
	m_u4SysData = 0;
	m_nLuaRef = -1;	
//	setMode(SM_WRITE);
//	m_IMemStream.SetPos(0);
//	m_pISerial   = (CISerial*)&m_IMemStream;
//	m_SerialFunc = (SERIAL_FUNC)&CIMemFile::Write;
//	m_nMode      = SM_WRITE;	
	//lae_log("message new:%u %u\n", m_u4Timer,msg_id);
}

CLXZMessage::CLXZMessage(const char* strHead)
{
	m_u4Test=123456;
	m_u4Timer = 0;
	m_nEvent = 0;
	m_SockId = 0;
	m_pAppData = NULL;	
	m_u4Data = 0;

	m_nLuaRef = -1;

	setMode(SM_WRITE);
	Serial(const_cast<char*>(strHead), strlen(strHead), strlen(strHead));
}

CLXZMessage::~CLXZMessage()
{
	//lae_log("message delete:%u\n", m_u4Timer);
	//CIArchive::~CIArchive();
}

CLXZMessage& CLXZMessage::operator= (CLXZMessage &other)
{
   m_u4Data   = other.m_u4Data;

/*
   char* pszBuffer  = m_pszBuffer;
   int   nBufferLen = m_nBufferLen;

   m_pszBuffer = other.m_pszBuffer;
   m_nBufferLen= other.m_nBufferLen;

   other.m_pszBuffer = pszBuffer;
   other.m_nBufferLen = nBufferLen;
   
   setBufferSize(m_nBufferLen);*/

   setMode(SM_WRITE);
   Serial(other.getMsgPtr(), other.getMsgSize());
   setMode(other.getMode());

   return *this;
}

int   CLXZMessage::getBufferSize(){ return m_IMemStream.GetBufferLen(); }
int   CLXZMessage::getMsgSize()  
{
	return m_IMemStream.GetValidCount();
}

	
char* CLXZMessage::getMsgPtr()    { return m_IMemStream.GetBuffer(); }
void  CLXZMessage::setMode(int nMode)
{ 
	CIArchive::Initial(&m_IMemStream, nMode);
}

void   CLXZMessage::setData(LXZuint32 u4Data) { m_u4Data = u4Data;}
LXZuint32 CLXZMessage::getData()              { return m_u4Data;  }


void CLXZMessage::SerialMsg(CIArchive& ar, bool bEncrypt/* = false*/)
{
	setVersion(ar.getVersion());

	if(ar.IsReading())
	{
		// length
		int len=0;
		ar.Serial(len);
			
		if(len>=0&&len<104857600)
		{	
			// content
			if(len>0)
			{
				char* buf = new char[len];
				ar.Serial(buf, len, len);				
				CIArchive::setMode(SM_WRITE);
				Serial(buf, len);
				delete []buf;
			}						
		}
		else
		{
			m_IMemStream.Read(NULL, 0x0FFFFFFF);
		}	

		CIArchive::setMode(SM_READ);		
	}
	else
	{
		int len = getMsgSize();
		ar.Serial(len);
		if(len>0)
		{
			ar.Serial(getMsgPtr(), len, len);
		}			
	}
}








#include "LXZMessage.h"
#include <string.h>
#include <stdio.h>

extern "C" void lae_log(const char* fmt,...);
LXZuint32 CLXZMessage::msg_id=0;
CLXZMessage::CLXZMessage(int size/* = 512*/)
{
	m_u4Test=123456;
	m_u4Timer = msg_id++;
	m_nEvent = -1;
	m_SockId = 0;
	m_pAppData = NULL;	
	m_pAppData1 = NULL;
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
	m_u4Test = 111111;
	msg_id--;
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
   Serial((char*)other.getMsgPtr(), other.getMsgSize());
   setMode(other.getMode());

   return *this;
}

int   CLXZMessage::getBufferSize(){ return m_IMemStream.GetBufferLen(); }
int   CLXZMessage::getMsgSize()  
{
	return m_IMemStream.GetValidCount();
}

	
unsigned char* CLXZMessage::getMsgPtr()  
{
	return m_IMemStream.GetBuffer(); 
}

void  CLXZMessage::setMode(int nMode)
{ 

	CIArchive::Initial(&m_IMemStream, nMode);
}

void   CLXZMessage::setData(LXZuint32 u4Data) { m_u4Data = u4Data;}
LXZuint32 CLXZMessage::getData()              { return m_u4Data;  }

const unsigned char* CLXZMessage::grown(size_t size){
	return m_IMemStream.grown(size);
}

int CLXZMessage::SetWebSocketFrame(WebSocketFrameType frame_type, const unsigned char* msg, int msg_length)
{	
	unsigned char frame = (unsigned char)frame_type;
	Serial(frame);
	if (msg_length <= 125) {
		unsigned char size_ = msg_length;
		Serial(size_);		
	}
	else if (msg_length <= 65535) {
		unsigned char data = 126;
		Serial(data);
		data = (msg_length >> 8) & 0xFF; // leftmost first
		Serial(data);
		data = msg_length & 0xFF;
		Serial(data);
	}
	else { // >2^16-1 (65535)		
		unsigned char data = 127;
		Serial(data);
							 // write 8 bytes length (significant first)

							 // since msg_length is int it can be no longer than 4 bytes = 2^32-1
							 // padd zeroes for the first 4 bytes
		data = 0x00;
		for (int i = 3; i >= 0; i--) {			
			Serial(data);			
		}

		// write the actual 32bit msg_length in the next 4 bytes
		for (int i = 3; i >= 0; i--) {
			data = ((msg_length >> 8 * i) & 0xFF);
			Serial(data);
		}
	}
	
	Write((const char*)msg, msg_length);
	return getMsgSize();
}

WebSocketFrameType CLXZMessage::GetWebSocketFrame(const unsigned char* in_buffer, int in_length, int& out_len)
{
	//printf("getTextFrame()\n");
	if (in_length < 3) return INCOMPLETE_FRAME;

	unsigned char msg_opcode = in_buffer[0] & 0x0F;
	unsigned char msg_fin = (in_buffer[0] >> 7) & 0x01;
	unsigned char msg_masked = (in_buffer[1] >> 7) & 0x01;

	// *** message decoding 

	int payload_length = 0;
	int pos = 2;
	int length_field = in_buffer[1] & (~0x80);
	unsigned int mask = 0;

	//printf("IN:"); for(int i=0; i<20; i++) printf("%02x ",buffer[i]); printf("\n");

	if (length_field <= 125) {
		payload_length = length_field;
	}
	else if (length_field == 126) { //msglen is 16bit!
									//payload_length = in_buffer[2] + (in_buffer[3]<<8);
		payload_length = (
			(in_buffer[2] << 8) |
			(in_buffer[3])
			);
		pos += 2;
	}
	else if (length_field == 127) { //msglen is 64bit!
		payload_length = (
			(in_buffer[2] << 56) |
			(in_buffer[3] << 48) |
			(in_buffer[4] << 40) |
			(in_buffer[5] << 32) |
			(in_buffer[6] << 24) |
			(in_buffer[7] << 16) |
			(in_buffer[8] << 8) |
			(in_buffer[9])
			);
		pos += 8;
	}

	//printf("PAYLOAD_LEN: %08x\n", payload_length);
	if (in_length < payload_length + pos) {
		return INCOMPLETE_FRAME;
	}

	if (msg_masked) {
		mask = *((unsigned int*)(in_buffer + pos));
		//printf("MASK: %08x\n", mask);
		pos += 4;

		// unmask data:
		const unsigned char* c = in_buffer + pos;
		for (int i = 0; i<payload_length; i++) {
			unsigned char cc = c[i] ^ ((unsigned char*)(&mask))[i % 4];
			Serial(cc);
		}
	}
	else {
		Write((const char*)in_buffer + pos, payload_length);
	}


	//memcpy((void*)out_buffer, (void*)(in_buffer + pos), payload_length);
	//out_buffer[payload_length] = 0;
	//*out_length = payload_length + 1;
	

	
	out_len = pos+payload_length;
	//("msg_opcode:%02x payload_length: %u out_len:%u\n", msg_opcode,  payload_length, out_len);

	if (msg_opcode == 0x0) return (msg_fin) ? TEXT_FRAME : INCOMPLETE_TEXT_FRAME; // continuation frame ?
	if (msg_opcode == 0x1) return (msg_fin) ? TEXT_FRAME : INCOMPLETE_TEXT_FRAME;
	if (msg_opcode == 0x2) return (msg_fin) ? BINARY_FRAME : INCOMPLETE_BINARY_FRAME;
	if (msg_opcode == 0x9) return PING_FRAME;
	if (msg_opcode == 0xA) return PONG_FRAME;

	return ERROR_FRAME;
}

const char* CLXZMessage::tohex(char* buf, size_t buf_size){
	memset(buf, 0x00, buf_size);
	buf_size = buf_size>>1;
	size_t msg_size = getMsgSize();
	size_t hex_size = (msg_size <= buf_size) ? msg_size : buf_size;
	unsigned char* pData = getMsgPtr();
	for (int i = 0; i < hex_size; i++){
		sprintf(&buf[i * 2],"%02x", pData[i]);
	}
	return buf;
}

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
			ar.Serial((char*)getMsgPtr(), len, len);
		}			
	}
}



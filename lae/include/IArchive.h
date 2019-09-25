

#ifndef _IARCHIVE_H
#define _IARCHIVE_H

#include "ICGui.h"
#include "ISerial.h"
#define LXZWINDOW_VER 15

#define SerialExtPtr(ar,type,p) \
if(1){\
if(ar.IsReading()){LXZuint8 flag=0x00; ar.Serial(flag);if(flag==0xFF){p=new type; p->Serial(ar);}}\
else { if(p==NULL){LXZuint8 flag=0x00;ar.Serial(flag);}else{LXZuint8 flag=0xFF;ar.Serial(flag);p->Serial(ar);}}}

#define ConSerial0(ar,a,b,c) if(a>=b) ar.Serial(c)
#define ConSerial1(ar,a,b,c) if(a>=b) c.Serial(ar)

class DLL_CLS CIArchive
{
public:
	CIArchive();
	CIArchive(CISerial* pISerial, ISerialMode nMode);
	virtual ~CIArchive();

	uiboolean IsReading();
	uiboolean IsWriting();
	void SetLittleEndian(bool isLittelEndian);

	template<class T> void Serial(T& t) { t.Serial(this); }

	void Serial(uiboolean& l);
	void Serial(bool& l);
	void Serial(int& l);
	void Serial(long& l);
	void Serial(float& l);
	void Serial(LXZsint8& l);
	void Serial(LXZuint8& l);
	void Serial(LXZsint16& l);
	void Serial(LXZuint16& l);
	void Serial(LXZuint32& l);
	void Serial(LXZuint64& l);
	void Serial(char* str, int len, int nMaxBuf);
	void Serial(char* str, int len);
	void Serial(char*& str);
	void SerialPtr(void*& ptr, const char* name);
	void SerialPtr(void*& ptr);
	char* SerialStr(char* str, int len) ;
	void Write(const char* buf, int len);

	int getError();

	void Initial(CISerial* pISerial, int nMode);	
	int  getMode() { return m_nMode; }
	virtual void setMode(int nMode);	
	size_t setPos(size_t pos);
	size_t  getPos();
	virtual unsigned char* getMsgPtr() { return NULL; }
	virtual int   getMsgSize() { return 0;   }

	void ResetAddress() { m_nIndex = 0;}
	void PushAddress();
	void setIndexPos(int index);
	int  getIndex() { return m_nIndex; }

	void setVersion(LXZuint8 byVer) { m_byVersion = byVer; }
	LXZuint8 getVersion() { return m_byVersion; }

	void     execute() { m_nExeCnt++; }
	void     resetExecuteCount() { m_nExeCnt = 0;};
	LXZuint8 getExecuteCount() { return m_nExeCnt; }

	void setResult(CIArchive* pArchive){m_pArchive = pArchive;}
	CIArchive* getResult() { return m_pArchive;}

protected:
	CISerial*   m_pISerial;
	SERIAL_FUNC m_SerialFunc;
	ISerialMode m_nMode;	
private:
	int m_arrAddress[10];	
	int m_nIndex;
	LXZuint8 m_byVersion;
	LXZuint8 m_nExeCnt;
	CIArchive* m_pArchive;

	short (*Short_Swap)(short);
	float (*Float_Swap)(const float*);
	int   (*Long_Swap)(int);
	LXZuint64 (*LongLong_Swap)(LXZuint64);
};



#endif

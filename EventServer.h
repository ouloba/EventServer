

#ifndef EVENT_SERVER_H
#define EVENT_SERVER_H

#ifdef WIN32
#include <WinSock2.h>
#include<ws2tcpip.h>
#endif

#include "LXZLock.h"
#include "LXZMessage.h"
#include "vvector.h"

#ifdef EVENT__HAVE_OPENSSL
extern "C" {
#include "https-common.h"
}
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <event2/bufferevent_ssl.h>
#endif
#include <event2/dns.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/bufferevent_struct.h>
#include <event2/thread.h>
#include <event2/buffer.h>  
#include <event2/http.h>  
#include <event2/http_struct.h>  
#include <event2/keyvalq_struct.h>  

extern "C" 
{
#include "lua.hpp"
#include "tolua++.h"
}

#include "LXZCoreCfg.h"

#define MAX_CFG_NAMELEN 256

#define  CMD_EVENT_READ     0x01
#define  CMD_EVENT_ERROR    0x02
#define  CMD_EVENT_ACCEPT   0x03
#define  CMD_EVENT_TIMER    0x04
#define  CMD_EVENT_NOTIFY   0x05
#define  CMD_EVENT_HTTPGET  0x06
#define  CMD_EVENT_HTTPPOST 0x07
#define  CMD_EVENT_CONNECTED 0x08
#define  CMD_EVENT_REQUEST  0x09
#define  CMD_EVENT_WRITE    0x0A
#define  CMD_EVENT_READ2    0x0B
#define  CMD_EVENT_WRITE2   0x0C 
#define  CMD_EVENT_ERROR2   0x0D
#define  CMD_EVENT_CHUNK    0x0E
#define  CMD_EVENT_REQUEST_ERROR  0x0F


class CEventServer{
public:
	CEventServer();
	~CEventServer();
	
	int Start(const char* cfgName);
	int Stop(bool destroy);
	int DoProc();

	/*cb*/
	void read_cb(struct bufferevent *bev, CLXZMessage *arg);
	void write_cb(struct bufferevent *bev, CLXZMessage *arg);
	void read_cb2(struct bufferevent *bev, CLXZMessage *arg);
	void write_cb2(struct bufferevent *bev, CLXZMessage *arg);
	void error_cb(struct bufferevent *bev, short event, CLXZMessage *arg);
	void error_cb2(struct bufferevent *bev, short event, CLXZMessage *arg);
	void timer_cb();
	void usertrigger_cb();
	void user_trigger();
	void accept_cb(evutil_socket_t listener, short event);
	void chunk_cb(struct evhttp_request *req);
	static void log(const char* fmt,...);

	/**/
	struct bufferevent *Connect(const char* ip, int port, bool ssl=false);
	void CloseClient(bufferevent* bev);
	int SSLHandShake(struct bufferevent * bev);

	/*http client cb*/
	void post_cb(struct evhttp_request *req, void *arg);
	void get_cb(struct evhttp_request *req, void *arg);
		
	/*thread safe transfer message.*/
	int Notify(int code, CLXZMessage& msg);
	void notify_timer_cb();

	/**/
	void Send2Client(bufferevent* bev, CLXZMessage* msg);
	void SendData2Client(bufferevent* bev, const char* buf, size_t len);

	
	/*http*/
	void request_cb(struct evhttp_request *req, void *arg);
	void request_cb_error(struct evhttp_request *req, void *arg, int error);
	int  as_http_server(const char* ip, int port);
	int send_http_get(const char* url,lua_State* co);
	int send_http_post(const char* url,const char* data,lua_State* co);
	void proc_cb(int code, struct evhttp_request *req,int param=0);
	struct evhttp * GetHttpServer() { return _http_server;  }
	struct evhttp_request *connect2websocket(const char* url);


	/*cfg*/
	ILXZCoreCfg* GetCfg() { return &_cfg; }
	int   _do_lua_file(const char* file);
	lua_State* luaState() {return _state;}
	struct event_base *GetBase(){ return _base; }
private:
	int   _listen(const char* ip, int port);
	void  _lua_proc(int cmd, CLXZMessage* msg,int code=0, lua_State* co=NULL,int ref=-1,bufferevent* bev=NULL);
	
	/*init  lua state*/
	int init_lua_state(lua_State* L);
	
private:
	LXZCoreCfg         _cfg;
	bool               _destroy;
	struct event_base *_base;
	lua_State*         _state;	
	int                _lua_ref;
	char               _cfg_name[MAX_CFG_NAMELEN+1];	
	Linux_Win_Lock     _lock;
	vvector<CLXZMessage*> _vvMsg;

	//time_t             _last_modify_time;
	struct evhttp * _http_server;
	struct event *  _user_trigger;

#ifdef EVENT__HAVE_OPENSSL
	SSL_CTX *ctx_client;
#endif

#ifdef WIN32
	uintptr_t _thread;
#else
	pthread_t _thread;
#endif
};

#endif
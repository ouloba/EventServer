

#include "EventServer.h"
#include "LuaThreadMgr.h"

#ifdef WIN32
#include <process.h>
#include <io.h>
#else
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include<netinet/tcp.h>
#include<fcntl.h>
#endif
#include <sys/stat.h>

#include <signal.h>
#define  PRINT_BUFF_LEN 2048
static char print_buf[PRINT_BUFF_LEN*2];

extern int LuaLXZMessageInitial(lua_State* pState);
extern int LuaLXZCoreCfgInitial(lua_State* pState);
extern "C" int luaopen_cjson(lua_State *l);
extern "C" int luaopen_mime_core(lua_State *L);
extern "C" int luaopen_snapshot(lua_State *L);
//#ifdef WIN32
//extern "C" int luaopen_socket_core(lua_State *L);
//#endif

//void* __cdecl __va_copy = memcpy;


extern "C" void lae_log(const char* fmt,...);

// (default)  
#define HTTP_CONTENT_TYPE_URL_ENCODED   "application/x-www-form-urlencoded"     
// (use for files: picture, mp3, tar-file etc.)                                          
#define HTTP_CONTENT_TYPE_FORM_DATA     "multipart/form-data"                   
// (use for plain text)  
#define HTTP_CONTENT_TYPE_TEXT_PLAIN    "text/plain"  
  
#define REQUEST_POST_FLAG               2  
#define REQUEST_GET_FLAG                3  
static int USE_HTTPS  = 0;
  
struct http_request_get {  
	lua_State* thread;
	int        thread_ref;
	void*  respone_msg;
	void*  event_server;
    struct evhttp_uri *uri;  
    struct event_base *base;  
    struct evhttp_connection *cn;  
    struct evhttp_request *req;  
};  
  
struct http_request_post {  
	lua_State* thread;
	int        thread_ref;
	void*  respone_msg;
	void*  event_server;
    struct evhttp_uri *uri;  
    struct event_base *base;  
    struct evhttp_connection *cn;  
    struct evhttp_request *req;  
    char *content_type;  
    char *post_data;  
};  

#ifndef WIN32
unsigned long GetTickCount()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif
  
/************************** Ahead Declare ******************************/  
void http_requset_post_cb(struct evhttp_request *req, void *arg);  
void http_requset_get_cb(struct evhttp_request *req, void *arg);  
int start_url_request(struct http_request_get *http_req, int req_get_flag);  
void http_request_free(struct http_request_get *http_req_get, int req_get_flag);

#define MITLOG_LEVEL_COMMON 0
#define MITLOG_LEVEL_ERROR  1
void MITLog_DetPrintf(int level, const char* fmt,...){
	/*	va_list args;
	int n;
	va_start(args, fmt);
	printf(fmt, args);
	va_end(args);*/
}

void MITLog_DetPuts(int level, const char* fmt,...){
}

  
/************************** Tools Function ******************************/  
static inline void print_request_head_info(struct evkeyvalq *header)  
{  
    struct evkeyval *first_node = header->tqh_first;  
    while (first_node) {  
      //  MITLog_DetPrintf(MITLOG_LEVEL_COMMON,"key:%s  value:%s", first_node->key, first_node->value);  
        first_node = first_node->next.tqe_next;  
    }  
}  
  
static inline void print_uri_parts_info(const struct evhttp_uri * http_uri)  
{  
}  

#ifdef EVENT__HAVE_OPENSSL
/**
* This callback is responsible for creating a new SSL connection
* and wrapping it in an OpenSSL bufferevent.  This is the way
* we implement an https server instead of a plain old http server.
*/
static struct bufferevent* bevcb(struct event_base *base, void *arg)
{
	struct bufferevent* r;
	SSL_CTX *ctx = (SSL_CTX *)arg;

	r = bufferevent_openssl_socket_new(base,
		-1,
		SSL_new(ctx),
		BUFFEREVENT_SSL_ACCEPTING,
		BEV_OPT_CLOSE_ON_FREE);
	return r;
}



static void server_setup_certs(SSL_CTX *ctx,
	const char *certificate_chain,
	const char *private_key)
{
	info_report("Loading certificate chain from '%s'\n"
		"and private key from '%s'\n",
		certificate_chain, private_key);

	if (1 != SSL_CTX_use_certificate_chain_file(ctx, certificate_chain))
		die_most_horribly_from_openssl_error("SSL_CTX_use_certificate_chain_file");

	if (1 != SSL_CTX_use_PrivateKey_file(ctx, private_key, SSL_FILETYPE_PEM))
		die_most_horribly_from_openssl_error("SSL_CTX_use_PrivateKey_file");

	if (1 != SSL_CTX_check_private_key(ctx))
		die_most_horribly_from_openssl_error("SSL_CTX_check_private_key");
}
#endif
  
/************************** Request Function ******************************/  
void http_requset_post_cb(struct evhttp_request *req, void *arg)  
{  
    struct http_request_post *http_req_post = (struct http_request_post *)arg;  
	CEventServer* pSrv = (CEventServer*)http_req_post->event_server;
	if(req) pSrv->post_cb(req, arg);
	else{
		http_req_post->req = NULL;
	}
}  
void http_requset_get_cb(struct evhttp_request *req, void *arg)  
{  
    struct http_request_get *http_req_get = (struct http_request_get *)arg;  
	CEventServer* pSrv = (CEventServer*)http_req_get->event_server;
	if(req)	pSrv->get_cb(req, arg);  
	else{
		http_req_get->req = NULL;
	}
}  

  
int start_url_request(struct http_request_get *http_req, int req_get_flag)  
{  
    if (http_req->cn)  
        evhttp_connection_free(http_req->cn);  
      
    int port = evhttp_uri_get_port(http_req->uri);  
    http_req->cn = evhttp_connection_base_new(http_req->base,  
                                                   NULL,  
                                                   evhttp_uri_get_host(http_req->uri),  
                                                   (port == -1 ? 80 : port));  
      
    /** 
     * Request will be released by evhttp connection 
     * See info of evhttp_make_request() 
     */  
    if (req_get_flag == REQUEST_POST_FLAG) {  
        http_req->req = evhttp_request_new(http_requset_post_cb, http_req);  
	//	evhttp_request_set_error_cb(http_req->req, RemoteRequestErrorCallback);
    } else if (req_get_flag ==  REQUEST_GET_FLAG) {  
        http_req->req = evhttp_request_new(http_requset_get_cb, http_req);  
	//	evhttp_request_set_error_cb(http_req->req, RemoteRequestErrorCallback);
    }  

	if(http_req->req==NULL)
		return 0;
      
    if (req_get_flag == REQUEST_POST_FLAG) {  
        const char *path = evhttp_uri_get_path(http_req->uri);  
        evhttp_make_request(http_req->cn, http_req->req, EVHTTP_REQ_POST,  
                            path ? path : "/");  
        /** Set the post data */  
        struct http_request_post *http_req_post = (struct http_request_post *)http_req;  
        evbuffer_add(http_req_post->req->output_buffer, http_req_post->post_data, strlen(http_req_post->post_data));  
        evhttp_add_header(http_req_post->req->output_headers, "Content-Type", http_req_post->content_type);  
    } else if (req_get_flag == REQUEST_GET_FLAG) {  
        const char *query = evhttp_uri_get_query(http_req->uri);  
        const char *path = evhttp_uri_get_path(http_req->uri);  
        size_t len = (query ? strlen(query) : 0) + (path ? strlen(path) : 0) + 1;  
        char *path_query = NULL;  
        if (len > 1) {  
            path_query = (char*)calloc(len+2, sizeof(char));  
			if (query)
			{
				sprintf(path_query, "%s?%s", path, query);
			}
			else
			{
				sprintf(path_query, "%s", path);
			}
        }          
       int ret= evhttp_make_request(http_req->cn, http_req->req, EVHTTP_REQ_GET,  
                             path_query ? path_query: "/");  
	   if(path_query){
		   free(path_query);
	   }

	   if(ret!=0){
		   return ret;
	   }
    }  

	if(http_req->req==NULL)
		return 0;

    /** Set the header properties */  
    evhttp_add_header(http_req->req->output_headers, "Host", evhttp_uri_get_host(http_req->uri));  
      
    return 0;  
}  
  
/************************** New/Free Function ******************************/  
/** 
 * @param get_flag: refer REQUEST_GET_* 
 * 
 */  
void *http_request_new(struct event_base* base, const char *url, int req_get_flag, const char *content_type, const char* data)  
{  
    int len = 0;  
    if (req_get_flag == REQUEST_GET_FLAG) {  
        len = sizeof(struct http_request_get);  
    } else if(req_get_flag == REQUEST_POST_FLAG) {  
        len = sizeof(struct http_request_post);  
    }  
      
    struct http_request_get *http_req_get = (struct http_request_get *)calloc(1, len);  
    http_req_get->uri = evhttp_uri_parse(url);  
    print_uri_parts_info(http_req_get->uri);  
      
    http_req_get->base = base;  
      
    if (req_get_flag == REQUEST_POST_FLAG) {  
        struct http_request_post *http_req_post = (struct http_request_post *)http_req_get;  
        if (content_type == NULL) {  
            content_type = HTTP_CONTENT_TYPE_URL_ENCODED;  
        }  
        http_req_post->content_type = strdup(content_type);  
          
        if (data == NULL) {  
            http_req_post->post_data = NULL;  
        } else {  
            http_req_post->post_data = strdup(data);  
        }  
    }  
      
    return http_req_get;  
}  
  
void http_request_free(struct http_request_get *http_req_get, int req_get_flag)  
{  
	if(http_req_get->respone_msg){
		CLXZMessage* msg = (CLXZMessage*)http_req_get->respone_msg;
		CEventServer* pSvr = (CEventServer*)msg->getAppData();
		if(msg->GetLuaRef()!=-1){
			lua_unref(pSvr->luaState(),msg->GetLuaRef());
		}
		delete msg;
		http_req_get->respone_msg = NULL;
	}
	
    evhttp_connection_free(http_req_get->cn);  
    evhttp_uri_free(http_req_get->uri);  
	//evhttp_request_free(http_req_get->req);

    if (req_get_flag == REQUEST_GET_FLAG) {  
        free(http_req_get);  
    } else if(req_get_flag == REQUEST_POST_FLAG) {  
        struct http_request_post *http_req_post = (struct http_request_post*)http_req_get;  
        if (http_req_post->content_type) {  
            free(http_req_post->content_type);  
        }  
        if (http_req_post->post_data) {  
            free(http_req_post->post_data);  
        }  
        free(http_req_post);  
    }  
    http_req_get = NULL;  
}  
  
/************************** Start POST/GET Function ******************************/  
/** 
 * @param content_type: refer HTTP_CONTENT_TYPE_*        
 */  
void *start_http_requset(struct event_base* base, const char *url, int req_get_flag, const char *content_type, const char* data)  
{  
    struct http_request_get *http_req_get = (struct http_request_get *)http_request_new(base, url, req_get_flag, content_type, data);  
    start_url_request(http_req_get, req_get_flag);        
    return http_req_get;  
}  

/*
struct http_request_post *http_req_post = start_http_requset(base,  
"http://172.16.239.93:8899/base/truck/delete",  
REQUEST_POST_FLAG,  
HTTP_CONTENT_TYPE_URL_ENCODED,  
"name=winlin&code=1234");  
struct http_request_get *http_req_get = start_http_requset(base,  
"http://127.0.0.1?name=winlin",  
REQUEST_GET_FLAG,  
NULL, NULL);  

event_base_dispatch(base);  

http_request_free((struct http_request_get *)http_req_post, REQUEST_POST_FLAG);  
http_request_free(http_req_get, REQUEST_GET_FLAG);  
*/

CEventServer::CEventServer(){
	_base = NULL;
	_state = NULL;	
	_lua_ref = -1;
//	memset(&_last_modify_time,0x00, sizeof(_last_modify_time));
	memset(_cfg_name,0x00,sizeof(_cfg_name));	
	_thread = 0;
	_destroy = false;
#ifdef EVENT__HAVE_OPENSSL
	ctx_client=NULL;
#endif
}

CEventServer::~CEventServer(){

}

#ifdef WIN32
static void WorkThread(void *data)
{
	CEventServer* pEventSvr = (CEventServer*)data;
	pEventSvr->DoProc();
}
#else
static void* WorkThread(void *data)
{
	CEventServer* pEventSvr = (CEventServer*)data;
	pEventSvr->DoProc();
	pthread_exit(NULL);
	return NULL;
}
#endif

int CEventServer::Start(const char* cfgName){
	//
	if(_base!=NULL){
		log("[%s] Server is running now!", cfgName);		
		return -1;
	}
	
	// initialize event base.
	int len = strlen(cfgName)>MAX_CFG_NAMELEN?MAX_CFG_NAMELEN:strlen(cfgName);
	memcpy(_cfg_name, cfgName, len);
	_cfg_name[len] = 0;

#ifdef WIN32
	_thread = _beginthread(WorkThread, 0, this);
#else
	pthread_create(&_thread, NULL, WorkThread, this);
	pthread_detach(_thread);
#endif	

	return 0;
}

int CEventServer::Stop(bool destroy){
	if(_base){
		_destroy = true;
		event_base_loopexit(_base, 0);  
	}

	return 0;
}

static void _read_cb2(struct bufferevent *bev, void *arg){
	CLXZMessage *pMsg = (CLXZMessage *)arg;
	CEventServer* pSvr = (CEventServer*)pMsg->getAppData();
	pSvr->read_cb2(bev,pMsg);
}


static void _write_cb2(struct bufferevent *bev, void *arg){
	CLXZMessage *pMsg = (CLXZMessage *)arg;
	CEventServer* pSvr = (CEventServer*)pMsg->getAppData();
	pSvr->write_cb2(bev,pMsg);
}



static void _write_cb(struct bufferevent *bev, void *arg){
	CLXZMessage *pMsg = (CLXZMessage *)arg;
	CEventServer* pSvr = (CEventServer*)pMsg->getAppData();
	pSvr->write_cb(bev,pMsg);
}

static void _error_cb(struct bufferevent *bev, short event, void *arg){
	CLXZMessage *pMsg = (CLXZMessage *)arg;
	CEventServer* pSvr = (CEventServer*)pMsg->getAppData();
	pSvr->error_cb(bev,event,pMsg);
}

static void _error_cb2(struct bufferevent *bev, short event, void *arg){
	CLXZMessage *pMsg = (CLXZMessage *)arg;
	CEventServer* pSvr = (CEventServer*)pMsg->getAppData();
	pSvr->error_cb2(bev,event,pMsg);
}

static void _read_cb(struct bufferevent *bev, void *arg){
	CLXZMessage *pMsg = (CLXZMessage *)arg;
	CEventServer* pSvr = (CEventServer*)pMsg->getAppData();
	pSvr->read_cb(bev,pMsg);
}

static void _timer_cb(int fd, short event, void *arg){
	CEventServer* pSvr = (CEventServer*)arg;
	pSvr->timer_cb();
}

#define MAX_LINE 1023
void CEventServer::read_cb(struct bufferevent *bev, CLXZMessage *msg){

	evutil_socket_t fd = bufferevent_getfd(bev);

	int old_mode = msg->getMode();
	int old_pos = msg->getPos();
	int pos = msg->getMsgSize();
	msg->setMode(SM_WRITE);
	msg->setPos(pos);

	int n = 0;
	char line[MAX_LINE+1];
	n = bufferevent_read(bev, line, MAX_LINE);
	while(n>0){
		line[n]=0;		
		msg->Write(line,n);		
		n = bufferevent_read(bev, line, MAX_LINE);
	}
	msg->setMode(old_mode);
	msg->setPos(old_pos);		
	if (msg->getMsgSize()>0){
		//log("read_cb:%d\n", msg->getMsgSize());
		_lua_proc(CMD_EVENT_READ, msg,0,NULL,-1, bev);
	}
}

void CEventServer::write_cb(struct bufferevent *bev, CLXZMessage *msg){
		_lua_proc(CMD_EVENT_WRITE, msg,0,NULL,-1, bev);
}


void CEventServer::read_cb2(struct bufferevent *bev, CLXZMessage *msg){
	//log("read_cb2");
	evutil_socket_t fd = bufferevent_getfd(bev);
	CLXZMessage* pMsg = (CLXZMessage*)bev->cbarg;
	int old_mode = msg->getMode();
	int old_pos = msg->getPos();
	int pos = msg->getMsgSize();
	msg->setMode(SM_WRITE);

	int n = 0;
	char line[MAX_LINE+1];
	n = bufferevent_read(bev, line, MAX_LINE);
	
	while(n>0){
		line[n]=0;		
		msg->Write(line,n);		
		n = bufferevent_read(bev, line, MAX_LINE);		
	}

	msg->setMode(old_mode);
	msg->setPos(old_pos);		
	if (msg->getMsgSize()>0){
		//log("read_cb:%d\n", msg->getMsgSize());
		_lua_proc(CMD_EVENT_READ2, msg,0,NULL,-1, bev);
	}
}

void CEventServer::write_cb2(struct bufferevent *bev, CLXZMessage *msg){
	//log("write_cb2");
	_lua_proc(CMD_EVENT_WRITE2, msg,0,NULL,-1, bev);
}


void CEventServer::error_cb2(struct bufferevent *bev, short event, CLXZMessage *msg){

	//通过传入参数bev找到socket fd
	evutil_socket_t fd = bufferevent_getfd(bev);
	//cout << "fd = " << fd << endl;

	if (event & BEV_EVENT_TIMEOUT){
		log("OnOffline Timed out[%d] %p\n",msg->getTime(),msg); //if bufferevent_set_timeouts() called
	}
	else if (event & BEV_EVENT_EOF){
		log("OnOffline connection closed[%d] %p\n",msg->getTime(),msg);
	}
	else if (event & BEV_EVENT_ERROR){
		log("OnOffline some other error[%d] %p\n",msg->getTime(),msg);
	}
	else if(event&BEV_EVENT_CONNECTED){
		log("connected\n");				
		_lua_proc(CMD_EVENT_CONNECTED, msg, 0, NULL, -1, bev);
		bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
		return;
	}

	_lua_proc(CMD_EVENT_ERROR2, msg,0,NULL,-1, bev);


	//if(msg->getEvent()!=-1){
	//	lua_unref(_state,msg->getEvent());
	//}

	//if(msg->GetLuaRef()!=-1){
	//	lua_unref(_state,msg->GetLuaRef());
	//}

	//delete msg;
	//bufferevent_free(bev);
}

int CEventServer::SSLHandShake(struct bufferevent * bev)
{
	return 0;
}

void CEventServer::error_cb(struct bufferevent *bev, short event, CLXZMessage *msg){

	//通过传入参数bev找到socket fd
	evutil_socket_t fd = bufferevent_getfd(bev);
	//cout << "fd = " << fd << endl;
	
	if (event & BEV_EVENT_TIMEOUT){
		log("OnOffline Timed out[%d] %p\n",msg->getTime(),msg); //if bufferevent_set_timeouts() called
	}
	else if (event & BEV_EVENT_EOF){
		log("OnOffline connection closed[%d] %p\n",msg->getTime(),msg);
	}
	else if (event & BEV_EVENT_ERROR){
		log("OnOffline some other error[%d] %p\n",msg->getTime(),msg);
	}
	else if(event&BEV_EVENT_CONNECTED){
		log("connected\n");
		_lua_proc(CMD_EVENT_CONNECTED, msg, 0, NULL, -1, bev);
		return;
	}

	_lua_proc(CMD_EVENT_ERROR, msg,0,NULL,-1, bev);


//	if(msg->getEvent()!=-1){
//		lua_unref(_state,msg->getEvent());
//	}

//	if(msg->GetLuaRef()!=-1){
//		lua_unref(_state,msg->GetLuaRef());
//	}

	//delete msg;
	//bufferevent_free(bev);
}

void CEventServer::user_trigger() {
	evuser_trigger(_user_trigger);
}

void CEventServer::usertrigger_cb() {
	CLuaThreadMgr::Instance().Update(_state);
}

void CEventServer::timer_cb(){	
	_lua_proc(CMD_EVENT_TIMER, NULL);
}

void CEventServer::accept_cb(evutil_socket_t listener, short event){
	
	//socket描述符
	evutil_socket_t fd;
	
	//声明地址
	struct sockaddr_in sin;
	
	//地址长度声明
	socklen_t slen = sizeof(sin);
	
	//接收客户端
	fd = accept(listener, (struct sockaddr *)&sin, &slen);
	if (fd < 0){
		printf("error accept:%d\r\n",fd);
		return;
	}

	log("ACCEPT: fd = %u\n", fd);

	evutil_make_socket_nonblocking(fd);

	//设置与某个套接字关联的选项
	//参数二 IPPROTO_TCP:TCP选项
	//参数三 TCP_NODELAY 不使用Nagle算法 选择立即发送数据而不是等待产生更多的数据然后再一次发送
	// 更多参数TCP_NODELAY 和 TCP_CORK
	//参数四 新选项TCP_NODELAY的值
	int yes = 1;
	if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&yes, sizeof(yes)) == -1) {
		printf("setsockopt(): TCP_NODELAY %s\n", strerror(errno));
		close(fd);
		return;
	}

	
	//注册一个bufferevent_socket_new事件
	struct bufferevent *bev = bufferevent_socket_new(_base, fd, BEV_OPT_CLOSE_ON_FREE);

	CLXZMessage* pMsg = new CLXZMessage();
	pMsg->setMode(SM_WRITE);
	pMsg->setAppData(this);

	tolua_pushusertype(_state, pMsg, "CLXZMessage");
	int ref = lua_ref(_state,-1);
	pMsg->SetLuaRef(ref);

	lua_pushlightuserdata(_state,bev);
	ref = lua_ref(_state, -1);
	pMsg->setEvent(ref);

	log("accept msg[%d]:%p ref[%d] bev:%p\n", pMsg->getTime(),pMsg, pMsg->GetLuaRef(), bev);

	//设置回掉函数
	bufferevent_setcb(bev, _read_cb, _write_cb, _error_cb, pMsg);

	//设置该事件的属性
	bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
	_lua_proc(CMD_EVENT_ACCEPT, pMsg);
}

void _usertrigger_cb(int fd, short event, void *arg)
{
	CEventServer* pSvr = (CEventServer*)arg;
	pSvr->usertrigger_cb();	
}

void _timeout_cb(int fd, short event, void *arg)
{
	//puts("111");
	//printf("_timeout_cb \r\n");
	CEventServer* pSvr = (CEventServer*)arg;
	pSvr->timer_cb();
}

void _notify_timeout_cb(int fd, short event, void *arg)
{
	//puts("111");
	CEventServer* pSvr = (CEventServer*)arg;
	pSvr->notify_timer_cb();
}

static void _do_accept_cb(evutil_socket_t listener, short event, void *arg){
	CEventServer* pSvr = (CEventServer*)arg;
	pSvr->accept_cb(listener, event);
}

void CEventServer::proc_cb(int code, struct evhttp_request *req, int param) {
	int top = lua_gettop(_state);
	lua_getref(_state, _lua_ref);
	lua_gettable(_state, LUA_GLOBALSINDEX);
	if (!lua_isfunction(_state, -1)) {
		lua_settop(_state, top);
		return;
	}

	//
	tolua_pushnumber(_state, code);
	lua_pushlightuserdata(_state, req);
	tolua_pushnumber(_state, param);
	if (lua_pcall(_state, 3, 0/*LUA_MULTRET*/, 0) != 0) {
		//debug_message("error running function `f': %s", lua_tostring(m_pState, -1));
		const char* err_ = lua_tostring(_state, -1);
		log("lau_pcall: %d %s\n", CMD_EVENT_REQUEST, err_);
		lua_settop(_state, top);
	}
}

void CEventServer::request_cb(struct evhttp_request *req, void *arg){
	proc_cb(CMD_EVENT_REQUEST, req);
}

void CEventServer::request_cb_error(struct evhttp_request *req,void *arg, int error) {
	//evhttp_send_reply_end(req);
	proc_cb(CMD_EVENT_REQUEST_ERROR, req, error);
}

/*http client cb*/
void CEventServer::post_cb(struct evhttp_request *req, void *arg){
	struct http_request_post *http_req_post = (struct http_request_post *)arg;  
	CLXZMessage* msg = (CLXZMessage*)http_req_post->respone_msg;
	switch(req->response_code)  
	{  
	case HTTP_OK:  
		{  
		/*	struct evbuffer* buf = evhttp_request_get_input_buffer(req);  
			size_t len =  evbuffer_get_length(buf);      			
			msg->setMode((SM_WRITE));
			msg->Write((const char*)evbuffer_pullup(buf, -1),len);		
			lua_State *thread = http_req_post->thread;
			int   thread_ref = http_req_post->thread_ref;*/	
			//_lua_proc(CMD_EVENT_HTTPPOST, msg, HTTP_OK,thread,thread_ref);
			proc_cb(CMD_EVENT_HTTPPOST, req);
			http_request_free((struct http_request_get *)http_req_post,REQUEST_POST_FLAG);	
		//	evbuffer_free(buf);
			break;  
		}  
	case HTTP_MOVEPERM:  
		MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "%s", "the uri moved permanently");  
		http_request_free((struct http_request_get *)http_req_post,REQUEST_POST_FLAG);		
		break;  
	case HTTP_MOVETEMP:  
		{  
			const char *new_location = evhttp_find_header(req->input_headers, "Location");  
			struct evhttp_uri *new_uri = evhttp_uri_parse(new_location);  
			evhttp_uri_free(http_req_post->uri);  
			http_req_post->uri = new_uri;  
			start_url_request((struct http_request_get *)http_req_post, REQUEST_POST_FLAG);  
			return;  
		}  

	default:  
		proc_cb(CMD_EVENT_HTTPPOST, req);
		/*lua_State *thread = http_req_post->thread;
		int   thread_ref = http_req_post->thread_ref;
		int response_code = req->response_code;
		_lua_proc(CMD_EVENT_HTTPPOST, msg, response_code,thread,thread_ref);*/
		http_request_free((struct http_request_get *)http_req_post,REQUEST_POST_FLAG);		
		return;  
	}  
}

void CEventServer::get_cb(struct evhttp_request *req, void *arg){
	struct http_request_get *http_req_get = (struct http_request_get *)arg;  
	CLXZMessage* msg = (CLXZMessage*)http_req_get->respone_msg;
	switch(req->response_code)  
	{  
	case HTTP_OK:  
		{  
			struct evbuffer* buf = evhttp_request_get_input_buffer(req);  
			size_t len = evbuffer_get_length(buf);  		
			msg->setMode((SM_WRITE));
			msg->Write((const char*)evbuffer_pullup(buf, -1),len);
			lua_State *thread = http_req_get->thread;
			int   thread_ref = http_req_get->thread_ref;
			_lua_proc(CMD_EVENT_HTTPGET, msg, HTTP_OK,thread,thread_ref);			
			http_request_free(http_req_get,REQUEST_GET_FLAG);			
			//evbuffer_free(buf);
			return;
		}  
	case HTTP_MOVEPERM:  
	//	MITLog_DetPrintf(MITLOG_LEVEL_ERROR, "%s", "the uri moved permanently");  
	//	http_request_free(http_req_get,REQUEST_GET_FLAG);	
	//	return;
	case HTTP_MOVETEMP:  
		{  
			const char *new_location = evhttp_find_header(req->input_headers, "Location");  
			struct evhttp_uri *new_uri = evhttp_uri_parse(new_location);  
			evhttp_uri_free(http_req_get->uri);  
			http_req_get->uri = new_uri;  
			start_url_request(http_req_get, REQUEST_GET_FLAG);  
			return;  
		}  

	default:  	
		lua_State *thread = http_req_get->thread;
		int   thread_ref = http_req_get->thread_ref;
		int response_code = req->response_code;
		_lua_proc(CMD_EVENT_HTTPGET, msg, response_code,thread,thread_ref);
		http_request_free(http_req_get,REQUEST_GET_FLAG);		
		return;  
	}  
}

void  CEventServer::_lua_proc(int cmd, CLXZMessage* msg,int code, lua_State* co,int ref,bufferevent* bev){
	
	if(co==NULL){
		int top = lua_gettop(_state); 
		lua_getref(_state, _lua_ref);
		lua_gettable(_state, LUA_GLOBALSINDEX);
		if(!lua_isfunction(_state, -1)){
			lua_settop(_state, top);	
			log("_lua_proc:%d\r\n",_lua_ref);
			return;
		}
	
		//
		tolua_pushnumber(_state, cmd);
		if(msg&&msg->GetLuaRef()!=-1){
			lua_getref(_state, msg->GetLuaRef());
		}else{
			tolua_pushusertype(_state, msg, "CLXZMessage");
		}

		if(bev==NULL&&msg&&msg->getEvent()!=-1){
			lua_getref(_state, msg->getEvent());}
		else{
			lua_pushlightuserdata(_state,bev);
		}

		tolua_pushnumber(_state, code);
		//printf("_lua_proc:%d %d\r\n",_lua_ref,code);
		if (lua_pcall(_state, 4, 0/*LUA_MULTRET*/, 0) != 0) {
			//debug_message("error running function `f': %s", lua_tostring(m_pState, -1));
			const char* err_ =lua_tostring(_state, -1);
			log("lau_pcall: %d %s\n", cmd, err_);
			lua_settop(_state,top);
		}
	}
	else{	
		if(lua_status(co)==LUA_YIELD){
			tolua_pushnumber(co, code);
			tolua_pushusertype(co, msg, "CLXZMessage");			
			int ret = lua_resume(co, 2);		
			if(ret!=LUA_YIELD&&ret!=0){
				//char buf[256];
				//vnsprintf(buf,"ERROR: %s",lua_tostring(co,-1));
				log("%s\n",lua_tostring(co,-1));
				lua_pop(co,-1);
			}

			if(ref!=-1)	lua_unref(co, ref);
		}
	}	
}

int CEventServer::_listen(const char* ip, int port){
	evutil_socket_t listener;

#ifdef WIN32
	//Init Windows Socket
	WSADATA  Ws;	
	if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0)
	{
		return -1;
	}
#endif

	listener = socket(AF_INET, SOCK_STREAM, 0);

	//assert(listener > 0);
	evutil_make_listen_socket_reuseable(listener);
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	sin.sin_port = htons(port);
	if (bind(listener, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("bind");
		return 1;
	}

	if (listen(listener, 1000) < 0) {
		perror("listen");
		return 1;
	}

	log("Listening...\n");
	evutil_make_socket_nonblocking(listener);
		
	struct event *listen_event = event_new(_base, listener, EV_READ | EV_PERSIST, _do_accept_cb, (void*)this);
	event_add(listen_event, NULL);

	return 0;
}

static int docall(lua_State *L, int narg, int clear)
{
	int status;
	int base = lua_gettop(L) - narg;  /* function index */
	status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), 0);
	//lua_remove(L, base);  /* remove traceback function */
	/* force a complete garbage collection in case of errors */
	if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
	return status;
}

int  CEventServer::_do_lua_file(const char* fileName){
	
	int error = 0;
	error = luaL_dofile(_state, fileName);
	if(error){
		const char* szErr = lua_tostring(_state,-1); 	
		lua_settop(_state, 0);	
		log("_do_lua_file %s: %s\n", fileName, szErr);
		return error;
	}

	return 0;
}

static int lua_EventServer_delete(lua_State* L)
{
	printf("EventServer delete?\n");
	return 0;
}

static int lua_ThreadExecute(lua_State* L)
{
	CEventServer* pSvr = (CEventServer*)tolua_tousertype(L, 1, NULL);	
	size_t threadid = (size_t)tolua_tonumber(L, 2, 0);
	lua_State* co = lua_tothread(L, 3);
	size_t count = 0;
	const char* code = (const char*)lua_tolstring(L, 4, &count);	
	CLuaThread* pThread = CLuaThreadMgr::Instance().GetThread(threadid);
	if (pThread) {
		size_t tasks = pThread->PostTask(code, count, co, pSvr->luaState());
		tolua_pushnumber(L, tasks);
		return 1;
	}	
	tolua_pushnumber(L, 0);
	return 1;
}

static int lua_GetLuaThread(lua_State* L)
{
	CEventServer* pSvr = (CEventServer*)tolua_tousertype(L, 1, NULL);
	size_t threadid = (size_t)tolua_tonumber(L, 2, 0);
	CLuaThread* pThread = CLuaThreadMgr::Instance().GetThread(threadid,false);
	if (pThread) {
		lua_pushlightuserdata(L, pThread);
		return 1;
	}
	return 0;
}

static int lua_GetLuaThreadTaskCount(lua_State* L)
{
	CEventServer* pSvr = (CEventServer*)tolua_tousertype(L, 1, NULL);
	size_t threadid = (size_t)tolua_tonumber(L, 2, 0);
	CLuaThread* pThread = CLuaThreadMgr::Instance().GetThread(threadid, false);
	if (pThread) {
		tolua_pushnumber(L, pThread->GetTaskCount());
		return 1;
	}

	tolua_pushnumber(L, 0);
	return 1;
}

static int lua_SetLuaThreadMaxTask(lua_State* L)
{
	CEventServer* pSvr = (CEventServer*)tolua_tousertype(L, 1, NULL);
	size_t threadid = (size_t)tolua_tonumber(L, 2, 0);
	size_t max_tasks = (size_t)tolua_tonumber(L, 3, 0);
	CLuaThread* pThread = CLuaThreadMgr::Instance().GetThread(threadid, false);
	if (pThread) {
		pThread->SetMaxTask(max_tasks);
		return 0;
	}

	return 0;
}


static int lua_Send2Client(lua_State* L)
{
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	struct bufferevent *bev=(struct bufferevent *)lua_touserdata(L,2);
	CLXZMessage* msg = (CLXZMessage*)tolua_tousertype(L, 3, NULL);
	pSvr->Send2Client(bev,msg);
	return 0;
}

static int lua_SendData2Client(lua_State* L)
{
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	struct bufferevent *bev=(struct bufferevent *)lua_touserdata(L,2);
	size_t count=0;
	const char* v = (const char*)luaL_checklstring(L, 3, &count);	
	pSvr->SendData2Client(bev,v,count);
	return 0;
}


void CEventServer::Send2Client(bufferevent* bev, CLXZMessage* msg){
	if(bev && msg) {
		log("Send2Client:%p %d\n", bev,msg->getMsgSize());
		CLXZMessage* pMsg = (CLXZMessage*)bev->cbarg;
		bufferevent_write(bev, msg->getMsgPtr(), msg->getMsgSize());		
		msg->ReleaseMemory();
	}
}

void CEventServer::SendData2Client(bufferevent* bev, const char* buf, size_t len){
	if(bev) {
		log("SendData2Client:%p %d\n", bev, len);
		CLXZMessage* pMsg = (CLXZMessage*)bev->cbarg;
		bufferevent_write(bev, buf, len);		
	}
}

struct bufferevent *CEventServer::Connect(const char* ip, int port, bool ssl_){
	int sockfd;
	struct event_base *p_base=_base;
	struct bufferevent *p_event;
	struct sockaddr_in addr;
	memset(&addr, 0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	SSL  *ssl = NULL;
	if (!ssl_){
		p_event = bufferevent_socket_new(p_base, -1, BEV_OPT_CLOSE_ON_FREE);
		if ((p_event = bufferevent_socket_new(p_base, -1, BEV_OPT_CLOSE_ON_FREE)) == NULL){
			log("bufferevent_socket_new ");
			return NULL;
		}
	}
	else{
		ssl = SSL_new(ctx_client);
		if (ssl == NULL)
		{
			printf("SSL_new error!\n");
			ERR_print_errors_fp(stderr);
			return NULL;
		}

		p_event = bufferevent_openssl_socket_new(p_base, -1, ssl,
			BUFFEREVENT_SSL_CONNECTING, BEV_OPT_CLOSE_ON_FREE);
		if (!p_event)
		{
			fprintf(stderr, "Could not create new ssl bufferevent\n");
			return NULL;
		}
	}

	char ch = ip[0];
	if ((ch < '0') || (ch > '9')){
		printf("Connect by dns:%s %u\r\n", ip, port);
		struct evdns_base *dns_base;
		dns_base = evdns_base_new(p_base, 1);		
		bufferevent_enable(p_event, EV_READ | EV_WRITE);		
		int ret = bufferevent_socket_connect_hostname(p_event, dns_base, AF_UNSPEC, ip, port);
		if (ret != 0){
			log("bufferevent_socket_connect_hostname error");
			bufferevent_free(p_event);
			return NULL;
		}
	}
	else{
		printf("Connect by ip:%s %u\r\n", ip, port);
		if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
			log("inet_pton");
			return NULL;
		}
		
		if ((sockfd = bufferevent_socket_connect(p_event, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
			log("bufferevent_socket_connect ");
			bufferevent_free(p_event);
			return NULL;
		}
	}
	
//	evutil_make_socket_nonblocking(sockfd);

	CLXZMessage* pMsg = new CLXZMessage();
	pMsg->setMode(SM_WRITE);
	pMsg->setAppData(this);

	tolua_pushusertype(_state, pMsg, "CLXZMessage");
	int ref = lua_ref(_state,-1);
	pMsg->SetLuaRef(ref);

	lua_pushlightuserdata(_state,p_event);
	ref = lua_ref(_state, -1);
	pMsg->setEvent(ref);

	//
	pMsg->setAppData1(ssl);

	//设置回掉函数
	bufferevent_setcb(p_event, _read_cb2, _write_cb2, _error_cb2, pMsg);

	//设置该事件的属性
	log("connect ip:%s %d bev:%p\n",ip, port, p_event);
	return p_event;
}

static int lua_Connect(lua_State* L){
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	const char* ip = tolua_tostring(L, 2, "");
	int port = (int)tolua_tonumber(L,3, 0);
	int ssl = (int)tolua_toboolean(L, 4, 0);
	struct bufferevent * bev=pSvr->Connect(ip, port, ssl==1);
	lua_pushlightuserdata(L,bev);
	return 1;
}

static int lua_SetClientData(lua_State* L)
{
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	struct bufferevent *bev=(struct bufferevent *)lua_touserdata(L,2);
	LXZuint32 data=(LXZuint32)lua_tonumber(L,3);
	if(bev->cbarg){
		CLXZMessage* pMsg = (CLXZMessage*)bev->cbarg;
		pMsg->setData(data);
	}
	
	return 0;
}

static int lua_createtable_new(lua_State* L)
{
	LXZuint32 narr = (LXZuint32)lua_tonumber(L, 1);
	LXZuint32 nhres = (LXZuint32)lua_tonumber(L, 2);
	lua_createtable(L, narr, nhres);
	return 1;
}

static int lua_GetClientData(lua_State* L)
{
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	struct bufferevent *bev=(struct bufferevent *)lua_touserdata(L,2);	
	if(bev->cbarg){
		CLXZMessage* pMsg = (CLXZMessage*)bev->cbarg;
		tolua_pushnumber(L, pMsg->getData());
		return 1;
	}

	return 0;
}


struct self_tv{  
	struct event* timeout;  
	struct timeval tv;  
	lua_State* thread;  
	int ref;
};  


void _lua_wait_timer_cb(int fd, short event, void *arg){
	self_tv* p =(self_tv*)arg;
	if(lua_status((lua_State*)p->thread)==LUA_YIELD){				
		int ret = lua_resume((lua_State*)p->thread,0);			 
		if(ret!=LUA_YIELD&&ret!=0){						
			lua_pop((lua_State*)p->thread,-1);
		}
	}

	event_free(p->timeout);
	lua_unref((lua_State*)p->thread, p->ref);
	delete p;
}

static int lua_wait_time(lua_State* L)
{	
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);

	size_t _timer = (size_t)tolua_tonumber(L, 2, 0);		
	if(_timer>0){			
		self_tv* p = new self_tv;		
		p->tv.tv_sec= _timer/1000;
		p->tv.tv_usec=(_timer%1000)*1000; //_timer为毫秒
		p->thread = lua_tothread(L,3);
		if(p->thread==NULL){
			lae_log("thread is null");
		}

		lua_pushthread((lua_State*)p->thread);
		p->ref=lua_ref((lua_State*)p->thread,LUA_REGISTRYINDEX);		
		p->timeout = event_new(pSvr->GetBase(), -1, 0, _lua_wait_timer_cb, p);
		evtimer_add(p->timeout, &p->tv);
	}
	return 0;
}

static int lua_GetClientIP(lua_State* L)
{
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	struct bufferevent *bev=(struct bufferevent *)lua_touserdata(L,2);	
	evutil_socket_t fd = bufferevent_getfd(bev);
	if(fd==-1){
		return 0;
	}

	struct sockaddr addr;
	socklen_t len=sizeof(addr);
	getpeername(fd,&addr, &len);

	char ip[256]={0};
	inet_ntop(AF_INET, &addr, ip, sizeof(ip));
	tolua_pushstring(L, ip);
	return 1;
}

static int lua_SSHandShake(lua_State* L)
{
	CEventServer* pSvr = (CEventServer*)tolua_tousertype(L, 1, NULL);
	struct bufferevent *bev = (struct bufferevent *)lua_touserdata(L, 2);
	int ret = pSvr->SSLHandShake(bev);
	tolua_pushnumber(L, ret);
	return 1;
}

static int lua_CloseClient(lua_State* L)
{
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	struct bufferevent *bev=(struct bufferevent *)lua_touserdata(L,2);
	pSvr->CloseClient(bev);
	return 0;
}

static int lua_GetInputBufferMsg(lua_State* L)
{
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	struct bufferevent *bev=(struct bufferevent *)lua_touserdata(L,2);
	CLXZMessage* pMsg = (CLXZMessage*)bev->cbarg;	
	tolua_pushusertype(L, pMsg, "CLXZMessage");		
	return 1;
}

static int lua_CloseReq(lua_State* L)
{
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);
	if(req&&req->evcon)evhttp_connection_free_on_completion(req->evcon);
	return 0;
}

static int lua_getpid(lua_State* L)
{
	tolua_pushnumber(L, getpid());
	return 1;
}

static int lua_string_to_table(lua_State* L){
	size_t count=0;
	const unsigned char* v = (const unsigned char*)luaL_checklstring(L, 1, &count);	
	LXZuint32 start = tolua_tonumber(L,2, 0);
	LXZuint32 stop  = tolua_tonumber(L,3, 0);

	int cnt = stop-start;
	const unsigned char* p=v+start-1;
	lua_newtable(L);	
	for(int i=0;i<cnt;i++)
	{
		lua_pushnumber(L,1+i);		
		lua_pushnumber(L,p[i]);
		lua_settable(L,-3);
	}
	//lae_log("start:%d stop:%d, cnt:%d\n", start, stop, cnt);
	return 1;
}

static int to_little_endian(lua_State* L){
	size_t n = 0;
	size_t t = 0;
	size_t count=0;
	const unsigned char* v = (const unsigned char*)luaL_checklstring(L, 1, &count);	
	if(count<=0){
		tolua_pushnumber(L, 0);
		return 1;
	}

	CLXZMessage msg;
    msg.Write((const char*)v,count);
	msg.setMode(SM_READ);
	if(count==4){
		LXZuint32 d=0;
		msg.Serial(d);
		tolua_pushnumber(L, d);		
	}else if(count==8){
		LXZuint32 d=0,dd=0;
		msg.Serial(d);	
		msg.Serial(dd);
		tolua_pushnumber(L, (long)dd<<32|d);		
	}
	else{
		return 0;
	}
	
	
	return 1;
}

static int lua_Print(lua_State* L)
{	
	size_t count = 0;
	const char* txt = (const char*)lua_tolstring(L, 1, &count);
	FILE* file = fopen("gs.log","a+");
	if(file){		
		fwrite(txt, count, 1, file);
		fwrite("\n", 1, 1, file);
		fclose(file);
	}
	return 0;
}

static int lua_new_CEventServer(lua_State* L)
{		
	CEventServer* pSvr = new CEventServer();
	tolua_pushusertype(L, pSvr, "CEventServer");
	return 1;
}

static int lua_start_CEventServer(lua_State* L)
{	
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	const char* cfgName =(const char*)tolua_tostring(L, 2, "");	
	int ret = pSvr->Start(cfgName);
	tolua_pushnumber(L, ret);
	return 1;
}

static int lua_stop_CEventServer(lua_State* L)
{
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	int ret = (int)tolua_toboolean(L, 2, 0);
	pSvr->Stop(ret==1);
	return 0;
}

static int lua_dofile_CEventServer(lua_State* L)
{	
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	const char* cfgName =(const char*)tolua_tostring(L, 2, "");	
	int ret = pSvr->_do_lua_file(cfgName);
	tolua_pushnumber(L, ret);
	return 1;
}



static int lua_Notify_CEventServer(lua_State* L)
{	
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	int           code=(int)tolua_tonumber(L,2,0);
	CLXZMessage*  pMsg=(CLXZMessage*)tolua_tousertype(L, 3, NULL);
	pSvr->Notify(code,*pMsg);
	return 0;
}


void CEventServer::CloseClient(struct bufferevent* bev){
	if(bev->cbarg){
		CLXZMessage* pMsg = (CLXZMessage*)bev->cbarg;
		if(pMsg->getEvent()!=-1){
			lua_unref(_state,pMsg->getEvent());
		}

		if(pMsg->GetLuaRef()!=-1){
			lua_unref(_state, pMsg->GetLuaRef());
		}
		delete pMsg;
		bev->cbarg=NULL;
	}
	log("CloseClient bev:%p\n",bev);
	bufferevent_free(bev);
}

extern "C" void lae_log(const char* fmt,...){
	va_list args;
	va_start(args, fmt);
	vsnprintf (print_buf,PRINT_BUFF_LEN,fmt, args);
	va_end(args);

	print_buf[sizeof(print_buf)-1]=0;

	FILE* file = fopen("log/gs.log","a+");
	if(file){		
		fwrite(print_buf, strnlen(print_buf, PRINT_BUFF_LEN), 1, file);
		fclose(file);
	}
}

void CEventServer::log(const char* fmt,...){
	va_list args;
	va_start(args, fmt);
	vsnprintf (print_buf,PRINT_BUFF_LEN,fmt, args);
	va_end(args);
	print_buf[sizeof(print_buf)-1]=0;
	printf("%s", print_buf);

	FILE* file = fopen("log/gs.log","a+");
	if(file){		
		fwrite(print_buf, strnlen(print_buf, PRINT_BUFF_LEN), 1, file);
		fclose(file);
	}
}

struct evhttp_request *CEventServer::connect2websocket(const char* url)
{
	struct http_request_get *http_req_get = (struct http_request_get*)start_http_requset(_base,
		url,
		REQUEST_GET_FLAG,
		NULL, NULL);

	http_req_get->thread = NULL;
	http_req_get->thread_ref = -1;

	struct evkeyvalq headers;
	memset(&headers, 0x00, sizeof(headers));
	//evhttp_add_header(&headers,)

	CLXZMessage* msg = new CLXZMessage();
	msg->setMode(SM_WRITE);
	http_req_get->event_server = this;
	http_req_get->respone_msg = msg;
	return NULL;
}

/*http
struct http_request_post *http_req_post = start_http_requset(base,  
	"http://172.16.239.93:8899/base/truck/delete",  
	REQUEST_POST_FLAG,  
	HTTP_CONTENT_TYPE_URL_ENCODED,  
	"name=winlin&code=1234");  
struct http_request_get *http_req_get = start_http_requset(base,  
	"http://127.0.0.1?name=winlin",  
	REQUEST_GET_FLAG,  
	NULL, NULL);  
*/

int  CEventServer::send_http_get(const char* url,lua_State* co){
	struct http_request_get *http_req_get =(struct http_request_get*)start_http_requset(_base,  
		url,  
		REQUEST_GET_FLAG,  
		NULL, NULL);  
	if(http_req_get->req==NULL){
		log("connect fail:%s\r\n", url);
		free(http_req_get);
		return -1;
	}

	http_req_get->thread=co;
	if(co){
		lua_pushthread(co);
		http_req_get->thread_ref = lua_ref(co,LUA_REGISTRYINDEX);
	}else{
		http_req_get->thread_ref=-1;
	}

	CLXZMessage* msg = new CLXZMessage();
	msg->setMode(SM_WRITE);
	http_req_get->event_server = this;
	http_req_get->respone_msg  = msg;	
	return 0;
}

int  CEventServer::send_http_post(const char* url,const char* data,lua_State* co){
	struct http_request_post *http_req_post = (struct http_request_post*)start_http_requset(_base,
		url,  
		REQUEST_POST_FLAG,  
		HTTP_CONTENT_TYPE_URL_ENCODED,  
		data);  
	if(http_req_post->req==NULL){
		log("connect fail:%s\r\n", url);
		free(http_req_post);
		return -1;
	}

	http_req_post->thread=co;
	if(co){
		lua_pushthread(co);
		http_req_post->thread_ref = lua_ref(co,LUA_REGISTRYINDEX);
	}else{
		http_req_post->thread_ref=-1;
	}

	CLXZMessage* msg = new CLXZMessage();
	msg->setMode(SM_WRITE);
	http_req_post->event_server = this;
	http_req_post->respone_msg  = msg;
	return 0;
}

//void evhttp_send_reply(struct evhttp_request *req, int code,
//	const char *reason, struct evbuffer *databuf);
static int lua_evhttp_send_reply(lua_State* L){
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);
	int code = (int)tolua_tonumber(L,2, 200);
	const char* reason =(const char*)tolua_tostring(L, 3, "");
	size_t count=0;		
	const char* v = (const char*)luaL_checklstring(L, 4, &count);	
	struct evbuffer *databuf = evbuffer_new();
	evbuffer_add(databuf, v, count);
	evhttp_send_reply(req, code, reason, databuf);
	evbuffer_free(databuf);
	return 0;
}

static int lua_evhttp_send_raw(lua_State* L) {
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);	
	size_t count = 0;
	const char* v = (const char*)luaL_checklstring(L, 2, &count);
	struct bufferevent* bev = evhttp_connection_get_bufferevent(req->evcon);
	bufferevent_write(bev, v, count);
	return 0;
}


static int lua_evhttp_request_get_uri(lua_State* L){
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);
	tolua_pushstring(L,evhttp_request_get_uri(req));
	return 1;
}

static int set_char_zero(unsigned char* utf8Buf, int nLen)
{
	int offset = 0;
	while (nLen > offset) {

		//DWORD code_point = *(DWORD*)(utf8Buf + offset);
		LXZuint32 a = ((LXZuint32)utf8Buf[offset + 0]);

		int size = 0;
		if (a < 0x80)
			size = 1;
		else if (a >= 192 && a <= 223)
			size = 2;
		else if (a >= 224 && a <= 239)
			size = 3;
		else if (a >= 240 && a <= 247)
			size = 4;
		else if (a >= 248 && a <= 251)
			size = 5;
		else
			size = 6;
		if (nLen < (offset + size)) {
			for (int t = offset; t < nLen; ++t) {
				utf8Buf[t] = 0x00;
			}
			break;
		}
		offset += size;
	}
	return offset;
}

static int lua_set_char_zero(lua_State* L) {
	size_t count = 0;
	const char* str = luaL_checklstring(L, 1, &count);
	unsigned char* buff = new unsigned char[count + 1];
	memset(buff, 0x00, count + 1);
	memcpy(buff, str, count);
	size_t size = set_char_zero(buff, count);
	lua_pushlstring(L, (char*)buff, size);
	return 1;
}



static int lua_evhttp_uri_get_path(lua_State* L){
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);
	const char* uri = evhttp_request_get_uri(req);
	struct evhttp_uri *decoded = evhttp_uri_parse(uri);
	if(decoded==NULL) return 0;
	const char *path = evhttp_uri_get_path(decoded);	
	if (!path) path = "/";
	tolua_pushstring(L,path);
	evhttp_uri_free(decoded);
	return 1;
}

static int lua_evhttp_uri_get_query(lua_State* L){
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);
	const char* uri = evhttp_request_get_uri(req);
	struct evhttp_uri *decoded = evhttp_uri_parse(uri);
	if(decoded==NULL) return 0;
	const char *path = evhttp_uri_get_query(decoded);	
	if (!path) path = "";
	tolua_pushstring(L,path);
	evhttp_uri_free(decoded);
	return 1;
}

static int lua_evhttp_uri_get_host(lua_State* L){
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);
	const char* uri = evhttp_request_get_uri(req);
	struct evhttp_uri *decoded = evhttp_uri_parse(uri);
	if (decoded == NULL) return 0;
	const char *path = evhttp_uri_get_host(decoded);
	if (!path) path = "";
	tolua_pushstring(L, path);
	evhttp_uri_free(decoded);
	return 1;
}

static int lua_evhttp_parse_query(lua_State* L){
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);
	const char* uri = evhttp_request_get_uri(req);	
	struct evkeyvalq headers;
	memset(&headers,0x00, sizeof(headers));	
	//lae_log("lua_evhttp_parse_query:%s\n", uri);
	evhttp_parse_query(uri, &headers);
	lua_newtable(L);//创建一个表格，放在栈顶
	struct evkeyval *header;
	for (header = headers.tqh_first; header;
		header = header->next.tqe_next) {
			//lae_log("  %s: %s\n", header->key, header->value);
			lua_pushstring(L, header->key);//压入key
			lua_pushstring(L, header->value);//压入value
			lua_settable(L,-3);//弹出key,value，并设置到table里面去
	}
	evhttp_clear_headers(&headers);
	return 1;
}


static int lua_evhttp_parse_string_query(lua_State* L) {
	size_t count = 0;
	const char* str = luaL_checklstring(L, 1, &count);
	struct evkeyvalq headers;
	memset(&headers, 0x00, sizeof(headers));
	evhttp_parse_query_str(str, &headers);
	lae_log("lua_evhttp_parse_query\n");
	lua_newtable(L);//创建一个表格，放在栈顶
	struct evkeyval *header;
	for (header = headers.tqh_first; header;
	header = header->next.tqe_next) {
		//printf("  %s: %s\n", header->key, header->value);

		lua_pushstring(L, header->key);//压入key
		lua_pushstring(L, header->value);//压入value
		lua_settable(L, -3);//弹出key,value，并设置到table里面去
	}
	evhttp_clear_headers(&headers);
	return 1;
}

static int lua_evhttp_uridecode(lua_State* L){
	const char* path = tolua_tostring(L, 1, "");
	char* decoded_path = evhttp_uridecode(path, 0, NULL);
	if(decoded_path){
		tolua_pushstring(L, decoded_path);
		free(decoded_path);
		return 1;
	}
	return 0;
}

static void heetp_chunk_cb(struct evhttp_request *req, void* arg){
	CEventServer* pEvr = (CEventServer*)arg;
	pEvr->chunk_cb(req);
}

void CEventServer::chunk_cb(struct evhttp_request *req){
	//_lua_proc(CMD_EVENT_CHUNK, NULL, 0,NULL,-1);
	int top = lua_gettop(_state); 
	lua_getref(_state, _lua_ref);
	lua_gettable(_state, LUA_GLOBALSINDEX);
	if(!lua_isfunction(_state, -1)){
		lua_settop(_state, top);	
		return;
	}

	//
	tolua_pushnumber(_state, CMD_EVENT_CHUNK);
	lua_pushlightuserdata(_state, req);
	if (lua_pcall(_state, 2, 0/*LUA_MULTRET*/, 0) != 0) {
		//debug_message("error running function `f': %s", lua_tostring(m_pState, -1));
		const char* err_ =lua_tostring(_state, -1);
		log("lau_pcall: %d %s\n", CMD_EVENT_CHUNK, err_);
		lua_settop(_state,top);
	}
}

static int lua_evhttp_request_set_chunked_cb(lua_State* L){
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);
	evhttp_request_set_chunked_cb(req, heetp_chunk_cb);
	return 0;
}

static int lua_evhttp_add_header(lua_State* L){
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);
	const char* key_ = tolua_tostring(L, 2, "");
	const char* value_ = tolua_tostring(L, 3, "");
	struct evkeyvalq *headers = evhttp_request_get_output_headers(req);
	evhttp_add_header(headers, key_, value_);
	return 0;
}

static int lua_evhttp_close_websocket(lua_State* L)
{
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);
	evhttp_connection_free_on_completion(req->evcon);	
	return 1;
}

static int lua_evhttp_is_websocket(lua_State* L)
{
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);
	tolua_pushboolean(L, req->websocket ? 1 : 0);
	return 1;
}

static int lua_tousertype(lua_State* L)
{
	void *req = (void *)lua_topointer(L, 1);
	const char* szType = (const char*)tolua_tostring(L, 2, "");
	tolua_pushusertype(L, req, szType);
	return 1;
}

static int lua_evhttp_get_input_buffers(lua_State* L){
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);
	struct evbuffer *buf=evhttp_request_get_input_buffer(req);
	size_t len = evbuffer_get_length(buf);
	FastBufAlloc<char,1024> alloc;
	alloc.grow(len+1);
	int n=evbuffer_remove(buf, alloc.buf(), len);
	if(n>0){
		lua_pushlstring(L, alloc.buf(), len);
		return 1;
	}
	return 0;
}

static int lua_evhttp_get_output_buffers(lua_State* L) {
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);
	struct evbuffer *buf = evhttp_request_get_output_buffer(req);
	lua_pushlightuserdata(L, buf);
	return 1;
}

static int lua_evbuffer_add_file(lua_State* L) {
	struct evbuffer *buf = (struct evbuffer *)tolua_touserdata(L, 1, NULL);
	const char* fdname  = tolua_tostring(L, 2, 0);
	ev_off_t offset = tolua_tonumber(L, 3, 0);
	ev_off_t length = tolua_tonumber(L, 4, 0);

	return 0;
}

static int lua_evhttp_get_input_headers(lua_State* L){
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL);
	lua_newtable(L);//创建一个表格，放在栈顶
	struct evkeyvalq *headers = evhttp_request_get_input_headers(req);
	struct evkeyval *header;
	for (header = headers->tqh_first; header;
		header = header->next.tqe_next) {
			//printf("  %s: %s\n", header->key, header->value);			
			lua_pushstring(L, header->key);//压入key
			lua_pushstring(L, header->value);//压入value
			lua_settable(L,-3);//弹出key,value，并设置到table里面去
	}
	return 1;
}

static int lua_CEventServer_SendHttpPost(lua_State* L){
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	const char* url  = (const char*)tolua_tostring(L, 2, "");
	const char* data = (const char*)tolua_tostring(L, 3, "");
	lua_State* co = NULL;
	if(lua_isthread(L,4)){
		co = (lua_State*)lua_tothread(L, 4);
	}

	int ret = pSvr->send_http_post(url, data,co);
	tolua_pushboolean(L, ret == 0 ? 1 : 0);
	return 1;
}

static int lua_CEventServer_SendHttpGet(lua_State* L){
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	const char* url  = (const char*)tolua_tostring(L, 2, "");	
	lua_State* co = NULL;
	if(lua_isthread(L,3)){
		co = (lua_State*)lua_tothread(L, 3);
	}

	int ret = pSvr->send_http_get(url,co);
	tolua_pushboolean(L, ret == 0 ? 1 : 0);
	return 1;
}

static int lua_CEventServer_GetCfg(lua_State* L){
	CEventServer* pSvr=(CEventServer* )tolua_tousertype(L,1,NULL);
	tolua_pushusertype(L, pSvr->GetCfg(), "ILXZCoreCfg");
	return 1;
}


/*-------------------------------------------------------------------------*\
* Gets time in s, relative to January 1, 1970 (UTC) 
* Returns
*   time in s.
\*-------------------------------------------------------------------------*/
#ifdef _WIN32
#pragma comment(lib, "winmm.lib")
double timeout_gettime(void) {
	static DWORD s=0;
    DWORD t=timeGetTime();  
	if(s==0) {
		s=t;
	}
	return (double)(t-s);
}
#else
double timeout_gettime(void) {
    struct timeval v;
    gettimeofday(&v, (struct timezone *) NULL);
    /* Unix Epoch time (time since January 1, 1970 (UTC)) */
	static uint64_t s=0; 
    uint64_t ss= v.tv_sec*1000 + v.tv_usec*0.001;
	if(s==0){
		s=ss;
		return 0.0;
	}
	return (double)(ss-s);
}
#endif


static int lua_timeout_gettime(lua_State* L)
{
	tolua_pushnumber(L, timeout_gettime());
	return 1;
}

static int lLuaLXZMessageBox(lua_State* pState)
{
	const char* szMessage = tolua_tostring(pState, 1, "");
#ifdef WIN32
	MessageBox(NULL,szMessage, "Debug", MB_OK);
	//#else
	//	LXZOutputDebugStr(szMessage);
#endif
	return 0;
};

extern "C" int luaopen_lfs (lua_State *L);

static int lua_evhttp_get_remote_host(lua_State* L){
	struct evhttp_request *req = (struct evhttp_request *)tolua_touserdata(L, 1, NULL); 
	tolua_pushstring(L, req->remote_host);
	return 1;
}

static int lua_evhttp_int64_2_uint64(lua_State* L) {
	const char* src = tolua_tostring(L, 1, "0");
	LXZuint64 srcD = atoll(src);
	char ret[64] = { 0 };
#ifdef _WIN32
	sprintf(ret, "%I64u", srcD);
#else
	sprintf(ret, "%llu", srcD);
#endif
	tolua_pushstring(L, ret);
	return 1;
}

#define cast(t, exp)	((t)(exp))
#define cast_void(i)	cast(void, (i))
#define cast_byte(i)	cast(unsigned char, (i))
static unsigned int lua_S_hash(const char *str, size_t l, unsigned int seed) {
	unsigned int h = seed ^ cast(unsigned int, l);
	size_t step = (l >> 5) + 1;
	for (; l >= step; l -= step)
		h ^= ((h << 5) + (h >> 2) + cast_byte(str[l - 1]));
	return h;
}


static int lua_evhttp_hash(lua_State* L)
{
	static unsigned int hash = 0;
	if (hash == 0) {
		hash = rand();
	}

	size_t count = 0;
	const char* v = (const char*)luaL_checklstring(L, 1, &count);
	unsigned int d = lua_S_hash(v, count, hash);
	tolua_pushnumber(L, d);
	return 1;
}

static int lua_evhttp_get_null(lua_State* L)
{
	lua_pushlightuserdata(L, NULL);
	return 1;
}

static int lua_evhttp_int64_op(lua_State* L)
{	
	const char* src = tolua_tostring(L, 1, "0");
	const char* op  = tolua_tostring(L, 2, "+");
	const char* dst = tolua_tostring(L, 3, "0");	

#ifdef _WIN32
	const char* fmt = "%I64d";
#else
	const char* fmt = "%lld";
#endif

	LXZuint64 srcD = (src!=NULL)?atoll(src):0;
	LXZuint64 dstD = (dst != NULL) ? atoll(dst) : 0;
	if (op[0] == '+') {
		char ret[64] = { 0 };
		sprintf(ret, fmt, (srcD + dstD));
		tolua_pushstring(L, ret);
		return 1;
	}else if (op[0] == '-') {
		char ret[64] = { 0 };
		sprintf(ret, fmt, (srcD - dstD));
		tolua_pushstring(L, ret);
		return 1;
	}
	else if (op[0] == '*') {
		char ret[64] = { 0 };
		sprintf(ret, fmt, (srcD * dstD));
		tolua_pushstring(L, ret);
		return 1;
	}
	else if (op[0] == '/') {
		char ret[64] = { 0 };
		sprintf(ret, fmt, (srcD / dstD));
		tolua_pushstring(L, ret);
		return 1;
	}
	else if (op[0] == '%') {
		char ret[64] = { 0 };
		sprintf(ret, fmt, (srcD % dstD));
		tolua_pushstring(L, ret);
		return 1;
	}
	else if (op[0] == '|') {
		char ret[64] = { 0 };
		sprintf(ret, fmt, (srcD | dstD));
		tolua_pushstring(L, ret);
		return 1;
	}
	else if (op[0] == '^') {
		char ret[64] = { 0 };
		sprintf(ret, fmt, (srcD ^ dstD));
		tolua_pushstring(L, ret);
		return 1;
	}
	else if (op[0] == '&') {
		char ret[64] = { 0 };
		sprintf(ret, fmt, (srcD & dstD));
		tolua_pushstring(L, ret);
		return 1;
	}
	else if (op[0] == '~') {
		char ret[64] = { 0 };
		sprintf(ret, fmt, ~(srcD ));
		tolua_pushstring(L, ret);
		return 1;
	}
	else if (op[0] == '<') {
		tolua_pushboolean(L, srcD < dstD ? 1 : 0);
		return 1;
	}
	else if (strcmp(op,"<=")==0) {
		tolua_pushboolean(L, srcD <= dstD ? 1 : 0);
		return 1;
	}
	else if (op[0] == '>') {
		tolua_pushboolean(L, srcD > dstD ? 1 : 0);
		return 1;
	}
	else if (strcmp(op, ">=") == 0) {
		tolua_pushboolean(L, srcD >= dstD ? 1 : 0);
		return 1;
	}
	else if (strcmp(op, "==") == 0) {
		tolua_pushboolean(L, srcD == dstD ? 1 : 0);
		return 1;
	}

	return 0;
}

static int lua_evhttp_set_max_headers_size(lua_State* L)
{
	CEventServer* pSvr = (CEventServer*)tolua_tousertype(L, 1, NULL);
	size_t size = (size_t)tolua_tonumber(L, 2, 0);
	evhttp_set_max_headers_size(pSvr->GetHttpServer(), size);
	return 0;
}

static int lua_evhttp_set_max_body_size(lua_State* L)
{
	CEventServer* pSvr = (CEventServer*)tolua_tousertype(L, 1, NULL);
	size_t size = (size_t)tolua_tonumber(L, 2, 0);
	evhttp_set_max_body_size(pSvr->GetHttpServer(), size);
	return 0;
}

int CEventServer::init_lua_state(lua_State* L)
{	
	luaL_openlibs(L);

	int ret = LuaLXZMessageInitial(L);
	LuaLXZCoreCfgInitial(L);
	luaopen_lfs(L);
	luaopen_mime_core(L);
	luaopen_cjson(L);

	tolua_open(L);
	tolua_module(L,NULL,0);
	tolua_beginmodule(L,NULL);
	tolua_function(L, "Print", lua_Print);	
	tolua_function(L, "createtable_new", lua_createtable_new);	
	tolua_function(L, "c_le_uint_to_num", to_little_endian);	
	tolua_function(L, "string_to_table", lua_string_to_table);	
	tolua_function(L, "getpid", lua_getpid);
	tolua_function(L, "gettime", lua_timeout_gettime);
	tolua_function(L, "luaopen_cjson",    luaopen_cjson);	
	tolua_function(L, "luaopen_snapshot", luaopen_snapshot);
	tolua_function(L, "LXZMessageBox",    lLuaLXZMessageBox);
	tolua_function(L, "evhttp_send_reply",   lua_evhttp_send_reply);		
	tolua_function(L, "evhttp_send_raw", lua_evhttp_send_raw);
	tolua_function(L, "evhttp_get_input_headers",   lua_evhttp_get_input_headers);
	tolua_function(L, "evhttp_get_input_buffers",   lua_evhttp_get_input_buffers);
	tolua_function(L, "evhttp_is_websocket", lua_evhttp_is_websocket);	
	tolua_function(L, "evhttp_request_get_uri",   lua_evhttp_request_get_uri);
	tolua_function(L, "evhttp_uri_get_path",   lua_evhttp_uri_get_path);
	tolua_function(L, "evhttp_uridecode",   lua_evhttp_uridecode);
	tolua_function(L, "evhttp_add_header",   lua_evhttp_add_header);	
	tolua_function(L, "evhttp_uri_get_query",   lua_evhttp_uri_get_query);	
	tolua_function(L, "evhttp_parse_query",   lua_evhttp_parse_query);	
	tolua_function(L, "evhttp_uri_get_host", lua_evhttp_uri_get_host);	
	tolua_function(L, "evhttp_parse_string_query", lua_evhttp_parse_string_query);	
	tolua_function(L, "evhttp_request_set_chunked_cb",   lua_evhttp_request_set_chunked_cb);	
	tolua_function(L, "evhttp_get_remote_host",   lua_evhttp_get_remote_host);	
	tolua_function(L, "evhttp_int64_op", lua_evhttp_int64_op);
	tolua_function(L, "evhttp_int64_2_uint64", lua_evhttp_int64_2_uint64);
	tolua_function(L, "SetCharZero", lua_set_char_zero);
	tolua_function(L, "evhttp_set_max_headers_size", lua_evhttp_set_max_headers_size);
	tolua_function(L, "evhttp_set_max_body_size", lua_evhttp_set_max_body_size);	
	tolua_function(L, "tousertype", lua_tousertype);
	tolua_function(L, "evhttp_hash", lua_evhttp_hash);	
	tolua_variable(L, "null", lua_evhttp_get_null, NULL);
				
	tolua_usertype(L, "CEventServer");
	tolua_cclass(L, "CEventServer", "CEventServer", "", lua_EventServer_delete);
	tolua_beginmodule(L, "CEventServer");
	tolua_function(L, "new",   lua_new_CEventServer);
	tolua_function(L, "Send2Client", lua_Send2Client);
	tolua_function(L, "SendData2Client", lua_SendData2Client);
	tolua_function(L, "Connect", lua_Connect);
	tolua_function(L, "CloseClient", lua_CloseClient);
	tolua_function(L, "Start",   lua_start_CEventServer);	
	tolua_function(L, "Stop",   lua_stop_CEventServer);	
	tolua_function(L, "DoFile",   lua_dofile_CEventServer);	
	tolua_function(L, "Notify",   lua_Notify_CEventServer);
	tolua_function(L, "GetCfg",         lua_CEventServer_GetCfg);
	tolua_function(L, "SendHttpPost",   lua_CEventServer_SendHttpPost);
	tolua_function(L, "SendHttpGet",   lua_CEventServer_SendHttpGet);
	tolua_function(L, "GetClientData",   lua_GetClientData);
	tolua_function(L, "SetClientData",   lua_SetClientData);
	tolua_function(L, "GetClientIP",   lua_GetClientIP);	
	tolua_function(L, "GetInputBufferMsg",   lua_GetInputBufferMsg);	
	tolua_function(L, "CloseReq", lua_CloseReq);	
	tolua_function(L, "WaitTime", lua_wait_time);	
	tolua_function(L, "ThreadExecute", lua_ThreadExecute);	
	tolua_function(L, "GetLuaThread", lua_GetLuaThread);	
	tolua_function(L, "GetLuaThreadTaskCount", lua_GetLuaThreadTaskCount);
	tolua_function(L, "SetLuaThreadMaxTask", lua_SetLuaThreadMaxTask);	
	tolua_function(L, "evhttp_get_output_buffers", lua_evhttp_get_output_buffers);	
	tolua_function(L, "evbuffer_add_file", lua_evbuffer_add_file);	
	tolua_function(L, "SSHandShake", lua_SSHandShake);	
	tolua_endmodule(L);

	tolua_endmodule(L);
		
	tolua_pushusertype(L,this,"CEventServer");
	lua_setglobal(L,"EventServer");

	return ret;
}

void CEventServer::notify_timer_cb(){
	vvector<CLXZMessage*> vvMessage;
	_lock.Linux_Win_Locked();
	vvMessage.swap(_vvMsg);
	_lock.Linux_Win_UnLocked();
	for(int i=0;i<vvMessage.size();i++){
		_lua_proc(CMD_EVENT_NOTIFY, vvMessage[i],vvMessage[i]->getSystemData());
	}
}

void generic_handler_error(struct evhttp_request *req, enum evhttp_request_error error, void *arg) {
	CEventServer* pSvr = (CEventServer*)arg;
	pSvr->request_cb_error(req, arg, error);
}

void generic_handler(struct evhttp_request *req, void *arg)
{	
	CEventServer* pSvr = (CEventServer*)arg;
	evhttp_request_set_error_cb(req, generic_handler_error,pSvr);
	pSvr->request_cb(req, arg);
}

int  CEventServer::as_http_server(const char* ip, int port){
	_http_server = evhttp_new(_base);
	if(!_http_server)
	{
		log("as_http_server fail:%s %d\n",ip, port);
		return -1;
	}

	int ret = evhttp_bind_socket(_http_server,ip,port);
	if(ret!=0)
	{
		log("as_http_server evhttp_bind_socket fail:%s %d\n",ip, port);
		return -1;
	}

	//evhttp_set_timeout(_http_server, 10);

#ifdef EVENT__HAVE_OPENSSL
	if (USE_HTTPS) {
		SSL_CTX *ctx = SSL_CTX_new(SSLv23_server_method());
		SSL_CTX_set_options(ctx,
			SSL_OP_SINGLE_DH_USE |
			SSL_OP_SINGLE_ECDH_USE |
			SSL_OP_NO_SSLv2);

		/* Cheesily pick an elliptic curve to use with elliptic curve ciphersuites.
		* We just hardcode a single curve which is reasonably decent.
		* See http://www.mail-archive.com/openssl-dev@openssl.org/msg30957.html */
		EC_KEY *ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
		if (!ecdh)
			die_most_horribly_from_openssl_error("EC_KEY_new_by_curve_name");
		if (1 != SSL_CTX_set_tmp_ecdh(ctx, ecdh))
			die_most_horribly_from_openssl_error("SSL_CTX_set_tmp_ecdh");

		const char *certificate_chain = "server.pem";
		const char *private_key = "server.key";
		/* 设置服务器证书 和 服务器私钥 到
		OPENSSL ctx上下文句柄中 */
		server_setup_certs(ctx, certificate_chain, private_key);

		evhttp_set_bevcb(_http_server, bevcb, ctx);
	}

#endif

	evhttp_set_gencb(_http_server, generic_handler, this);
	return 0;
}

#define CA_CERT_FILE "ca.crt"
#define CLIENT_CERT_FILE "client.crt"
int CEventServer::DoProc(){	
	// get cfg	
	if(_cfg.load(_cfg_name)==false){
		return -1;
	}

	USE_HTTPS  = _cfg.GetInt("https", NULL, NULL,0);
	int _timer = _cfg.GetInt("timer", NULL, NULL, -1);
	int port = _cfg.GetInt("port", NULL, NULL, 8888);
	int httpport = _cfg.GetInt("httpport", NULL, NULL, 0);
	const char* ip = _cfg.GetCString("ip", NULL, NULL, "127.0.0.1");
	const char* httpip = _cfg.GetCString("httpip", NULL, NULL, "localhost");
	const char* luafile = _cfg.GetCString("lua", NULL, NULL, "game_server.lua");
	log("IP:%s Port:%d %s http ip:%s port:%d\n", ip, port, luafile, httpip,httpport);

	// lua
	_state = lua_open();
	init_lua_state(_state);
	//RegisterInt642Lua(_state);
	if(_do_lua_file(luafile)!=0){
		log("_do_lua_file error\r\n");
		lua_close(_state);
		_state = NULL;
		return -2;
	}

	tolua_pushstring(_state, "server_dispacher");
	_lua_ref =lua_ref(_state, -1);
	if(_lua_ref==-1){
		log("server_dispacher lua_ref error\r\n");
		lua_close(_state);
		_state = NULL;
		return -3;
	}

#ifdef WIN32
	evthread_use_windows_threads();

	struct event_config* cfg = event_config_new();
	event_config_set_flag(cfg, EVENT_BASE_FLAG_STARTUP_IOCP);
	//根据CPU实际数量配置libEvent的CPU数
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	event_config_set_num_cpus_hint(cfg, si.dwNumberOfProcessors);
		
	_base = event_base_new_with_config(cfg);
	event_config_free(cfg);
#else
	evthread_use_pthreads();

	// base
	_base = event_base_new();
#endif

#ifdef EVENT__HAVE_OPENSSL	
	common_setup();
#endif

	// tcp server
	if(port>0){
		_listen(ip,port);	
	}

	//http
	if(httpport>0&&httpip){
		as_http_server(httpip,httpport);
		log("as_http_server\r\n");
	}


	// 使用SSL V3,V2  
#ifdef EVENT__HAVE_OPENSSL	
	/*	SSLeay_add_ssl_algorithms();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	ERR_load_BIO_strings();*/

	ctx_client = SSL_CTX_new(SSLv23_method());
	if (ctx_client == NULL)
	{
		printf("SSL_CTX_new error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	// 要求校验对方证书，表示需要验证服务器端，若不需要验证则使用  SSL_VERIFY_NONE
	SSL_CTX_set_verify(ctx_client, SSL_VERIFY_NONE, NULL);


	// 加载CA的证书
	printf("SSL_CTX_load_verify_locations start!\n");
	if (!SSL_CTX_load_verify_locations(ctx_client, CA_CERT_FILE, NULL))
	{
		printf("SSL_CTX_load_verify_locations error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	// 加载自己的证书  
	if (SSL_CTX_use_certificate_file(ctx_client, CLIENT_CERT_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		printf("SSL_CTX_use_certificate_file error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}
#endif

	// user timer
	struct event *timeout = NULL;
	if(_timer>0){				
		struct timeval tv;
		tv.tv_sec= _timer/1000;
		tv.tv_usec=(_timer%1000)*1000; //_timer为毫秒
		log("user timer:%d sec:%d usec:%d\r\n",_timer, tv.tv_sec, tv.tv_usec);
		timeout = event_new(_base, -1, EV_PERSIST, _timeout_cb, this);//evtimer_new(_base, _timeout_cb, arg);
		evtimer_add(timeout, &tv);
	}

	// notify timer
	_user_trigger=evuser_new(_base, _usertrigger_cb, this);

	// notify timer
	struct event *notify_timeout = NULL;
	struct timeval tv_ = {1,0}; //_timer为毫秒
	notify_timeout = event_new(_base, -1, EV_PERSIST, _notify_timeout_cb, this);//evtimer_new(_base, _timeout_cb, arg);
	evtimer_add(notify_timeout, &tv_);
	
	// running
	event_base_dispatch(_base);

	// delete user timer.
	if(timeout)	evtimer_del(timeout);

	//delete notify timer
	if(notify_timeout)	evtimer_del(notify_timeout);
	
	lua_close(_state);
	_state = NULL;

	//
	if(_destroy) delete this;
	printf("gameserver destroyed.");
	return 0;
}

/*thread safe transfer message.*/
int CEventServer::Notify(int code, CLXZMessage& msg){
	CLXZMessage* pMsg=new CLXZMessage(msg);
	pMsg->setSystemData(code);
	_lock.Linux_Win_Locked();
	_vvMsg.push_back(pMsg);
	_lock.Linux_Win_UnLocked();
	return 0;
}


/*
 * File: service.h
 */

#include "util.h"
 
#ifndef _SERVICE_H_
#define _SERVICE_H_

typedef enum {login, logout, servertime, browser, redirect, getfile, putfile, addcart, delcart, checkout, cclose} command_t;

typedef struct cookies {
	char* key;
	char* val;
	struct cookies* next;
} cookies_t;

typedef struct request {
	http_method method;

	char* URI;
	char* command;
	command_t e_command;
	char* query;
	char* protocol;
	char* cookie;
	char* cookie_username;
	char* accept_charset;
	char* accept_encoding;
	char* accept_language;
	char* cache_control;
	char* connection;
	char* content_length;
	char* host;
	char* pragma;
	char* user_agent;
	char* if_modified_since;
	cookies_t* cookies_head;
	
	const char* body;
} request;

typedef struct response {
	char* protocol;
	char*status;
	char* char_status;
	char* connection;
	char* cache_control;
	char* content_length;
	char* content_type;
	char* date;
	char* last_modified;
	
	char* body;
} response;

void create_request(char* buffer, request* req, cookies_t* cookies);
void handle_client(int socket);
void process_servertime(int socket, response* res);
void create_success_response(request* req, response* res);
int sendall(int s, char *buf, int *len);
void cookies_insert(request *req, cookies_t* cookies, char* key, char* val);
void create_error404_response(response* res);
void create_501_response(response* res);
void create_seeother303_response(response* res);
void createForbiddenResponse(response* res);
char *substr (const char *inpStr, int startPos, int strLen);

void freeCookieList(cookies_t* head);
cookies_t* findCookieKey(cookies_t* head, char* key);
void deleteItem(cookies_t* head,char* key);
void reorderList(cookies_t* head);

#endif

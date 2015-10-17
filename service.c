/*
* File: service.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#include "service.h"
#include "util.h"

void handle_client(int socket) {

	/* TODO Loop receiving requests and sending appropriate responses,
	*      until one of the conditions to close the connection is
	*      met.
	*/

	while(1) {
		/*if (send(socket, "respon\n", 8, 0) == -1)
		perror("send");	*/

		char *buffer = (char*) malloc(1000);
		int numbytes;
		int totalbytesread;

		if ((numbytes = recv(socket, buffer, 1000-1, 0)) == -1) {
			perror("recv");
			exit(1);
		}
		buffer[numbytes] = '\0';
		totalbytesread = numbytes;

		while(http_header_complete(buffer, totalbytesread) == -1) {

			buffer = (char*) realloc(buffer, sizeof(buffer) + 1000);

			/*if (send(socket, "while\n", 7, 0) == -1)
			perror("send");	*/

			char temp_buffer[1000];
			if ((numbytes = recv(socket, temp_buffer, 1000-1, 0)) == -1) {
				perror("recv");
				exit(1);
			}
			temp_buffer[numbytes] = '\0';
			totalbytesread = totalbytesread + numbytes;
			strcat(buffer, temp_buffer);
			/*if (send(socket, "while1\n", 8, 0) == -1)
			perror("send");	*/

		}
		cookies_t cookies;
		request req;
		create_request(buffer, &req, &cookies);

		/*if (send(socket, "line67\n", 8, 0) == -1)
		perror("send");*/


		if (METHOD_GET == req.method) {			

			switch(req.e_command){

			case login:
				{
					response res; 
					char res_buffer[3000];
					char body[1000];
					strcpy(body, "");
					char cookie[200];
						

					if(strncmp(req.query, "username=", 9) == 0){
						
						create_success_response(&req, &res);
						res.cache_control = "Cache-Control: private\r\n";
						res.body = req.query;
						sprintf (cookie, "Set-Cookie: %s; Max-Age=86400; Path= /\r\n", req.query);
						//char *cookie = "Set-Cookie: UserID=JohnDoe; Max-Age=3600;\r\n";
						
						strcpy(res_buffer, res.protocol);
						strcat(res_buffer, res.status);
						strcat(res_buffer, res.char_status);
						strcat(res_buffer, res.connection);
						strcat(res_buffer, res.date);
						strcat(res_buffer, res.cache_control);
						strcat(res_buffer, res.content_type);
						strcat(res_buffer, cookie);
					}
					else{
						createForbiddenResponse(&res);
						res.cache_control = "Cache-Control: private\r\n";
						sprintf(body, "Not Found.\r\n");
						res.body = body;


						strcpy(res_buffer, res.protocol);
						strcat(res_buffer, res.status);
						strcat(res_buffer, res.char_status);
						strcat(res_buffer, res.connection);
						strcat(res_buffer, res.date);
						strcat(res_buffer, res.cache_control);	
						strcat(res_buffer, res.content_type);
						
					}
					
					int clen = strlen(res.body);
					char conLen[80];
					sprintf (conLen, "Content-Length: %d\r\n", clen);
				
					strcat(res_buffer, conLen);
					strcat(res_buffer, "\r\n");
					strcat(res_buffer, res.body);

					int length = strlen(res_buffer);
					if (sendall(socket, res_buffer, &length) == -1)
						perror("send");

					free(res.date);
					res.date = NULL;
					freeCookieList(req.cookies_head);
					/*
					int length = strlen(req.query);
					if (send(socket, req.query , length , 0) == -1)
					perror("send");
					*/
				}
				break;
			case logout:
				{
					response res; 
					char res_buffer[3000];
					create_success_response(&req, &res);
					res.cache_control = "Cache-Control: private\r\n";

					cookies_t* userNode = findCookieKey(req.cookies_head, "username");
					char body[1000];
					char cookie[500];
					strcpy(body, "");
					strcpy(cookie, "");

					if(userNode!=NULL){
						sprintf(body, "User %s was logged out\r\n", userNode->val);
						sprintf (cookie, "Set-Cookie: %s=%s; Max-Age=0; Path= /\r\n", userNode->key,userNode->val);
						
					}
					else{
						sprintf(body, "No user is logged in\r\n");
					}
					
										
					res.body = body;
					int clen = strlen(res.body);
					char conLen[80];
					sprintf (conLen, "Content-Length: %d\r\n", clen);

					strcpy(res_buffer, res.protocol);
					strcat(res_buffer, res.status);
					strcat(res_buffer, res.char_status);
					strcat(res_buffer, res.connection);
					strcat(res_buffer, res.date);
					strcat(res_buffer, res.cache_control);
					strcat(res_buffer, conLen);
					strcat(res_buffer, res.content_type);
					strcat(res_buffer, cookie);
					strcat(res_buffer, "\r\n");
					strcat(res_buffer, res.body);

					int length = strlen(res_buffer);
					if (sendall(socket, res_buffer, &length) == -1)
						perror("send");

					free(res.date);
					res.date = NULL;
					freeCookieList(req.cookies_head);
				}
				break;
			case servertime:
				{

					response res; 
					char res_buffer[3000];
					create_success_response(&req, &res);
					res.cache_control = "Cache-Control: no-cache\r\n";

					printf("in server command\n");
					cookies_t* userNode = findCookieKey(req.cookies_head, "username");
				
					time_t rawtime;
					struct tm * timeinfo;
					char date[100];
					time ( &rawtime );
					timeinfo = localtime ( &rawtime );
					strftime (date,100,"%a, %d %b %Y %T %Z",timeinfo);
					//Sun, 18 Nov 2012 04:39:54 GMT
					//printf ( "Current local time and date: %s", asctime (timeinfo) );
					
					char body[400];
					if(userNode!=NULL){
						sprintf(body, "username: %s\r\n%s", userNode->val, date);
						res.body = body;
					}
					else{					
						res.body = date;
					}

					int clen = strlen(res.body);
					char conLen[80];
					sprintf (conLen, "Content-Length: %d\r\n", clen);

					strcpy(res_buffer, res.protocol);
					strcat(res_buffer, res.status);
					strcat(res_buffer, res.char_status);
					strcat(res_buffer, res.connection);
					strcat(res_buffer, res.cache_control);
					strcat(res_buffer, conLen);
					strcat(res_buffer, res.content_type);
					strcat(res_buffer, res.date);
					strcat(res_buffer, "\r\n");
					strcat(res_buffer, res.body);

					int length = strlen(res_buffer);

					
					//printf("Cookies are %s", req.cookie);

					if (sendall(socket, res_buffer, &length) == -1)
						perror("send");

					free(res.date);
					res.date = NULL;
					freeCookieList(req.cookies_head);
				}
				break;
			case browser:
				{
					response res; 
					char res_buffer[3000];
					create_success_response(&req, &res);
					res.cache_control = "Cache-Control: private\r\n";	

					char browser[200];
					strcpy(browser, req.user_agent);
					strcat(browser, "\r\n");
					
					cookies_t* userNode = findCookieKey(req.cookies_head, "username");
					char body[400];
					if(userNode!=NULL){
						sprintf(body, "username: %s\r\n%s", userNode->val, browser);
						res.body = body;
					}
					else{					
						res.body = browser;
					}
					
					int clen = strlen(res.body);
					char conLen[80];
					sprintf (conLen, "Content-Length: %d\r\n", clen);

					strcpy(res_buffer, res.protocol);
					strcat(res_buffer, res.status);
					strcat(res_buffer, res.char_status);
					strcat(res_buffer, res.connection);
					strcat(res_buffer, res.content_type);					
					strcat(res_buffer, conLen);
					strcat(res_buffer, res.date);
					strcat(res_buffer, res.cache_control);					
					strcat(res_buffer, "\r\n");
					strcat(res_buffer, res.body);

					int length = strlen(res_buffer);

					if (sendall(socket, res_buffer, &length) == -1)
						perror("send");

					free(res.date);
					res.date = NULL;
					freeCookieList(req.cookies_head);
				}
				break;
			case redirect:
				{
				send(socket, req.query, strlen(req.query), 0);
				
				response res; 
				char res_buffer[3000];
				create_seeother303_response(&res);
				res.cache_control = "Cache-Control: no-cache\r\n";
				
				char* query;
				query = substr(req.query, 4, strlen(req.query));
				char decoded[strlen(query)+1];
				decode(query, decoded);
				char location[strlen(query)+30];
				sprintf (location, "Location: %s\r\n", decoded);
				
				cookies_t* userNode = findCookieKey(req.cookies_head, "username");
					char body[400];
					if(userNode!=NULL){
						sprintf(body, "username: %s\r\nRedirected", userNode->val);
						res.body = body;
					}
					else{					
						res.body = "Redirected";
					}

				
				int clen = strlen(res.body);
				char conLen[80];
				sprintf (conLen, "Content-Length: %d\r\n", clen);
				
				strcpy(res_buffer, res.protocol);
				strcat(res_buffer, res.status);
				strcat(res_buffer, res.char_status);
				strcat(res_buffer, res.connection);
				strcat(res_buffer, res.content_type);					
				strcat(res_buffer, conLen);
				strcat(res_buffer, res.date);
				strcat(res_buffer, res.cache_control);
				strcat(res_buffer, location);
				strcat(res_buffer, "\r\n");
				strcat(res_buffer, res.body);	

				int length = strlen(res_buffer);
				
				if (sendall(socket, res_buffer, &length) == -1)
					perror("send");

				free(res.date);
				res.date = NULL;				
				}
				break;
			case getfile:
				{
					response res; 
					char res_buffer[3000];
					create_success_response(&req, &res);
					res.cache_control = "Cache-Control: public\r\n";
					char *file_name = substr(req.query, 9, strlen(req.query));
					struct stat *buf;
					buf = (struct stat*) malloc (sizeof(struct stat));
					
					stat(file_name, buf);
					
					time_t rawtime;
					rawtime = buf->st_mtime;
					struct tm * timeinfo;
					char date[100];
					timeinfo = localtime ( &rawtime );
					strftime (date,100,"%a, %d %b %Y %T %Z",timeinfo);
					
					strcpy(res_buffer,date);
					int length = strlen(res_buffer);
					if (sendall(socket, res_buffer, &length) == -1)
						perror("send");
						
					free(buf);
				}
				break;
			case addcart:
				{
					response res; 
					char res_buffer[3000];
					create_success_response(&req, &res);
					res.cache_control = "Cache-Control: private\r\n";

					//check if person is logged in
					cookies_t* userNode = findCookieKey(req.cookies_head, "username");
					char body[2000];
					if(userNode!=NULL){
						sprintf(body, "username: %s\r\n", userNode->val);
					}
					else{
						strcpy(body, "");
					}
					//check if there are any previous items in the cart
					userNode = findCookieKey(req.cookies_head, "item1");
					int i = 1;
					char key[10];
					// this should be right
					while(userNode){
						//printf("entered while loop. i: %d\n", i)

						sprintf(body+strlen(body), "%d. %s\r\n" , i, userNode->val);
						i++;
						sprintf(key, "item%d", i);
						userNode = findCookieKey(req.cookies_head, key);
					}
					
					//set cookie for the response
					char cookie[200];
					char *itemName = strchr(req.query, '=') + 1;
					sprintf (cookie, "Set-Cookie:item%d= %s; Max-Age=86400; Path= /\r\n", i, itemName);
					//char *cookie = "Set-Cookie: UserID=JohnDoe; Max-Age=3600;\r\n";
					
					//add cookie to body that came from this command to addcart
					sprintf(body+strlen(body), "%d. %s\r\n" , i, itemName);

					res.body = body;
					int clen = strlen(res.body);
					char conLen[80];
					sprintf (conLen, "Content-Length: %d\r\n", clen);

					strcpy(res_buffer, res.protocol);
					strcat(res_buffer, res.status);
					strcat(res_buffer, res.char_status);
					strcat(res_buffer, res.connection);
					strcat(res_buffer, res.date);
					strcat(res_buffer, res.cache_control);
					strcat(res_buffer, conLen);
					strcat(res_buffer, res.content_type);
					strcat(res_buffer, cookie);
					strcat(res_buffer, "\r\n");
					strcat(res_buffer, res.body);

					int length = strlen(res_buffer);
					if (sendall(socket, res_buffer, &length) == -1)
						perror("send");

					free(res.date);
					res.date = NULL;
					freeCookieList(req.cookies_head);
					/*
					int length = strlen(req.query);
					if (send(socket, req.query , length , 0) == -1)
					perror("send");
					*/

				}

				break;
			case delcart:
				{
					response res; 
					char res_buffer[3000];
					create_success_response(&req, &res);
					res.cache_control = "Cache-Control: private\r\n";

					//check if person is logged in
					cookies_t* userNode = findCookieKey(req.cookies_head, "username");
					char body[2000];
					if(userNode!=NULL){
						sprintf(body, "username: %s\r\n", userNode->val);
					}
					else{
						strcpy(body, "");
					}
					//find the item to delete in the cart
					char *itemNumber = strchr(req.query, '=') + 1;
					char key[10];
					sprintf(key, "item%s", itemNumber);
					deleteItem(req.cookies_head, key);


					char cookie[1000];
					strcpy(cookie, "");
					userNode = findCookieKey(req.cookies_head, key);
					char *itemNum = key + 4;
					int num = atoi (itemNum);
					

					

					//userNode = findCookieKey(req.cookies_head, key);
					
					//set cookies
					while(userNode!= NULL){
						sprintf (cookie + strlen(cookie), "Set-Cookie:%s=%s; ", userNode->key, userNode->val);
						sprintf (cookie + strlen(cookie), "Max-Age=86400; Path= /\r\n");
						userNode = findCookieKey(req.cookies_head, key);
						num++;
						sprintf(key, "item%d", num);
						userNode = findCookieKey(req.cookies_head, key);
					}
					//sprintf (cookie + strlen(cookie), "\r\n");
					//expire cookie
					if(userNode==NULL){
						//strcpy(cookie, "Set-Cookie:");
						sprintf (cookie + strlen(cookie), "Set-Cookie:%s=%s; ", key, "whatever");
						sprintf (cookie + strlen(cookie), "Max-Age=0; Path= /\r\n");
						
					}



					//output the list
					userNode = findCookieKey(req.cookies_head, "item1");
					int i = 1;					
					while(userNode){
						printf("entered while loop. i: %d\n", i);
						sprintf(body+strlen(body), "%d. %s\r\n" , i, userNode->val);
						i++;
						sprintf(key, "item%d", i);
						userNode = findCookieKey(req.cookies_head, key);
					}
					
					res.body = body;
					int clen = strlen(res.body);
					char conLen[80];
					sprintf (conLen, "Content-Length: %d\r\n", clen);

					strcpy(res_buffer, res.protocol);
					strcat(res_buffer, res.status);
					strcat(res_buffer, res.char_status);
					strcat(res_buffer, res.connection);
					strcat(res_buffer, res.date);
					strcat(res_buffer, res.cache_control);
					strcat(res_buffer, conLen);
					strcat(res_buffer, res.content_type);
					strcat(res_buffer, cookie);
					strcat(res_buffer, "\r\n");
					strcat(res_buffer, res.body);

					int length = strlen(res_buffer);
					if (sendall(socket, res_buffer, &length) == -1)
						perror("send");

					free(res.date);
					res.date = NULL;
					freeCookieList(req.cookies_head);
				}

				break;
			case checkout:
				{
					response res; 
					char res_buffer[3000];
					FILE *f;
					f = fopen("CHECKOUT.txt","a");
					//check if person is logged in
					cookies_t* userNode = findCookieKey(req.cookies_head, "username");
					char body[2000];
					char cookie[1000];
					strcpy(cookie, "");

					if(userNode!=NULL){
						create_success_response(&req, &res);
					
						sprintf(body, "username: %s\r\n", userNode->val);
						fprintf(f, "username: %s\r\n", userNode->val);
							//check if there are any previous items in the cart
						userNode = findCookieKey(req.cookies_head, "item1");
						int i = 1;
						char key[10];
						// this should be right
						while(userNode){
							//printf("entered while loop. i: %d\n", i)

							sprintf(body+strlen(body), "%d. %s\r\n" , i, userNode->val);
							fprintf(f, "%d. %s\r\n" , i, userNode->val);
							sprintf (cookie+strlen(cookie), "Set-Cookie:item%d= %s; Max-Age=0; Path= /\r\n", i, userNode->val);
						
							i++;
							sprintf(key, "item%d", i);
							userNode = findCookieKey(req.cookies_head, key);
						}
					
					}
					else{
						createForbiddenResponse(&res);
						sprintf(body, "User must be logged in to checkout.\r\n");
					}
					
					res.cache_control = "Cache-Control: private\r\n";
					res.body = body;
					int clen = strlen(res.body);
					char conLen[80];
					sprintf (conLen, "Content-Length: %d\r\n", clen);

					strcpy(res_buffer, res.protocol);
					strcat(res_buffer, res.status);
					strcat(res_buffer, res.char_status);
					strcat(res_buffer, res.connection);
					strcat(res_buffer, res.date);
					strcat(res_buffer, res.cache_control);
					strcat(res_buffer, conLen);
					strcat(res_buffer, res.content_type);
					strcat(res_buffer, cookie);
					strcat(res_buffer, "\r\n");
					strcat(res_buffer, res.body);

					int length = strlen(res_buffer);
					if (sendall(socket, res_buffer, &length) == -1)
						perror("send");

					fclose(f);
					free(res.date);
					res.date = NULL;
					freeCookieList(req.cookies_head);
					/*
					int length = strlen(req.query);
					if (send(socket, req.query , length , 0) == -1)
					perror("send");
					*/

				}
				break;
			case cclose:
				{

					response res; 
					char res_buffer[3000];
					create_success_response(&req, &res);
					res.cache_control = "Cache-Control: public\r\n";	
					res.connection = "Connection: close\r\n";

					cookies_t* userNode = findCookieKey(req.cookies_head, "username");
					char body[400];
					if(userNode!=NULL){
						sprintf(body, "username: %s\r\nThe connection will now be closed.", userNode->val);
						res.body = body;
					}
					else{					
						res.body = "The connection will now be closed.";
					}

					int clen = strlen(res.body);
					char conLen[80];
					sprintf (conLen, "Content-Length: %d\r\n", clen);

					strcpy(res_buffer, res.protocol);
					strcat(res_buffer, res.status);
					strcat(res_buffer, res.char_status);
					strcat(res_buffer, res.connection);
					strcat(res_buffer, res.content_type);					
					strcat(res_buffer, conLen);
					strcat(res_buffer, res.date);
					strcat(res_buffer, res.cache_control);					
					strcat(res_buffer, "\r\n");
					strcat(res_buffer, res.body);

					int length = strlen(res_buffer);

					if (sendall(socket, res_buffer, &length) == -1)
						perror("send");

					free(res.date);
					res.date = NULL;
					freeCookieList(req.cookies_head);
					free(buffer);
					buffer = NULL;

					goto end;
				}
				break;
			default: 
				{
					// unsupported command; send 404 error
					response res; 
					char res_buffer[3000];
					create_error404_response(&res);
					res.cache_control = "Cache-Control: public\r\n";	
					res.connection = "Connection: keep-alive\r\n";

					res.body = "Command not found.";
					int clen = strlen(res.body);
					char conLen[80];
					sprintf (conLen, "Content-Length: %d\r\n", clen);

					strcpy(res_buffer, res.protocol);
					strcat(res_buffer, res.status);
					strcat(res_buffer, res.char_status);
					strcat(res_buffer, res.connection);
					strcat(res_buffer, res.content_type);					
					strcat(res_buffer, conLen);
					strcat(res_buffer, res.date);
					strcat(res_buffer, res.cache_control);					
					strcat(res_buffer, "\r\n");
					strcat(res_buffer, res.body);

					int length = strlen(res_buffer);

					if (sendall(socket, res_buffer, &length) == -1)
						perror("send");

					free(res.date);
					res.date = NULL;
					freeCookieList(req.cookies_head);

				}
				break;
			}
		}

		// POST METHOD
		else if (METHOD_POST == req.method) {
		
			if (req.e_command == putfile) {
			
				char res_buffer[3000];
				strcpy(res_buffer, "TODO: Putfile command in POST method");
				int length = strlen(res_buffer);
				if (sendall(socket, res_buffer, &length) == -1)
						perror("send");
			}
				
			else {
			// unsupported command; send 404 error
				response res; 
				char res_buffer[3000];
				create_error404_response(&res);
				res.cache_control = "Cache-Control: public\r\n";	
				res.connection = "Connection: keep-alive\r\n";

				res.body = "Command not found.";
				int clen = strlen(res.body);
				char conLen[80];
				sprintf (conLen, "Content-Length: %d\r\n", clen);

				strcpy(res_buffer, res.protocol);
				strcat(res_buffer, res.status);
				strcat(res_buffer, res.char_status);
				strcat(res_buffer, res.connection);
				strcat(res_buffer, res.content_type);					
				strcat(res_buffer, conLen);
				strcat(res_buffer, res.date);
				strcat(res_buffer, res.cache_control);					
				strcat(res_buffer, "\r\n");
				strcat(res_buffer, res.body);

				int length = strlen(res_buffer);

				if (sendall(socket, res_buffer, &length) == -1)
					perror("send");

				free(res.date);
				res.date = NULL;
				freeCookieList(req.cookies_head);
			}
		}

		// UNSUPPORTED METHODS
		else {
			// unsupported method; send 501 Not Implemented
			response res; 
			char res_buffer[3000];
			create_501_response(&res);
			res.cache_control = "Cache-Control: public\r\n";	
			
			res.body = "Unsupported Method.";
			int clen = strlen(res.body);
			char conLen[80];
			sprintf (conLen, "Content-Length: %d\r\n", clen);

			strcpy(res_buffer, res.protocol);
			strcat(res_buffer, res.status);
			strcat(res_buffer, res.char_status);
			strcat(res_buffer, res.connection);
			strcat(res_buffer, res.content_type);					
			strcat(res_buffer, conLen);
			strcat(res_buffer, res.date);
			strcat(res_buffer, res.cache_control);					
			strcat(res_buffer, "\r\n");
			strcat(res_buffer, res.body);

			int length = strlen(res_buffer);

			if (sendall(socket, res_buffer, &length) == -1)
				perror("send");

			free(res.date);
			res.date = NULL;
			freeCookieList(req.cookies_head);
		}
		free(buffer);
		buffer = NULL;
	}

end:
	return;
}

void create_request(char* buffer, request* req, cookies_t* cookies) {

	int len = strlen(buffer);
	// FILL IN COMMAND AND QUERY
	req->URI =  http_parse_uri(buffer);
	const char *path = http_parse_path(http_parse_uri(buffer));

	if(strncmp(path, "/login", 6) == 0){
		req->e_command = login;
		req->command = "log";
	}
	if(strncmp(path, "/logout", 7) == 0){
		req->e_command = logout;
		req->command = "logout";
	}
	if(strncmp(path, "/servertime", 11) == 0){
		req->e_command = servertime;
		req->command = "servertime";
	}
	if(strncmp(path, "/browser", 8) == 0){
		req->e_command = browser;
		req->command = "browser";
	}
	if(strncmp(path, "/redirect", 9) == 0){
		req->e_command = redirect;
		req->command = "redirect";
	}
	if(strncmp(path, "/getfile", 8) == 0){
		req->e_command = getfile;
		req->command = "getfile";
	}
	if(strncmp(path, "/putfile", 8) == 0){
		req->e_command = putfile;
		req->command = "putfile";
	}
	if(strncmp(path, "/addcart", 8) == 0){
		req->e_command = addcart;
		req->command = "addcart";
	}
	if(strncmp(path, "/delcart", 8) == 0){
		req->e_command = delcart;
		req->command = "delcart";
	}
	if(strncmp(path, "/checkout", 9) == 0){
		req->e_command = checkout;
		req->command = "checkout";
	}
	if(strncmp(path, "/close", 6) == 0){
		req->e_command = cclose;
		req->command = "close";
	}

	req->cookies_head = NULL;

	// FILL IN COOKIES STRUCT
	char* cookies_str = http_parse_header_field(buffer, len, "Cookie");
	char* cookies_key = strtok(cookies_str, " =");
	char* cookies_val = strtok(NULL, " ;\r\n");
	while(cookies_key || cookies_val) {
		cookies_insert(req, cookies, cookies_key, cookies_val);
		cookies_key = strtok(NULL, " =");
		cookies_val = strtok(NULL, " ;\r\n");
	}

	//work here
	// FILL IN REST OF STUCT
	char *queryPt = strchr(path, '?');
	if(queryPt!=NULL){
		req->query = queryPt + 1;
	}
	else{
		req->query = NULL;
	}
	req->protocol = "HTTP/1.1";
	req->method = http_parse_method(buffer);
	//req->cookie = http_parse_header_field(buffer, len, "Cookie");

	req->if_modified_since = http_parse_header_field(buffer, len, "If-Modified-Since");
	req->accept_charset = http_parse_header_field(buffer, len, "Accept-Charset");
	req->accept_encoding = http_parse_header_field(buffer, len, "Accept-Encoding");
	req->accept_language = http_parse_header_field(buffer, len, "Accept-Language");
	req->cache_control = http_parse_header_field(buffer, len, "Cache-Control");
	req->connection = http_parse_header_field(buffer, len, "Connection");
	req->content_length = http_parse_header_field(buffer, len, "Content-Length");
	req->host = http_parse_header_field(buffer, len, "Host");
	req->pragma = http_parse_header_field(buffer, len, "Pragma");
	req->user_agent = http_parse_header_field(buffer, len, "User-Agent");
	//req->body = http_parse_body(buffer, len);
}

void process_servertime(int socket, response* res) {
	char* res_str = "TESTING SERVERTIME RESPONSE\r\n";

	// !!! TODO: fill in res_str from res struct WITH DATE IN BODY

	if (send(socket, res_str, 31, 0) == -1)
		perror("send");
}

void create_success_response(request* req, response* res){
	res->protocol = "HTTP/1.1";
	res->status = " 200";
	res->char_status = " OK\r\n";
	res->connection = "Connection: keep-alive\r\n";
	res->content_type = "Content-Type: text/plain\r\n";

	time_t rawtime;
	struct tm * timeinfo;
	char *date = (char*) malloc(120);
	time ( &rawtime );
	timeinfo = gmtime( &rawtime );
	strftime (date,100,"DATE: %a, %d %b %Y %T GMT\r\n", timeinfo);
	res->date = date;	
}


/*
s is the socket you want to send the data to, buf is the buffer containing the data, 
and len is a pointer to an int containing the number of bytes in the buffer.

The function returns -1 on error (and errno is still set from the call to send().) 
Also, the number of bytes actually sent is returned in len. 
This will be the same number of bytes you asked it to send, unless there was an error.
*/
int sendall(int s, char *buf, int *len)
{
	int total = 0;        // how many bytes we've sent
	int bytesleft = *len; // how many we have left to send
	int n;

	while(total < *len) {
		n = send(s, buf+total, bytesleft, 0);
		if (n == -1) { break; }
		total += n;
		bytesleft -= n;
	}

	*len = total; // return number actually sent here

	return n==-1?-1:0; // return -1 on failure, 0 on success
} 


void cookies_insert(request *req, cookies_t* cookies, char* key, char* val) {
	cookies_t *new_cookies = (cookies_t*) malloc(sizeof (cookies_t));
	new_cookies->key = key;
	printf("key is: ");
	printf(key);
	printf("\n");
	new_cookies->val = val;

	new_cookies->next = NULL;
	printf("val is: ");
	printf(val);
	printf("\n");

	if(req->cookies_head==NULL){
		req->cookies_head = new_cookies;
		return;
	}

	cookies_t* temp = req->cookies_head;
	cookies_t* prev; 
	while(temp!= NULL){
		prev = temp;
		temp = temp->next;
		printf("inside while loop\n");
	}
	prev->next = new_cookies;

	/*new_cookies->next = req->cookies_head;

	req->cookies_head = new_cookies;*/

}

void create_error404_response(response* res) {
	res->protocol = "HTTP/1.1";
	res->status = " 404";
	res->char_status = " Not found\r\n";
	res->connection = "Connection: close\r\n";
	res->content_type = "Content-Type: text/plain\r\n";

	time_t rawtime;
	struct tm * timeinfo;
	char *date = (char*) malloc(120);
	time ( &rawtime );
	timeinfo = gmtime( &rawtime );
	strftime (date,100,"DATE: %a, %d %b %Y %T GMT\r\n", timeinfo);
	res->date = date;
}

void create_501_response(response* res) {
	res->protocol = "HTTP/1.1";
	res->status = " 501";
	res->char_status = " Not Implemented\r\n";
	res->connection = "Connection: close\r\n";
	res->content_type = "Content-Type: text/plain\r\n";

	time_t rawtime;
	struct tm * timeinfo;
	char *date = (char*) malloc(120);
	time ( &rawtime );
	timeinfo = gmtime( &rawtime );
	strftime (date,100,"DATE: %a, %d %b %Y %T GMT\r\n", timeinfo);
	res->date = date;
}

void create_seeother303_response(response* res) {
	res->protocol = "HTTP/1.1";
	res->status = " 303";
	res->char_status = " See Other\r\n";
	res->connection = "Connection: keep-alive\r\n";
	res->content_type = "Content-Type: text/plain\r\n";

	time_t rawtime;
	struct tm * timeinfo;
	char *date = (char*) malloc(120);
	time ( &rawtime );
	timeinfo = gmtime( &rawtime );
	strftime (date,100,"DATE: %a, %d %b %Y %T GMT\r\n", timeinfo);
	res->date = date;
}

void createForbiddenResponse(response* res){
	res->protocol = "HTTP/1.1";
	res->status = " 403";
	res->char_status = " Forbidden\r\n";
	res->connection = "Connection: keep-alive\r\n";
	res->content_type = "Content-Type: text/plain\r\n";

	time_t rawtime;
	struct tm * timeinfo;
	char *date = (char*) malloc(120);
	time ( &rawtime );
	timeinfo = gmtime( &rawtime );
	strftime (date,100,"DATE: %a, %d %b %Y %T GMT\r\n", timeinfo);
	res->date = date;
}
char *substr (const char *inpStr, int startPos, int strLen) {
	
	char* buff;
	
    if (inpStr == NULL) return NULL;
    if ((buff = (char*) malloc (strLen + 1)) == NULL)
        return NULL;

    /* Transfer string section and return it. */
    memcpy (buff, &(inpStr[startPos]), strLen);
    buff[strLen] = '\0';

    return buff;
}

void freeCookieList(cookies_t* head){
	cookies_t* node = head;
	cookies_t* temp;
	while(node!= NULL){
		temp = node;
		node = node->next;
		free(temp);
	}
	head=NULL;


}

cookies_t* findCookieKey(cookies_t* head, char* key){
	cookies_t* node = head;
	printf("looking for key: ");
	printf(key);
	printf("\n");
	while(node!=NULL)
	{
		if(strcmp(node->key, key) == 0) //key is found.
		{

			printf(node->key);
			printf(": ");
			printf(node->val);

			printf(key);
			printf(" found \n");
			return node;
		}
		
		printf("going through the linked list\n");
		printf(node->key);
		printf("\n");
		printf("key not found yet\n");

		node = node->next;//Search in the next node.
	}
	
	printf(key);
	printf(" not found \n");
	/*Key is not found */
	return NULL;

}
void deleteItem(cookies_t* head,char* key){
	
	if(head == NULL){
		return;
	}

	//cookies_t* node= findCookieKey(head, key);
	cookies_t* current = head;
	cookies_t* prev;

	// delete node from beginning
	if(strcmp(current->key, key) == 0){
		head = current->next;
			
		free(current);
		current = NULL;

		reorderList(head);
		return;
	}


	// delete node from middle or end - this while loop will run at least once because node to be deleted is not the head node
	while(current && (strcmp(current->key, key) != 0)){
		prev = current;
		current = current->next;
	}

	if(current){
			prev->next = current->next;	
			printf("item deleted: ");
			printf(current->key);
			printf("\n");
			free(current);
			current = NULL;
			reorderList(prev->next);
	}
	return;	 
}

/* 
*/
void reorderList(cookies_t* head){
	cookies_t* node = head;

	while(node!=NULL){
		if(node->key != NULL){
			printf("entered line 859\n");
			printf("node's key is: ");
			printf(node->key);
			printf("\n");
			if((strncmp(node->key, "item",4))==0){
				printf("key matched entered if statement\n");
				char *itemNum = node->key + 4;
				printf("itemNumber is: ");
				printf(itemNum);
				printf("\n");
				int num = atoi (itemNum);
				if(num == 0){
					printf("num turned out to be 0 somehow");
					return;
				}
				num = num-1;
				sprintf(node->key, "item%d", num);
				node = node->next;
			}
			else{
			printf("entered line 875\n");
				node = node->next;	
			}
			
		}
		else{
			printf("entered line 877\n");
				node = node->next;	
			}
	}
	return;

}

//int sizeOfList(cookies_t* head){
//	cookies_t* node = head;
//	int i = 0;
//	while(node!=NULL){
//		node = node->next;
//		i++;
//	}
//	return i; 
//}
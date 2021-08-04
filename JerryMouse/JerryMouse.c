#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdarg.h>
#include<pthread.h>
#include<winsock2.h>

#define JerryMousePoolSize 21

//Typedefs

typedef struct {
char **arr;
int size; 
}SplittedArray;

typedef struct {
}Cookie;

typedef struct{
}Session;


typedef struct 
{
SOCKET clientSocket;
Session session;
}Response;

typedef struct {
const char requestType[10];
const char * queryString;
Cookie * cookies;
}Header;                                    

typedef struct
{
const char key[256];
const char value[256];
}KeyValuePair;

typedef struct {
KeyValuePair ** kvps;
int size;
}QueryStringData;

typedef struct{
SOCKET clientSocket;
Session session;
QueryStringData *qsd;
}Request;
	

typedef  void (*Process) (Request * request , Response * response);

typedef struct{
const char * processType ;
Process process; 
}Service;

typedef  Service * (*Resolver) (const char *);


//Utiltiy

struct node
{
struct node * next;
int *client_socket;
};

struct node* head = NULL; 
struct node* tail = NULL;

int * dequeue()
{
if(head == NULL) 
{ 
return NULL;
} 
else 
{ 
int *result = head->client_socket; 
struct node *temp = head;
head = head->next;
if (head == NULL) 
{
tail = NULL;
} 
free (temp); 
return result;
}
}

void enqueue (int * client_socket)
{
struct node *newnode = (struct node*)malloc(sizeof(struct node));
newnode->client_socket = client_socket;
newnode->next = NULL; 
if(tail == NULL) 
{ 
head = newnode; 
} 
else 
{ 
tail->next = newnode;
}
tail = newnode;
}

const char * getExtension(const char * str)
{
const char * dot = strrchr(str,'.');
if(!dot || dot == str) return "";
return dot+1;
}

const char * getMimeType(const char * str)
{
if(!strcmp("html",str)) return "text/html";
if(!strcmp("js",str)) return "application/x-javascript";
if(!strcmp("xml",str)) return "text/xml";
if(!strcmp("jpg",str)) return "image/jpeg";
if(!strcmp("json",str)) return "application/json";
if(!strcmp("jpeg",str)) return "image/jpeg";
if(!strcmp("png",str)) return "image/png";
if(!strcmp("ico",str)) return "image/x-icon";
if(!strcmp("css",str)) return "text/css";
if(!strcmp("gif",str)) return "image/gif";
if(!strcmp("txt",str)) return "text/plain";
return "*/*";
}

void join(SplittedArray * sa ,const char * delim,char * result)
{
for(int i=0; i<sa->size ; i++)
{
strcat(result,sa->arr[i]);
if( i < sa->size-1 )strcat(result,delim);
}
}

void split(char * str,const char * token,SplittedArray * splittedArray)
{
char * arr[1024];

int i=0;

char *t=strtok(str,token);
while(t != NULL)
{
arr[i]=t;
t=strtok(NULL,token);
i++;
}

splittedArray->size=i;
splittedArray->arr=arr;

}

int indexOf(const char *s ,const char * delim)
{
int found=-1;
char str[strlen(s)+1];
strcpy(str,s);
for(int i=0;i<strlen(str);i++)
{
if(str[i] == *delim)
{
found=1;
break;
}
}
return found;
}

void replaceAll(char *str,char old,char new)
{
int i=0;
while(str[i] != '\0')
{
if(str[i] == old ) str[i]=new;
i++;
}
}

int endswith(const char * s , const char * suffix)
{
if(!s || !suffix) return 0;
char str[strlen(s)+1];
strcpy(str,s);
int strLength=strlen(str);
int suffixLength=strlen(suffix);
if(suffixLength > strLength) return 0;
return strcmp(str + strLength-suffixLength,suffix)==0;
}


//Global Varibale
Resolver JerryMouseResolver;
int JerryMousePortNumber=7860;
const char * webApplication;
const char * JerryMouseIPAddress="127.0.0.1";
const char * clientIpAddress;
const char * clientPortNumber;
pthread_t threadPool[JerryMousePoolSize];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var= PTHREAD_COND_INITIALIZER;


//Framework Methods

void  setResolver(Resolver resolver)
{
JerryMouseResolver = resolver;
}

void setPortNumber(int portNumber)
{
JerryMousePortNumber=portNumber;
}

void setIPAddress(const char * ipAddress)
{
JerryMouseIPAddress=ipAddress;
}

void setAppliationName(const char * wa)
{
webApplication=wa;
}

int sendHeader(Response *rs , int responseCode ,const char * mimeType)
{
char  h[1024];
if(responseCode == 200)
{
if(!strcmp(mimeType,"text/html")) sprintf(h,"HTTP/1.1 200 OK\r\nContent-Type: %s; charset=UTF-8\r\n\r\n",mimeType);
else sprintf(h,"HTTP/1.1 200 OK\r\nContent-Type: %s;\r\n\r\n",mimeType);
if(send(rs->clientSocket,h,strlen(h), 0) != -1 ) return 1;
return 0;
}
if(responseCode == 500)
{
sprintf(h,"HTTP/1.1 500 INTERNAL SERVER ERROR\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n");
printf("%s\n",h);
if(send(rs->clientSocket,h,strlen(h), 0) != -1 ) return 1;
return 0;
}
if(responseCode == 404)
{
sprintf(h,"HTTP/1.1 404 NOT FOUND\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n");
printf("%s\n",h);
if(send(rs->clientSocket,h,strlen(h), 0) != -1 ) return 1;
return 0;
}

if(responseCode == 405)
{
sprintf(h,"HTTP/1.1 405 METHOD NOT ALLOWED\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n");
printf("%s\n",h);
if(send(rs->clientSocket,h,strlen(h), 0) != -1 ) return 1;
return 0;
}

return 0;
}

int sendFile(Response *rs,FILE *f)
{
char buffer[1024] = {0};
int len=0;
int result=0;
if(f == NULL)
{
return 0;
}
else
{
while((result=fread(buffer,1,1024,f)) > 0)
{
len=send(rs->clientSocket, buffer, result, 0);
if(len == -1) return 0;
if(len == 0) return 1;
}
}
}

int sendString(Response *rs,const char * str)
{
if(send(rs->clientSocket,str,strlen(str),0) != -1)
{
return 1;
}
return 0;
}

void get(Request *rs,const char * str  , ...)
{

}

int getInt(Request *rs ,const char * name)
{
QueryStringData *qsd=rs->qsd;
for(int i = 0 ; i <  qsd->size ; i++)
{
if(!strcmp(qsd->kvps[i]->key,name))
{
return atoi((char *)qsd->kvps[i]->value);
}
}
}


float getFloat(Request *rs ,const char * name)
{
QueryStringData *qsd=rs->qsd;
for(int i = 0 ; i <  qsd->size ; i++)
{
if(!strcmp(qsd->kvps[i]->key,name))
{
return atof((char *)qsd->kvps[i]->value);
}
}
}

const char * getString(Request * rs,const char * name)
{
QueryStringData *qsd=rs->qsd;
for(int i = 0 ; i <  qsd->size ; i++)
{
if(!strcmp(qsd->kvps[i]->key,name))
{
return qsd->kvps[i]->value;
}
}
}

Session * getSession(Request * rs)
{
}

void setCookie(Session * s , const char * key , const char * value)
{
}

void getCookie(Session * s , const char * key)
{
}

void getCookies(Session * s)
{
}


void parseHttpHeader(Header *header,char * buffer)
{
char requestType[10],queryString[4096];
sscanf(buffer,"%s  %s",requestType,queryString);
header->queryString=queryString;
sprintf((char *)header->requestType,"%s",requestType);
}

void getKeyValuePair(char * str,KeyValuePair * kvp)
{
char temp[strlen(str)+1];
strcpy(temp,str);
SplittedArray sa;
split(temp,"=",&sa);
sprintf((char *)kvp->key,"%s",sa.arr[0]);
sprintf((char *)kvp->value,"%s",sa.arr[1]);
}



void parseQueryString(const char * arr , QueryStringData * qsd)
{
char * url=strdup(arr);
for(int i=0;i<strlen(url);i++)
{
if(url[i] == '+') url[i]=' ';
}
KeyValuePair * kvps[1024];
char *ee;
char * e = strtok_r(url,"&",&ee);
int i=0;
KeyValuePair * kvp;
while(e != NULL )
{
char temp[strlen(e)+1];
strcpy(temp,e);
kvp=(KeyValuePair *)malloc(sizeof(KeyValuePair));
getKeyValuePair(temp,kvp);
kvps[i]=kvp;
e=strtok_r(NULL,"&",&ee);
i++;
}
qsd->kvps=kvps;
qsd->size=i;
}


void * requestProcessor (void * p_client_socket )
{
SOCKET clientSocket = *(int *)p_client_socket ;
free(p_client_socket);
Request * request=(Request*)malloc(sizeof(Request));
Response * response=(Response*)malloc(sizeof(Response));
request->clientSocket=clientSocket;
response->clientSocket=clientSocket;
char buffer[4096];
recv(clientSocket,buffer,sizeof(buffer),0);
Header header;
parseHttpHeader(&header,buffer);
const char * queryString  = header.queryString;
printf("queryString : %s\n",queryString);

if(strlen(queryString) == 0)
{
free(request);
free(response);
closesocket(clientSocket);
return NULL;
}

if(!strcmp(queryString,"/favicon.ico"))
{
FILE * f ;
if(f = fopen("default\\favicon.ico","rb"))
{
sendHeader(response,200,getMimeType("ico"));
sendFile(response,f);
fclose(f);
}
else sendHeader(response,500,getMimeType("html"));
free(request);
free(response);
closesocket(clientSocket);
return NULL;
}

if(!strcmp(queryString,"/"))
{
FILE * f;
f = fopen("default\\index.html","rb");
if(f != NULL)
{
sendHeader(response,200,getMimeType("html"));
sendFile(response,f);
fclose(f);
}
else 
{
sendHeader(response,500,getMimeType("html"));
}
free(request);
free(response);
closesocket(clientSocket);
return NULL;
}


else
{

SplittedArray splittedArray;

char tempQsSplittedArray[strlen(queryString)+1] ;
strcpy(tempQsSplittedArray,queryString);
split(tempQsSplittedArray,"/",&splittedArray);

if(endswith(queryString,"/") && splittedArray.size == 1)
{
if(!strcmp(splittedArray.arr[0],webApplication))
{
FILE * f;
f = fopen("public\\index.html","rb");
if(f != NULL)
{
sendHeader(response,200,getMimeType("html"));
sendFile(response,f);
fclose(f);
}
else 
{
sendHeader(response,404,getMimeType("html"));
}
free(request);
free(response);
closesocket(clientSocket);
return NULL;
}

if(!strcmp(splittedArray.arr[0],"default"))
{
FILE * f;
f = fopen("default\\index.html","rb");
if(f != NULL)
{
sendHeader(response,200,getMimeType("html"));
sendFile(response,f);
fclose(f);
}
else 
{
sendHeader(response,500,getMimeType("html"));
}
free(request);
free(response);
closesocket(clientSocket);
return NULL;
}

}//if endswith

else
{
SplittedArray splittedByQuestionMark,splittedByUrlSeparator;
int index=indexOf(queryString,"?");
if(index != -1 )
{
char tempQsSplittedByQuestionMark[strlen(queryString)+1] ;
strcpy(tempQsSplittedByQuestionMark,queryString);
split(tempQsSplittedByQuestionMark,"?",&splittedByQuestionMark);

if(splittedByQuestionMark.size == 2)
{
QueryStringData qsd;
parseQueryString(splittedByQuestionMark.arr[1],&qsd);
request->qsd=&qsd;
char tempQs[strlen(splittedByQuestionMark.arr[0])+1] ;
strcpy(tempQs,splittedByQuestionMark.arr[0]);
split(tempQs,"/",&splittedByUrlSeparator);
if(!strcmp(splittedByUrlSeparator.arr[0],webApplication))
{
if(endswith(splittedByUrlSeparator.arr[splittedByUrlSeparator.size-1],".service"))
{
char serviceName[30];
sprintf(serviceName,"%s",splittedByUrlSeparator.arr[splittedByUrlSeparator.size-1]);
printf("%s\n",serviceName);
Service * service=JerryMouseResolver(serviceName);
if(service != NULL)
{
Process process=service->process;
printf("%s\n",service->processType);
printf("%s\n",header.requestType);
if(!strcmp(service->processType,header.requestType))
{
process(request,response);
}
else
{
printf("Method NOT ALLOWED !!!!");
sendHeader(response,405,"text/html");
}
free(request);
free(response);
closesocket(clientSocket);
}
}
}
}
else
{
//more than one ?
}
}//indexof 
else
{
if(!strcmp(splittedArray.arr[0],webApplication))
{
char path[4096];
splittedArray.arr[0]="public";
join(&splittedArray,"\\",path);
FILE *f;
if( f= fopen((const char *)path,"rb")) 
{
sendHeader(response,200,getMimeType(getExtension((const char *)path)));
sendFile(response,f);
fclose(f);
}
else sendHeader(response,404,getMimeType("html"));
free(request);
free(response);
closesocket(clientSocket);
return NULL;
}
else
{
if(!strcmp(splittedArray.arr[0],"default"))
{
char path[4096];
join(&splittedArray,"\\",path);
FILE *f;
if( f= fopen((const char *)path,"rb")) 
{
sendHeader(response,200,getMimeType(getExtension((const char *)path)));
sendFile(response,f);
fclose(f);
}
else sendHeader(response,404,getMimeType("html"));
free(request);
free(response);
closesocket(clientSocket);
return NULL;
}
}
}
}//endswith
}//else

free(request);
free(response);
closesocket(clientSocket);
return NULL;
}//request processor ends

void * threadFunction(void *args)
{
while(1)
{
int *p_client_socket;
pthread_mutex_lock(&mutex);
pthread_cond_wait(&condition_var,&mutex);
p_client_socket=dequeue();
pthread_mutex_unlock(&mutex);
if(p_client_socket != NULL)
{
requestProcessor(p_client_socket);
}
}
}

void setUpThreadPool()
{
for(int i=0 ; i < JerryMousePoolSize ; i++) 
{
pthread_create(&threadPool[i],NULL,threadFunction,NULL);
}
}

void setUpServer()
{
int status,addr_len;
WSADATA wsa;
SOCKET server_socket,client_socket;
struct sockaddr_in server_addr , client_addr;
status=WSAStartup(MAKEWORD(2,2),&wsa);
if(status != 0) {} 
server_socket=socket(AF_INET,SOCK_STREAM,0);
server_addr.sin_family = AF_INET;
server_addr.sin_addr.s_addr = inet_addr(JerryMouseIPAddress);
server_addr.sin_port=htons(JerryMousePortNumber);
status=bind(server_socket,(struct sockaddr *)&server_addr,sizeof(server_addr));
if(status == SOCKET_ERROR) {}
status=listen(server_socket,128);
if(status == SOCKET_ERROR) {}
printf("listening on port  %d...\n",JerryMousePortNumber);
while(1)
{
addr_len=sizeof(client_addr);
client_socket=accept(server_socket,(struct sockaddr *)&client_addr,&addr_len);
if(client_socket ==  INVALID_SOCKET){}
else
{
int *p_client_socket=(int *)malloc(sizeof(SOCKET));
*p_client_socket=client_socket;
pthread_mutex_lock(&mutex);
enqueue(p_client_socket);
pthread_cond_signal(&condition_var);
pthread_mutex_unlock(&mutex);
}
}
}

void start()
{
setUpThreadPool();
setUpServer();
}

//Developer
void addEmployee(Request *,Response *);

Service * getService(const char * name)
{
if(!strcmp(name,"addEmployee.service"))
{
Service * service ;
service =(Service *)malloc(sizeof(Service));
service->processType="GET";
service->process=&addEmployee;
return service;
} 
return NULL;
}

void addEmployee(Request * request ,Response * response)
{
char buff[51];
const char * name=getString(request,"nm");
const char * gender=getString(request,"sex");
int cityCode = getInt(request,"ct");
printf("name :  %s\n",name);
printf("gender :%s\n",gender);
printf("city :  %d\n",cityCode);
sendHeader(response,200,getMimeType("html"));
sendString(response,"<html>");
sendString(response,"<body>");
sendString(response,"<center>");
sendString(response,"<h1><i>DataSaved</i></h1>");
sprintf(buff,"<b>Name : %s </b><br>",name);
sendString(response,buff);
sprintf(buff,"<b>Gender : %s </br>",gender);
sendString(response,buff);
sprintf(buff,"<b>City : %d </br>",cityCode);
sendString(response,buff);
sendString(response,"</center>");
sendString(response,"</body>");
sendString(response,"</html>");
}

int main()
{
Resolver resolver;
resolver=&getService;
setResolver(resolver);
setAppliationName("test");
setPortNumber(7860);
setIPAddress("127.0.0.1");
start();
return 0;
}
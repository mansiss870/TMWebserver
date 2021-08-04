#include<tmwp>
#include<windows.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
using namespace tmwp;

typedef struct _request
{
char *method;
char *resource;
char isClientSideTechnologyResource;
char *mimeType;
}REQUEST;


int extentionEquals(const char *left,const char *right)
{
char a,b;
while(*left && *right)
{
a=*left;
b=*right;
if(a>=65 && a<=90) a+=32;
if(b>=65 && b<=90) b+=32;
if(a!=b) return 0;
left++;
right++;
}
return *left==*right;
}

char * getMIMEType(char *resource)
{
char *mimeType;
mimeType=NULL;
int len=strlen(resource);
if(strlen(resource)<4) return mimeType;
int lastIndexOfDot=len-1;
while(lastIndexOfDot>0 && resource[lastIndexOfDot]!='.') lastIndexOfDot--;
if(lastIndexOfDot==0) return mimeType;
if(extentionEquals(resource+lastIndexOfDot+1,"html"))
{
mimeType=(char *)malloc(sizeof(char)*10);
strcpy(mimeType,"text/html");
}
if(extentionEquals(resource+lastIndexOfDot+1,"js"))
{
mimeType=(char *)malloc(sizeof(char)*16);
strcpy(mimeType,"text/javascript");
}
if(extentionEquals(resource+lastIndexOfDot+1,"css"))
{
mimeType=(char *)malloc(sizeof(char)*9);
strcpy(mimeType,"text/css");
}
if(extentionEquals(resource+lastIndexOfDot+1,"jpg"))
{
mimeType=(char *)malloc(sizeof(char)*11);
strcpy(mimeType,"image/jpeg");
}
if(extentionEquals(resource+lastIndexOfDot+1,"jpeg"))
{
mimeType=(char *)malloc(sizeof(char)*11);
strcpy(mimeType,"image/jpeg");
}
if(extentionEquals(resource+lastIndexOfDot+1,"png"))
{
mimeType=(char *)malloc(sizeof(char)*10);
strcpy(mimeType,"image/png");
}
if(extentionEquals(resource+lastIndexOfDot+1,"ico"))
{
mimeType=(char *)malloc(sizeof(char)*13);
strcpy(mimeType,"image/x-icon");
}

return mimeType;
}

char isClientSideResource(char *resource)
{
int i=0;
for(i=0; (resource[i]!='\0' && resource[i]!='.'); i++);
if(resource[i]=='\0') return 'N';
return 'Y'; //this will have to be change later on
}

REQUEST * parseRequest(char *bytes)
{
char method[11];
char resource[1001];
int i,j;
for(i=0; bytes[i]!=' '; i++) method[i]=bytes[i];
method[i]='\0';
i+=2;


for(j=0; bytes[i]!=' '; j++,i++) resource[j]=bytes[i];
resource[j]='\0';

REQUEST *request=(REQUEST *)malloc(sizeof(REQUEST));
request->method=(char *)malloc((sizeof(char)*strlen(method))+1);
if(resource[0]=='\0')
{
request->resource=NULL;
request->isClientSideTechnologyResource='Y';
request->mimeType=NULL;
}else{
request->resource=(char *)malloc((sizeof(char)*strlen(resource))+1);
strcpy(request->resource,resource);
request->isClientSideTechnologyResource=isClientSideResource(resource);
request->mimeType=getMIMEType(resource);
}
return request;
}

TMWebProjector:: TMWebProjector(int portNumber)
{
this->portNumber=portNumber;
this->url=NULL;
this->ptrOnRequest=NULL;
}


void TMWebProjector::onRequest(const char *url,void (*ptrOnRequest)(void))
{
if(this->url) delete [] this->url;
this->url=NULL;
this->ptrOnRequest=NULL;
if(url==NULL || ptrOnRequest==NULL) return;
this->url=new char[strlen(url)+1];
strcpy(this->url,url);
this->ptrOnRequest=ptrOnRequest;
}


TMWebProjector:: ~TMWebProjector()
{
if(this->url) delete [] this->url; // because dynamically memory allocte hogi iske liye
}



void TMWebProjector::start()
{
FILE *f;
int length;
int i;
int rc;
char g;


char requestBuffer[1024]; // A chunk of 1024+1 space for string terminator
char responseBuffer[8192]; //1024 x 8
WORD ver;  // it contains version of socket in 2 bytes
WSADATA wsaData; //neccessary information related will be filled in this struct by windows functionality
int serverSocketDescriptor;
int clientSocketDescriptor;
struct sockaddr_in serverSocketInformation;
struct sockaddr_in clientSocketInformation;
int successCode;
int bytesExtracted;
int len;

ver=MAKEWORD(1,1);  //(1.1 is the version)
WSAStartup(ver,&wsaData);
serverSocketDescriptor=socket(AF_INET,SOCK_STREAM,0);
if(serverSocketDescriptor<0)
{
printf("Unable to create socket\n");
return;
}
serverSocketInformation.sin_family=AF_INET;
serverSocketInformation.sin_port=htons(this->portNumber);
serverSocketInformation.sin_addr.s_addr=htonl(INADDR_ANY); // kisi bhi ip pr request aaye hume sb pr teyyar rehna chiye agr multiple network interface card h toh
successCode=bind(serverSocketDescriptor,(struct sockaddr *)&serverSocketInformation,sizeof(struct sockaddr));
char message[101];
if(successCode<0)
{
sprintf(message,"Unable to start TMServer on port %d",this->portNumber);
printf("%s\n",message);
WSACleanup();
return;
}
listen(serverSocketDescriptor,10);
sprintf(message,"TMServer is ready to accept request on port %d",this->portNumber);
printf("%s\n",message);
len=sizeof(clientSocketInformation);
while(1)
{
clientSocketDescriptor=accept(serverSocketDescriptor,(struct sockaddr *)&clientSocketInformation,&len);
if(clientSocketDescriptor<0)
{
printf("Unable to accept client connection");
closesocket(serverSocketDescriptor);
WSACleanup();
return;
}

bytesExtracted=recv(clientSocketDescriptor,requestBuffer,sizeof(requestBuffer),0);

//printf("%s\n",bytesExtracted);
if(bytesExtracted<0)
{
//what to do yet to be decided
}else{
if(bytesExtracted==0)
{
//yet to be decided

}else{
requestBuffer[bytesExtracted]='\0';
REQUEST *request=parseRequest(requestBuffer);
printf("Request arrived for: %s\n",request->resource);
if(request->isClientSideTechnologyResource=='Y')
{
if(request->resource==NULL)
{
f=fopen("index.html","rb");
if(f==NULL)
{
f=fopen("index.htm","rb");
}
if(f==NULL)
{
strcpy(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:156\n\n<DOCTYPE HTML><html lang='en'><head><meta charset='UTF-8'><title>Jerry-Tom</title></head><body><h2 style='color:red'>Resource / not found</h2></body></html>");
successCode=send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
}
else
{
fseek(f,0,2); // move the internal pointer to the end of file
length=ftell(f);
fseek(f,0,0); // move the internal pointer to teh start of file
sprintf(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:%s\nContent-Length:%d\nKeep-Alive: timeout=5, max=1000\n\n",request->mimeType,length);
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
i=0;
while(i<length)
{
rc=length-i;
if(rc>1024) rc=1024;
fread(&responseBuffer,rc,1,f);
send(clientSocketDescriptor,responseBuffer,1024,0);
i+=rc;
}
fclose(f);
}
}
else
{
f=fopen(request->resource,"rb");
if(f==NULL)
{
printf("Sending 404 page\n");
char tmp[501];
sprintf(tmp,"<DOCTYPE HTML><html lang='en'><head><meta charset='UTF-8'><title>Jerry-Tom</title></head><body><h2 style='color:red'>Resource %s not found</h2></body></html>",request->resource);
sprintf(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:%d\n\n",strlen(tmp));
strcat(responseBuffer,tmp);
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
}
else
{
fseek(f,0,2); // move the internal pointer to the end of file
length=ftell(f);
fseek(f,0,0); // move the internal pointer to teh start of file
sprintf(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:%s\nContent-Length:%d\nKeep-Alive:timeout=5,max=1000\n\n",request->mimeType,length);
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
i=0;
while(i<length)
{
rc=length-i;
if(rc>1024) rc=1024;
fread(&responseBuffer,rc,1,f);
send(clientSocketDescriptor,responseBuffer,1024,0);
i+=rc;
}
fclose(f);
}
}
}
else
{
//what to do in case of server side resource, is yet to be decided
if(this->url==NULL)
{
printf("Sending 404 page\n");
char tmp[501];
sprintf(tmp,"<DOCTYPE HTML><html lang='en'><head><meta charset='UTF-8'><title>Jerry-Tom</title></head><body><h2 style='color:red'>Resource %s not found</h2></body></html>",request->resource);
sprintf(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:%d\n\n",strlen(tmp));
strcat(responseBuffer,tmp);
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
}else{
int ii=0;
if(this->url[0]=='/') ii=1;
if(strcmp(this->url+ii,request->resource)==0)
{
this->ptrOnRequest();
printf("Sendig processed page\n");
char tmp[501];
sprintf(tmp,"<DOCTYPE HTML><html lang='en'><head><meta charset='UTF-8'><title>Jerry-Tom</title></head><body><h2 style='color:blue'>Resource %s processed</h2></body></html>",request->resource);
sprintf(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:%d\n\n",strlen(tmp));
strcat(responseBuffer,tmp);
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
}else
{
printf("Sending 404 page else\n");
char tmp[501];
sprintf(tmp,"<DOCTYPE HTML><html lang='en'><head><meta charset='UTF-8'><title>Jerry-Tom</title></head><body><h2 style='color:red'>Resource %s not found</h2></body></html>",request->resource);
sprintf(responseBuffer,"HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length:%d\n\n",strlen(tmp));
strcat(responseBuffer,tmp);
send(clientSocketDescriptor,responseBuffer,strlen(responseBuffer),0);
}
}



}

}

}
}

/*if(successCode>0)
{
printf("responseBuffer sent\n");
}
else{
printf("Unable to send responseBuffer\n");
}*/
closesocket(serverSocketDescriptor);
closesocket(clientSocketDescriptor);
WSACleanup();
return;
}
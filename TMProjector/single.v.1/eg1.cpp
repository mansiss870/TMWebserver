#include<TMWP.h>
#include<stdio.h>
void get_names()
{
ServiceContainer *sc=get_service_container("one.com");
RESPONSE *response=sc->response;
Response_writer *reswriter=response->response_writer;
response->mimeType="text/html";
FILE *f=fopen("data.txt","r");
char name[101];
write_response(reswriter,"<!DOCTYPE HTML>");
write_response(reswriter,"<html lang='en'>");
write_response(reswriter,"<head>");
write_response(reswriter,"<title>");
write_response(reswriter,"<meta charset='utf-8'>");
write_response(reswriter,"list of names");
write_response(reswriter,"</title>");
write_response(reswriter,"</head>");
write_response(reswriter,"<body>");
write_response(reswriter,"<center>");
write_response(reswriter,"<ul>");
while(fgets(name,101,f)!=NULL)
{
write_response(reswriter,"<li>%s</li>",name);
}
fclose(f);
write_response(reswriter,"</ul>");
write_response(reswriter,"</center>");
write_response(reswriter,"</body>");
write_response(reswriter,"</html>");
close_writer(reswriter);
}



int main()
{
ServiceContainer *sc=get_service_container("one.com");
register_service(sc,"/getNames",&get_names);
start_server();
return 0;
}
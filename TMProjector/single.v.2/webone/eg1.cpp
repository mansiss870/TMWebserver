#include<tmwp>
#include<iostream>
using namespace std;
using namespace tmwp;
void doSomething()
{
cout<<"DO Something got executed"<<endl;
}
int main()
{
TMWebProjector server(7777);
server.onRequest("/kkk",doSomething);
server.start();
return 0;
}
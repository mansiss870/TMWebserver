#include<stdio.h>
int main()
{
FILE *f=fopen("data.txt","r");
char name[101];
while(fgets(name,101,f)!=NULL)
{
printf("%s.",name);
}
fclose(f);
return 0;
}
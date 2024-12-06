#include <stdio.h>
#include <stdbool.h>
int main()
{
int a, b, c;
double doub, doub1;
bool bb, bbb;
int x;
doub1 = 7.2;
doub = 5.3 + 3.1 / doub1;
a = b;
bbb = a < 3;
bb = !bbb;
scanf("%d", &a);
scanf("%d", &b);
scanf("%d", &bb);
doub = 1;
if(a < c)
{
doub = 2;
}
else
{
doub = 0;
}
for(x = 0;
 x < 5; )
{
int n;
n = a * b * c;
scanf("%d %d", &a, &b);
if(c < n)
{
x = x;
}
else
{
x = 35 + 1 * 5;
}
c = n;
printf("%d %d", x * 4, bb);
printf("%d", c);
}
while(x < 5)
{
a = b * 3;
printf("%d", a);
}

return 0;
}
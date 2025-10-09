#include <stdio.h>

int min(int *p,int size);

int main(void)
{
	int n;
	printf("给出n与n个整数\n");
	scanf("%d",&n);
	int a[n];
	for(int i=0;i<n;i++){
		scanf("%d",&a[i]);
		//TODO
	}
	int mini=min(a,n);
	printf("%d\n",mini);
	
	return 0;
}

int min(int *p,int size)
{
	int mini=p[0];
	for(int i=1;i<size;i++){
		if(p[i]<mini){
			mini=p[i];
			//TODO
		}
		//TODO
	}
	return mini;
}

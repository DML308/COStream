
typedef int ted;

int main(int ted, char *argv[])
{
	{ typedef int ted;
	 { extern ted;

	   printf("%d\n", sizeof(ted)); } }
}

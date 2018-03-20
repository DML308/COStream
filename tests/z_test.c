composite MyCompositeCall(input stream<int x> In, output stream<int x> Out){
	int x=1;
	if(x>0){
		stream<int x>G;
		G=MyOperator(In)//2.内置operator定义 
		{ 
			work
			{
				G[0].x = In[0].x;
			}
			window
			{
				In sliding(3,2);
				Out tumbling(1);
			}
		};
	}
}
composite CS(input stream<int x> In, stream<int x> In1, output stream<int x> Out)
{
	param int x, int y;
	int i;float f;
	int realparam;
	stream<int x> G,G1,G3;//1.stream独立声明
	if(x>0){
		G= MyCompositeCall(In)(f);//3.其他composite调用 
	}
	else
		(G,G1)=MyOperatorx(In,In1)//2.内置operator定义 
		{ 
			work
			{
				G[0].x = x+In[0].x+In1[0];
				x += 1;
			}
			window
			{
				In sliding(3,2);
				G tumbling(1);
			}
		};
		
	
		//4.在非pipeline和splitjion结构中不允许出现循环控制语句内含有composite调用语句，只允许出现普通语句 
	
	f=100000.0; 
	for(i=1;i<3;i++)
		f += 3;
		//5.关于pipeline 
	G1=pipeline(G) 
	{ 
		//5.1普通C语句 
		int a,b;
		x=(4+y)/2;
		a =3;
		b =5;
		for(i=0;i<10;i++)//5.2循环控制可以出现在pipeline结构中 
			if(x>0)//5.3条件控制 
				add MyCompositeCall(realparam[i]);// composite-call必须是单输入单输出 
			else 
				add MyCompositeCall(realparam[i]); 
		add MyCompositeCall(realparam); 
	};
	//6.splitjion 
	G3=splitjoin(G) 
	{ 
		int t=12315;
		split duplicate(); 
		for(i=0;i<10;i++)//6.1循环控制可以出现在pipeline结构中 
			if(t>0)//6.2条件控制 
				add MyCompositeCall(realparam[i]);// 6.3composite-call必须是单输入单输出 
			else 
				add MyCompositeCall(realparam[i]); 
		add MyCompositeCall(realparam); 
		join roundrobin(); 
	}; 
} 

composite MyCompositeCall(input stream<int x> In, output stream<int x> Out){
	int x=1;
	if(x>0){
		stream<int x>G;
		G=MyOperator(In)//2.����operator���� 
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
	stream<int x> G,G1,G3;//1.stream��������
	if(x>0){
		G= MyCompositeCall(In)(f);//3.����composite���� 
	}
	else
		(G,G1)=MyOperatorx(In,In1)//2.����operator���� 
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
		
	
		//4.�ڷ�pipeline��splitjion�ṹ�в��������ѭ����������ں���composite������䣬ֻ���������ͨ��� 
	
	f=100000.0; 
	for(i=1;i<3;i++)
		f += 3;
		//5.����pipeline 
	G1=pipeline(G) 
	{ 
		//5.1��ͨC��� 
		int a,b;
		x=(4+y)/2;
		a =3;
		b =5;
		for(i=0;i<10;i++)//5.2ѭ�����ƿ��Գ�����pipeline�ṹ�� 
			if(x>0)//5.3�������� 
				add MyCompositeCall(realparam[i]);// composite-call�����ǵ����뵥��� 
			else 
				add MyCompositeCall(realparam[i]); 
		add MyCompositeCall(realparam); 
	};
	//6.splitjion 
	G3=splitjoin(G) 
	{ 
		int t=12315;
		split duplicate(); 
		for(i=0;i<10;i++)//6.1ѭ�����ƿ��Գ�����pipeline�ṹ�� 
			if(t>0)//6.2�������� 
				add MyCompositeCall(realparam[i]);// 6.3composite-call�����ǵ����뵥��� 
			else 
				add MyCompositeCall(realparam[i]); 
		add MyCompositeCall(realparam); 
		join roundrobin(); 
	}; 
} 

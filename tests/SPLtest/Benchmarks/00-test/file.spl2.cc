#ifndef _FILE_H
#define _FILE_H

#define GenAddFunc(FuncName,OutputType,InputType,ParamType) \
composite FuncName(output OutputType Out,input InputType In) \
{ \
	param \
		ParamType a,ParamType b; \
	Out = Add(){ \
		work \
		{\
			Out[0].x = a+b; \
		}\
	};\
}


#endif
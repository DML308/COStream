void X86CodeGenerate::SPL2X86_Decl(Node *node, declNode *u, int offset)
{
	if (extractDecl)
	{
		ExtractDeclVariables(node);
		return;
	}
	else 
	{
		//if(strcmp(node->u.decl.name, "SPLExtTyp") ==0 && node->u.decl.type->typ== Sdcl)
		string name = node->u.decl.name;
		if((name.substr(0, 9)=="SPLExtTyp") && node->u.decl.type->typ== Sdcl)  //chenzhen
		{
			isExternType = TRUE;
			needExternType = TRUE;
			ExternTypeBuf<<"typedef struct \n";
			ExternTypeBuf<<"{ \n";
			SPL2X86_Node(u->type, offset);
			ExternTypeBuf<<"}"<<node->u.decl.name<<";\n\n";
			isExternType = FALSE;
		}else{
			if(isCGGlobalVar && STORAGE_CLASS(node->u.decl.tq) == T_EXTERN)
			{
				declInitList<<"extern ";
				globalvarbuf<<"extern ";
			}
			SPL2X86_Node(u->type, offset);
			if(u->type->typ == Adcl)	//多维数组
			{
				Node *tempNode = u->type;
				globalvarbuf<<node->u.decl.name;
				if (flag_Global)			//输出在GlobalVar中，为整个流程序的全局变量
					declInitList<<node->u.decl.name;
				else
					declInitList_temp<<node->u.decl.name;
				while(tempNode->typ == Adcl)
				{
					string dim = GetArrayDim(tempNode->u.adcl.dim);
					if (flag_Global)			
						declInitList<<"["<<dim<<"]";
					else
						declInitList_temp<<"["<<dim<<"]";
					globalvarbuf<<"["<<dim<<"]";
					tempNode = tempNode->u.adcl.type;
				}
				temp_declInitList<<declInitList.str()<<";\n";
				globalvarbuf<<";\n";

				if(isExternType)
					ExternTypeBuf << globalvarbuf.str()<<";\n";
				else{
					if (flag_Global)			//输出在GlobalVar中，为整个流程序的全局变量
						declInitList<< globalvarbuf.str();
					else
						declInitList_temp<< globalvarbuf.str();
					globalvarbuf << globalvarbuf.str()<<";\n";
				}

			}
			else		//标量
			{
				if(isExternType)
					ExternTypeBuf<<node->u.decl.name<<";\n";//声明变量
				else{
					declInitList<<" "<<node->u.decl.name;   //变量名
					temp_declInitList<<declInitList.str()<<";\n";
					globalvarbuf<<node->u.decl.name<<";\n";//声明变量
				}
			}

			if(u->type->typ == Adcl)
			{
				if(STORAGE_CLASS(node->u.decl.tq) != T_EXTERN)
					AdclInit(node,offset);		//初始化数组
			}
			else
			{
				if (node->u.decl.init) 
				{
					if (u->type->typ == Prim && u->type->u.prim.basic == Char)//如果是个字符声明
					{
						declInitList<<" = "<<u->init->u.implicitcast.expr->u.Const.text;
					}
					else
					{
						declInitList<<" = ";			//初始化变量 如int a = 2;
						SPL2X86_Node(u->init,offset );
					}
				}
			}
			if (u->type->typ != Adcl && !isExternType)		
				declInitList<<";\n";
				//declInitList<<";";

		}
	}
}

//注：相比 DeblockAndSAO_1.spl2.cc 文件 把SAO的5种类型整合到一个节点中
//相比DeAndSao.spl2.cc 展开offsetCTU
//相比DeAndSao_1.spl2.cc 水平和垂直滤波采用函数调用

//#include <CommonDef.h>   //Clip3函数的声明文件
//#include <math.h>
//#include <stdlib.h>


#define MAX_QP 51
#define DEFAULT_INTRA_TC_OFFSET 2

//注：此SAO是对于单帧的处理；使用的是m_pcSAO指针所指的公用空间；其中统计5种类型是分开在pipeline的5个节点中的，见getCopntStatistics;
//也可以将这5个节点整合在一个节点中完成


#define UInt unsigned int
//#define bool bool   //costream不支持bool类型，用unsigned char型代替
#define bool int
#define Pel short
#define Int int
#define Double double
#define Float float
#define Char char
#define UChar unsigned char
#define NULL 0x0000000000000000
#define true 1
#define false 0



//typedef       long Int64;  //8 bytes
//typedef       unsigned long  UInt64 //8 bytes




typedef       long long         Int64;   //在linux下测试大小为 8 bytes
typedef       unsigned long long  UInt64;//在linux下测试大小为 8 bytes

struct TComDataCU;  //前向声明结构体
typedef struct TComDataCU TComDataCU;

struct TComSlice
{
	Double      m_lambdas[3];
	Int         m_iDepth;
	Int  m_aiNumRefIdx[2];    //  for multiple reference of current slice
	bool m_bLMvdL1Zero;
	Int  m_list1IdxToList0Idx[16];
	
};
typedef struct TComSlice TComSlice; 


struct TComPicSym
{
	UInt          m_uiWidthInCU;
	UInt          m_uiHeightInCU;
  
	UInt          m_uiMaxCUWidth;
	UInt          m_uiMaxCUHeight;
	UInt          m_uiMinCUWidth;
	UInt          m_uiMinCUHeight;
  
	UChar         m_uhTotalDepth;       ///< max. depth
	UInt          m_uiNumPartitions;
	UInt          m_uiNumPartInWidth;
	UInt          m_uiNumPartInHeight;
	UInt          m_uiNumCUsInFrame;
	
	TComSlice**   m_apcTComSlice;
	TComDataCU**  m_apcTComDataCU; 	
};

typedef struct TComPicSym TComPicSym;

struct TComPicYuv
{
	Pel*  m_apiPicBufY;           ///< Buffer (including margin)
	Pel*  m_apiPicBufU;
	Pel*  m_apiPicBufV;
  
	Pel*  m_piPicOrgY;            ///< m_apiPicBufY + m_iMarginLuma*getStride() + m_iMarginLuma
	Pel*  m_piPicOrgU;
	Pel*  m_piPicOrgV;
  
	// ------------------------------------------------------------------------------------------------
	//  Parameter for general YUV buffer usage
	// ------------------------------------------------------------------------------------------------
  
	Int   m_iPicWidth;            ///< Width of picture
	Int   m_iPicHeight;           ///< Height of picture
	
	Int*  m_cuOffsetY;
	Int*  m_cuOffsetC;
	Int*  m_buOffsetY;
	Int*  m_buOffsetC;
  
	Int   m_iLumaMarginX;
	Int   m_iLumaMarginY;
	Int   m_iChromaMarginX;
	Int   m_iChromaMarginY;
	
	bool  m_bIsBorderExtended;
	
};

typedef struct TComPicYuv TComPicYuv;

struct TComPic
{
	
	UInt                  m_uiTLayer;               //  Temporal layer
	bool                  m_bUsedByCurr;            //  Used by current picture
	bool                  m_bIsLongTerm;            //  IS long term picture
	
	TComPicSym*           m_apcPicSym;  
	TComPicYuv*           m_apcPicYuv[2];           //  Texture,  0:org / 1:rec
  
    TComPicYuv*           m_pcPicYuvPred;           //  Prediction
    TComPicYuv*           m_pcPicYuvResi;           //  Residual
	
};
typedef struct TComPic TComPic;

struct TComDataCU
{
	UInt m_uiCUAddr;
	UInt m_uiAbsIdxInLCU;
	UInt          m_uiCUPelX;           ///< CU position in a pixel (X)
    UInt          m_uiCUPelY;  
	TComPic*      m_pcPic;              ///< picture class pointer
	TComSlice*    m_pcSlice;            ///< slice header pointer
	
	UChar* m_puhDepth;  
	bool* m_skipFlag;
	Char* m_pePartSize;
	Char* m_pePredMode;         ///< array of prediction modes
	Char* m_CUTransquantBypass;
	//bool* m_CUTransquantBypass;   ///< array of cu_transquant_bypass flags
	Char* m_phQP;               ///< array of QP values
	
	UChar* m_puhTrIdx;
	UChar* m_puhCbf[3];   
	UChar* m_puhTransformSkip[3];
	UChar* m_puhLumaIntraDir; 
	
	Double m_dTotalCost;         ///< sum of partition RD costs
	UInt m_uiTotalDistortion; 
	UInt m_uiTotalBits;        ///< sum of partition bits
	UInt m_uiTotalBins;       ///< sum of partition bins 

	UChar*        m_puhChromaIntraDir;  ///< array of intra directions (chroma)
	UChar*        m_puhInterDir;	
	Char*         m_apiMVPIdx[2];       ///< array of motion vector predictor candidates
	Char*         m_apiMVPNum[2];  
};

struct SAOStatData
{
	Int64 diff[32];
	Int64 count[32];
};

typedef struct SAOStatData SAOStatData;

struct TEncSampleAdaptiveOffset
{
	Double                 m_lambda[3];
	
	//statistics
	SAOStatData***         m_statData; //[ctu][comp][classes]
	
	Double                 m_saoDisabledRate[3][7];
	
	Int                    m_skipLinesR[3][32];
	Int                    m_skipLinesB[3][32];
	Int* m_offsetClip[3]; //clip table for fast operation
	TComPicYuv*   m_tempPicYuv; //temporary buffer
};

typedef struct TEncSampleAdaptiveOffset TEncSampleAdaptiveOffset;

struct TEncTop
{
	TEncSampleAdaptiveOffset m_cEncSAO;                     ///< sample adaptive offset class
};

typedef struct TEncTop TEncTop;


struct SAOOffset
{
  Int modeIdc; //NEW, MERGE, OFF
  Int typeIdc; //NEW: EO_0, EO_90, EO_135, EO_45, BO. MERGE: left, above
  Int typeAuxInfo; //BO: starting band index
  Int offset[32];

  //SAOOffset();
  //~SAOOffset();
  //Void reset();

  //const SAOOffset& operator= (const SAOOffset& src);
};
typedef struct SAOOffset SAOOffset;

struct SAOBlkParam
{

  //SAOBlkParam();
  // ~SAOBlkParam();
  //Void reset();
  //const SAOBlkParam& operator= (const SAOBlkParam& src);
  //SAOOffset& operator[](Int compIdx){ return offsetParam[compIdx];}
  //private:
  SAOOffset offsetParam[3];

};
typedef struct SAOBlkParam SAOBlkParam;

extern UInt m_uiNumCUsInFrame;
extern int m_numCTUInWidth; //整张图像共含有的LCU的列数
extern int m_numCTUInHeight; //整张图像共含有的LCU的行数
extern int m_picWidth;
extern int m_picHeight;


//iStride = (m_picWidth     ) + (m_LumaMarginX  <<1)
extern int iStride;
//iCStride = (m_picWidth     ) + (m_ChromaMarginX  <<1)
extern int iCStride;
//extern SAOStatData***  m_statData; //[ctu][comp][classes] 

extern TEncTop* m_pcEncTop;

int m_maxCUHeight = 64;
int m_maxCUWidth = 64;
int m_numCTUsPic = 9;

	
extern TComPic* pcPic;	//将pcPic作为外部变量，只能够实现单帧的区块滤波，若要实现多帧，将pcPic作为输入数据，放入In中，可定位到正确图像
//extern TComPicSym* 	pcPicSym = pcPic->m_apcPicSym;
extern TComPicSym* 	pcPicSym;

UChar g_aucChromaScale[58]=
{
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,
  17,18,19,20,21,22,23,24,25,26,27,28,29,29,30,31,32,
  33,33,34,34,35,35,36,36,37,37,38,39,40,41,42,43,44,
  45,46,47,48,49,50,51
};

#define   QpUV(iQpY)  ( ((iQpY) < 0) ? (iQpY) : (((iQpY) > 57) ? ((iQpY)-6) : g_aucChromaScale[(iQpY)]) )

int g_auiRasterToZscan[256] = {0, 1, 4, 5,16,17,20,21,64,65,68,69,80,81,84,85,
                               2, 3, 6, 7,18,19,22,23,66,67,70,71,82,83,86,87,
							   8, 9, 12,13,24,25,28,29,72,73,76,77,88,89,92,93,
							   10,11,14,15,26,27,30,31,74,75,78,79,90,91,94,95,
							   32,33,36,37,48,49,52,53,96,97,100,101,112,113,116,117,
							   34,35,38,39,50,51,54,55,98,99,102,103,114,115,118,119,
							   40,41,44,45,56,57,60,61,104,105,108,109,120,121,124,125,
							   42,43,46,47,58,59,62,63,106,107,110,111,122,123,126,127,
							   128,129,132,133,144,145,148,149,192,193,196,197,208,209,212,213,
							   130,131,134,135,146,147,150,151,194,195,198,199,210,211,214,215,
							   136,137,140,141,152,153,156,157,200,201,204,205,216,217,220,221,
							   138,139,142,143,154,155,158,159,202,203,206,207,218,219,222,223,
							   160,161,164,165,176,177,180,181,224,225,228,229,240,241,244,245,
							   162,163,166,167,178,179,182,183,226,227,230,231,242,243,246,247,
							   168,169,172,173,184,185,188,189,232,233,236,237,248,249,252,253,
							   170,171,174,175,186,187,190,191,234,235,238,239,250,251,254,255};
							   

int g_auiZscanToRaster[256] = {0,1,16,17,2,3,18,19,32,33,48,49,34,35,50,51,
                               4,5,20,21,6,7,22,23,36,37,52,53,38,39,54,55,
							   64,65,80,81,66,67,82,83,96,97,112,113,98,99,114,115,
							   68,69,84,85,70,71,86,87,100,101,116,117,102,103,118,119,
							   8,9,24,25,10,11,26,27,40,41,56,57,42,43,58,59,
							   12,13,28,29,14,15,30,31,44,45,60,61,46,47,62,63,
							   72,73,88,89,74,75,90,91,104,105,120,121,106,107,122,123,
							   76,77,92,93,78,79,94,95,108,109,124,125,110,111,126,127,
							   128,129,144,145,130,131,146,147,160,161,176,177,162,163,178,179,
							   132,133,148,149,134,135,150,151,164,165,180,181,166,167,182,183,
							   192,193,208,209,194,195,210,211,224,225,240,241,226,227,242,243,
							   196,197,212,213,198,199,214,215,228,229,244,245,230,231,246,247,
							   136,137,152,153,138,139,154,155,168,169,184,185,170,171,186,187,
							   140,141,156,157,142,143,158,159,172,173,188,189,174,175,190,191,
							   200,201,216,217,202,203,218,219,232,233,248,249,234,235,250,251,
							   204,205,220,221,206,207,222,223,236,237,252,253,238,239,254,255};

							   
int g_auiRasterToPelX[256] = {0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
                              0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
							  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
							  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
							  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
							  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
							  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
							  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
							  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
							  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
							  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
							  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
							  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
							  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
							  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
							  0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60};
							   
							   
int g_auiRasterToPelY[256] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
							  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
							  12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
							  16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
							  20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
							  24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,24,
							  28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,
							  32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
							  36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,
							  40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,
							  44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,
							  48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,
							  52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,52,
							  56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,56,
							  60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60};

const UChar sm_tcTable[54] =
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,5,5,6,6,7,8,9,10,11,13,14,16,18,20,22,24
};

const UChar sm_betaTable[52] =
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,7,8,9,10,11,12,13,14,15,16,17,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64
};

struct _LFUParam
{
	bool bInternalEdge;
	bool bLeftEdge;
	bool bTopEdge;
};
typedef struct _LFUParam LFUParam;




//8x8 cu块的的水平滤波，也即垂直边界的去块滤波,滤波的单位是每一个zscan的8*8块
//为了防止和水平边界的滤波发生写冲突，8x8输入块按照光栅扫描的顺序输入。
//光栅顺序为第0行 0,2,4,6,8,10,12,14;第2行 32,34,36,38,40,42,44,46; .... 第14行224,226,228,230,232,234,236,238;
//对应的uiAbsZorderIdx 可以根据 g_auiRasterToZscan[rasterOrder]求出

//编译成功！！
composite DeblockCU_VER_EDGE(output stream<UInt rasterOrder,unsigned long ul_pcPic>Out, input stream<UInt rasterOrder,unsigned long ul_pcPic>In)
{
	
	Out = xDeBlockCU_VER_EDGE(In)
	{
		Int uiCUAddr;
	    TComDataCU* pcCU = NULL;   //当前LCU的指针
		init
		{
			uiCUAddr = 0;
			pcCU = pcPicSym->m_apcTComDataCU[uiCUAddr];
		}
				work
		{
			
			UInt rasterOrder = In[0].rasterOrder;//输入In[0]为当前要进行垂直边界滤波处理的8x8块
			Int iter = 1; //默认只迭代一次，对于位于64x64块的最后一列的8x8块，则需要迭代两次，因为该块需要下一个LCU的第一列的8x8块对其进行垂直边界的滤波
			Int cur = 0;
			
			UInt cpRasterOrder = rasterOrder;  //将当前块的光栅顺序保存至变量 cpRasterOrder
			UInt colNum = rasterOrder/16;  //8x8块所处的行数                                                                             
			UInt uiAbsZorderIdx = g_auiRasterToZscan[rasterOrder];
			
			UInt uiDepth = pcCU->m_puhDepth[uiAbsZorderIdx]; //求出当前块所在的深度
			UInt TransformIdx = pcCU->m_puhTrIdx[uiAbsZorderIdx]; //变换索引
			
			UInt uiX = pcCU->m_uiCUPelX + g_auiRasterToPelX[rasterOrder];
			UInt uiY = pcCU->m_uiCUPelY + g_auiRasterToPelY[rasterOrder];
			
			if((rasterOrder%16==14) && ( (uiCUAddr%m_numCTUInWidth)!= m_numCTUInWidth-1 ) ) //8x8块位于当前LCU的最后一列 且 该LCU不位于整张图像最右列LCU时
			{
				iter = 2;
				cur = -1; // cur初始值为-1，-1增到1，迭代2次
			}
			if((uiCUAddr!=0) && (rasterOrder%16 == 0))  //非第一个LCU且8x8块位于第一列，此时不用进行垂直边界滤波，因为该块的滤波在前一个LCU的最后一列8x8块进行滤波时一起处理了
			{
				iter = 0;
				cur = 1;  // cur初始为1，则1次也不迭代
			}
			
			
			for(cur;cur<1;cur++)  //默认只迭代一次,这里通过改变cur的起始值来实际改变循环的次数
			{
				
				if(iter == 2 && cur == 0) //当第二次迭代时，说明要处理的是下一个LCU的第一列，此时要重置该8x8所在LCU的相关信息
				{
					uiCUAddr++;
					pcCU = pcPicSym->m_apcTComDataCU[uiCUAddr]; //获取下一个LCU
					rasterOrder = colNum*16;//根据原8x8所在行数，得出当前位于第一列的8x8块的光栅顺序
				                                                                            
					uiAbsZorderIdx = g_auiRasterToZscan[rasterOrder];
			
					uiDepth = pcCU->m_puhDepth[uiAbsZorderIdx]; //求出当前块所在的深度
					TransformIdx = pcCU->m_puhTrIdx[uiAbsZorderIdx]; //变换索引
			
					uiX = pcCU->m_uiCUPelX + g_auiRasterToPelX[rasterOrder];
					uiY = pcCU->m_uiCUPelY + g_auiRasterToPelY[rasterOrder];	
				}
				
				VerEdgefilter_cos(uiX,uiY,pcCU,uiCUAddr,uiDepth,uiAbsZorderIdx,rasterOrder,TransformIdx);
				
				//位于最后一列的8x8已完成,重置pcCU指针为原来LCU的指针 
				if(iter == 2 && cur == 0) {uiCUAddr--; pcCU = pcPicSym->m_apcTComDataCU[uiCUAddr];}
			}
			
			
			//一个LCU的最后一个8x8已完成,重置pcCU指针
			if(cpRasterOrder == 238) {uiCUAddr++; pcCU = pcPicSym->m_apcTComDataCU[uiCUAddr];}
			
			Out[0].rasterOrder = In[0].rasterOrder ; 
			Out[0].ul_pcPic = In[0].ul_pcPic;
		
		}
		window
		{
			In sliding(1,1);
			Out tumbling(1);
		}
	};
}


//8x8 cu块的的水平边界去块滤波,滤波的单位是每一个zscan的8*8块
//且输入的顺序为垂直边界去块滤波的输出顺序，也即光栅顺序，为 0,2,4,6,8,10,12,14; 32,34,36,38,40,42,44,46; .... 224,226,228,230,232,234,236,238
//对应的uiAbsZorderIdx 可以根据 g_auiRasterToZscan[rasterOrder]求出


composite DeblockCU_HOR_EDGE(output stream<unsigned long ul_pcPic>Out, input stream<UInt rasterOrder,unsigned long ul_pcPic>In)
{
	
	Out = xDeBlockCU_HOR_EDGE(In)
	{
		Int uiCUAddr;
	    TComDataCU* pcCU = NULL;   //当前LCU的指针
		init
		{
			uiCUAddr = 0;
			pcCU = pcPicSym->m_apcTComDataCU[uiCUAddr];
		}
		work
		{
			
			UInt rasterOrder = In[0].rasterOrder;//输入块为2个相邻的8*8块,In[0]为当前要进行水平滤波处理的8x8块
			UInt uiAbsZorderIdx = g_auiRasterToZscan[rasterOrder];
			
			UInt uiDepth = pcCU->m_puhDepth[uiAbsZorderIdx]; //求出当前块所在的深度
			UInt TransformIdx = pcCU->m_puhTrIdx[uiAbsZorderIdx]; //变换索引
			
			
			UInt uiX = pcCU->m_uiCUPelX + g_auiRasterToPelX[rasterOrder];
			UInt uiY = pcCU->m_uiCUPelY + g_auiRasterToPelY[rasterOrder];
			
			HorEdgefilter_cos(uiX,uiY,pcCU,uiCUAddr,uiDepth,uiAbsZorderIdx,rasterOrder,TransformIdx);
			
			//一个LCU的最后一个8x8已完成,重置pcCU指针
			if(rasterOrder == 238) {uiCUAddr++; pcCU = pcPicSym->m_apcTComDataCU[uiCUAddr];}
			Out[0].ul_pcPic = In[0].ul_pcPic;
			//Out[0].ul_m_pcEncTop = (unsigned long)m_pcEncTop;
		}
		window
		{
			In sliding(2,1);
			Out tumbling(1);
		}
	};
}

composite getCopntStatistics(output stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pcPic>Out,
                              input stream<unsigned long ul_srcYuv,unsigned long ul_pcPic>In)
{
	param
		int compIdx;  //compIdx代表颜色分量：0亮度，1,2色度；
	
	bool isLuma = (compIdx == 0);
	int formatShift = isLuma?0:1;
	
	Out = getBlkStats(In)
	{
		int iCount = 0;
		init
		{
			iCount = 0;
		}
		work
		{
			int ctu = iCount++;
			Int yPos   = (ctu / m_numCTUInWidth)*m_maxCUHeight;
			Int xPos   = (ctu % m_numCTUInWidth)*m_maxCUWidth;
			Int height = (yPos + m_maxCUHeight > m_picHeight)?(m_picHeight- yPos):m_maxCUHeight;
			Int width  = (xPos + m_maxCUWidth  > m_picWidth )?(m_picWidth - xPos):m_maxCUWidth;
			TComPic*   pPic    = (TComPic*)(In[0].ul_pcPic);
			TComPicYuv* orgYuv = pPic->m_apcPicYuv[0];
			TComPicYuv* srcYuv = (TComPicYuv*)(In[0].ul_srcYuv);
			
			//TEncTop*    m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //获取SAO滤波的类指针，或者说句柄
			
			//SAOStatData***  m_statData = (SAOStatData***)(In[0].ul_m_statData);
			SAOStatData***  m_statData = m_pcSAO->m_statData;
			
			bool isLeftAvail = 0,isRightAvail = 0,isAboveAvail = 0,isBelowAvail = 0,isAboveLeftAvail = 0,isAboveRightAvail = 0,isBelowLeftAvail = 0,isBelowRightAvail = 0;
			
			int iStride_src = srcYuv->m_iPicWidth + (srcYuv->m_iLumaMarginX)<<1;
			int iCStride_src = (srcYuv->m_iPicWidth)>>1 + (srcYuv->m_iChromaMarginX)<<1;
			
			int srcStride = isLuma?iStride_src:iCStride_src;
			
			int iStride_org = orgYuv->m_iPicWidth + (orgYuv->m_iLumaMarginX)<<1;
			int iCStride_org = (orgYuv->m_iPicWidth)>>1 + (orgYuv->m_iChromaMarginX)<<1;
			
			int orgStride = isLuma?iStride_org:iCStride_org;
			
			Pel *srcYuvBuf = NULL,*orgYuvBuf = NULL;
			
			//pPic->getPicSym()->deriveLoopFilterBoundaryAvailibility(ctu, isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail);
			//使用函数调用
			my_deriveLoopFilterBoundaryAvailibility(pPic,ctu, isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail);
			
			isRightAvail      = (xPos + m_maxCUWidth  < m_picWidth );
			isBelowAvail      = (yPos + m_maxCUHeight < m_picHeight);
			isBelowRightAvail = (isRightAvail && isBelowAvail);
			isBelowLeftAvail  = ((xPos > 0) && (isBelowAvail));
			isAboveRightAvail = ((yPos > 0) && (isRightAvail));
			
			
			switch(compIdx)
			{
			case 0:
			  {
				srcYuvBuf = srcYuv->m_piPicOrgY;
				orgYuvBuf = orgYuv->m_piPicOrgY;
			  }
			  break;
			case 1:
			  {
				srcYuvBuf = srcYuv->m_piPicOrgU;
				orgYuvBuf = orgYuv->m_piPicOrgU;
			  }
			  break;
			case 2:
			  {
				srcYuvBuf = srcYuv->m_piPicOrgV;
				orgYuvBuf = orgYuv->m_piPicOrgV;
			  }
			  break;
			}
			
			{
				Pel* srcBlk = srcYuvBuf + (yPos >> formatShift)*srcStride+ (xPos >> formatShift);
				Pel* orgBlk = orgYuvBuf + (yPos >> formatShift)*orgStride+ (xPos >> formatShift);
				//Int  srcStride = isLuma?srcYuv->getStride():srcYuv->getCStride();
				//Pel* srcBlk    = getPicBuf(srcYuv, compIdx)+ (yPos >> formatShift)*srcStride+ (xPos >> formatShift);

				//int  orgStride  = isLuma?orgYuv->getStride():orgYuv->getCStride();
				//Pel* orgBlk     = getPicBuf(orgYuv, compIdx)+ (yPos >> formatShift)*orgStride+ (xPos >> formatShift);
			
				char m_signLineBuf1[m_maxCUWidth+1],m_signLineBuf2[m_maxCUWidth+1];
				
				Int x,y, startX, startY, endX, endY, edgeType, firstLineStartX, firstLineEndX;
				Char signLeft, signRight, signDown;
				Int64 *diff, *count;
				Pel *srcLine, *orgLine;
				
				Int* skipLinesR = m_pcSAO->m_skipLinesR[compIdx]; 
				Int* skipLinesB = m_pcSAO->m_skipLinesB[compIdx];
				Int typeIdx = 0;
				
				width >>= formatShift;
				height >>= formatShift;
				
				for(typeIdx=0; typeIdx< 5; typeIdx++)
				{
					SAOStatData* statsDataTypes = m_statData[ctu][compIdx];
				
					statsDataTypesSet(statsDataTypes,typeIdx,diff,count);
					
					srcLine = srcBlk;
					orgLine = orgBlk;
					
					switch(typeIdx)
					{
						case 0:
						  {
							diff +=2;
							count+=2;
							endY   = (isBelowAvail) ? (height - skipLinesB[typeIdx]) : height;
							startX = isLeftAvail  ? 0 : 1;
							endX   = isRightAvail ? (width - skipLinesR[typeIdx]) : (width - 1);
																					  
							for (y=0; y<endY; y++)
							{
								signLeft = (Char)sgn(srcLine[startX] - srcLine[startX-1]);
								for (x=startX; x<endX; x++)
								{
									signRight =  (Char)sgn(srcLine[x] - srcLine[x+1]);
									edgeType  =  signRight + signLeft;
									signLeft  = -signRight;

									diff [edgeType] += (orgLine[x] - srcLine[x]);
									count[edgeType] ++;
								}
								srcLine  += srcStride;
								orgLine  += orgStride;
							}
						  }
						  break;
						case 1:
						  {
							char *signUpLine = m_signLineBuf1; 
							Pel* srcLineAbove = srcLine - srcStride;	
							Pel* srcLineBelow;
							
							diff +=2;
							count+=2;
				
				
							startX = 0;
							startY = isAboveAvail ? 0 : 1;
							endX   = (isRightAvail ? (width - skipLinesR[typeIdx]) : width); 
							endY   = isBelowAvail ? (height - skipLinesB[typeIdx]) : (height - 1);
							 
							if (!isAboveAvail)
							{
								srcLine += srcStride;
								orgLine += orgStride;
							}

				
							for (x=startX; x<endX; x++) 
							{
								signUpLine[x] = (Char)sgn(srcLine[x] - srcLineAbove[x]);
							}	
				
							for (y=startY; y<endY; y++)
							{
								srcLineBelow = srcLine + srcStride;
								for (x=startX; x<endX; x++)
								{
									signDown  = (Char)sgn(srcLine[x] - srcLineBelow[x]); 
									edgeType  = signDown + signUpLine[x];
									signUpLine[x]= -signDown;

									diff [edgeType] += (orgLine[x] - srcLine[x]);
									count[edgeType] ++;
								} 
								
								srcLine += srcStride;
								orgLine += orgStride;
							}	
						  }
						  break;
						case 2:
						  {
							char *signUpLine, *signDownLine, *signTmpLine;
							Pel* srcLineBelow = srcLine + srcStride;
							Pel* srcLineAbove = srcLine - srcStride;
							
							
							diff +=2;
							count+=2;
						
							signUpLine  = m_signLineBuf1;
							signDownLine= m_signLineBuf2;
							
							startX = (isLeftAvail  ? 0 : 1);
							endX   = (isRightAvail ? (width - skipLinesR[typeIdx]): (width - 1)); 
							endY   = isBelowAvail ? (height - skipLinesB[typeIdx]) : (height - 1);
							
							//prepare 2nd line's upper sign
				
							for (x=startX; x<endX+1; x++)
							{
								signUpLine[x] = (Char)sgn(srcLineBelow[x] - srcLine[x-1]);
							}
							
							firstLineStartX = (isAboveLeftAvail ? 0    : 1);
							firstLineEndX   = (isAboveAvail     ? endX : 1);
							
							for(x=firstLineStartX; x<firstLineEndX; x++)
							{
							  edgeType = sgn(srcLine[x] - srcLineAbove[x-1]) - signUpLine[x+1];

							  diff [edgeType] += (orgLine[x] - srcLine[x]);
							  count[edgeType] ++;
							}
							srcLine  += srcStride;
							orgLine  += orgStride;
							
							//middle lines
							for (y=1; y<endY; y++)
							{
								srcLineBelow = srcLine + srcStride;

								for (x=startX; x<endX; x++)
								{

									signDown = (Char)sgn(srcLine[x] - srcLineBelow[x+1]);

									edgeType = signDown + signUpLine[x];
									diff [edgeType] += (orgLine[x] - srcLine[x]);
									count[edgeType] ++;

									signDownLine[x+1] = -signDown; 
								}

								signDownLine[startX] = (Char)sgn(srcLineBelow[startX] - srcLine[startX-1]);


								signTmpLine  = signUpLine;
								signUpLine   = signDownLine;
								signDownLine = signTmpLine;

								srcLine += srcStride;
								orgLine += orgStride;
							}	
										
						  }
						  break;
						case 3:
						  {
							char *signUpLine = m_signLineBuf1+1;
							Pel* srcLineBelow = srcLine + srcStride;
							Pel* srcLineAbove = srcLine - srcStride;
							diff +=2;
							count+=2;
							
							

							startX =  (isLeftAvail  ? 0 : 1) ;
							endX   =  (isRightAvail ? (width - skipLinesR[typeIdx]) : (width - 1));
					
							endY   = isBelowAvail ? (height - skipLinesB[typeIdx]) : (height - 1);

							//prepare 2nd line upper sign
							
							for (x=startX-1; x<endX; x++)
							{
							  signUpLine[x] = (Char)sgn(srcLineBelow[x] - srcLine[x+1]);
							}
							//first line
							//
							firstLineStartX = (isAboveAvail ? startX : endX);
							firstLineEndX   = ((!isRightAvail && isAboveRightAvail) ? width : endX);
                                                         
							for(x=firstLineStartX; x<firstLineEndX; x++)
							{
							  edgeType = sgn(srcLine[x] - srcLineAbove[x+1]) - signUpLine[x-1];
					
							  diff [edgeType] += (orgLine[x] - srcLine[x]);
							  count[edgeType] ++;
							}

							srcLine += srcStride;
							orgLine += orgStride;

							//middle lines
							for (y=1; y<endY; y++)
							{
								srcLineBelow = srcLine + srcStride;

								for(x=startX; x<endX; x++)
								{
					
									signDown = (Char)sgn(srcLine[x] - srcLineBelow[x-1]);
									edgeType = signDown + signUpLine[x];

									diff [edgeType] += (orgLine[x] - srcLine[x]);
									count[edgeType] ++;

									signUpLine[x-1] = -signDown; 
								}
					
								signUpLine[endX-1] = (Char)sgn(srcLineBelow[endX-1] - srcLine[endX]);
					
								srcLine  += srcStride;
								orgLine  += orgStride;
							}
						  }
						  break;
						case 4:
						  {
							startX = 0;
							endX   = (isRightAvail ? (width - skipLinesR[typeIdx]) : width );

							endY = isBelowAvail ? (height- skipLinesB[typeIdx]) : height;
							
							for (y=0; y< endY; y++)
							{
					
								for (x=startX; x< endX; x++)
								{

									Int bandIdx= srcLine[x] >> 3; 
									diff [bandIdx] += (orgLine[x] - srcLine[x]);
									count[bandIdx] ++;
								}
								srcLine += srcStride;
								orgLine += orgStride;
							}	
						  }
						  break;
						
						
					}
				}
				
			}
			//Out[0].ul_m_statData = In[0].ul_m_statData ;
			Out[0].ul_m_statData =  (unsigned int)m_statData;
			Out[0].ul_srcYuv = In[0].ul_srcYuv ;
			Out[0].ul_pcPic = In[0].ul_pcPic ;
			//Out[0].ul_m_pcEncTop= In[0].ul_m_pcEncTop ;
		}	
		window
		{
			In sliding(1,1);
			Out tumbling(1);
		}	
	};	
}


//getStatistics(m_statData, orgYuv, srcYuv, pPic); 
 //功能：对各个CTU进行信息统计
 //输入：包含orgYuv,srcYuv,pPic,m_statData,this指针 
composite getStatistics(output stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pcPic>Out,
                         input stream<unsigned long ul_srcYuv,unsigned long ul_pcPic>In)
{
	Out = splitjoin(In)  //
	{
		int num_sao_components = 3;
		int compIdx = 0;
		split duplicate();  //窗口暂定为1
		for(compIdx= 0; compIdx < num_sao_components;compIdx++)  //对三个颜色分量采用split
		{
			add getCopntStatistics(compIdx);  
		}
		join roundrobin(1);
	};
}


//输出: 原输入+reconParams; 其中输出窗口为m_numCTUsPic
//这部分要封装，里面的vector<>，new,等都是c 不支持的；

composite decideModeRDO_recBlkSAOParam(output stream<unsigned long ul_reconParams,unsigned long ul_srcYuv,unsigned long ul_pcPic>Out,
                                        input stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pcPic>In)
{
	Out = decideModeRDO_recBlkSAOParam(In)
	{
		
		int iCount = 0;
		SAOBlkParam* codedParams = NULL;
		SAOBlkParam* reconParams = NULL;
		SAOBlkParam modeParam;
		
		init
		{
			iCount = 0;
			codedParams = NULL;
			reconParams = NULL;
		}
		work
		{
			int ctu = iCount;
			
			if(iCount == 0)
			{
				
				//my_BlkParamsRDOinit(codedParams,reconParams,In[0].ul_pcPic,In[0].ul_m_pcEncTop);
				my_BlkParamsRDOinit(codedParams,reconParams,In[0].ul_pcPic,(unsigned long)m_pcEncTop);
			}	
			
			//将下列用一个函数封装void my_BlkModeRDO_recSAOParam(int ctu,SAOBlkParam &modeParam,SAOBlkParam* codedParams,SAOBlkParam* reconParams,unsigned long ul_m_statData,unsigned long ul_pcPic,unsigned long ul_m_pcEncTop);
			my_BlkModeRDO_recSAOParam(ctu,modeParam,codedParams,reconParams,In[0].ul_pcPic,(unsigned long)m_pcEncTop);
			
			Out[0].ul_srcYuv = In[0].ul_srcYuv;
			Out[0].ul_pcPic = In[0].ul_pcPic;
			//Out[0].ul_m_pcEncTop = In[0].ul_m_pcEncTop;
			Out[0].ul_reconParams = (unsigned long)reconParams;
			
			if(++iCount == m_numCTUsPic) iCount = 0;
			
			
		}
		window
		{
			In sliding(3,3);  //
			Out tumbling(1); 
			
		}
		
	};
}

//输入: ctu + srcYuv + resYuv + pPic + reconParams；
//输出：pPic + reconParams

//从这里开始
composite offsetCTU_Y(output stream<unsigned long ul_reconParams,unsigned long ul_pcPic>Out, 
					   input stream<unsigned long ul_reconParams,unsigned long ul_srcYuv,unsigned long ul_pcPic>In)
{
	param 
		int compIdx;
	
	bool isLuma = (compIdx == 0);
	int formatShift = isLuma?0:1;
	
	Out = offsetCTU_Y(In)
	{
		int iCount = 0;
		init
		{
			iCount = 0;
		}
		work
		{
			int ctu = iCount++;
			SAOBlkParam* reconParams = (SAOBlkParam*)(In[0].ul_reconParams);
			TComPic*   pPic    = (TComPic*)(In[0].ul_pcPic);
		//	TEncTop*   m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //获取SAO滤波的类指针，或者说句柄
			
			TComPicYuv* resYuv = pPic->m_apcPicYuv[1];
			TComPicYuv* srcYuv = (TComPicYuv*)(In[0].ul_srcYuv);
			
			//SAOBlkParam& saoblkParam = reconParams[ctu];
			bool isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail;

			if(reconParams[ctu].offsetParam[compIdx].modeIdc != 0) 
			{
				Int yPos   = (ctu / m_numCTUInWidth)*m_maxCUHeight;
				Int xPos   = (ctu % m_numCTUInWidth)*m_maxCUWidth;
				Int height = (yPos + m_maxCUHeight > m_picHeight)?(m_picHeight- yPos):m_maxCUHeight;
				Int width  = (xPos + m_maxCUWidth  > m_picWidth )?(m_picWidth - xPos):m_maxCUWidth;
				
				//SAOOffset& ctbOffset = saoblkParam[compIdx];
				Int  blkWidth   = width >>formatShift;
				Int  blkHeight  = height>>formatShift;
				Int  blkYPos    = yPos>>formatShift;
				Int  blkXPos    = xPos>>formatShift;

				int iStride_src = srcYuv->m_iPicWidth + (srcYuv->m_iLumaMarginX)<<1;
				int iCStride_src = (srcYuv->m_iPicWidth)>>1 + (srcYuv->m_iChromaMarginX)<<1;
			
				int srcStride = isLuma?iStride_src:iCStride_src;
			
				int iStride_res = resYuv->m_iPicWidth + (resYuv->m_iLumaMarginX)<<1;
				int iCStride_res = (resYuv->m_iPicWidth)>>1 + (resYuv->m_iChromaMarginX)<<1;
			
				int resStride = isLuma?iStride_res:iCStride_res;
			
				Pel* srcBlk    = srcYuv->m_piPicOrgY + blkYPos*srcStride+ blkXPos;
				Pel* resBlk     = resYuv->m_piPicOrgY + blkYPos*resStride+ blkXPos;
				
				Int typeIdx = reconParams[ctu].offsetParam[compIdx].typeIdc;
				Int* offset = reconParams[ctu].offsetParam[compIdx].offset;
				
				//block boundary availability
				//pPic->getPicSym()->deriveLoopFilterBoundaryAvailibility(ctu, isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail);
				my_deriveLoopFilterBoundaryAvailibility(pPic, ctu, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
				
				//offsetBlock( compIdx, ctbOffset.typeIdc, ctbOffset.offset, srcBlk, resBlk, srcStride, resStride, blkWidth, blkHeight, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
				{
				  char m_signLineBuf1[m_maxCUWidth+1], m_signLineBuf2[m_maxCUWidth+1];
				  Int* offsetClip = m_pcSAO->m_offsetClip[compIdx];

				  Int x,y, startX, startY, endX, endY, edgeType;
				  Int firstLineStartX, firstLineEndX, lastLineStartX, lastLineEndX;
				  Char signLeft, signRight, signDown;

				  Pel* srcLine = srcBlk;
				  Pel* resLine = resBlk;

				  switch(typeIdx)
				  {
				  case 0:
					{
					  offset += 2;
					  startX = isLeftAvail ? 0 : 1;
					  endX   = isRightAvail ? width : (width -1);
					  for (y=0; y< height; y++)
					  {
						signLeft = (Char)sgn(srcLine[startX] - srcLine[startX-1]);
						for (x=startX; x< endX; x++)
						{

						  signRight = (Char)sgn(srcLine[x] - srcLine[x+1]); 
						  edgeType =  signRight + signLeft;
						  signLeft  = -signRight;
						  resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
						}
						srcLine  += srcStride;
						resLine += resStride;
					  }
					}
					break;
				  case 1:
					{
					  char *signUpLine = m_signLineBuf1;
					  Pel* srcLineAbove= srcLine- srcStride;
					  Pel* srcLineBelow;
					  
					  offset += 2;
					  startY = isAboveAvail ? 0 : 1;
					  endY   = isBelowAvail ? height : height-1;
					  if (!isAboveAvail)
					  {
						srcLine += srcStride;
						resLine += resStride;
					  }

					  for (x=0; x< width; x++)
					  {
						signUpLine[x] = (Char)sgn(srcLine[x] - srcLineAbove[x]);
					  }

					  for (y=startY; y<endY; y++)
					  {
						srcLineBelow= srcLine+ srcStride;

						for (x=0; x< width; x++)
						{
						  signDown  = (Char)sgn(srcLine[x] - srcLineBelow[x]);

						  edgeType = signDown + signUpLine[x];
						  signUpLine[x]= -signDown;

						  resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
						}
						srcLine += srcStride;
						resLine += resStride;
					  }
					}
					break;
				  case 2:
					{
					  
					  Char *signUpLine, *signDownLine, *signTmpLine;
					  Pel* srcLineBelow= srcLine+ srcStride;
					  Pel* srcLineAbove= srcLine- srcStride;
					  
					  offset += 2;
					  signUpLine  = m_signLineBuf1;
					  signDownLine= m_signLineBuf2;

					  startX = isLeftAvail ? 0 : 1 ;
					  endX   = isRightAvail ? width : (width-1);

					  //prepare 2nd line's upper sign
					  
					  for (x=startX; x< endX+1; x++)
					  {
						signUpLine[x] = (Char)sgn(srcLineBelow[x] - srcLine[x- 1]);
					  }

					  //1st line
					  firstLineStartX = isAboveLeftAvail ? 0 : 1;
					  firstLineEndX   = isAboveAvail? endX: 1;
					  for(x= firstLineStartX; x< firstLineEndX; x++)
					  {
						edgeType  =  sgn(srcLine[x] - srcLineAbove[x- 1]) - signUpLine[x+1];
						resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
					  }
					  srcLine  += srcStride;
					  resLine  += resStride;
					  
					  //middle lines
					  for (y= 1; y< height-1; y++)
					  {
						srcLineBelow= srcLine+ srcStride;

						for (x=startX; x<endX; x++)
						{
						  signDown =  (Char)sgn(srcLine[x] - srcLineBelow[x+ 1]);

						  edgeType =  signDown + signUpLine[x];
						  resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];

						  signDownLine[x+1] = -signDown; 
						}

						signDownLine[startX] = (Char)sgn(srcLineBelow[startX] - srcLine[startX-1]);

						signTmpLine  = signUpLine;
						signUpLine   = signDownLine;
						signDownLine = signTmpLine;

						srcLine += srcStride;
						resLine += resStride;
					  }

					  //last line
					  srcLineBelow= srcLine+ srcStride;
					  lastLineStartX = isBelowAvail ? startX : (width -1);
					  lastLineEndX   = isBelowRightAvail ? width : (width -1);
					  for(x= lastLineStartX; x< lastLineEndX; x++)
					  {
						edgeType =  sgn(srcLine[x] - srcLineBelow[x+ 1]) + signUpLine[x];
						resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
					  }
					}
					break;
				  case 3:
					{
					  char *signUpLine = m_signLineBuf1+1;
					  Pel* srcLineBelow= srcLine+ srcStride;
					  Pel* srcLineAbove= srcLine- srcStride;
					  
					  offset += 2;
					  startX = isLeftAvail ? 0 : 1;
					  endX   = isRightAvail ? width : (width -1);

					  //prepare 2nd line upper sign
					  
					  for (x=startX-1; x< endX; x++)
					  {
						signUpLine[x] = (Char)sgn(srcLineBelow[x] - srcLine[x+1]);
					  }

					  //first line
					  firstLineStartX = isAboveAvail ? startX : (width -1 );
					  firstLineEndX   = isAboveRightAvail ? width : (width-1);
					  for(x= firstLineStartX; x< firstLineEndX; x++)
					  {
						edgeType = sgn(srcLine[x] - srcLineAbove[x+1]) -signUpLine[x-1];
						resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
					  }
					  srcLine += srcStride;
					  resLine += resStride;

					  //middle lines
					  for (y= 1; y< height-1; y++)
					  {
						srcLineBelow= srcLine+ srcStride;

						for(x= startX; x< endX; x++)
						{
						  signDown =  (Char)sgn(srcLine[x] - srcLineBelow[x-1]);

						  edgeType =  signDown + signUpLine[x];
						  resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
						  signUpLine[x-1] = -signDown; 
						}

						signUpLine[endX-1] = (Char)sgn(srcLineBelow[endX-1] - srcLine[endX]);

						srcLine  += srcStride;
						resLine += resStride;
					  }

					  //last line
					  srcLineBelow= srcLine+ srcStride;
					  lastLineStartX = isBelowLeftAvail ? 0 : 1;
					  lastLineEndX   = isBelowAvail ? endX : 1;
					  for(x= lastLineStartX; x< lastLineEndX; x++)
					  {
						edgeType = sgn(srcLine[x] - srcLineBelow[x-1]) + signUpLine[x];
						resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
					  }
					}
					break;
				  case 4:
					{
					  Int shiftBits = 3;
					  for (y=0; y< height; y++)
					  {
						for (x=0; x< width; x++)
						{
						  resLine[x] = offsetClip[ srcLine[x] + offset[srcLine[x] >> shiftBits] ];
						}
						srcLine += srcStride;
						resLine += resStride;
					  }
					}
					break;
				  }
				}
			}
	
			Out[0].ul_pcPic = In[0].ul_pcPic;
			//Out[0].ul_m_pcEncTop = In[0].ul_m_pcEncTop;
			Out[0].ul_reconParams = In[0].ul_reconParams;	
		}
		window
		{
			In sliding(1,1);
			Out tumbling(1);
		}
		
	};
}

composite offsetCTU_Cb(output stream<unsigned long ul_reconParams,unsigned long ul_pcPic>Out, 
                        input stream<unsigned long ul_reconParams,unsigned long ul_srcYuv,unsigned long ul_pcPic>In)
{ 
	param 
		int compIdx;
	bool isLuma = (compIdx == 0);
	int formatShift = isLuma?0:1;
	
	Out = offsetCTU_Cb(In)
	{
		int iCount = 0;
		init
		{
			iCount = 0;
		}
		work
		{
			int ctu = iCount++;
			SAOBlkParam* reconParams = (SAOBlkParam*)(In[0].ul_reconParams);
			TComPic*   pPic    = (TComPic*)(In[0].ul_pcPic);
			//TEncTop*   m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //获取SAO滤波的类指针，或者说句柄
			
			TComPicYuv* resYuv = pPic->m_apcPicYuv[1];
			TComPicYuv* srcYuv = (TComPicYuv*)(In[0].ul_srcYuv);
			
			//SAOBlkParam& saoblkParam = reconParams[ctu];
			bool isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail;

			if(reconParams[ctu].offsetParam[compIdx].modeIdc != 0) 
			{
				
				Int yPos   = (ctu / m_numCTUInWidth)*m_maxCUHeight;
				Int xPos   = (ctu % m_numCTUInWidth)*m_maxCUWidth;
				Int height = (yPos + m_maxCUHeight > m_picHeight)?(m_picHeight- yPos):m_maxCUHeight;
				Int width  = (xPos + m_maxCUWidth  > m_picWidth )?(m_picWidth - xPos):m_maxCUWidth;
				
				//SAOOffset& ctbOffset = saoblkParam[compIdx];
				

				Int  blkWidth   = width>>formatShift;
				Int  blkHeight  = height>>formatShift;
				Int  blkYPos    = yPos>>formatShift;
				Int  blkXPos    = xPos>>formatShift;
				
				int iStride_src = srcYuv->m_iPicWidth + (srcYuv->m_iLumaMarginX)<<1;
				int iCStride_src = (srcYuv->m_iPicWidth)>>1 + (srcYuv->m_iChromaMarginX)<<1;
			
				int srcStride = isLuma?iStride_src:iCStride_src;
			
				int iStride_res = resYuv->m_iPicWidth + (resYuv->m_iLumaMarginX)<<1;
				int iCStride_res = (resYuv->m_iPicWidth)>>1 + (resYuv->m_iChromaMarginX)<<1;
			
				int resStride = isLuma?iStride_res:iCStride_res;
			
				Pel* srcBlk    = srcYuv->m_piPicOrgU + blkYPos*srcStride+ blkXPos;
				Pel* resBlk     = resYuv->m_piPicOrgU + blkYPos*resStride+ blkXPos;
				
				Int typeIdx = reconParams[ctu].offsetParam[compIdx].typeIdc;
				Int* offset = reconParams[ctu].offsetParam[compIdx].offset;
				
				//block boundary availability
				//pPic->getPicSym()->deriveLoopFilterBoundaryAvailibility(ctu, isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail);
				my_deriveLoopFilterBoundaryAvailibility(pPic, ctu, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
				
				//offsetBlock( compIdx, ctbOffset.typeIdc, ctbOffset.offset, srcBlk, resBlk, srcStride, resStride, blkWidth, blkHeight, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
				{
				  char m_signLineBuf1[m_maxCUWidth+1], m_signLineBuf2[m_maxCUWidth+1];
				  Int* offsetClip = m_pcSAO->m_offsetClip[compIdx];

				  Int x,y, startX, startY, endX, endY, edgeType;
				  Int firstLineStartX, firstLineEndX, lastLineStartX, lastLineEndX;
				  Char signLeft, signRight, signDown;

				  Pel* srcLine = srcBlk;
				  Pel* resLine = resBlk;

				  switch(typeIdx)
				  {
				  case 0:
					{
					  offset += 2;
					  startX = isLeftAvail ? 0 : 1;
					  endX   = isRightAvail ? width : (width -1);
					  for (y=0; y< height; y++)
					  {
						signLeft = (Char)sgn(srcLine[startX] - srcLine[startX-1]);
						for (x=startX; x< endX; x++)
						{

						  signRight = (Char)sgn(srcLine[x] - srcLine[x+1]); 
						  edgeType =  signRight + signLeft;
						  signLeft  = -signRight;
						  resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
						}
						srcLine  += srcStride;
						resLine += resStride;
					  }
					}
					break;
				  case 1:
					{
					  char *signUpLine = m_signLineBuf1;
					  Pel* srcLineAbove= srcLine- srcStride;
					  Pel* srcLineBelow;
					  
					  offset += 2;
					  startY = isAboveAvail ? 0 : 1;
					  endY   = isBelowAvail ? height : height-1;
					  if (!isAboveAvail)
					  {
						srcLine += srcStride;
						resLine += resStride;
					  }

					  for (x=0; x< width; x++)
					  {
						signUpLine[x] = (Char)sgn(srcLine[x] - srcLineAbove[x]);
					  }

					  for (y=startY; y<endY; y++)
					  {
						srcLineBelow= srcLine+ srcStride;

						for (x=0; x< width; x++)
						{
						  signDown  = (Char)sgn(srcLine[x] - srcLineBelow[x]);

						  edgeType = signDown + signUpLine[x];
						  signUpLine[x]= -signDown;

						  resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
						}
						srcLine += srcStride;
						resLine += resStride;
					  }
					}
					break;
				  case 2:
					{
					  
					  Char *signUpLine, *signDownLine, *signTmpLine;
					  Pel* srcLineBelow= srcLine+ srcStride;
					  Pel* srcLineAbove= srcLine- srcStride;
					  
					  offset += 2;
					  signUpLine  = m_signLineBuf1;
					  signDownLine= m_signLineBuf2;

					  startX = isLeftAvail ? 0 : 1 ;
					  endX   = isRightAvail ? width : (width-1);

					  //prepare 2nd line's upper sign
					  
					  for (x=startX; x< endX+1; x++)
					  {
						signUpLine[x] = (Char)sgn(srcLineBelow[x] - srcLine[x- 1]);
					  }

					  //1st line
					  firstLineStartX = isAboveLeftAvail ? 0 : 1;
					  firstLineEndX   = isAboveAvail? endX: 1;
					  for(x= firstLineStartX; x< firstLineEndX; x++)
					  {
						edgeType  =  sgn(srcLine[x] - srcLineAbove[x- 1]) - signUpLine[x+1];
						resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
					  }
					  srcLine  += srcStride;
					  resLine  += resStride;
					  
					  //middle lines
					  for (y= 1; y< height-1; y++)
					  {
						srcLineBelow= srcLine+ srcStride;

						for (x=startX; x<endX; x++)
						{
						  signDown =  (Char)sgn(srcLine[x] - srcLineBelow[x+ 1]);

						  edgeType =  signDown + signUpLine[x];
						  resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];

						  signDownLine[x+1] = -signDown; 
						}

						signDownLine[startX] = (Char)sgn(srcLineBelow[startX] - srcLine[startX-1]);

						signTmpLine  = signUpLine;
						signUpLine   = signDownLine;
						signDownLine = signTmpLine;

						srcLine += srcStride;
						resLine += resStride;
					  }

					  //last line
					  srcLineBelow= srcLine+ srcStride;
					  lastLineStartX = isBelowAvail ? startX : (width -1);
					  lastLineEndX   = isBelowRightAvail ? width : (width -1);
					  for(x= lastLineStartX; x< lastLineEndX; x++)
					  {
						edgeType =  sgn(srcLine[x] - srcLineBelow[x+ 1]) + signUpLine[x];
						resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
					  }
					}
					break;
				  case 3:
					{
					  char *signUpLine = m_signLineBuf1+1;
					  Pel* srcLineBelow= srcLine+ srcStride;
					  Pel* srcLineAbove= srcLine- srcStride;
					  
					  offset += 2;
					  startX = isLeftAvail ? 0 : 1;
					  endX   = isRightAvail ? width : (width -1);

					  //prepare 2nd line upper sign
					  
					  for (x=startX-1; x< endX; x++)
					  {
						signUpLine[x] = (Char)sgn(srcLineBelow[x] - srcLine[x+1]);
					  }

					  //first line
					  firstLineStartX = isAboveAvail ? startX : (width -1 );
					  firstLineEndX   = isAboveRightAvail ? width : (width-1);
					  for(x= firstLineStartX; x< firstLineEndX; x++)
					  {
						edgeType = sgn(srcLine[x] - srcLineAbove[x+1]) -signUpLine[x-1];
						resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
					  }
					  srcLine += srcStride;
					  resLine += resStride;

					  //middle lines
					  for (y= 1; y< height-1; y++)
					  {
						srcLineBelow= srcLine+ srcStride;

						for(x= startX; x< endX; x++)
						{
						  signDown =  (Char)sgn(srcLine[x] - srcLineBelow[x-1]);

						  edgeType =  signDown + signUpLine[x];
						  resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
						  signUpLine[x-1] = -signDown; 
						}

						signUpLine[endX-1] = (Char)sgn(srcLineBelow[endX-1] - srcLine[endX]);

						srcLine  += srcStride;
						resLine += resStride;
					  }

					  //last line
					  srcLineBelow= srcLine+ srcStride;
					  lastLineStartX = isBelowLeftAvail ? 0 : 1;
					  lastLineEndX   = isBelowAvail ? endX : 1;
					  for(x= lastLineStartX; x< lastLineEndX; x++)
					  {
						edgeType = sgn(srcLine[x] - srcLineBelow[x-1]) + signUpLine[x];
						resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
					  }
					}
					break;
				  case 4:
					{
					  Int shiftBits = 3;
					  for (y=0; y< height; y++)
					  {
						for (x=0; x< width; x++)
						{
						  resLine[x] = offsetClip[ srcLine[x] + offset[srcLine[x] >> shiftBits] ];
						}
						srcLine += srcStride;
						resLine += resStride;
					  }
					}
					break;
				  }
				}
			}
	
			Out[0].ul_pcPic = In[0].ul_pcPic;
			//Out[0].ul_m_pcEncTop = In[0].ul_m_pcEncTop;
			Out[0].ul_reconParams = In[0].ul_reconParams;
			
		}
		window
		{
			In sliding(1,1);
			Out tumbling(1);
		}	
	};
}

composite offsetCTU_Cr(output stream<unsigned long ul_reconParams,unsigned long ul_pcPic>Out, 
                        input stream<unsigned long ul_reconParams,unsigned long ul_srcYuv,unsigned long ul_pcPic>In)
{
	param 
		int compIdx;
	bool isLuma = (compIdx == 0);
	int formatShift = isLuma?0:1;
	
	Out = offsetCTU_Cr(In)
	{
		int iCount;
		init
		{
			iCount = 0;
		}
		work
		{
			int ctu = iCount++;
			SAOBlkParam* reconParams = (SAOBlkParam*)(In[0].ul_reconParams);
			TComPic*   pPic    = (TComPic*)(In[0].ul_pcPic);
			//TEncTop*   m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //获取SAO滤波的类指针，或者说句柄
			
			TComPicYuv* resYuv = pPic->m_apcPicYuv[1];
			TComPicYuv* srcYuv = (TComPicYuv*)(In[0].ul_srcYuv);
			
			//SAOBlkParam& saoblkParam = reconParams[ctu];
			bool isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail;

			if(reconParams[ctu].offsetParam[compIdx].modeIdc != 0) 
			{
				Int yPos   = (ctu / m_numCTUInWidth)*m_maxCUHeight;
				Int xPos   = (ctu % m_numCTUInWidth)*m_maxCUWidth;
				Int height = (yPos + m_maxCUHeight > m_picHeight)?(m_picHeight- yPos):m_maxCUHeight;
				Int width  = (xPos + m_maxCUWidth  > m_picWidth )?(m_picWidth - xPos):m_maxCUWidth;
				
				//SAOOffset& ctbOffset = saoblkParam[compIdx];
				
				Int  blkWidth   = width>>formatShift;
				Int  blkHeight  = height>>formatShift;
				Int  blkYPos    = yPos>>formatShift;
				Int  blkXPos    = xPos>>formatShift;
				
				int iStride_src = srcYuv->m_iPicWidth + (srcYuv->m_iLumaMarginX)<<1;
				int iCStride_src = (srcYuv->m_iPicWidth)>>1 + (srcYuv->m_iChromaMarginX)<<1;
			
				int srcStride = isLuma?iStride_src:iCStride_src;
			
				int iStride_res = resYuv->m_iPicWidth + (resYuv->m_iLumaMarginX)<<1;
				int iCStride_res = (resYuv->m_iPicWidth)>>1 + (resYuv->m_iChromaMarginX)<<1;
			
				int resStride = isLuma?iStride_res:iCStride_res;
				
				Pel* srcBlk    = srcYuv->m_piPicOrgV + blkYPos*srcStride+ blkXPos;
				Pel* resBlk     = resYuv->m_piPicOrgV + blkYPos*resStride+ blkXPos;
				
				Int typeIdx = reconParams[ctu].offsetParam[compIdx].typeIdc;
				Int* offset = reconParams[ctu].offsetParam[compIdx].offset;
				//block boundary availability
				//pPic->getPicSym()->deriveLoopFilterBoundaryAvailibility(ctu, isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail);
				my_deriveLoopFilterBoundaryAvailibility(pPic, ctu, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
				
				//offsetBlock( compIdx, ctbOffset.typeIdc, ctbOffset.offset, srcBlk, resBlk, srcStride, resStride, blkWidth, blkHeight, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
				{
				  char m_signLineBuf1[m_maxCUWidth+1], m_signLineBuf2[m_maxCUWidth+1];
				  Int* offsetClip = m_pcSAO->m_offsetClip[compIdx];

				  Int x,y, startX, startY, endX, endY, edgeType;
				  Int firstLineStartX, firstLineEndX, lastLineStartX, lastLineEndX;
				  Char signLeft, signRight, signDown;

				  Pel* srcLine = srcBlk;
				  Pel* resLine = resBlk;

				  switch(typeIdx)
				  {
				  case 0:
					{
					  offset += 2;
					  startX = isLeftAvail ? 0 : 1;
					  endX   = isRightAvail ? width : (width -1);
					  for (y=0; y< height; y++)
					  {
						signLeft = (Char)sgn(srcLine[startX] - srcLine[startX-1]);
						for (x=startX; x< endX; x++)
						{

						  signRight = (Char)sgn(srcLine[x] - srcLine[x+1]); 
						  edgeType =  signRight + signLeft;
						  signLeft  = -signRight;
						  resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
						}
						srcLine  += srcStride;
						resLine += resStride;
					  }
					}
					break;
				  case 1:
					{
					  char *signUpLine = m_signLineBuf1;
					  Pel* srcLineAbove= srcLine- srcStride;
					  Pel* srcLineBelow;
					  
					  offset += 2;
					  startY = isAboveAvail ? 0 : 1;
					  endY   = isBelowAvail ? height : height-1;
					  if (!isAboveAvail)
					  {
						srcLine += srcStride;
						resLine += resStride;
					  }

					  for (x=0; x< width; x++)
					  {
						signUpLine[x] = (Char)sgn(srcLine[x] - srcLineAbove[x]);
					  }

					  for (y=startY; y<endY; y++)
					  {
						srcLineBelow= srcLine+ srcStride;

						for (x=0; x< width; x++)
						{
						  signDown  = (Char)sgn(srcLine[x] - srcLineBelow[x]);

						  edgeType = signDown + signUpLine[x];
						  signUpLine[x]= -signDown;

						  resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
						}
						srcLine += srcStride;
						resLine += resStride;
					  }
					}
					break;
				  case 2:
					{
					  
					  Char *signUpLine, *signDownLine, *signTmpLine;
					  Pel* srcLineBelow= srcLine+ srcStride;
					  Pel* srcLineAbove= srcLine- srcStride;
					  
					  offset += 2;
					  signUpLine  = m_signLineBuf1;
					  signDownLine= m_signLineBuf2;

					  startX = isLeftAvail ? 0 : 1 ;
					  endX   = isRightAvail ? width : (width-1);

					  //prepare 2nd line's upper sign
					  
					  for (x=startX; x< endX+1; x++)
					  {
						signUpLine[x] = (Char)sgn(srcLineBelow[x] - srcLine[x- 1]);
					  }

					  //1st line
					  firstLineStartX = isAboveLeftAvail ? 0 : 1;
					  firstLineEndX   = isAboveAvail? endX: 1;
					  for(x= firstLineStartX; x< firstLineEndX; x++)
					  {
						edgeType  =  sgn(srcLine[x] - srcLineAbove[x- 1]) - signUpLine[x+1];
						resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
					  }
					  srcLine  += srcStride;
					  resLine  += resStride;
					  
					  //middle lines
					  for (y= 1; y< height-1; y++)
					  {
						srcLineBelow= srcLine+ srcStride;

						for (x=startX; x<endX; x++)
						{
						  signDown =  (Char)sgn(srcLine[x] - srcLineBelow[x+ 1]);

						  edgeType =  signDown + signUpLine[x];
						  resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];

						  signDownLine[x+1] = -signDown; 
						}

						signDownLine[startX] = (Char)sgn(srcLineBelow[startX] - srcLine[startX-1]);

						signTmpLine  = signUpLine;
						signUpLine   = signDownLine;
						signDownLine = signTmpLine;

						srcLine += srcStride;
						resLine += resStride;
					  }

					  //last line
					  srcLineBelow= srcLine+ srcStride;
					  lastLineStartX = isBelowAvail ? startX : (width -1);
					  lastLineEndX   = isBelowRightAvail ? width : (width -1);
					  for(x= lastLineStartX; x< lastLineEndX; x++)
					  {
						edgeType =  sgn(srcLine[x] - srcLineBelow[x+ 1]) + signUpLine[x];
						resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
					  }
					}
					break;
				  case 3:
					{
					  char *signUpLine = m_signLineBuf1+1;
					  Pel* srcLineBelow= srcLine+ srcStride;
					  Pel* srcLineAbove= srcLine- srcStride;
					  
					  offset += 2;
					  startX = isLeftAvail ? 0 : 1;
					  endX   = isRightAvail ? width : (width -1);

					  //prepare 2nd line upper sign
					  
					  for (x=startX-1; x< endX; x++)
					  {
						signUpLine[x] = (Char)sgn(srcLineBelow[x] - srcLine[x+1]);
					  }

					  //first line
					  firstLineStartX = isAboveAvail ? startX : (width -1 );
					  firstLineEndX   = isAboveRightAvail ? width : (width-1);
					  for(x= firstLineStartX; x< firstLineEndX; x++)
					  {
						edgeType = sgn(srcLine[x] - srcLineAbove[x+1]) -signUpLine[x-1];
						resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
					  }
					  srcLine += srcStride;
					  resLine += resStride;

					  //middle lines
					  for (y= 1; y< height-1; y++)
					  {
						srcLineBelow= srcLine+ srcStride;

						for(x= startX; x< endX; x++)
						{
						  signDown =  (Char)sgn(srcLine[x] - srcLineBelow[x-1]);

						  edgeType =  signDown + signUpLine[x];
						  resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
						  signUpLine[x-1] = -signDown; 
						}

						signUpLine[endX-1] = (Char)sgn(srcLineBelow[endX-1] - srcLine[endX]);

						srcLine  += srcStride;
						resLine += resStride;
					  }

					  //last line
					  srcLineBelow= srcLine+ srcStride;
					  lastLineStartX = isBelowLeftAvail ? 0 : 1;
					  lastLineEndX   = isBelowAvail ? endX : 1;
					  for(x= lastLineStartX; x< lastLineEndX; x++)
					  {
						edgeType = sgn(srcLine[x] - srcLineBelow[x-1]) + signUpLine[x];
						resLine[x] = offsetClip[srcLine[x] + offset[edgeType]];
					  }
					}
					break;
				  case 4:
					{
					  Int shiftBits = 3;
					  for (y=0; y< height; y++)
					  {
						for (x=0; x< width; x++)
						{
						  resLine[x] = offsetClip[ srcLine[x] + offset[srcLine[x] >> shiftBits] ];
						}
						srcLine += srcStride;
						resLine += resStride;
					  }
					}
					break;
				  }
				}					
			}
	
			Out[0].ul_pcPic = In[0].ul_pcPic;
			//Out[0].ul_m_pcEncTop = In[0].ul_m_pcEncTop;
			Out[0].ul_reconParams = In[0].ul_reconParams;
		}
		window
		{
			In sliding(1,1);
			Out tumbling(1);
		}

	};
}

//输入: ctu + srcYuv + resYuv + pPic + reconParams；
//对CTU进行补偿，分三个颜色分量进行
composite offsetCTU(output stream<unsigned long ul_reconParams,unsigned long ul_pcPic>Out,
                     input stream<unsigned long ul_reconParams,unsigned long ul_srcYuv,unsigned long ul_pcPic>In)
{
	Out = splitjoin(In)
	{
		int compIdx,num_sao_components = 3;
		split duplicate();  //窗口暂定为1,即一个CTU
		for(compIdx = 0; compIdx < num_sao_components;compIdx++)  //对三个颜色分量采用split
		{
			if(compIdx == 0) add offsetCTU_Y(compIdx);  
			else if(compIdx == 1) add offsetCTU_Cb(compIdx);  
			else add offsetCTU_Cr(compIdx);  
		}
		join roundrobin(1);
	};
}

//输入: pPic + reconParams;
composite numLcusForSAO(output stream<unsigned long ul_pcPic>Out, input stream<unsigned long ul_reconParams,unsigned long ul_pcPic>In)
{
	Out = numLcusForSAO(In)
	{
		work
		{
			SAOBlkParam* reconParams = (SAOBlkParam*)(In[0].ul_reconParams);
			TComPic*   pPic    = (TComPic*)(In[0].ul_pcPic);
			//TEncTop*   m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //获取SAO滤波的类指针，或者说句柄
			
			//Int picTempLayer = pPic->m_apcPicSym->m_apcTComSlice[0]->m_iDepth;
			TComPicSym* picSym = pPic->m_apcPicSym;
			TComSlice** pSlice = picSym->m_apcTComSlice;
			Int picTempLayer = pSlice[0]->m_iDepth;
			
			Int numLcusForSAOOff[3] = {0,0,0};
			Int compIdx = 0,ctu=0;
			for (compIdx = 0; compIdx < 3; compIdx++)
			{
				for(ctu = 0; ctu < m_numCTUsPic; ctu++)
				{
					
				  //if( reconParams[ctu][compIdx].modeIdc == 0)
				  if( reconParams[ctu].offsetParam[compIdx].modeIdc==0)
				  {
					numLcusForSAOOff[compIdx]++;
				  }
				}
			}
			for (compIdx=0; compIdx<3; compIdx++)
			{
				m_pcSAO->m_saoDisabledRate[compIdx][picTempLayer] = (Double)numLcusForSAOOff[compIdx]/(Double)m_numCTUsPic;
			}
			//
			freeReconParams(reconParams);//delete[] reconParams; 删除空间
			Out[0].ul_pcPic = In[0].ul_pcPic;
			//Out[0].ul_m_pcEncTop = In[0].ul_m_pcEncTop;
			
		}
		window
		{
			In sliding(3*m_numCTUsPic,3*m_numCTUsPic);
			Out tumbling(1);
		}	
	};
}


//输入为 pPic,m_statData,srcYuv,resYuv,this指针
composite decideBlkParams(output stream<unsigned long ul_pcPic>Out, input stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pcPic>In)
{
	stream<unsigned long ul_reconParams,unsigned long ul_srcYuv,unsigned long ul_pcPic> Out1;
	stream<unsigned long ul_reconParams,unsigned long ul_pcPic> Out2;
	Out1 = decideModeRDO_recBlkSAOParam(In)();
	Out2 = offsetCTU(Out1)();
	Out = numLcusForSAO(Out2)();
}

//功能：完成重构图像的拷贝的扩展标记的设置，只有在处理一个新图片时进行，因此需要用计数count标记是否在处理一个新图片
//输入：pPic,lambdas(函数调用获取),this,编码类TEncTop指针；
//输出: LCU的总个数按0，num-1输出；orgYuv, srcYuv,pPic,this指针
composite BordedExtension(output stream<unsigned long ul_srcYuv,unsigned long ul_pcPic>Out, input stream<unsigned long ul_pcPic>In)
{
	Out = BordedExtension(In)
	{
		unsigned long count = 0;
		TComPicYuv* srcYuv = NULL;
		
		init 
		{
			count = 0;
		}
		work
		{
			
			
			if(count == 0)
			{
				//TEncTop                *m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
				TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //获取SAO滤波的类指针，或者说句柄
				TComPic*                   pPic    = (TComPic*)(In[0].ul_pcPic);
				TComPicYuv* orgYuv= pPic->m_apcPicYuv[0];
				TComPicYuv* resYuv= pPic->m_apcPicYuv[1];
				
				
				//const double* lambdas = pcPic->getSlice(0)->getLambdas()
				//double *lambdas = pPic->m_apcPicSym->m_apcTComSlice[0]->m_lambdas;
				TComPicSym* picSym = pPic->m_apcPicSym;
				TComSlice** pSlice = picSym->m_apcTComSlice;
				double *lambdas = pSlice[0]->m_lambdas;
				
				
				int m_iPicWidth = resYuv->m_iPicWidth;
				int m_iPicHeight = resYuv->m_iPicHeight;
				int m_iLumaMarginX = resYuv->m_iLumaMarginX;
				int m_iChromaMarginX = resYuv->m_iChromaMarginX;
				int m_iLumaMarginY = resYuv->m_iLumaMarginY;
				int m_iChromaMarginY = resYuv->m_iChromaMarginY;
				
				srcYuv = m_pcSAO->m_tempPicYuv;  
				
				m_pcSAO->m_lambda[0]= lambdas[0]; m_pcSAO->m_lambda[1]= lambdas[1]; m_pcSAO->m_lambda[2]= lambdas[2];
				
				memcpy ( srcYuv->m_apiPicBufY, resYuv->m_apiPicBufY, 2 * ( m_iPicWidth       + (m_iLumaMarginX   << 1)) * ( m_iPicHeight       + (m_iLumaMarginY   << 1)) );
				memcpy ( srcYuv->m_apiPicBufU, resYuv->m_apiPicBufU, 2 * ((m_iPicWidth >> 1) + (m_iChromaMarginX << 1)) * ((m_iPicHeight >> 1) + (m_iChromaMarginY << 1)) );
				memcpy ( srcYuv->m_apiPicBufV, resYuv->m_apiPicBufV, 2 * ((m_iPicWidth >> 1) + (m_iChromaMarginX << 1)) * ((m_iPicHeight >> 1) + (m_iChromaMarginY << 1)) );
				
				srcYuv->m_bIsBorderExtended = false;
			
				//xExtendPicCompBorder( srcYuv->m_piPicOrgY, getStride(),  getWidth(),      getHeight(),      m_iLumaMarginX,   m_iLumaMarginY   );
				//xExtendPicCompBorder( srcYuv->m_piPicOrgU  , getCStride(), getWidth() >> 1, getHeight() >> 1, m_iChromaMarginX, m_iChromaMarginY );
				//xExtendPicCompBorder( srcYuv->m_piPicOrgV  , getCStride(), getWidth() >> 1, getHeight() >> 1, m_iChromaMarginX, m_iChromaMarginY );
				{
					Pel* pi = srcYuv->m_piPicOrgY;
					int iHeight = m_iPicHeight;
					int iWidth = m_iPicWidth;
					int iMarginX = m_iLumaMarginX;
					int iMarginY = m_iLumaMarginY;
					int iStride = (m_iPicWidth     ) + (m_iLumaMarginX  <<1);
					
					int x,y,i;
					for(i = 0; i < 3; i++)  
					{
						if(i != 0 ) //色度分量
						{
							if(i==1) pi = srcYuv->m_piPicOrgU;
							else pi = srcYuv->m_piPicOrgV;
							
							iStride = (m_iPicWidth >> 1) + (m_iChromaMarginX<<1);
							iWidth >>= 1;
							iHeight >>= 1;
							iMarginX = m_iChromaMarginX;
							iMarginY = m_iChromaMarginY;
						}					
						
					  
						for ( y = 0; y < iHeight; y++)
						{
							for ( x = 0; x < iMarginX; x++ )
							{
								pi[ -iMarginX + x ] = pi[0];
								pi[    iWidth + x ] = pi[iWidth-1];
							}
							pi += iStride;
						}
		  
						pi -= (iStride + iMarginX);
						for ( y = 0; y < iMarginY; y++ )
						{
							memcpy( pi + (y+1)*iStride, pi, 2*(iWidth + (iMarginX<<1)) );
						}
		  
						pi -= ((iHeight-1) * iStride);
						for ( y = 0; y < iMarginY; y++ )
						{
							memcpy( pi - (y+1)*iStride, pi, 2*(iWidth + (iMarginX<<1)) );
						}
						
					}
				}
				
				srcYuv->m_bIsBorderExtended = true;
			}
			
			count +=  64;  //一个LCU有64个8x8块
			if(count == 64*m_uiNumCUsInFrame) count = 0;  //一张图像处理完
			
			//unsigned long ul_srcYuv = (unsigned long)srcYuv;
			//unsigned long ul_m_statData = (unsigned long)m_pcSAO->m_statData;
			
			Out[0].ul_pcPic = In[0].ul_pcPic;
				//Out[i].ul_m_pcEncTop = In[0].ul_m_pcEncTop;
			Out[0].ul_srcYuv = (unsigned long)srcYuv;
				//Out[i].ul_m_statData = (unsigned long)m_pcSAO->m_statData;  //
		}
		window
		{
			In sliding(64,64);  //输入为64个8x8块，即一个LCU
			Out tumbling(1);   //
		}	
	};
}

//composite SAOProcess
composite SAOProcess(output stream<unsigned long ul_pcPic>Out, input stream<unsigned long ul_pcPic>In)
{
	stream<unsigned long ul_srcYuv,unsigned long ul_pcPic>Out1;
	stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pcPic>Out2;
	Out1 = BordedExtension(In)(); //输出：LCU的总个数按0，num-1输出；orgYuv, srcYuv,pPic,this指针
	Out2 = getStatistics(Out1)(); //输出：包含orgYuv,srcYuv,pPic,m_statData,LCU编号,this指针
	Out = decideBlkParams(Out2)(); //输入：pPic，m_statData,srcYuv,resYuv
}

composite  Main()
{
	stream<UInt rasterOrder,unsigned long ul_pcPic>Out1,Out2;
	stream<unsigned long ul_pcPic>Out3,Out4;
	Out1 = Source()
	{
		int rasCount = 0;
		init
		{
			rasCount = 0;
		}
		work
		{
			int rasorder[64] = {0,2,4,6,8,10,12,14,
				                32,34,36,38,40,42,44,46,
								64,66,68,70,72,74,76,78,
								96,98,100,102,104,106,108,110,
								128,130,132,134,136,138,140,142,
								160,162,164,166,168,170,172,174,
								192,194,196,198,200,202,204,206,
								224,226,228,230,232,234,236,238};
			
			Out1[0].rasterOrder = rasorder[rasCount++];  //输出为 去块滤波8x8块的光栅顺序
			Out1[0].ul_pcPic = (unsigned long)pcPic;
			if(rasCount == 64) rasCount = 0;  //一个LCU处理完成，重置下一个LCU的8x8块扫描顺序
		}
		window
		{
			Out1 tumbling(1);
		}
	};
	//垂直边界滤波
	Out2 = DeblockCU_VER_EDGE(Out1)();
	
	Out3 = DeblockCU_HOR_EDGE(Out2)();  //水平边界滤波后的输出作为SAO的输入
	
	Out4 = SAOProcess(Out3)();
	Sink(Out4)
	{
		work
		{
			//printf("%X\n",Out4[0].ul_pcPic);
			//println(Out4[0].ul_pcPic);
		}
		window
		{
			Out4 tumbling(1);
		}
	};	
}
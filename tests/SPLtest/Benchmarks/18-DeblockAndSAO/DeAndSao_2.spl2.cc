//ע����� DeblockAndSAO_1.spl2.cc �ļ� ��SAO��5���������ϵ�һ���ڵ���
//���DeAndSao.spl2.cc չ��offsetCTU
//���DeAndSao_1.spl2.cc ˮƽ�ʹ�ֱ�˲����ú�������

//#include <CommonDef.h>   //Clip3�����������ļ�
//#include <math.h>
//#include <stdlib.h>


#define MAX_QP 51
#define DEFAULT_INTRA_TC_OFFSET 2

//ע����SAO�Ƕ��ڵ�֡�Ĵ���ʹ�õ���m_pcSAOָ����ָ�Ĺ��ÿռ䣻����ͳ��5�������Ƿֿ���pipeline��5���ڵ��еģ���getCopntStatistics;
//Ҳ���Խ���5���ڵ�������һ���ڵ������


#define UInt unsigned int
//#define bool bool   //costream��֧��bool���ͣ���unsigned char�ʹ���
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




typedef       long long         Int64;   //��linux�²��Դ�СΪ 8 bytes
typedef       unsigned long long  UInt64;//��linux�²��Դ�СΪ 8 bytes

struct TComDataCU;  //ǰ�������ṹ��
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
extern int m_numCTUInWidth; //����ͼ�񹲺��е�LCU������
extern int m_numCTUInHeight; //����ͼ�񹲺��е�LCU������
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

	
extern TComPic* pcPic;	//��pcPic��Ϊ�ⲿ������ֻ�ܹ�ʵ�ֵ�֡�������˲�����Ҫʵ�ֶ�֡����pcPic��Ϊ�������ݣ�����In�У��ɶ�λ����ȷͼ��
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




//8x8 cu��ĵ�ˮƽ�˲���Ҳ����ֱ�߽��ȥ���˲�,�˲��ĵ�λ��ÿһ��zscan��8*8��
//Ϊ�˷�ֹ��ˮƽ�߽���˲�����д��ͻ��8x8����鰴�չ�դɨ���˳�����롣
//��դ˳��Ϊ��0�� 0,2,4,6,8,10,12,14;��2�� 32,34,36,38,40,42,44,46; .... ��14��224,226,228,230,232,234,236,238;
//��Ӧ��uiAbsZorderIdx ���Ը��� g_auiRasterToZscan[rasterOrder]���

//����ɹ�����
composite DeblockCU_VER_EDGE(output stream<UInt rasterOrder,unsigned long ul_pcPic>Out, input stream<UInt rasterOrder,unsigned long ul_pcPic>In)
{
	
	Out = xDeBlockCU_VER_EDGE(In)
	{
		Int uiCUAddr;
	    TComDataCU* pcCU = NULL;   //��ǰLCU��ָ��
		init
		{
			uiCUAddr = 0;
			pcCU = pcPicSym->m_apcTComDataCU[uiCUAddr];
		}
				work
		{
			
			UInt rasterOrder = In[0].rasterOrder;//����In[0]Ϊ��ǰҪ���д�ֱ�߽��˲������8x8��
			Int iter = 1; //Ĭ��ֻ����һ�Σ�����λ��64x64������һ�е�8x8�飬����Ҫ�������Σ���Ϊ�ÿ���Ҫ��һ��LCU�ĵ�һ�е�8x8�������д�ֱ�߽���˲�
			Int cur = 0;
			
			UInt cpRasterOrder = rasterOrder;  //����ǰ��Ĺ�դ˳�򱣴������� cpRasterOrder
			UInt colNum = rasterOrder/16;  //8x8������������                                                                             
			UInt uiAbsZorderIdx = g_auiRasterToZscan[rasterOrder];
			
			UInt uiDepth = pcCU->m_puhDepth[uiAbsZorderIdx]; //�����ǰ�����ڵ����
			UInt TransformIdx = pcCU->m_puhTrIdx[uiAbsZorderIdx]; //�任����
			
			UInt uiX = pcCU->m_uiCUPelX + g_auiRasterToPelX[rasterOrder];
			UInt uiY = pcCU->m_uiCUPelY + g_auiRasterToPelY[rasterOrder];
			
			if((rasterOrder%16==14) && ( (uiCUAddr%m_numCTUInWidth)!= m_numCTUInWidth-1 ) ) //8x8��λ�ڵ�ǰLCU�����һ�� �� ��LCU��λ������ͼ��������LCUʱ
			{
				iter = 2;
				cur = -1; // cur��ʼֵΪ-1��-1����1������2��
			}
			if((uiCUAddr!=0) && (rasterOrder%16 == 0))  //�ǵ�һ��LCU��8x8��λ�ڵ�һ�У���ʱ���ý��д�ֱ�߽��˲�����Ϊ�ÿ���˲���ǰһ��LCU�����һ��8x8������˲�ʱһ������
			{
				iter = 0;
				cur = 1;  // cur��ʼΪ1����1��Ҳ������
			}
			
			
			for(cur;cur<1;cur++)  //Ĭ��ֻ����һ��,����ͨ���ı�cur����ʼֵ��ʵ�ʸı�ѭ���Ĵ���
			{
				
				if(iter == 2 && cur == 0) //���ڶ��ε���ʱ��˵��Ҫ���������һ��LCU�ĵ�һ�У���ʱҪ���ø�8x8����LCU�������Ϣ
				{
					uiCUAddr++;
					pcCU = pcPicSym->m_apcTComDataCU[uiCUAddr]; //��ȡ��һ��LCU
					rasterOrder = colNum*16;//����ԭ8x8�����������ó���ǰλ�ڵ�һ�е�8x8��Ĺ�դ˳��
				                                                                            
					uiAbsZorderIdx = g_auiRasterToZscan[rasterOrder];
			
					uiDepth = pcCU->m_puhDepth[uiAbsZorderIdx]; //�����ǰ�����ڵ����
					TransformIdx = pcCU->m_puhTrIdx[uiAbsZorderIdx]; //�任����
			
					uiX = pcCU->m_uiCUPelX + g_auiRasterToPelX[rasterOrder];
					uiY = pcCU->m_uiCUPelY + g_auiRasterToPelY[rasterOrder];	
				}
				
				VerEdgefilter_cos(uiX,uiY,pcCU,uiCUAddr,uiDepth,uiAbsZorderIdx,rasterOrder,TransformIdx);
				
				//λ�����һ�е�8x8�����,����pcCUָ��Ϊԭ��LCU��ָ�� 
				if(iter == 2 && cur == 0) {uiCUAddr--; pcCU = pcPicSym->m_apcTComDataCU[uiCUAddr];}
			}
			
			
			//һ��LCU�����һ��8x8�����,����pcCUָ��
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


//8x8 cu��ĵ�ˮƽ�߽�ȥ���˲�,�˲��ĵ�λ��ÿһ��zscan��8*8��
//�������˳��Ϊ��ֱ�߽�ȥ���˲������˳��Ҳ����դ˳��Ϊ 0,2,4,6,8,10,12,14; 32,34,36,38,40,42,44,46; .... 224,226,228,230,232,234,236,238
//��Ӧ��uiAbsZorderIdx ���Ը��� g_auiRasterToZscan[rasterOrder]���


composite DeblockCU_HOR_EDGE(output stream<unsigned long ul_pcPic>Out, input stream<UInt rasterOrder,unsigned long ul_pcPic>In)
{
	
	Out = xDeBlockCU_HOR_EDGE(In)
	{
		Int uiCUAddr;
	    TComDataCU* pcCU = NULL;   //��ǰLCU��ָ��
		init
		{
			uiCUAddr = 0;
			pcCU = pcPicSym->m_apcTComDataCU[uiCUAddr];
		}
		work
		{
			
			UInt rasterOrder = In[0].rasterOrder;//�����Ϊ2�����ڵ�8*8��,In[0]Ϊ��ǰҪ����ˮƽ�˲������8x8��
			UInt uiAbsZorderIdx = g_auiRasterToZscan[rasterOrder];
			
			UInt uiDepth = pcCU->m_puhDepth[uiAbsZorderIdx]; //�����ǰ�����ڵ����
			UInt TransformIdx = pcCU->m_puhTrIdx[uiAbsZorderIdx]; //�任����
			
			
			UInt uiX = pcCU->m_uiCUPelX + g_auiRasterToPelX[rasterOrder];
			UInt uiY = pcCU->m_uiCUPelY + g_auiRasterToPelY[rasterOrder];
			
			HorEdgefilter_cos(uiX,uiY,pcCU,uiCUAddr,uiDepth,uiAbsZorderIdx,rasterOrder,TransformIdx);
			
			//һ��LCU�����һ��8x8�����,����pcCUָ��
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
		int compIdx;  //compIdx������ɫ������0���ȣ�1,2ɫ�ȣ�
	
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
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //��ȡSAO�˲�����ָ�룬����˵���
			
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
			//ʹ�ú�������
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
 //���ܣ��Ը���CTU������Ϣͳ��
 //���룺����orgYuv,srcYuv,pPic,m_statData,thisָ�� 
composite getStatistics(output stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pcPic>Out,
                         input stream<unsigned long ul_srcYuv,unsigned long ul_pcPic>In)
{
	Out = splitjoin(In)  //
	{
		int num_sao_components = 3;
		int compIdx = 0;
		split duplicate();  //�����ݶ�Ϊ1
		for(compIdx= 0; compIdx < num_sao_components;compIdx++)  //��������ɫ��������split
		{
			add getCopntStatistics(compIdx);  
		}
		join roundrobin(1);
	};
}


//���: ԭ����+reconParams; �����������Ϊm_numCTUsPic
//�ⲿ��Ҫ��װ�������vector<>��new,�ȶ���c ��֧�ֵģ�

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
			
			//��������һ��������װvoid my_BlkModeRDO_recSAOParam(int ctu,SAOBlkParam &modeParam,SAOBlkParam* codedParams,SAOBlkParam* reconParams,unsigned long ul_m_statData,unsigned long ul_pcPic,unsigned long ul_m_pcEncTop);
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

//����: ctu + srcYuv + resYuv + pPic + reconParams��
//�����pPic + reconParams

//�����￪ʼ
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
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //��ȡSAO�˲�����ָ�룬����˵���
			
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
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //��ȡSAO�˲�����ָ�룬����˵���
			
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
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //��ȡSAO�˲�����ָ�룬����˵���
			
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

//����: ctu + srcYuv + resYuv + pPic + reconParams��
//��CTU���в�������������ɫ��������
composite offsetCTU(output stream<unsigned long ul_reconParams,unsigned long ul_pcPic>Out,
                     input stream<unsigned long ul_reconParams,unsigned long ul_srcYuv,unsigned long ul_pcPic>In)
{
	Out = splitjoin(In)
	{
		int compIdx,num_sao_components = 3;
		split duplicate();  //�����ݶ�Ϊ1,��һ��CTU
		for(compIdx = 0; compIdx < num_sao_components;compIdx++)  //��������ɫ��������split
		{
			if(compIdx == 0) add offsetCTU_Y(compIdx);  
			else if(compIdx == 1) add offsetCTU_Cb(compIdx);  
			else add offsetCTU_Cr(compIdx);  
		}
		join roundrobin(1);
	};
}

//����: pPic + reconParams;
composite numLcusForSAO(output stream<unsigned long ul_pcPic>Out, input stream<unsigned long ul_reconParams,unsigned long ul_pcPic>In)
{
	Out = numLcusForSAO(In)
	{
		work
		{
			SAOBlkParam* reconParams = (SAOBlkParam*)(In[0].ul_reconParams);
			TComPic*   pPic    = (TComPic*)(In[0].ul_pcPic);
			//TEncTop*   m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //��ȡSAO�˲�����ָ�룬����˵���
			
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
			freeReconParams(reconParams);//delete[] reconParams; ɾ���ռ�
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


//����Ϊ pPic,m_statData,srcYuv,resYuv,thisָ��
composite decideBlkParams(output stream<unsigned long ul_pcPic>Out, input stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pcPic>In)
{
	stream<unsigned long ul_reconParams,unsigned long ul_srcYuv,unsigned long ul_pcPic> Out1;
	stream<unsigned long ul_reconParams,unsigned long ul_pcPic> Out2;
	Out1 = decideModeRDO_recBlkSAOParam(In)();
	Out2 = offsetCTU(Out1)();
	Out = numLcusForSAO(Out2)();
}

//���ܣ�����ع�ͼ��Ŀ�������չ��ǵ����ã�ֻ���ڴ���һ����ͼƬʱ���У������Ҫ�ü���count����Ƿ��ڴ���һ����ͼƬ
//���룺pPic,lambdas(�������û�ȡ),this,������TEncTopָ�룻
//���: LCU���ܸ�����0��num-1�����orgYuv, srcYuv,pPic,thisָ��
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
				TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //��ȡSAO�˲�����ָ�룬����˵���
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
						if(i != 0 ) //ɫ�ȷ���
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
			
			count +=  64;  //һ��LCU��64��8x8��
			if(count == 64*m_uiNumCUsInFrame) count = 0;  //һ��ͼ������
			
			//unsigned long ul_srcYuv = (unsigned long)srcYuv;
			//unsigned long ul_m_statData = (unsigned long)m_pcSAO->m_statData;
			
			Out[0].ul_pcPic = In[0].ul_pcPic;
				//Out[i].ul_m_pcEncTop = In[0].ul_m_pcEncTop;
			Out[0].ul_srcYuv = (unsigned long)srcYuv;
				//Out[i].ul_m_statData = (unsigned long)m_pcSAO->m_statData;  //
		}
		window
		{
			In sliding(64,64);  //����Ϊ64��8x8�飬��һ��LCU
			Out tumbling(1);   //
		}	
	};
}

//composite SAOProcess
composite SAOProcess(output stream<unsigned long ul_pcPic>Out, input stream<unsigned long ul_pcPic>In)
{
	stream<unsigned long ul_srcYuv,unsigned long ul_pcPic>Out1;
	stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pcPic>Out2;
	Out1 = BordedExtension(In)(); //�����LCU���ܸ�����0��num-1�����orgYuv, srcYuv,pPic,thisָ��
	Out2 = getStatistics(Out1)(); //���������orgYuv,srcYuv,pPic,m_statData,LCU���,thisָ��
	Out = decideBlkParams(Out2)(); //���룺pPic��m_statData,srcYuv,resYuv
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
			
			Out1[0].rasterOrder = rasorder[rasCount++];  //���Ϊ ȥ���˲�8x8��Ĺ�դ˳��
			Out1[0].ul_pcPic = (unsigned long)pcPic;
			if(rasCount == 64) rasCount = 0;  //һ��LCU������ɣ�������һ��LCU��8x8��ɨ��˳��
		}
		window
		{
			Out1 tumbling(1);
		}
	};
	//��ֱ�߽��˲�
	Out2 = DeblockCU_VER_EDGE(Out1)();
	
	Out3 = DeblockCU_HOR_EDGE(Out2)();  //ˮƽ�߽��˲���������ΪSAO������
	
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
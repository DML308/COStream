#define UInt unsigned int
//#define bool bool   //costream不支持bool类型，用unsigned char型代替
#define bool unsigned char
#define Pel short
#define Int int
#define Double double
#define Float float
#define Char char
#define UChar unsigned char
#define NULL 0
#define true 1
#define false 0

typedef       long long           Int64;
typedef       unsigned long long  UInt64;

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
	
	Int   m_iLumaMarginX;
	Int   m_iLumaMarginY;
	Int   m_iChromaMarginX;
	Int   m_iChromaMarginY;
	
	bool  m_bIsBorderExtended;
	
};

typedef struct TComPicYuv TComPicYuv;

struct TComSlice
{
	 Double      m_lambdas[3];
	 Int         m_iDepth;
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
		
};

typedef struct TComPicSym TComPicSym;

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

extern int m_numCTUInWidth;
extern int m_numCTUInHeight;
extern int m_picWidth;
extern int m_picHeight;

extern int m_LumaMarginX;
extern int m_ChromaMarginX;
//iStride = (m_picWidth     ) + (m_LumaMarginX  <<1)
extern int iStride;
//iCStride = (m_picWidth     ) + (m_ChromaMarginX  <<1)
extern int iCStride;
extern SAOStatData***  m_statData; //[ctu][comp][classes] 

extern TComPic* pPic;
extern TEncTop* m_pcEncTop;

int m_maxCUHeight = 64;
int m_maxCUWidth = 64;
int m_numCTUsPic = 9;

//m_pcSAO                = pcTEncTop->getSAO();  //获取SAO滤波的类指针，或者说句柄
//TEncTop*                m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
//			TEncSampleAdaptiveOffset*  m_pcSAO = m_pcEncTop->getSAO();  //获取SAO滤波的类指针，或者说句柄
			
//			TComPic*   pPic    = (TComPic*)(In[0].ul_pPic);
//			TComPicYuv* orgYuv = pPic->m_apcPicYuv[0];
//			TComPicYuv* resYuv = pPic->m_apcPicYuv[1];

/*
composite getBlkStats_EO_0(output stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out, 
                            input stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	param
		int compIdx;  //compIdx代表颜色分量：0亮度，1,2色度；
		
	bool isLuma = (compIdx == 0);
	int formatShift = isLuma?0:1;
	
	Out = getBlkStats_EO_0(In)
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
			TComPic*   pPic    = (TComPic*)(In[0].ul_pPic);
			TComPicYuv* orgYuv = pPic->m_apcPicYuv[0];
			
			bool isLeftAvail = 0,isRightAvail = 0,isAboveAvail = 0,isBelowAvail = 0,isAboveLeftAvail = 0,isAboveRightAvail = 0,isBelowLeftAvail = 0,isBelowRightAvail = 0;
			int srcStride = isLuma?iStride:iCStride;
			int orgStride = srcStride;
			Pel* srcYuvBuf = NULL,*orgYuvBuf = NULL;
			
			TComPicYuv* srcYuv = (TComPicYuv*)(In[0].ul_srcYuv);
			TEncTop*    m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //获取SAO滤波的类指针，或者说句柄
			
			SAOStatData***  m_statData = (SAOStatData***)(In[0].ul_m_statData);
			
			
			
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
				
				int typeIdx = 0; // SAO_TYPE_EO_0
				SAOStatData* statsDataTypes = m_statData[ctu][compIdx];
				
				statsDataTypesSet(statsDataTypes,typeIdx,diff,count);
				
				//statsDataTypes[typeIdx].reset(); // reset(statsDataTypes[0]);
				
				
				srcLine = srcBlk;
				orgLine = orgBlk;
				//diff    = statsDataTypes[typeIdx].diff;
				//count   = statsDataTypes[typeIdx].count;
				
				
				width >>= formatShift;
				height >>= formatShift;
				
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
			
			Out[0].ul_m_statData = In[0].ul_m_statData ;
			Out[0].ul_srcYuv = In[0].ul_srcYuv ;
			Out[0].ul_pPic = In[0].ul_pPic ;
			Out[0].ul_m_pcEncTop= In[0].ul_m_pcEncTop ;
		}	
		window
		{
			In sliding(1,1);
			Out tumbling(1);
		}
	};
}

composite getBlkStats_EO_90(output stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out, 
                             input stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	param
		int compIdx;  //compIdx代表颜色分量：0亮度，1,2色度；
		
	bool isLuma = (compIdx == 0);
	int formatShift = isLuma?0:1;
	
	Out = getBlkStats_EO_90(In)
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
			TComPic*   pPic    = (TComPic*)(In[0].ul_pPic);
			TComPicYuv* orgYuv = pPic->m_apcPicYuv[0];
			TComPicYuv* srcYuv = (TComPicYuv*)(In[0].ul_srcYuv);
			TEncTop*    m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //获取SAO滤波的类指针，或者说句柄
			
			SAOStatData***  m_statData = (SAOStatData***)(In[0].ul_m_statData);
			
			int srcStride = isLuma?iStride:iCStride;
			int orgStride = srcStride;
			Pel* srcYuvBuf = NULL,*orgYuvBuf = NULL;
			bool isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail;
			
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
				//Pel* orgBlk     = getPicBuf(orgYuv, compIdx)+ (yPos >> formatShift)*orgStride+ (xPos >> formatShift)
				
				char m_signLineBuf1[m_maxCUWidth+1],m_signLineBuf2[m_maxCUWidth+1];
				
				Int x,y, startX, startY, endX, endY, edgeType, firstLineStartX, firstLineEndX;
				Char signLeft, signRight, signDown;
				Int64 *diff, *count;
				Pel *srcLine, *orgLine;
				
				Char *signUpLine = m_signLineBuf1;
				Pel* srcLineAbove = srcLine - srcStride;
				Pel* srcLineBelow;
				
				Int* skipLinesR = m_pcSAO->m_skipLinesR[compIdx]; 
				Int* skipLinesB = m_pcSAO->m_skipLinesB[compIdx];
				
				int typeIdx = 1; // SAO_TYPE_EO_90
				
				SAOStatData* statsDataTypes = m_statData[ctu][compIdx];
				statsDataTypesSet(statsDataTypes,typeIdx,diff,count);
				//SAOStatData* statsDataTypes = m_statData[ctu][compIdx];
				//statsDataTypes[typeIdx].reset(); // reset(statsDataTypes[0]);
				//memset(statsDataTypes[typeIdx].diff, 0, sizeof(Int64)*32);
				//memset(statsDataTypes[typeIdx].count, 0, sizeof(Int64)*32);
				
				srcLine = srcBlk;
				orgLine = orgBlk;
				//diff    = statsDataTypes[typeIdx].diff;
				//count   = statsDataTypes[typeIdx].count;
				
				width >>= formatShift;
				height >>= formatShift;
				
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
			
			
			Out[0].ul_m_statData = In[0].ul_m_statData ;
			Out[0].ul_srcYuv = In[0].ul_srcYuv ;
			Out[0].ul_pPic = In[0].ul_pPic ;
			Out[0].ul_m_pcEncTop= In[0].ul_m_pcEncTop ;
		}
		
		window
		{
			In sliding(1,1);
			Out tumbling(1);
		}
	};
}

//从此处开始
composite getBlkStats_EO_135(output stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out,
                              input stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	param
		int compIdx;  //compIdx代表颜色分量：0亮度，1,2色度；
		
	bool isLuma = (compIdx == 0);
	int formatShift = isLuma?0:1;
	
	Out = getBlkStats_EO_135(In)
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
			TComPic*   pPic    = (TComPic*)(In[0].ul_pPic);
			TComPicYuv* orgYuv = pPic->m_apcPicYuv[0];
			TComPicYuv* srcYuv = (TComPicYuv*)(In[0].ul_srcYuv);
			TEncTop*    m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //获取SAO滤波的类指针，或者说句柄
			
			SAOStatData***  m_statData = (SAOStatData***)(In[0].ul_m_statData);
			
			bool isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail;
			int srcStride = isLuma?iStride:iCStride;
			int orgStride = srcStride;
			Pel* srcYuvBuf = NULL,*orgYuvBuf = NULL;
			
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
				
				char *signUpLine, *signDownLine, *signTmpLine;
				Pel* srcLineBelow = srcLine + srcStride;
				Pel* srcLineAbove = srcLine - srcStride;
				
				Int* skipLinesR = m_pcSAO->m_skipLinesR[compIdx]; 
				Int* skipLinesB = m_pcSAO->m_skipLinesB[compIdx];
				
				int typeIdx = 2; // SAO_TYPE_EO_135
				
				SAOStatData* statsDataTypes = m_statData[ctu][compIdx];
				statsDataTypesSet(statsDataTypes,typeIdx,diff,count);
				//SAOStatData* statsDataTypes = m_statData[ctu][compIdx];
				//statsDataTypes[typeIdx].reset(); // reset(statsDataTypes[0]);
				//memset(statsDataTypes[typeIdx].diff, 0, sizeof(Int64)*32);
				//memset(statsDataTypes[typeIdx].count, 0, sizeof(Int64)*32);
				
				srcLine = srcBlk;
				orgLine = orgBlk;
				//diff    = statsDataTypes[typeIdx].diff;
				//count   = statsDataTypes[typeIdx].count;
				
				width >>= formatShift;
				height >>= formatShift;
				
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

				//1st line
				

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
			
			
			Out[0].ul_m_statData = In[0].ul_m_statData ;
			Out[0].ul_srcYuv = In[0].ul_srcYuv ;
			Out[0].ul_pPic = In[0].ul_pPic ;
			Out[0].ul_m_pcEncTop= In[0].ul_m_pcEncTop ;
		}
		
		window
		{
			In sliding(1,1);
			Out tumbling(1); //
		}
	};
}

composite getBlkStats_EO_45(output stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out,
                             input stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	param
		int compIdx;  //compIdx代表颜色分量：0亮度，1,2色度；
		
	bool isLuma = (compIdx == 0);
	int formatShift = isLuma?0:1;
	
	Out = getBlkStats_EO_45(In)
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
			TComPic*   pPic    = (TComPic*)(In[0].ul_pPic);
			TComPicYuv* orgYuv = pPic->m_apcPicYuv[0];
			TComPicYuv* srcYuv = (TComPicYuv*)(In[0].ul_srcYuv);
			TEncTop*    m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //获取SAO滤波的类指针，或者说句柄
			
			SAOStatData***  m_statData = (SAOStatData***)(In[0].ul_m_statData);
			
			bool isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail;
			int srcStride = isLuma?iStride:iCStride;
			int orgStride = srcStride;
			Pel* srcYuvBuf = NULL,*orgYuvBuf = NULL;
			
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
				
				int typeIdx = 3; // SAO_TYPE_EO_45
				SAOStatData* statsDataTypes = m_statData[ctu][compIdx];
				
				Char *signUpLine = m_signLineBuf1+1;
				Pel* srcLineBelow = srcLine + srcStride;
				Pel* srcLineAbove = srcLine - srcStride;
				
	
				statsDataTypesSet(statsDataTypes,typeIdx,diff,count);
				
				//statsDataTypes[typeIdx].reset(); // reset(statsDataTypes[0]);
				//memset(statsDataTypes[typeIdx].diff, 0, sizeof(Int64)*32);
				//memset(statsDataTypes[typeIdx].count, 0, sizeof(Int64)*32);
				
				srcLine = srcBlk;
				orgLine = orgBlk;
				//diff    = statsDataTypes[typeIdx].diff;
				//count   = statsDataTypes[typeIdx].count;
				
				width >>= formatShift;
				height >>= formatShift;
				
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
			
			
			Out[0].ul_m_statData = In[0].ul_m_statData ;
			Out[0].ul_srcYuv = In[0].ul_srcYuv ;
			Out[0].ul_pPic = In[0].ul_pPic ;
			Out[0].ul_m_pcEncTop= In[0].ul_m_pcEncTop ;
		}
		
		window
		{
			In sliding(1,1);
			Out tumbling(1); //
		}
	};
}

composite getBlkStats_BO(output stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out,
                          input stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	param
		int compIdx;  //compIdx代表颜色分量：0亮度，1,2色度；
		
	bool isLuma = (compIdx == 0);
	int formatShift = isLuma?0:1;
	
	Out = getBlkStats_BO(In)
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
			TComPic*   pPic    = (TComPic*)(In[0].ul_pPic);
			TComPicYuv* orgYuv = pPic->m_apcPicYuv[0];
			TComPicYuv* srcYuv = (TComPicYuv*)(In[0].ul_srcYuv);
			TEncTop*    m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //获取SAO滤波的类指针，或者说句柄
			
			SAOStatData***  m_statData = (SAOStatData***)(In[0].ul_m_statData);
			
			bool isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail;
			int srcStride = isLuma?iStride:iCStride;
			int orgStride = srcStride;
			Pel* srcYuvBuf = NULL,*orgYuvBuf = NULL;
			
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
				
				int typeIdx = 4; // SAO_TYPE_BO
				SAOStatData* statsDataTypes = m_statData[ctu][compIdx];
				
				Int shiftBits = 3;
				
				//SAOStatData* statsDataTypes = NULL;
				statsDataTypesSet(statsDataTypes,typeIdx,diff,count);
				
				//statsDataTypes[typeIdx].reset(); // reset(statsDataTypes[0]);
				//memset(statsDataTypes[typeIdx].diff, 0, sizeof(Int64)*32);
				//memset(statsDataTypes[typeIdx].count, 0, sizeof(Int64)*32);
				
				srcLine = srcBlk;
				orgLine = orgBlk;
				//diff    = statsDataTypes[typeIdx].diff;
				//count   = statsDataTypes[typeIdx].count;
				
				
				width >>= formatShift;
				height >>= formatShift;
				
				startX = 0;
				endX   = (isRightAvail ? (width - skipLinesR[typeIdx]) : width );

				endY = isBelowAvail ? (height- skipLinesB[typeIdx]) : height;
				
				for (y=0; y< endY; y++)
				{
		
					for (x=startX; x< endX; x++)
					{

						Int bandIdx= srcLine[x] >> shiftBits; 
						diff [bandIdx] += (orgLine[x] - srcLine[x]);
						count[bandIdx] ++;
					}
					srcLine += srcStride;
					orgLine += orgStride;
				}	
				
			}
			
			Out[0].ul_m_statData = In[0].ul_m_statData ;
			Out[0].ul_srcYuv = In[0].ul_srcYuv ;
			Out[0].ul_pPic = In[0].ul_pPic ;
			Out[0].ul_m_pcEncTop= In[0].ul_m_pcEncTop ;
		}
		window
		{
			In sliding(1,1);
			Out tumbling(1); //
		}
	};
}

composite getCopntStatistics(output stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out,
                              input stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	param
		int compIdx;  //compIdx代表颜色分量：0亮度，1,2色度；

	Out = pipeline(In)
	{
		add getBlkStats_EO_0(compIdx);
		add getBlkStats_EO_90(compIdx);
		add getBlkStats_EO_135(compIdx);
		add getBlkStats_EO_45(compIdx);
		add getBlkStats_BO(compIdx);
		
	};
	
}

//getStatistics(m_statData, orgYuv, srcYuv, pPic); 
 //功能：对各个CTU进行信息统计
 //输入：包含orgYuv,srcYuv,pPic,m_statData,this指针 
composite getStatistics(output stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out,
                         input stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	Out = splitjoin(In)  //
	{
		int num_sao_components = 3;
		int compIdx = 0;
		split duplicate(1);  //窗口暂定为1
		for(compIdx= 0; compIdx < num_sao_components;compIdx++)  //对三个颜色分量采用split
		{
			add getCopntStatistics(compIdx);  
		}
		join roundrobin(1);
	};
}


//输出: 原输入+reconParams; 其中输出窗口为m_numCTUsPic
//这部分要封装，里面的vector<>，new,等都是c 不支持的；

composite decideModeRDO_recBlkSAOParam(output stream<unsigned long ul_reconParams,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out,
                                        input stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
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
				//将下列用一个函数封装
				
				//SAOBlkParam* codedParams = pPic->getPicSym()->getSAOBlkParam();
				//reconParams = new SAOBlkParam[m_numCTUsPic];
				//m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[ SAO_CABACSTATE_PIC_INIT ]);
				
				my_BlkParamsRDOinit(codedParams,reconParams,In[0].ul_pPic,In[0].ul_m_pcEncTop);
			}	
			
			//将下列用一个函数封装void my_BlkModeRDO_recSAOParam(int ctu,SAOBlkParam &modeParam,SAOBlkParam* codedParams,SAOBlkParam* reconParams,unsigned long ul_m_statData,unsigned long ul_pPic,unsigned long ul_m_pcEncTop);
			my_BlkModeRDO_recSAOParam(ctu,modeParam,codedParams,reconParams,In[0].ul_pPic,In[0].ul_m_pcEncTop);
			
			Out[0].ul_srcYuv = In[0].ul_srcYuv;
			Out[0].ul_pPic = In[0].ul_pPic;
			Out[0].ul_m_pcEncTop = In[0].ul_m_pcEncTop;
			Out[0].ul_reconParams = (unsigned long)reconParams;
			
			if(++iCount == m_numCTUsPic) iCount = 0;
			
		}
		window
		{
			In sliding(3,3);
			Out tumbling(1); 
			
		}
		
	};
}

//输入: ctu + srcYuv + resYuv + pPic + reconParams；
//输出：pPic + reconParams

//从这里开始
composite offsetCTU_Y(output stream<unsigned long ul_reconParams,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out, 
					   input stream<unsigned long ul_reconParams,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	param 
		int compIdx;
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
			TComPic*   pPic    = (TComPic*)(In[0].ul_pPic);
			TEncTop*   m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
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
				

				Int  blkWidth   = width;
				Int  blkHeight  = height;
				Int  blkYPos    = yPos;
				Int  blkXPos    = xPos;

				Int  srcStride = iStride;
				Pel* srcBlk    = srcYuv->m_piPicOrgY + blkYPos*srcStride+ blkXPos;

				Int  resStride  = iStride;
				Pel* resBlk     = resYuv->m_piPicOrgY + blkYPos*resStride+ blkXPos;
				
				//block boundary availability
				//pPic->getPicSym()->deriveLoopFilterBoundaryAvailibility(ctu, isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail);
				my_deriveLoopFilterBoundaryAvailibility(pPic, ctu, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
				
				//offsetBlock( compIdx, ctbOffset.typeIdc, ctbOffset.offset, srcBlk, resBlk, srcStride, resStride, blkWidth, blkHeight, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
				my_offsetBlock(m_pcSAO,compIdx, reconParams[ctu].offsetParam[compIdx].typeIdc, reconParams[ctu].offsetParam[compIdx].offset, srcBlk, resBlk, srcStride, resStride, blkWidth, blkHeight, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
			}
	
			Out[0].ul_pPic = In[0].ul_pPic;
			Out[0].ul_m_pcEncTop = In[0].ul_m_pcEncTop;
			Out[0].ul_reconParams = In[0].ul_reconParams;
			
		}
		window
		{
			In sliding(1,1);
			Out tumbling(1);
		}
		
	};
}

composite offsetCTU_Cb(output stream<unsigned long ul_reconParams,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out, 
                        input stream<unsigned long ul_reconParams,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	param 
		int compIdx;
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
			TComPic*   pPic    = (TComPic*)(In[0].ul_pPic);
			TEncTop*   m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
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
				

				Int  blkWidth   = width>>1;
				Int  blkHeight  = height>>1;
				Int  blkYPos    = yPos>>1;
				Int  blkXPos    = xPos>>1;
				
				Int  srcStride = iCStride;
				Pel* srcBlk    = srcYuv->m_piPicOrgU + blkYPos*srcStride+ blkXPos;

				Int  resStride  = iStride;
				Pel* resBlk     = resYuv->m_piPicOrgU + blkYPos*resStride+ blkXPos;
				
				//block boundary availability
				//pPic->getPicSym()->deriveLoopFilterBoundaryAvailibility(ctu, isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail);
				my_deriveLoopFilterBoundaryAvailibility(pPic, ctu, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
				
				//offsetBlock( compIdx, ctbOffset.typeIdc, ctbOffset.offset, srcBlk, resBlk, srcStride, resStride, blkWidth, blkHeight, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
				my_offsetBlock(m_pcSAO,compIdx, reconParams[ctu].offsetParam[compIdx].typeIdc, reconParams[ctu].offsetParam[compIdx].offset, srcBlk, resBlk, srcStride, resStride, blkWidth, blkHeight, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
			}
	
			Out[0].ul_pPic = In[0].ul_pPic;
			Out[0].ul_m_pcEncTop = In[0].ul_m_pcEncTop;
			Out[0].ul_reconParams = In[0].ul_reconParams;
			
		}
		window
		{
			In sliding(1,1);
			Out tumbling(1);
		}	
	};
}

composite offsetCTU_Cr(output stream<unsigned long ul_reconParams,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out, 
                        input stream<unsigned long ul_reconParams,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	param 
		int compIdx;
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
			TComPic*   pPic    = (TComPic*)(In[0].ul_pPic);
			TEncTop*   m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
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
				

				Int  blkWidth   = width>>1;
				Int  blkHeight  = height>>1;
				Int  blkYPos    = yPos>>1;
				Int  blkXPos    = xPos>>1;
				
				Int  srcStride = iCStride;
				Pel* srcBlk    = srcYuv->m_piPicOrgV + blkYPos*srcStride+ blkXPos;

				Int  resStride  = iStride;
				Pel* resBlk     = resYuv->m_piPicOrgV + blkYPos*resStride+ blkXPos;
				
				//block boundary availability
				//pPic->getPicSym()->deriveLoopFilterBoundaryAvailibility(ctu, isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail);
				my_deriveLoopFilterBoundaryAvailibility(pPic, ctu, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
				
				//offsetBlock( compIdx, ctbOffset.typeIdc, ctbOffset.offset, srcBlk, resBlk, srcStride, resStride, blkWidth, blkHeight, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
				my_offsetBlock(m_pcSAO,compIdx, reconParams[ctu].offsetParam[compIdx].typeIdc, reconParams[ctu].offsetParam[compIdx].offset, srcBlk, resBlk, srcStride, resStride, blkWidth, blkHeight, isLeftAvail, isRightAvail, isAboveAvail, isBelowAvail, isAboveLeftAvail, isAboveRightAvail, isBelowLeftAvail, isBelowRightAvail);
					
			}
	
			Out[0].ul_pPic = In[0].ul_pPic;
			Out[0].ul_m_pcEncTop = In[0].ul_m_pcEncTop;
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
composite offsetCTU(output stream<unsigned long ul_reconParams,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out,
                     input stream<unsigned long ul_reconParams,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	Out = splitjoin(In)
	{
		int compIdx,num_sao_components = 3;
		split duplicate(1);  //窗口暂定为1,即一个CTU
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
composite numLcusForSAO(output stream<unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out, input stream<unsigned long ul_reconParams,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	Out = numLcusForSAO(In)
	{
		work
		{
			SAOBlkParam* reconParams = (SAOBlkParam*)(In[0].ul_reconParams);
			TComPic*   pPic    = (TComPic*)(In[0].ul_pPic);
			TEncTop*   m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //获取SAO滤波的类指针，或者说句柄
			
			Int picTempLayer = pPic->m_apcPicSym->m_apcTComSlice[0]->m_iDepth;
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
			freeReconParams(reconParams);//delete[] reconParams;
			Out[0].ul_pPic = In[0].ul_pPic;
			Out[0].ul_m_pcEncTop = In[0].ul_m_pcEncTop;
			
		}
		window
		{
			In sliding(m_numCTUsPic*3,m_numCTUsPic*3);
			Out tumbling(1);
		}	
	};
}


//输入为 pPic,m_statData,srcYuv,resYuv,this指针
composite decideBlkParams(output stream<unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out, input stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	stream<unsigned long ul_reconParams,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop> Out1;
	stream<unsigned long ul_reconParams,unsigned long ul_pPic,unsigned long ul_m_pcEncTop> Out2;
	Out1 = decideModeRDO_recBlkSAOParam(In)();
	Out2 = offsetCTU(Out1)();
	Out = numLcusForSAO(Out2)();
}

//composite SAOProcess
composite SAOProcess(output stream<unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out, input stream<unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out1,Out2,Out3;
	Out1 = BordedExtension(In)(); //输出：LCU的总个数按0，num-1输出；orgYuv, srcYuv,pPic,this指针
	Out2 = getStatistics(Out1)(); //输出：包含orgYuv,srcYuv,pPic,m_statData,LCU编号,this指针
	Out3 = decideBlkParams(Out2)(); //输入：pPic，m_statData,srcYuv,resYuv
}

*/
//输入：pPic,lambdas(函数调用获取),this,编码类TEncTop指针；
//输出: LCU的总个数按0，num-1输出；orgYuv, srcYuv,pPic,this指针
composite BordedExtension(output stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>Out, input stream<unsigned long ul_pPic,unsigned long ul_m_pcEncTop>In)
{
	Out = BordedExtension(In)
	{
		work
		{
			
			TEncTop*                m_pcEncTop = (TEncTop*)(In[0].ul_m_pcEncTop);
			TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //获取SAO滤波的类指针，或者说句柄
			TComPic*                   pPic    = (TComPic*)(In[0].ul_pPic);
			TComPicYuv* orgYuv= pPic->m_apcPicYuv[0];
			TComPicYuv* resYuv= pPic->m_apcPicYuv[1];
			
			TComPicYuv* srcYuv = m_pcSAO->m_tempPicYuv;
			
			//const double* lambdas = pcPic->getSlice(0)->getLambdas()
			const double* lambdas = pPic->m_apcPicSym->m_apcTComSlice[0]->m_lambdas;
			
			
			int m_iPicWidth = resYuv->m_iPicWidth;
			int m_iPicHeight = resYuv->m_iPicHeight;
			int m_iLumaMarginX = resYuv->m_iLumaMarginX;
			int m_iChromaMarginX = resYuv->m_iChromaMarginX;
			int m_iLumaMarginY = resYuv->m_iLumaMarginY;
			int m_iChromaMarginY = resYuv->m_iChromaMarginY;
			
			int i = 0;
			m_pcSAO->m_lambda[0]= lambdas[0]; m_pcSAO->m_lambda[1]= lambdas[1]; m_pcSAO->m_lambda[2]= lambdas[2];
			
			//temporary picture buffer
			/*if ( !m_tempPicYuv )
			{
				m_tempPicYuv = new TComPicYuv;
				m_tempPicYuv->create( m_picWidth, m_picHeight, m_maxCUWidth, m_maxCUHeight, maxCUDepth );
			}
			*/
			//TComPicYuv* srcYuv = m_tempPicYuv;
			//resYuv->copyToPic(srcYuv);
			//srcYuv->setBorderExtension(false);
			//srcYuv->extendPicBorder();
			
			
			
			memcpy ( srcYuv->m_apiPicBufY, resYuv->m_apiPicBufY, sizeof (Pel) * ( m_iPicWidth       + (m_iLumaMarginX   << 1)) * ( m_iPicHeight       + (m_iLumaMarginY   << 1)) );
			memcpy ( srcYuv->m_apiPicBufU, resYuv->m_apiPicBufU, sizeof (Pel) * ((m_iPicWidth >> 1) + (m_iChromaMarginX << 1)) * ((m_iPicHeight >> 1) + (m_iChromaMarginY << 1)) );
			memcpy ( srcYuv->m_apiPicBufV, resYuv->m_apiPicBufV, sizeof (Pel) * ((m_iPicWidth >> 1) + (m_iChromaMarginX << 1)) * ((m_iPicHeight >> 1) + (m_iChromaMarginY << 1)) );
			
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
				
				int x,y;
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
						memcpy( pi + (y+1)*iStride, pi, sizeof(Pel)*(iWidth + (iMarginX<<1)) );
					}
	  
					pi -= ((iHeight-1) * iStride);
					for ( y = 0; y < iMarginY; y++ )
					{
						memcpy( pi - (y+1)*iStride, pi, sizeof(Pel)*(iWidth + (iMarginX<<1)) );
					}
					
				}
			}
			
			srcYuv->m_bIsBorderExtended = true;
			
			//unsigned long ul_srcYuv = (unsigned long)srcYuv;
			//unsigned long ul_m_statData = (unsigned long)m_pcSAO->m_statData;
			for(i = 0; i < m_numCTUsPic; i++)
			{
				Out[i].ul_pPic = In[0].ul_pPic;
				Out[i].ul_m_pcEncTop = In[0].ul_m_pcEncTop;
				Out[i].ul_srcYuv = (unsigned long)srcYuv;
				Out[i].ul_m_statData = (unsigned long)m_pcSAO->m_statData;
			}
		}
		window
		{
			In sliding(1,1);
			Out tumbling(m_numCTUsPic); //
		}	
	};
}

composite Main()
{
	stream<unsigned long ul_pPic,unsigned long ul_m_pcEncTop>pSrc;
    //stream<unsigned long ul_pPic,unsigned long ul_m_pcEncTop>pDst; 
	stream<unsigned long ul_m_statData,unsigned long ul_srcYuv,unsigned long ul_pPic,unsigned long ul_m_pcEncTop>pDst; 
	
	pSrc = Source()
	{
		work
		{
			pSrc[0].ul_pPic = (unsigned long)pPic;
			pSrc[0].ul_m_pcEncTop = (unsigned long)m_pcEncTop;			
		}
		window
		{
			pSrc tumbling(1);
		}
	};
	
	pDst = BordedExtension(pSrc)();
	
	Sink(pDst)
	{
		work
		{
				/*
				println(pDst[0].ul_pPic);
				println(pDst[0].ul_m_pcEncTop);
				*/
				println(pDst[0].ul_m_statData);
				println(pDst[0].ul_srcYuv);
				println(pDst[0].ul_pPic);
				println(pDst[0].ul_m_pcEncTop);
		}
	};
	
	
}
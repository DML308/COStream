//自定义函数
#include <CommonDef.h>   //Clip3函数的声明文件
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "TEncSampleAdaptiveOffset.h"
#include <string.h>

#include "TComLoopFilter.h"
#include "TComSlice.h"
#include "TComMv.h"

TComDataCU* getPULeft_cos(TComDataCU* pcCU, UInt& uiLPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction, Bool bEnforceTileRestriction )
{
	return pcCU->getPULeft(uiLPartUnitIdx, uiCurrPartUnitIdx, bEnforceSliceRestriction, bEnforceTileRestriction);
}
	
TComDataCU* getPUAbove_cos(TComDataCU* pcCU, UInt& uiAPartUnitIdx,UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction, Bool planarAtLCUBoundary , Bool bEnforceTileRestriction )
{
	return pcCU->getPUAbove(uiAPartUnitIdx, uiCurrPartUnitIdx, bEnforceSliceRestriction, planarAtLCUBoundary, bEnforceTileRestriction);
}

UChar xGetBoundaryStrengthSingle_cos(TComData* pcCU,Int iDir,UInt uiAbsPartIdx,UChar pucBS)
{
	TComSlice* const pcSlice = pcCU->getSlice();
  
	const UInt uiPartQ = uiAbsPartIdx;   //!< 当前CU的地址  
	TComDataCU* const pcCUQ = pcCU;   //!< 当前CU的指针  
  
	UInt uiPartP;
	TComDataCU* pcCUP;
	UInt uiBs = 0;
  
	//-- Calculate Block Index
	if (iDir == EDGE_VER)   //!< 垂直边界滤波，获取左邻CU  
	{
		pcCUP = pcCUQ->getPULeft(uiPartP, uiPartQ, !pcCU->getSlice()->getLFCrossSliceBoundaryFlag(), !m_bLFCrossTileBoundary);
	}
	else  // (iDir == EDGE_HOR)  // (iDir == EDGE_HOR) //!< 水平边界滤波，获取上邻CU  
	{
		pcCUP = pcCUQ->getPUAbove(uiPartP, uiPartQ, !pcCU->getSlice()->getLFCrossSliceBoundaryFlag(), false, !m_bLFCrossTileBoundary);
	}
  
	//-- Set BS for Intra MB : BS = 4 or 3
	if ( pcCUP->isIntra(uiPartP) || pcCUQ->isIntra(uiPartQ) )   //!< 至少有一个CU是帧内预测模式  
	{
		uiBs = 2;
	}
  
	//! 讨论CU不是帧内预测模式的情况  
	//-- Set BS for not Intra MB : BS = 2 or 1 or 0
	if ( !pcCUP->isIntra(uiPartP) && !pcCUQ->isIntra(uiPartQ) )
	{
		UInt nsPartQ = uiPartQ;
		UInt nsPartP = uiPartP;
    
		//!< 至少有一个邻块具有非零残差系数且该边界为TU边界  
		//if ( m_aapucBS[iDir][uiAbsPartIdx] && (pcCUQ->getCbf( nsPartQ, TEXT_LUMA, pcCUQ->getTransformIdx(nsPartQ)) != 0 || pcCUP->getCbf( nsPartP, TEXT_LUMA, pcCUP->getTransformIdx(nsPartP) ) != 0) )
		if( pucBS && (pcCUQ->getCbf( nsPartQ, TEXT_LUMA, pcCUQ->getTransformIdx(nsPartQ)) != 0 || pcCUP->getCbf( nsPartP, TEXT_LUMA, pcCUP->getTransformIdx(nsPartP) ) != 0) )
		{
			uiBs = 1;
		}
		else
		{
			if (iDir == EDGE_HOR)
			{
				pcCUP = pcCUQ->getPUAbove(uiPartP, uiPartQ, !pcCU->getSlice()->getLFCrossSliceBoundaryFlag(), false, !m_bLFCrossTileBoundary);
			}

			//! 接下来的是P slice 和 B slice的情况，咋一看，代码显得比较啰嗦，特别是B slice的情况。
			//但是，不要被这个表象给迷惑了，其实这部分代码对应于    draft 8.7.2.3中关于变量bS[xDi][yDj]的推导过程的描述。
			//先把那部分的内容看了，再回过头来看下面代码，会觉得一点都不复杂，甚至是理所当然的。  
			if (pcSlice->isInterB() || pcCUP->getSlice()->isInterB())   //!< 当前片为B slice
			{
			  //! 根据P块和Q块所参考的参考图像是否相同(此时要讨论list0和list1的情况)、它们的运动矢量（水平、垂直两个分量）差值的绝对值是否超过4（即以1/4像素为单位）来设定BS  
				Int iRefIdx;
				TComPic *piRefP0, *piRefP1, *piRefQ0, *piRefQ1;
				iRefIdx = pcCUP->getCUMvField(REF_PIC_LIST_0)->getRefIdx(uiPartP);
				piRefP0 = (iRefIdx < 0) ? NULL : pcCUP->getSlice()->getRefPic(REF_PIC_LIST_0, iRefIdx);
				iRefIdx = pcCUP->getCUMvField(REF_PIC_LIST_1)->getRefIdx(uiPartP);
				piRefP1 = (iRefIdx < 0) ? NULL : pcCUP->getSlice()->getRefPic(REF_PIC_LIST_1, iRefIdx);
				iRefIdx = pcCUQ->getCUMvField(REF_PIC_LIST_0)->getRefIdx(uiPartQ);
				piRefQ0 = (iRefIdx < 0) ? NULL : pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx);
				iRefIdx = pcCUQ->getCUMvField(REF_PIC_LIST_1)->getRefIdx(uiPartQ);
				piRefQ1 = (iRefIdx < 0) ? NULL : pcSlice->getRefPic(REF_PIC_LIST_1, iRefIdx);
				
				TComMv pcMvP0 = pcCUP->getCUMvField(REF_PIC_LIST_0)->getMv(uiPartP);
				TComMv pcMvP1 = pcCUP->getCUMvField(REF_PIC_LIST_1)->getMv(uiPartP);
				TComMv pcMvQ0 = pcCUQ->getCUMvField(REF_PIC_LIST_0)->getMv(uiPartQ);
				TComMv pcMvQ1 = pcCUQ->getCUMvField(REF_PIC_LIST_1)->getMv(uiPartQ);

				if (piRefP0 == NULL) pcMvP0.setZero();
				if (piRefP1 == NULL) pcMvP1.setZero();
				if (piRefQ0 == NULL) pcMvQ0.setZero();
				if (piRefQ1 == NULL) pcMvQ1.setZero();

				if ( ((piRefP0==piRefQ0)&&(piRefP1==piRefQ1)) || ((piRefP0==piRefQ1)&&(piRefP1==piRefQ0)) )
				{
				  if ( piRefP0 != piRefP1 )   // Different L0 & L1
				  {
					if ( piRefP0 == piRefQ0 )
					{
					  uiBs  = ((abs(pcMvQ0.getHor() - pcMvP0.getHor()) >= 4) ||
							   (abs(pcMvQ0.getVer() - pcMvP0.getVer()) >= 4) ||
							   (abs(pcMvQ1.getHor() - pcMvP1.getHor()) >= 4) ||
							   (abs(pcMvQ1.getVer() - pcMvP1.getVer()) >= 4)) ? 1 : 0;
					}
					else
					{
					  uiBs  = ((abs(pcMvQ1.getHor() - pcMvP0.getHor()) >= 4) ||
							   (abs(pcMvQ1.getVer() - pcMvP0.getVer()) >= 4) ||
							   (abs(pcMvQ0.getHor() - pcMvP1.getHor()) >= 4) ||
							   (abs(pcMvQ0.getVer() - pcMvP1.getVer()) >= 4)) ? 1 : 0;
					}
				  }
				  else    // Same L0 & L1
				  {
					uiBs  = ((abs(pcMvQ0.getHor() - pcMvP0.getHor()) >= 4) ||
							 (abs(pcMvQ0.getVer() - pcMvP0.getVer()) >= 4) ||
							 (abs(pcMvQ1.getHor() - pcMvP1.getHor()) >= 4) ||
							 (abs(pcMvQ1.getVer() - pcMvP1.getVer()) >= 4)) &&
							((abs(pcMvQ1.getHor() - pcMvP0.getHor()) >= 4) ||
							 (abs(pcMvQ1.getVer() - pcMvP0.getVer()) >= 4) ||
							 (abs(pcMvQ0.getHor() - pcMvP1.getHor()) >= 4) ||
							 (abs(pcMvQ0.getVer() - pcMvP1.getVer()) >= 4)) ? 1 : 0;
				  }
				}
				else // for all different Ref_Idx
				{
				  uiBs = 1;
				}
			}
			else  // pcSlice->isInterP()   //!< 当前片为P slice 
			{
				//! 根据P块和Q块所参考的参考图像是否相同、它们的运动矢量（水平、垂直两个分量）差值的绝对值是否超过4（即以1/4像素为单位）来设定BS  
				Int iRefIdx;
				TComPic *piRefP0, *piRefQ0;
				iRefIdx = pcCUP->getCUMvField(REF_PIC_LIST_0)->getRefIdx(uiPartP);
				piRefP0 = (iRefIdx < 0) ? NULL : pcCUP->getSlice()->getRefPic(REF_PIC_LIST_0, iRefIdx);
				iRefIdx = pcCUQ->getCUMvField(REF_PIC_LIST_0)->getRefIdx(uiPartQ);
				piRefQ0 = (iRefIdx < 0) ? NULL : pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx);
				TComMv pcMvP0 = pcCUP->getCUMvField(REF_PIC_LIST_0)->getMv(uiPartP);
				TComMv pcMvQ0 = pcCUQ->getCUMvField(REF_PIC_LIST_0)->getMv(uiPartQ);

				if (piRefP0 == NULL) pcMvP0.setZero();
				if (piRefQ0 == NULL) pcMvQ0.setZero();
				
				uiBs  = ((piRefP0 != piRefQ0) ||
						 (abs(pcMvQ0.getHor() - pcMvP0.getHor()) >= 4) ||
						 (abs(pcMvQ0.getVer() - pcMvP0.getVer()) >= 4)) ? 1 : 0;
			}
		}   // enf of "if( one of BCBP == 0 )"
	}   // enf of "if( not Intra )"
  
	//m_aapucBS[iDir][uiAbsPartIdx] = uiBs;
	return uiBs;
}


void  TComLoopFilter::xEdgeFilterLuma_VER(TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, Int iEdge,UChar* m_aapucBS)
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();  //!< 重建图像（滤波前）
  Pel* piSrc    = pcPicYuvRec->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );   //!< 指向当前PU对应的重建像素的首地址  
  Pel* piTmpSrc = piSrc;
  
  Int  iStride = pcPicYuvRec->getStride();   //!< 图像的跨度  
  Int iQP = 0;
  Int iQP_P = 0;
  Int iQP_Q = 0;
  
  
  UInt  uiPelsInPart = 4;
  UInt  uiBsAbsIdx = 0, uiBs = 0;
  Int   iOffset = 1, iSrcStep = iStride;
 
  
  Bool  bPCMFilter =  false;
  Bool  bPartPNoFilter = false;
  Bool  bPartQNoFilter = false; 
  UInt  uiPartPIdx = 0;
  UInt  uiPartQIdx = 0;
  TComDataCU* pcCUP = pcCU; 
  TComDataCU* pcCUQ = pcCU;
  Int  betaOffsetDiv2 = 0;
  Int  tcOffsetDiv2 = 0;
  UInt iIdx = 0;
  
  piTmpSrc += iEdge*uiPelsInPart;  //!< 每个8x8滤波单元是不重叠的 
  
  for (iIdx = 0; iIdx < 2; iIdx++ )   //!< 遍历PU中的每个partition  
  {
    //uiBsAbsIdx = xCalcBsIdx( pcCU, uiAbsZorderIdx, iDir, iEdge, iIdx);  //!< partition对应的ZScan地址  
    //uiBs = m_aapucBS[iDir][uiBsAbsIdx];
	uiBsAbsIdx = uiAbsZorderIdx + iIdx<<1;  //获得垂直边界的ZScan地址
	uiBs = m_aapucBS[iIdx<<1];  //对应垂直边界，分别获取 m_aapucBS[0],m_aapucBS[2]
	
    if ( uiBs )  //!< uiBs == 1 or uiBs == 2  
    {
      iQP_Q = pcCU->getQP(uiBsAbsIdx);
      uiPartQIdx = uiBsAbsIdx;
      // Derive neighboring PU index
     
      pcCUP = pcCUQ->getPULeft (uiPartPIdx, uiPartQIdx,false,false);
      
      iQP_P = pcCUP->getQP(uiPartPIdx);
      iQP = (iQP_P + iQP_Q + 1) >> 1;  //!< draft (8-264)  
      Int iBitdepthScale = 1 << (g_bitDepthY-8);
      
      Int iIndexTC = Clip3(0, MAX_QP+DEFAULT_INTRA_TC_OFFSET, Int(iQP + DEFAULT_INTRA_TC_OFFSET*(uiBs-1) + (tcOffsetDiv2 << 1)));
      Int iIndexB = Clip3(0, MAX_QP, iQP + (betaOffsetDiv2 << 1));   //!< draft (8-267) 
      
      Int iTc =  sm_tcTable[iIndexTC]*iBitdepthScale;   //!< draft (8-265) 
      Int iBeta = sm_betaTable[iIndexB]*iBitdepthScale;   //!< draft (8-266) 
      Int iSideThreshold = (iBeta+(iBeta>>1))>>3;  //!< 阈值 
      Int iThrCut = iTc*10;

      UInt  uiBlocksInPart = 1;  //!< 4 / 1 = 1  
      for (UInt iBlkIdx = 0; iBlkIdx<uiBlocksInPart; iBlkIdx ++)
      {
        Int dp0 = xCalcDP( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+0), iOffset);
        Int dq0 = xCalcDQ( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+0), iOffset);  //!< draft (8-269)
        Int dp3 = xCalcDP( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+3), iOffset);  //!< draft (8-270)
        Int dq3 = xCalcDQ( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+3), iOffset);  //!< draft (8-271) 
        Int d0 = dp0 + dq0;   //!< draft (8-272)  
        Int d3 = dp3 + dq3;   //!< draft (8-273)  
        
        Int dp = dp0 + dp3;    //!< draft (8-274)  
        Int dq = dq0 + dq3;    //!< draft (8-275)  
        Int d =  d0 + d3;     //!< draft (8-276)  
        
        if (d < iBeta)
        { 
          Bool bFilterP = (dp < iSideThreshold);  //!< dEp  
          Bool bFilterQ = (dq < iSideThreshold);   //!< dEq  
          //! xUseStrongFiltering对应于draft 8.7.2.4.6  
          Bool sw =  xUseStrongFiltering( iOffset, 2*d0, iBeta, iTc, piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+0))
          && xUseStrongFiltering( iOffset, 2*d3, iBeta, iTc, piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+3));
          
		   //! sw即strong weak，该函数用于判断最终滤波的强弱
          for ( Int i = 0; i < 4; i++)
          {
			  //! 最终进行真正的滤波  
            xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+i), iOffset, iTc, sw, bPartPNoFilter, bPartQNoFilter, iThrCut, bFilterP, bFilterQ);
          }
        }
      }
    }
  }
}


void  TComLoopFilter::xEdgeFilterChroma_VER(TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, Int iEdge,UChar* m_aapucBS)
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Int         iStride     = pcPicYuvRec->getCStride();
  Pel*        piSrcCb     = pcPicYuvRec->getCbAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Pel*        piSrcCr     = pcPicYuvRec->getCrAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Int iQP = 0;
  Int iQP_P = 0;
  Int iQP_Q = 0;
  
  UInt  uiPelsInPartChroma = 2;
  
  Int   iOffset = 1, iSrcStep = iStride;
  
  const UInt uiLCUWidthInBaseUnits = pcCU->getPic()->getNumPartInWidth();
  
  Bool  bPCMFilter = false;
  Bool  bPartPNoFilter = false;
  Bool  bPartQNoFilter = false; 
  UInt  uiPartPIdx;
  UInt  uiPartQIdx;
  TComDataCU* pcCUP; 
  TComDataCU* pcCUQ = pcCU;
  Int tcOffsetDiv2 = 0;
  
  // Vertical Position
  UInt uiEdgeNumInLCUVert = g_auiZscanToRaster[uiAbsZorderIdx]%uiLCUWidthInBaseUnits + iEdge;
  UInt uiEdgeNumInLCUHor = g_auiZscanToRaster[uiAbsZorderIdx]/uiLCUWidthInBaseUnits + iEdge;
  
  if ( (uiPelsInPartChroma < DEBLOCK_SMALLEST_BLOCK) && (( (uiEdgeNumInLCUVert%(DEBLOCK_SMALLEST_BLOCK/uiPelsInPartChroma))&&(iDir==0) ) || ( (uiEdgeNumInLCUHor%(DEBLOCK_SMALLEST_BLOCK/uiPelsInPartChroma))&& iDir ) ))
  {
    return;
  }
  
  UInt  uiNumParts = 2;
  
  UInt  uiBsAbsIdx;
  UChar ucBs;
  
  Pel* piTmpSrcCb = piSrcCb + iEdge*uiPelsInPartChroma;
  Pel* piTmpSrcCr = piSrcCr + iEdge*uiPelsInPartChroma;
  
  for ( UInt iIdx = 0; iIdx < uiNumParts; iIdx++ )
  {
    uiBsAbsIdx = uiAbsZorderIdx+(iIdx<<1);
    ucBs = m_aapucBS[iIdx<<1];
    
    if ( ucBs > 1)
    {
      iQP_Q = pcCU->getQP( uiBsAbsIdx );
      uiPartQIdx = uiBsAbsIdx;
      // Derive neighboring PU index
     // if (iDir == EDGE_VER)
      pcCUP = pcCUQ->getPULeft (uiPartPIdx, uiPartQIdx,!pcCU->getSlice()->getLFCrossSliceBoundaryFlag(), !m_bLFCrossTileBoundary);
      
      iQP_P = pcCUP->getQP(uiPartPIdx);
      
      for ( UInt chromaIdx = 0; chromaIdx < 2; chromaIdx++ )
      {
        Int chromaQPOffset  = (chromaIdx == 0) ? pcCU->getSlice()->getPPS()->getChromaCbQpOffset() : pcCU->getSlice()->getPPS()->getChromaCrQpOffset();
        Pel* piTmpSrcChroma = (chromaIdx == 0) ? piTmpSrcCb : piTmpSrcCr;

        iQP = QpUV( ((iQP_P + iQP_Q + 1) >> 1) + chromaQPOffset );
        Int iBitdepthScale = 1 << (g_bitDepthC-8);

        Int iIndexTC = Clip3(0, MAX_QP+DEFAULT_INTRA_TC_OFFSET, iQP + DEFAULT_INTRA_TC_OFFSET*(ucBs - 1) + (tcOffsetDiv2 << 1));
        Int iTc =  sm_tcTable[iIndexTC]*iBitdepthScale;

        for ( UInt uiStep = 0; uiStep < uiPelsInPartChroma; uiStep++ )
        {
          xPelFilterChroma( piTmpSrcChroma + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma), iOffset, iTc , bPartPNoFilter, bPartQNoFilter);
        }
      }
    }
  }	
}

Void TComLoopFilter::xEdgeFilterLuma_HOR( TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, Int iEdge, UChar* m_aapucBS  )
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();  //!< 重建图像（滤波前）
  Pel* piSrc    = pcPicYuvRec->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );   //!< 指向当前PU对应的重建像素的首地址  
  Pel* piTmpSrc = piSrc;
  
  Int  iStride = pcPicYuvRec->getStride();   //!< 图像的跨度  
  Int iQP = 0;
  Int iQP_P = 0;
  Int iQP_Q = 0;
  UInt uiNumParts = 2;   //!< 当前PU的以partition为单位的宽度 
  
  UInt  uiPelsInPart = 4;
  UInt  uiBsAbsIdx = 0, uiBs = 0;
  Int   iOffset = iStride, iSrcStep  = 1;
  
  Bool  bPCMFilter =  false;
  Bool  bPartPNoFilter = false;
  Bool  bPartQNoFilter = false; 
  UInt  uiPartPIdx = 0;
  UInt  uiPartQIdx = 0;
  TComDataCU* pcCUP = pcCU; 
  TComDataCU* pcCUQ = pcCU;
  Int  betaOffsetDiv2 = 0;
  Int  tcOffsetDiv2 = 0;

  
  // (iDir == EDGE_HOR)
  piTmpSrc += iEdge*uiPelsInPart*iStride;
  
  for ( UInt iIdx = 0; iIdx < uiNumParts; iIdx++ )   //!< 遍历PU中的每个partition  
  {
    //uiBsAbsIdx = xCalcBsIdx( pcCU, uiAbsZorderIdx, iDir, iEdge, iIdx);  //!< partition对应的ZScan地址  
    //uiBs = m_aapucBS[iDir][uiBsAbsIdx]; 
	uiBsAbsIdx = uiAbsZorderIdx + iIdx; //获得水平边界的ZScan地址
	uiBs = m_aapucBS[iIdx]; //对应水平边界，分别获取 m_aapucBS[0],m_aapucBS[1]
	
    if ( uiBs )  //!< uiBs == 1 or uiBs == 2  
    {
      iQP_Q = pcCU->getQP( uiBsAbsIdx );
      uiPartQIdx = uiBsAbsIdx;
      // Derive neighboring PU index
      
      // (iDir == EDGE_HOR)
      pcCUP = pcCUQ->getPUAbove(uiPartPIdx, uiPartQIdx,!pcCU->getSlice()->getLFCrossSliceBoundaryFlag(), false, !m_bLFCrossTileBoundary);

      iQP_P = pcCUP->getQP(uiPartPIdx);
      iQP = (iQP_P + iQP_Q + 1) >> 1;  //!< draft (8-264)  
      Int iBitdepthScale = 1 << (g_bitDepthY-8);
      
      Int iIndexTC = Clip3(0, MAX_QP+DEFAULT_INTRA_TC_OFFSET, Int(iQP + DEFAULT_INTRA_TC_OFFSET*(uiBs-1) + (tcOffsetDiv2 << 1)));
      Int iIndexB = Clip3(0, MAX_QP, iQP + (betaOffsetDiv2 << 1));   //!< draft (8-267) 
      
      Int iTc =  sm_tcTable[iIndexTC]*iBitdepthScale;   //!< draft (8-265) 
      Int iBeta = sm_betaTable[iIndexB]*iBitdepthScale;   //!< draft (8-266) 
      Int iSideThreshold = (iBeta+(iBeta>>1))>>3;  //!< 阈值 
      Int iThrCut = iTc*10;

      UInt  uiBlocksInPart = uiPelsInPart / 4 ? uiPelsInPart / 4 : 1;  //!< 4 / 1 = 1  
      for (UInt iBlkIdx = 0; iBlkIdx<uiBlocksInPart; iBlkIdx ++)
      {
        Int dp0 = xCalcDP( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+0), iOffset);
        Int dq0 = xCalcDQ( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+0), iOffset);  //!< draft (8-269)
        Int dp3 = xCalcDP( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+3), iOffset);  //!< draft (8-270)
        Int dq3 = xCalcDQ( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+3), iOffset);  //!< draft (8-271) 
        Int d0 = dp0 + dq0;   //!< draft (8-272)  
        Int d3 = dp3 + dq3;   //!< draft (8-273)  
        
        Int dp = dp0 + dp3;    //!< draft (8-274)  
        Int dq = dq0 + dq3;    //!< draft (8-275)  
        Int d =  d0 + d3;     //!< draft (8-276)  
        
        if (d < iBeta)
        { 
          Bool bFilterP = (dp < iSideThreshold);  //!< dEp  
          Bool bFilterQ = (dq < iSideThreshold);   //!< dEq  
          //! xUseStrongFiltering对应于draft 8.7.2.4.6  
          Bool sw =  xUseStrongFiltering( iOffset, 2*d0, iBeta, iTc, piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+0))
          && xUseStrongFiltering( iOffset, 2*d3, iBeta, iTc, piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+3));
          
		   //! sw即strong weak，该函数用于判断最终滤波的强弱
          for ( Int i = 0; i < DEBLOCK_SMALLEST_BLOCK/2; i++)
          {
			  //! 最终进行真正的滤波  
            xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+i), iOffset, iTc, sw, bPartPNoFilter, bPartQNoFilter, iThrCut, bFilterP, bFilterQ);
          }
        }
      }
    }
  }
}

Void TComLoopFilter::xEdgeFilterChroma_HOR( TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, Int iEdge, UChar* m_aapucBS  )
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Int         iStride     = pcPicYuvRec->getCStride();
  Pel*        piSrcCb     = pcPicYuvRec->getCbAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Pel*        piSrcCr     = pcPicYuvRec->getCrAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Int iQP = 0;
  Int iQP_P = 0;
  Int iQP_Q = 0;
  
  UInt  uiPelsInPartChroma = 2;
  
  Int   iOffset = iStride, iSrcStep = 1;
  
  const UInt uiLCUWidthInBaseUnits = pcCU->getPic()->getNumPartInWidth();
  
  Bool  bPCMFilter = (pcCU->getSlice()->getSPS()->getUsePCM() && pcCU->getSlice()->getSPS()->getPCMFilterDisableFlag())? true : false;
  Bool  bPartPNoFilter = false;
  Bool  bPartQNoFilter = false; 
  UInt  uiPartPIdx;
  UInt  uiPartQIdx;
  TComDataCU* pcCUP; 
  TComDataCU* pcCUQ = pcCU;
  Int tcOffsetDiv2 = pcCU->getSlice()->getDeblockingFilterTcOffsetDiv2();
  
  // Vertical Position
  UInt uiEdgeNumInLCUVert = g_auiZscanToRaster[uiAbsZorderIdx]%uiLCUWidthInBaseUnits + iEdge;
  UInt uiEdgeNumInLCUHor = g_auiZscanToRaster[uiAbsZorderIdx]/uiLCUWidthInBaseUnits + iEdge;
  
  if ( (uiPelsInPartChroma < DEBLOCK_SMALLEST_BLOCK) && (( (uiEdgeNumInLCUVert%(DEBLOCK_SMALLEST_BLOCK/uiPelsInPartChroma))&&(iDir==0) ) || ( (uiEdgeNumInLCUHor%(DEBLOCK_SMALLEST_BLOCK/uiPelsInPartChroma))&& iDir ) ))
  {
    return;
  }
  
  UInt  uiNumParts = 2;
  
  UInt  uiBsAbsIdx;
  UChar ucBs;
  
  Pel* piTmpSrcCb = piSrcCb + iEdge*iStride*uiPelsInPartChroma;
  Pel* piTmpSrcCr = piSrcCr + iEdge*iStride*uiPelsInPartChroma;
  
  for ( UInt iIdx = 0; iIdx < uiNumParts; iIdx++ )
  {
    //uiBsAbsIdx = xCalcBsIdx( pcCU, uiAbsZorderIdx, iDir, iEdge, iIdx);
    //ucBs = m_aapucBS[iDir][uiBsAbsIdx];
    uiBsAbsIdx = uiAbsZorderIdx + iIdx; //获得水平边界的ZScan地址
	uiBs = m_aapucBS[iIdx]; //对应水平边界，分别获取 m_aapucBS[0],m_aapucBS[1]
	
    if ( ucBs > 1)
    {
      iQP_Q = pcCU->getQP( uiBsAbsIdx );
      uiPartQIdx = uiBsAbsIdx;
      // Derive neighboring PU index
     
       // (iDir == EDGE_HOR)
      
      pcCUP = pcCUQ->getPUAbove(uiPartPIdx, uiPartQIdx,!pcCU->getSlice()->getLFCrossSliceBoundaryFlag(), false, !m_bLFCrossTileBoundary);
      

      iQP_P = pcCUP->getQP(uiPartPIdx);
      
      for ( UInt chromaIdx = 0; chromaIdx < 2; chromaIdx++ )
      {
        Int chromaQPOffset  = (chromaIdx == 0) ? pcCU->getSlice()->getPPS()->getChromaCbQpOffset() : pcCU->getSlice()->getPPS()->getChromaCrQpOffset();
        Pel* piTmpSrcChroma = (chromaIdx == 0) ? piTmpSrcCb : piTmpSrcCr;

        iQP = QpUV( ((iQP_P + iQP_Q + 1) >> 1) + chromaQPOffset );
        Int iBitdepthScale = 1 << (g_bitDepthC-8);

        Int iIndexTC = Clip3(0, MAX_QP+DEFAULT_INTRA_TC_OFFSET, iQP + DEFAULT_INTRA_TC_OFFSET*(ucBs - 1) + (tcOffsetDiv2 << 1));
        Int iTc =  sm_tcTable[iIndexTC]*iBitdepthScale;

        for ( UInt uiStep = 0; uiStep < uiPelsInPartChroma; uiStep++ )
        {
          xPelFilterChroma( piTmpSrcChroma + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma), iOffset, iTc , bPartPNoFilter, bPartQNoFilter);
        }
      }
    }
  }
}

void xEdgeFilterLuma_VER_cos(TComLoopFilter* pcLoopFilter,TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, Int iEdge,UChar* m_aapucBS)
{
	pcLoopFilter->xEdgeFilterLuma_VER(pcCU, uiAbsZorderIdx, iDir, iEdge, m_aapucBS)
}

void xEdgeFilterChroma_VER_cos(TComLoopFilter* pcLoopFilter,TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, Int iEdge,UChar* m_aapucBS)
{
	pcLoopFilter->xEdgeFilterChroma_VER(pcCU, uiAbsZorderIdx, iDir, iEdge, m_aapucBS)
}

void xEdgeFilterLuma_HOR_cos(TComLoopFilter* pcLoopFilter,TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, Int iEdge,UChar* m_aapucBS)
{
	pcLoopFilter->xEdgeFilterLuma_HOR(pcCU, uiAbsZorderIdx, iDir, iEdge, m_aapucBS)
}

void xEdgeFilterChroma_HOR_cos(TComLoopFilter* pcLoopFilter,TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, Int iEdge,UChar* m_aapucBS)
{
	pcLoopFilter->xEdgeFilterChroma_HOR(pcCU, uiAbsZorderIdx, iDir, iEdge, m_aapucBS)
}

GLOBAL VerEdgefilter_cos(int uiX,int uiY,TComDataCU* pcCU, Int uiCUAddr,UInt uiDepth,UInt uiAbsZorderIdx,UInt rasterOrder,UInt TransformIdx)
{
	UChar m_aapucBS_VER[4] = {0,0,0,0};  //垂直边界的滤波强度，每次只处理一个8x8块，其中含有4个子块
	UChar m_aapucBS_HOR[4] = {0,0,0,0};  //垂直边界的滤波强度，每次只处理一个8x8块，其中含有4个子块
	bool  m_aapbEdgeFilter_VER[4] = {0,0,0,0};
	bool  m_aapbEdgeFilter_HOR[4] = {0,0,0,0};
	
	bool bInternalEdge = true;
	bool bLeftEdge = true;
	bool bTopEdge = true;
	
	bool value = bInternalEdge;
	
	UInt uiCurNumParts = 4;  //!< 当前CU中的4x4 partition 数目
	
	Int iDir = 1; // 1代表水平边界滤波 EDGE_HOR
	Int iEdge = 0;
	
	TComDataCU* pcTempCU;
	UInt        uiTempPartIdx;
	
	//设置滤波参数  xSetLoopfilterParam
	if(uiX == 0)
	{
		bLeftEdge = false;
	}
	else
	{
		pcTempCU = pcCU->getPULeft( uiTempPartIdx, uiAbsZorderIdx, false, false);
		//pcTempCU = (TComDataCU*)getPULeft_cos(pcCU, uiTempPartIdx, uiAbsZorderIdx, false, false);
		if(pcTempCU)
		{
			bLeftEdge = true;
		}
		else
		{
			bLeftEdge = false;
		}
	}
	if ( uiY == 0 )
	{
		bTopEdge = false;
	}
	else
	{
		bTopEdge = true;
		pcTempCU = pcCU->getPUAbove( uiTempPartIdx, uiAbsZorderIdx, false,false,false);
		//pcTempCU = (TComDataCU*)getPUAbove_cos(pcCU,uiTempPartIdx, uiAbsZorderIdx,false,false,false);
		
		if ( pcTempCU )    //!< 只有在上邻PU存在的情况下才进行水平边界的滤波 
		{
			bTopEdge = true;
		}
		else
		{
			bTopEdge = false;
		}
	}
	//设置TU滤波边界参数:xSetEdgefilterTU( pcCU, uiAbsZorderIdx , uiAbsZorderIdx, uiDepth );
	{
		if( (uiDepth == 3 && (TransformIdx == 0)) || (uiDepth == 2 && (TransformIdx == 1) ) || (uiDepth == 1 && (TransformIdx == 2) ) || (uiDepth == 0 && (TransformIdx == 3) ) )
		{
			m_aapbEdgeFilter_VER[0] = bValue; m_aapbEdgeFilter_VER[2] = bValue;  //垂直边界
			m_aapucBS_VER[0] = bValue; m_aapucBS_VER[2] = bValue;
			
			m_aapbEdgeFilter_HOR[0] = bValue;m_aapbEdgeFilter_HOR[1] = bValue;  //水平边界
			m_aapucBS_HOR[0] = bValue;m_aapucBS_HOR[1] = bValue;
		}
		
		// //单独的4个4x4块均要设置TU 滤波
		else if( (uiDepth == 3 && (TransformIdx == 1) ) || (uiDepth == 2 && (TransformIdx == 2) ) || (uiDepth == 1 && (TransformIdx == 3) ) || ( uiDepth == 0 && (TransformIdx == 4) ) )
		{
			int i;
			for(i = 0; i < 4; i++)
			{
				m_aapbEdgeFilter_VER[i] = bValue;  //垂直边界
				m_aapucBS_VER[i] = bValue;
				m_aapbEdgeFilter_HOR[i] = bValue;  //水平边界
				m_aapucBS_HOR[i] = bValue;
			}
		}
		else if( (uiDepth == 2 && (TransformIdx == 0) ) || ( uiDepth == 1 && (TransformIdx == 0 || TransformIdx == 1) ) || ( uiDepth == 0 && (TransformIdx == 0 || TransformIdx == 1 || TransformIdx == 2) ) )
		{
			int col  = 16>>(uiDepth+TransformIdx);  //4x4块的raster位于col的整数倍列
			if(rasterOrder%16%col==0)  //位于垂直边界
			{
				m_aapbEdgeFilter_VER[0] = bValue; m_aapbEdgeFilter_VER[2] = bValue;  //垂直边界
				m_aapucBS_VER[0] = bValue; m_aapucBS_VER[2] = bValue;
			}
			if(rasterOrder/16%col==0)  //位于水平边界
			{
				m_aapbEdgeFilter_HOR[0] = bValue;m_aapbEdgeFilter_HOR[1] = bValue;  //水平边界
				m_aapucBS_HOR[0] = bValue;m_aapucBS_HOR[1] = bValue;
			}
			
		}
		else  //其他未考虑到的情况
		{
			
		}
	}

	//设置PU
	{
		 bValue = bLeftEdge;
		 //uiNumElem = uiHeightInBaseUnits;
		//xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, 0, bLeftEdge );   //!< 设置垂直边界
		
		if( (uiDepth == 3) || ( (uiDepth == 2) &&(rasterOrder%16%4==0) ) || ( uiDepth == 1 && (rasterOrder % 16 % 8 == 0) ) || ( uiDepth == 0 && (rasterOrder % 16 == 0) ) )
		{
			m_aapbEdgeFilter_VER[0] = bValue; m_aapbEdgeFilter_VER[2] = bValue;  //垂直边界
			m_aapucBS_VER[0] = bValue; m_aapucBS_VER[2] = bValue;
			
		}
		
		bValue = bTopEdge;
		//uiNumElem = uiWidthInBaseUnits;
		//xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, 0, bTopEdge ); //!< 设置水平边界
		
		if( (uiDepth == 3) || (uiDepth == 2 && (rasterOrder/16%4 == 0) ) || (uiDepth == 1 && (rasterOrder/16%8 == 0) ) || (uiDepth = 0 && (rasterOrder/16 == 0) ) )
		{
			m_aapbEdgeFilter_HOR[0] = bValue;m_aapbEdgeFilter_HOR[1] = bValue;  //水平边界
			m_aapucBS_HOR[0] = bValue;m_aapucBS_HOR[1] = bValue;
		}
		
		bValue = bInternalEdge;
		
		switch ( pcCU->m_pePartSize[uiAbsZorderIdx] )  //!< PU的划分模式  
		{
			//!< 以下根据PU的划分模式对滤波边界进行相应的设置
			case 0:
			{
			  break;
			}
			case 1: //SIZE_2NxN
			{
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, EDGE_HOR, uiHHeightInBaseUnits, bInternalEdge );
			  {
					if(uiDepth == 3)  //深度为3要特殊处理
					{ 
						m_aapbEdgeFilter_HOR[2] = bValue;m_aapbEdgeFilter_HOR[3] = bValue;  //水平边界
						//if(uiHHeightInBaseUnits == 0) { m_aapucBS_HOR[2] = bValue;m_aapucBS_HOR[3] = bValue;}
					} 
					else
					{
						//if( (uiDepth == 2 && (rasterOrder/16%4 != 0) &&(rasterOrder/16%2 == 0) ) || (uiDepth == 1 && (rasterOrder/16%8 != 0)&&(rasterOrder/16%4 == 0) ) || (uiDepth = 0 && (rasterOrder/16 != 0) && (rasterOrder/16%8 == 0) ) )
						if( (uiDepth == 2 && (rasterOrder/16%4 == 2) ) || (uiDepth == 1 && (rasterOrder/16%8 == 4) ) || (uiDepth = 0 && (rasterOrder/16%16 == 8) ) )
						{
							m_aapbEdgeFilter_HOR[0] = bValue;m_aapbEdgeFilter_HOR[1] = bValue;  //水平边界
						}
					}	
			  }
			  break;
			}
			case 2: //SIZE_Nx2N
			{
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, EDGE_VER, uiHWidthInBaseUnits, bInternalEdge );
			  {
					if(uiDepth == 3)  //深度为3要特殊处理
					{ 
						m_aapbEdgeFilter_VER[1] = bValue; m_aapbEdgeFilter_VER[3] = bValue;  //垂直边界
					}
					else
					{
						//if( ((uiDepth == 2)&&(rasterOrder%16%4!=0)&&(rasterOrder%16%2==0))||( uiDepth == 1 && (rasterOrder%16%8 != 0)  && (rasterOrder%16%4 == 0) ) || ( uiDepth == 0 && (rasterOrder %16 != 0) && (rasterOrder%16 %8 == 0) ) )
						if( ((uiDepth == 2)&&(rasterOrder%16%4==2))||( (uiDepth == 1) && (rasterOrder%16%8 == 4)) || ( (uiDepth == 0) && (rasterOrder %16 == 8) ) )
						{
							m_aapbEdgeFilter_VER[0] = bValue; m_aapbEdgeFilter_VER[2] = bValue;  //垂直边界
						}
					}
						
					//if(uiHWidthInBaseUnits == 0) { m_aapucBS_VER[1] = bValue; m_aapucBS_VER[3] = bValue;}
			  }
			  break;
			}
			case 3:	// SIZE_NxN, SIZE_NxN 只有8x8块也即uiDepth = 3 才有
			{
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, 0, uiHWidthInBaseUnits, bInternalEdge );
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, 1, uiHHeightInBaseUnits, bInternalEdge );
			  {
				  if(uiDepth == 3)
					{
						m_aapbEdgeFilter_VER[1] = bValue; m_aapbEdgeFilter_VER[3] = bValue;  //垂直边界
						m_aapbEdgeFilter_HOR[2] = bValue;m_aapbEdgeFilter_HOR[3] = bValue;  //水平边界
					}
			  }
			  break;
			}
			//对于8x8块是不会有下面的PU划分的，讨论其他的块即可
			case 4: 
			{
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, 1, uiQHeightInBaseUnits, bInternalEdge );
			  {
					if((uiDepth == 2) && (rasterOrder/16%4 == 0) )  //CU为16x16,高度和宽度上存在4个4x4块,由于是按照1:3划分,而当前处理的8x8块的起始4x4块位于16x16块的第4*(0/4)行时，当前8x8块的第1行也就是8x8块的第2,3号的水平滤波边界需要设置
					{
						m_aapbEdgeFilter_HOR[2] = bValue;m_aapbEdgeFilter_HOR[3] = bValue;  //水平边界
					}
					else
					{
						if( ( (uiDepth == 1) && (rasterOrder/16%8 == 2) ) || ( (uiDepth = 0)&& (rasterOrder/16%16 == 4) ) )
						{
							m_aapbEdgeFilter_HOR[0] = bValue;m_aapbEdgeFilter_HOR[1] = bValue;  //水平边界
						}
					}							
			  }
			  break;
			}
			case 5:
			{
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, 1, uiHeightInBaseUnits - uiQHeightInBaseUnits, bInternalEdge );
			  {
					if(uiDepth == 2 && (rasterOrder/16%4 == 2) )  //CU为16x16,高度和宽度上存在4个4x4块,由于是按照3:1划分,而当前处理的8x8块的起始4x4块位于16x16块的第4*(2/4)行时，下面一行的4x4块，即当前8x8块的第1行，也就是8x8块的第2,3号的水平滤波边界需要设置
					{
						m_aapbEdgeFilter_HOR[2] = bValue;m_aapbEdgeFilter_HOR[3] = bValue;  //水平边界
					}
					else
					{
						if( ( (uiDepth == 1) && (rasterOrder/16%8 == 6) ) || ( (uiDepth = 0) && (rasterOrder/16%16 == 12) ) )
						{
							m_aapbEdgeFilter_HOR[0] = bValue;m_aapbEdgeFilter_HOR[1] = bValue;  //水平边界
						}
					}
			  }
			 break;
			}
			case 6:
			{
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, 0, uiQWidthInBaseUnits, bInternalEdge );
			  {
					if((uiDepth == 2)&& (rasterOrder%16%4==0))  ////CU为16x16,高度和宽度上存在4个4x4块,由于是按照1:3划分,而当前处理的8x8块的起始4x4块位于16x16块的第4*(1/4)列时，当前8x8块的第二列也就是8x8块的第1,3号的垂直滤波边界需要设置
					{ 
						m_aapbEdgeFilter_VER[1] = bValue; m_aapbEdgeFilter_VER[3] = bValue;  //垂直边界
					}
					else
					{
						if( ( (uiDepth == 1) && (rasterOrder%16%8 == 2) ) || ( (uiDepth == 0) && (rasterOrder %16 == 4) ) )
						{
							m_aapbEdgeFilter_VER[0] = bValue; m_aapbEdgeFilter_VER[2] = bValue;  //垂直边界
						}
					}
			  }
			  break;
			}
			case 7:
			{
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, 0, uiWidthInBaseUnits - uiQWidthInBaseUnits, bInternalEdge );
			  {
					if((uiDepth == 2)&& (rasterOrder%16%4==2))  ////CU为16x16,高度和宽度上存在4个4x4块,由于是按照3:1划分,而当前处理的8x8块的起始4x4块位于16x16块的第4*(2/4)列时，当前8x8块的第二列也就是8x8块的第1,3号的垂直滤波边界需要设置
					{ 
						m_aapbEdgeFilter_VER[1] = bValue; m_aapbEdgeFilter_VER[3] = bValue;  //垂直边界
					}
					else 
					{
						if( ( (uiDepth == 1) && (rasterOrder%16%8 == 6) ) || ( (uiDepth == 0) && (rasterOrder %16 == 12) ) )
						{
							m_aapbEdgeFilter_VER[0] = bValue; m_aapbEdgeFilter_VER[2] = bValue;  //垂直边界
						}
					}
			  }
			  break;
			}
			default:
			{
			  break;
			}
		}
	}
	//设置滤波强度
	{
		int uiPartIdx = 0;
		for(uiPartIdx = 0; uiPartIdx < uiCurNumParts; uiPartIdx++ )
		{
			//! 对于水平边界滤波的情况，检测partition是否属于上边界  
			UInt uiBSCheck = ((uiPartIdx+uiAbsZorderIdx)%2 == 0 );	
			if (uiBSCheck && m_aapbEdgeFilter_VER[uiPartIdx])  //!< m_aapbEdgeFilter的值在xSetEdgefilterMultiple中设置过  
			{
				m_aapucBS_VER[uiPartIdx] = xGetBoundaryStrengthSingle_cos( pcCU, iDir, uiPartIdx+uiAbsZorderIdx,m_aapucBS_VER[uiPartIdx] );  //!< 设置滤波强度
				
			}
		}	
			
	}
	//滤波
	{
		//xEdgeFilterLuma_VER_cos( pcLoopFilter, pcCU, uiAbsZorderIdx, iDir, iEdge, m_aapucBS_VER );   //!< 对亮度分量进行滤波  
		//TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();  //!< 重建图像（滤波前）
		//Pel* piTmpSrc    = pcPicYuvRec->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );   //!< 指向当前PU对应的重建像素的首地址  
		TComPicYuv* pcPicYuvRec = pcCU->m_pcPic->m_apcPicYuv[1];  // 
		Pel* piTmpSrc    = pcPicYuvRec->m_piPicOrgY + pcPicYuvRec->m_cuOffsetY[uiCUAddr] + pcPicYuvRec->m_buOffsetY[g_auiZscanToRaster[uiAbsZorderIdx]];
		
		//Int  iStride = pcPicYuvRec->getStride();   //!< 图像的跨度  
		Int iStride = pcPicYuvRec->m_iPicWidth + (pcPicYuvRec->m_iLumaMarginX)<<1;
		Int iQP = 0;
		Int iQP_P = 0;
		Int iQP_Q = 0;
	  
		UInt  uiPelsInPart = 4;
		UInt  uiBsAbsIdx = 0, uiBs = 0;
		Int   iOffset = 1, iSrcStep = iStride;
		
		bool  bPartPNoFilter = false;
		bool  bPartQNoFilter = false; 
		UInt  uiPartPIdx = 0;
		UInt  uiPartQIdx = 0;
		TComDataCU* pcCUP = pcCU; 
		TComDataCU* pcCUQ = pcCU;
		Int  betaOffsetDiv2 = 0;
		Int  tcOffsetDiv2 = 0;
		UInt iIdx = 0;
	  
		Int iBitdepthScale = 1;
		Int iIndexTC = 0;
		Int iIndexB = 0;
		Int iTc = 0;
		Int iBeta = 0;
		Int iSideThreshold = 0;
		Int iThrCut = 0;
		UInt  uiBlocksInPart = 0;
		UInt iBlkIdx = 0;
		
		
		piTmpSrc += iEdge*uiPelsInPart;  //!< 每个8x8滤波单元是不重叠的 
		
		
		for (iIdx = 0; iIdx < 2; iIdx++ )   //!< 遍历PU中的每个partition  
		{

			uiBsAbsIdx = uiAbsZorderIdx + iIdx<<1;  //获得垂直边界的ZScan地址
			uiBs = m_aapucBS_VER[iIdx<<1];  //对应垂直边界，分别获取 m_aapucBS_VER[0],m_aapucBS_VER[2]

			if ( uiBs )  //!< uiBs == 1 or uiBs == 2  
			{
				//iQP_Q = pcCU->getQP(uiBsAbsIdx);
				iQP_Q = pcCU->m_phQP[uiBsAbsIdx];
				
				uiPartQIdx = uiBsAbsIdx;
				
				// Derive neighboring PU index
				//pcCUP = (TComDataCU*)getPULeft_cos(pcCUQ,uiPartPIdx, uiPartQIdx,false,false);
				pcCUP = pcCUQ->getPULeft( uiPartPIdx, uiPartQIdx, false, false);
				
				//iQP_P = pcCUP->getQP(uiPartPIdx);
				iQP_P = pcCUP->m_phQP[uiPartPIdx];
				
				iQP = (iQP_P + iQP_Q + 1) >> 1;  //!< draft (8-264)  
				//iBitdepthScale = 1 << (g_bitDepthY-8);
			  
				iIndexTC = Clip3(0, MAX_QP+DEFAULT_INTRA_TC_OFFSET, (Int)(iQP + DEFAULT_INTRA_TC_OFFSET*(uiBs-1) + (tcOffsetDiv2 << 1)));
				iIndexB = Clip3(0, MAX_QP, iQP + (betaOffsetDiv2 << 1));   //!< draft (8-267) 
			  
				iTc =  sm_tcTable[iIndexTC]*iBitdepthScale;   //!< draft (8-265) 
				iBeta = sm_betaTable[iIndexB]*iBitdepthScale;   //!< draft (8-266) 
				iSideThreshold = (iBeta+(iBeta>>1))>>3;  //!< 阈值 
				iThrCut = iTc*10;

				uiBlocksInPart = 1;  //!< 4 / 1 = 1  
				for (iBlkIdx = 0; iBlkIdx<uiBlocksInPart; iBlkIdx ++)
				{
					Pel* piSrc1 = piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+0);
					Pel* piSrc2 = piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+3);
					
					//Int dp0 = xCalcDP( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+0), iOffset);
					Int dp0 = abs( piSrc1[-iOffset*3] - 2*piSrc1[-iOffset*2] + piSrc1[-iOffset] );
					
					//Int dq0 = xCalcDQ( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+0), iOffset);  //!< draft (8-269)
					Int dq0 = abs( piSrc1[0] - 2*piSrc1[iOffset] + piSrc1[iOffset*2]);
					
					
					//Int dp3 = xCalcDP( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+3), iOffset);  //!< draft (8-270)
					Int dp3 = abs( piSrc2[-iOffset*3] - 2*piSrc2[-iOffset*2] + piSrc2[-iOffset] );
					
					//Int dq3 = xCalcDQ( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+3), iOffset);  //!< draft (8-271) 
					Int dq3 = abs( piSrc2[0] - 2*piSrc2[iOffset] + piSrc2[iOffset*2]);
					
					Int d0 = dp0 + dq0;   //!< draft (8-272)  
					Int d3 = dp3 + dq3;   //!< draft (8-273)  
					
					Int dp = dp0 + dp3;    //!< draft (8-274)  
					Int dq = dq0 + dq3;    //!< draft (8-275)  
					Int d =  d0 + d3;     //!< draft (8-276)  
					
					if (d < iBeta)
					{ 
						Pel m4 = piSrc1[0];
						Pel m3 = piSrc1[-iOffset];  //!< p0 
						Pel m7 = piSrc1[ iOffset*3];  //!< q3 
						Pel m0 = piSrc1[-iOffset*4];   //!< p3 
						Int d_strong = abs(m0-m3)+abs(m7-m4); 
						Int i = 0;
						
						//xUseStrongFiltering( iOffset, 2*d0, iBeta, iTc, piSrc1)
						bool useFlag1 = ( (d_strong < (iBeta>>3)) && ((2*d0)<(iBeta>>2)) && ( abs(m3-m4) < ((iTc*5+1)>>1)) ); 
						
						bool useFlag2 = false;
						bool sw = false;
						
						bool bFilterP = (dp < iSideThreshold);  //!< dEp  
						bool bFilterQ = (dq < iSideThreshold);   //!< dEq 
						
						m4 = piSrc2[0];
						m3 = piSrc2[-iOffset];  //!< p0 
						m7 = piSrc2[ iOffset*3];  //!< q3 
						m0 = piSrc2[-iOffset*4];   //!< p3 
						
						d_strong = abs(m0-m3)+abs(m7-m4);
						
						// xUseStrongFiltering( iOffset, 2*d3, iBeta, iTc, piSrc2);
						useFlag2 = ( (d_strong < (iBeta>>3)) && ((2*d3)<(iBeta>>2)) && ( abs(m3-m4) < ((iTc*5+1)>>1)) ); 
						
						//bool sw =  xUseStrongFiltering( iOffset, 2*d0, iBeta, iTc, piSrc1)&& xUseStrongFiltering( iOffset, 2*d3, iBeta, iTc, piSrc2);
						sw = useFlag1 && useFlag2;
					
						//! sw即strong weak，该函数用于判断最终滤波的强弱
						for (i = 0; i < 4; i++)
						{
						  //! 最终进行真正的滤波  
						//xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+i), iOffset, iTc, sw, bPartPNoFilter, bPartQNoFilter, iThrCut, bFilterP, bFilterQ);
							
						  Int delta;
						  Pel* piSrc = piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+i);
						  //! m0 m1 m2 m3 | m4 m5 m6 m7  
						  Pel m4  = piSrc[0];
						  Pel m3  = piSrc[-iOffset];
						  Pel m5  = piSrc[ iOffset];
						  Pel m2  = piSrc[-iOffset*2];
						  Pel m6  = piSrc[ iOffset*2];
						  Pel m1  = piSrc[-iOffset*3];
						  Pel m7  = piSrc[ iOffset*3];
						  Pel m0  = piSrc[-iOffset*4];

						  if (sw)   //!< 强滤波
						  {   //! draft (8-305) to (8-310)   
							piSrc[-iOffset]   = Clip3(m3-2*iTc, m3+2*iTc, ((m1 + 2*m2 + 2*m3 + 2*m4 + m5 + 4) >> 3));
							piSrc[0]          = Clip3(m4-2*iTc, m4+2*iTc, ((m2 + 2*m3 + 2*m4 + 2*m5 + m6 + 4) >> 3));
							piSrc[-iOffset*2] = Clip3(m2-2*iTc, m2+2*iTc, ((m1 + m2 + m3 + m4 + 2)>>2));
							piSrc[ iOffset]   = Clip3(m5-2*iTc, m5+2*iTc, ((m3 + m4 + m5 + m6 + 2)>>2));
							piSrc[-iOffset*3] = Clip3(m1-2*iTc, m1+2*iTc, ((2*m0 + 3*m1 + m2 + m3 + m4 + 4 )>>3));
							piSrc[ iOffset*2] = Clip3(m6-2*iTc, m6+2*iTc, ((m3 + m4 + m5 + 3*m6 + 2*m7 +4 )>>3));
						  }
						  else   //!< 弱滤波  
						  {
							// Weak filter 
							delta = (9*(m4-m3) -3*(m5-m2) + 8)>>4 ;   //!< draft (8-311)  

							if ( abs(delta) < iThrCut )
							{  //! draft (8-312) to (8-318) 
							  Int tc2 = iTc>>1;
							  delta = Clip3(-iTc, iTc, delta);        
							  piSrc[-iOffset] = ClipY((m3+delta));
							  piSrc[0] = ClipY((m4-delta));

							  
							  if(bFilterP)
							  {
								Int delta1 = Clip3(-tc2, tc2, (( ((m1+m3+1)>>1)- m2+delta)>>1));
								piSrc[-iOffset*2] = ClipY((m2+delta1));
							  }
							  if(bFilterQ)
							  {
								Int delta2 = Clip3(-tc2, tc2, (( ((m6+m4+1)>>1)- m5-delta)>>1));
								piSrc[ iOffset] = ClipY((m5+delta2));
							  }
							}
						  }
						  if(bPartPNoFilter)   //!< Part P不滤波，保留原值  
						  {
							piSrc[-iOffset] = m3;
							piSrc[-iOffset*2] = m2;
							piSrc[-iOffset*3] = m1;
						  }
						  if(bPartQNoFilter)  //!< Part Q不滤波，保留原值  
						  {
							piSrc[0] = m4;
							piSrc[ iOffset] = m5;
							piSrc[ iOffset*2] = m6;
						  }
						}
					}
					
				}
			}
		} 
		//色度分量进行滤波的条件：深度为3，因为此时iEdge只能取到0，满足iEdge%4 ==0 ;
		//或深度不等于3时，对于垂直滤波等价于4x4块对应的raster所在列数为4的倍数，对于水平滤波等价于4x4块对应的raster所在行数为4的倍数
		
		if( (uiDepth == 3) || (rasterOrder%16%4==0) || ( (rasterOrder/16%4)==0 ) )
		{
			//iStride     = pcPicYuvRec->getCStride();  //U,V分量的跨度
			
			//Pel*        piTmpSrcCb     = pcPicYuvRec->getCbAddr( pcCU->getAddr(), uiAbsZorderIdx );
			//Pel*        piTmpSrcCr     = pcPicYuvRec->getCrAddr( pcCU->getAddr(), uiAbsZorderIdx );
			Pel*  piTmpSrcCb  = pcPicYuvRec->m_piPicOrgU + pcPicYuvRec->m_cuOffsetC[uiCUAddr] + pcPicYuvRec->m_buOffsetC[g_auiZscanToRaster[uiAbsZorderIdx]];
			Pel*  piTmpSrcCr  = pcPicYuvRec->m_piPicOrgV + pcPicYuvRec->m_cuOffsetC[uiCUAddr] + pcPicYuvRec->m_buOffsetC[g_auiZscanToRaster[uiAbsZorderIdx]];
			
			UInt  uiPelsInPartChroma = 2;
			
			// Vertical Position
			UInt uiEdgeNumInLCUVert = rasterOrder%16;
			UInt uiEdgeNumInLCUHor =  rasterOrder/16;
		 
			UInt chromaIdx  = 0;
			
			iStride  = (pcPicYuvRec->m_iPicWidth)>>1 + (pcPicYuvRec->m_iChromaMarginX)<<1;
			if (!( (uiEdgeNumInLCUVert%4)&&(iDir==0) ) )
			{
				
				iOffset   = 1;
				iSrcStep  = iStride;
				piTmpSrcCb += iEdge*uiPelsInPartChroma;
				piTmpSrcCr += iEdge*uiPelsInPartChroma;
				
				for (iIdx = 0; iIdx < 2; iIdx++ )
				{
					
					uiBsAbsIdx = uiAbsZorderIdx + iIdx<<1;  //获得垂直边界的ZScan地址
					uiBs = m_aapucBS_VER[iIdx<<1];  //对应垂直边界，分别获取 m_aapucBS_VER[0],m_aapucBS_VER[2]
					
					if ( uiBs > 1)
					{
					  //iQP_Q = pcCU->getQP( uiBsAbsIdx );
					  iQP_Q = pcCU->m_phQP[uiBsAbsIdx];
					 
					 uiPartQIdx = uiBsAbsIdx;
					  // Derive neighboring PU index
					  if (iDir == 0)
					  {
						  //pcCUP = (TComDataCU*)getPULeft_cos (pcCUQ,uiPartPIdx, uiPartQIdx,false, false);
					      pcCUP= pcCUQ->getPULeft(uiPartPIdx, uiPartQIdx, false, false);
					  }
					  
					  //iQP_P = pcCUP->getQP(uiPartPIdx);
					  iQP_P = pcCUP->m_phQP[uiPartPIdx];
					   
					  for ( chromaIdx = 0; chromaIdx < 2; chromaIdx++ )
					  {
							//Int chromaQPOffset  = (chromaIdx == 0) ? pcCU->getSlice()->getPPS()->getChromaCbQpOffset() : pcCU->getSlice()->getPPS()->getChromaCrQpOffset();
							Int chromaQPOffset  = 0;
							Pel* piTmpSrcChroma = (chromaIdx == 0) ? piTmpSrcCb : piTmpSrcCr;
							UInt uiStep = 0;
					
							iQP = QpUV( ((iQP_P + iQP_Q + 1) >> 1) + chromaQPOffset );
							iIndexTC = Clip3(0, MAX_QP+DEFAULT_INTRA_TC_OFFSET, iQP + DEFAULT_INTRA_TC_OFFSET*(uiBs - 1) + (tcOffsetDiv2 << 1));
							iTc =  sm_tcTable[iIndexTC]*iBitdepthScale;
						
							for ( uiStep = 0; uiStep < uiPelsInPartChroma; uiStep++ )
							{
							  //xPelFilterChroma( piTmpSrcChroma + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma), iOffset, iTc , bPartPNoFilter, bPartQNoFilter);
								Int delta;
								Pel* piSrc = piTmpSrcChroma + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma);
								
								Pel m4  = piSrc[0];
								Pel m3  = piSrc[-iOffset];
								Pel m5  = piSrc[ iOffset];
								Pel m2  = piSrc[-iOffset*2];
								  
								delta = Clip3(-iTc,iTc, (((( m4 - m3 ) << 2 ) + m2 - m5 + 4 ) >> 3) );
								piSrc[-iOffset] = ClipC(m3+delta);
								piSrc[0] = ClipC(m4-delta);

								if(bPartPNoFilter)
								{
									piSrc[-iOffset] = m3;
								}
								if(bPartQNoFilter)
								{
									piSrc[0] = m4;
								}
							}
					  }
					}
				}	
			} 	
		}
	}
}

GLOBAL void HorEdgefilter_cos(int uiX,int uiY,TComDataCU* pcCU, Int uiCUAddr,UInt uiDepth,UInt uiAbsZorderIdx,UInt rasterOrder,UInt TransformIdx)
{
	UChar m_aapucBS_VER[4] = {0,0,0,0};  //垂直边界的滤波强度，每次只处理一个8x8块，其中含有4个子块
	UChar m_aapucBS_HOR[4] = {0,0,0,0};  //垂直边界的滤波强度，每次只处理一个8x8块，其中含有4个子块
	bool  m_aapbEdgeFilter_VER[4] = {0,0,0,0};
	bool  m_aapbEdgeFilter_HOR[4] = {0,0,0,0};
	
	bool bInternalEdge = true;
	bool bLeftEdge = true;
	bool bTopEdge = true;
	
	bool value = bInternalEdge;
	
	UInt uiCurNumParts = 4;  //!< 当前CU中的4x4 partition 数目
	
	Int iDir = 1; // 1代表水平边界滤波 EDGE_HOR
	Int iEdge = 0;
	
	TComDataCU* pcTempCU;
	UInt        uiTempPartIdx;
	
	//设置滤波参数  xSetLoopfilterParam
	//!< 标记边界是否进行去方块滤波
	if ( uiX == 0 )
	{
		 bLeftEdge = false;
	}
	else
	{
		 bLeftEdge = true;
		pcTempCU = pcCU->getPULeft( uiTempPartIdx, uiAbsZorderIdx, false, false);
		//pcTempCU = (TComDataCU*)getPULeft_cos(pcCU, uiTempPartIdx, uiAbsZorderIdx, false, false);
		if ( pcTempCU )   //!< 只有在左邻PU存在的情况下才进行垂直边界的滤波  
		{
			 bLeftEdge = true;
		}
		else
		{
			 bLeftEdge = false;
		}
	}
	  
	if ( uiY == 0 )
	{
		 bTopEdge = false;
	}
	else
	{
		 bTopEdge = true;
		pcTempCU = pcCU->getPUAbove( uiTempPartIdx, uiAbsZorderIdx, false, false, false);
		//pcTempCU = (TComDataCU*)getPUAbove_cos(pcCU,uiTempPartIdx, uiAbsZorderIdx,false,false,false);
		if ( pcTempCU )    //!< 只有在上邻PU存在的情况下才进行水平边界的滤波 
		{
			 bTopEdge = true;
		}
		else
		{
			 bTopEdge = false;
		}
	}
	
	//设置TU滤波边界参数:xSetEdgefilterTU( pcCU, uiAbsZorderIdx , uiAbsZorderIdx, uiDepth );
	{
		if( (uiDepth == 3 && (TransformIdx == 0)) || (uiDepth == 2 && (TransformIdx == 1) ) || (uiDepth == 1 && (TransformIdx == 2) ) || (uiDepth == 0 && (TransformIdx == 3) ) )
		{
			m_aapbEdgeFilter_VER[0] = bValue; m_aapbEdgeFilter_VER[2] = bValue;  //垂直边界
			m_aapucBS_VER[0] = bValue; m_aapucBS_VER[2] = bValue;
			
			m_aapbEdgeFilter_HOR[0] = bValue;m_aapbEdgeFilter_HOR[1] = bValue;  //水平边界
			m_aapucBS_HOR[0] = bValue;m_aapucBS_HOR[1] = bValue;
		}
		
		// //单独的4个4x4块均要设置TU 滤波
		else if( (uiDepth == 3 && (TransformIdx == 1) ) || (uiDepth == 2 && (TransformIdx == 2) ) || (uiDepth == 1 && (TransformIdx == 3) ) || ( uiDepth == 0 && (TransformIdx == 4) ) )
		{
			int i;
			for(i = 0; i < 4; i++)
			{
				m_aapbEdgeFilter_VER[i] = bValue;  //垂直边界
				m_aapucBS_VER[i] = bValue;
				m_aapbEdgeFilter_HOR[i] = bValue;  //水平边界
				m_aapucBS_HOR[i] = bValue;
			}
		}
		else if( (uiDepth == 2 && (TransformIdx == 0) ) || ( uiDepth == 1 && (TransformIdx == 0 || TransformIdx == 1) ) || ( uiDepth == 0 && (TransformIdx == 0 || TransformIdx == 1 || TransformIdx == 2) ) )
		{
			int col  = 16>>(uiDepth+TransformIdx);  //4x4块的raster位于col的整数倍列
			if(rasterOrder%16%col==0)  //位于垂直边界
			{
				m_aapbEdgeFilter_VER[0] = bValue; m_aapbEdgeFilter_VER[2] = bValue;  //垂直边界
				m_aapucBS_VER[0] = bValue; m_aapucBS_VER[2] = bValue;
			}
			if(rasterOrder/16%col==0)  //位于水平边界
			{
				m_aapbEdgeFilter_HOR[0] = bValue;m_aapbEdgeFilter_HOR[1] = bValue;  //水平边界
				m_aapucBS_HOR[0] = bValue;m_aapucBS_HOR[1] = bValue;
			}
			
		}
		else  //其他未考虑到的情况
		{
			
		}
	}
	//设置PU
	{
		 bValue =  bLeftEdge;
		 //uiNumElem = uiHeightInBaseUnits;
		//xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, 0,  bLeftEdge );   //!< 设置垂直边界
		
		if( (uiDepth == 3) || ( (uiDepth == 2) &&(rasterOrder%16%4==0) ) || ( uiDepth == 1 && (rasterOrder % 16 % 8 == 0) ) || ( uiDepth == 0 && (rasterOrder % 16 == 0) ) )
		{
			m_aapbEdgeFilter_VER[0] = bValue; m_aapbEdgeFilter_VER[2] = bValue;  //垂直边界
			m_aapucBS_VER[0] = bValue; m_aapucBS_VER[2] = bValue;
			
		}
		
		bValue =  bTopEdge;
		//uiNumElem = uiWidthInBaseUnits;
		//xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, 0,  bTopEdge ); //!< 设置水平边界
		
		if( (uiDepth == 3) || (uiDepth == 2 && (rasterOrder/16%4 == 0) ) || (uiDepth == 1 && (rasterOrder/16%8 == 0) ) || (uiDepth = 0 && (rasterOrder/16 == 0) ) )
		{
			m_aapbEdgeFilter_HOR[0] = bValue;m_aapbEdgeFilter_HOR[1] = bValue;  //水平边界
			m_aapucBS_HOR[0] = bValue;m_aapucBS_HOR[1] = bValue;
		}
		
		bValue = bInternalEdge;
		
		switch ( pcCU->m_pePartSize[uiAbsZorderIdx] )  //!< PU的划分模式  
		{
			//!< 以下根据PU的划分模式对滤波边界进行相应的设置
			case 0:
			{
			  break;
			}
			case 1: //SIZE_2NxN
			{
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, EDGE_HOR, uiHHeightInBaseUnits,bInternalEdge );
			  {
					if(uiDepth == 3)  //深度为3要特殊处理
					{ 
						m_aapbEdgeFilter_HOR[2] = bValue;m_aapbEdgeFilter_HOR[3] = bValue;  //水平边界
						//if(uiHHeightInBaseUnits == 0) { m_aapucBS_HOR[2] = bValue;m_aapucBS_HOR[3] = bValue;}
					} 
					else
					{
						//if( (uiDepth == 2 && (rasterOrder/16%4 != 0) &&(rasterOrder/16%2 == 0) ) || (uiDepth == 1 && (rasterOrder/16%8 != 0)&&(rasterOrder/16%4 == 0) ) || (uiDepth = 0 && (rasterOrder/16 != 0) && (rasterOrder/16%8 == 0) ) )
						if( (uiDepth == 2 && (rasterOrder/16%4 == 2) ) || (uiDepth == 1 && (rasterOrder/16%8 == 4) ) || (uiDepth = 0 && (rasterOrder/16%16 == 8) ) )
						{
							m_aapbEdgeFilter_HOR[0] = bValue;m_aapbEdgeFilter_HOR[1] = bValue;  //水平边界
						}
					}	
			  }
			  break;
			}
			case 2: //SIZE_Nx2N
			{
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, EDGE_VER, uiHWidthInBaseUnits,bInternalEdge );
			  {
					if(uiDepth == 3)  //深度为3要特殊处理
					{ 
						m_aapbEdgeFilter_VER[1] = bValue; m_aapbEdgeFilter_VER[3] = bValue;  //垂直边界
					}
					else
					{
						//if( ((uiDepth == 2)&&(rasterOrder%16%4!=0)&&(rasterOrder%16%2==0))||( uiDepth == 1 && (rasterOrder%16%8 != 0)  && (rasterOrder%16%4 == 0) ) || ( uiDepth == 0 && (rasterOrder %16 != 0) && (rasterOrder%16 %8 == 0) ) )
						if( ((uiDepth == 2)&&(rasterOrder%16%4==2))||( (uiDepth == 1) && (rasterOrder%16%8 == 4)) || ( (uiDepth == 0) && (rasterOrder %16 == 8) ) )
						{
							m_aapbEdgeFilter_VER[0] = bValue; m_aapbEdgeFilter_VER[2] = bValue;  //垂直边界
						}
					}
						
					//if(uiHWidthInBaseUnits == 0) { m_aapucBS_VER[1] = bValue; m_aapucBS_VER[3] = bValue;}
			  }
			  break;
			}
			case 3:	// SIZE_NxN, SIZE_NxN 只有8x8块也即uiDepth = 3 才有
			{
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, 0, uiHWidthInBaseUnits,bInternalEdge );
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, 1, uiHHeightInBaseUnits,bInternalEdge );
			  {
				  if(uiDepth == 3)
					{
						m_aapbEdgeFilter_VER[1] = bValue; m_aapbEdgeFilter_VER[3] = bValue;  //垂直边界
						m_aapbEdgeFilter_HOR[2] = bValue;m_aapbEdgeFilter_HOR[3] = bValue;  //水平边界
					}
			  }
			  break;
			}
			//对于8x8块是不会有下面的PU划分的，讨论其他的块即可
			case 4: 
			{
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, 1, uiQHeightInBaseUnits,bInternalEdge );
			  {
					if((uiDepth == 2) && (rasterOrder/16%4 == 0) )  //CU为16x16,高度和宽度上存在4个4x4块,由于是按照1:3划分,而当前处理的8x8块的起始4x4块位于16x16块的第4*(0/4)行时，当前8x8块的第1行也就是8x8块的第2,3号的水平滤波边界需要设置
					{
						m_aapbEdgeFilter_HOR[2] = bValue;m_aapbEdgeFilter_HOR[3] = bValue;  //水平边界
					}
					else
					{
						if( ( (uiDepth == 1) && (rasterOrder/16%8 == 2) ) || ( (uiDepth = 0)&& (rasterOrder/16%16 == 4) ) )
						{
							m_aapbEdgeFilter_HOR[0] = bValue;m_aapbEdgeFilter_HOR[1] = bValue;  //水平边界
						}
					}							
			  }
			  break;
			}
			case 5:
			{
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, 1, uiHeightInBaseUnits - uiQHeightInBaseUnits,bInternalEdge );
			  {
					if(uiDepth == 2 && (rasterOrder/16%4 == 2) )  //CU为16x16,高度和宽度上存在4个4x4块,由于是按照3:1划分,而当前处理的8x8块的起始4x4块位于16x16块的第4*(2/4)行时，下面一行的4x4块，即当前8x8块的第1行，也就是8x8块的第2,3号的水平滤波边界需要设置
					{
						m_aapbEdgeFilter_HOR[2] = bValue;m_aapbEdgeFilter_HOR[3] = bValue;  //水平边界
					}
					else
					{
						if( ( (uiDepth == 1) && (rasterOrder/16%8 == 6) ) || ( (uiDepth = 0) && (rasterOrder/16%16 == 12) ) )
						{
							m_aapbEdgeFilter_HOR[0] = bValue;m_aapbEdgeFilter_HOR[1] = bValue;  //水平边界
						}
					}
			  }
			 break;
			}
			case 6:
			{
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, 0, uiQWidthInBaseUnits,bInternalEdge );
			  {
					if((uiDepth == 2)&& (rasterOrder%16%4==0))  ////CU为16x16,高度和宽度上存在4个4x4块,由于是按照1:3划分,而当前处理的8x8块的起始4x4块位于16x16块的第4*(1/4)列时，当前8x8块的第二列也就是8x8块的第1,3号的垂直滤波边界需要设置
					{ 
						m_aapbEdgeFilter_VER[1] = bValue; m_aapbEdgeFilter_VER[3] = bValue;  //垂直边界
					}
					else
					{
						if( ( (uiDepth == 1) && (rasterOrder%16%8 == 2) ) || ( (uiDepth == 0) && (rasterOrder %16 == 4) ) )
						{
							m_aapbEdgeFilter_VER[0] = bValue; m_aapbEdgeFilter_VER[2] = bValue;  //垂直边界
						}
					}
			  }
			  break;
			}
			case 7:
			{
			  //xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, 3, 0, uiWidthInBaseUnits - uiQWidthInBaseUnits,bInternalEdge );
			  {
					if((uiDepth == 2)&& (rasterOrder%16%4==2))  ////CU为16x16,高度和宽度上存在4个4x4块,由于是按照3:1划分,而当前处理的8x8块的起始4x4块位于16x16块的第4*(2/4)列时，当前8x8块的第二列也就是8x8块的第1,3号的垂直滤波边界需要设置
					{ 
						m_aapbEdgeFilter_VER[1] = bValue; m_aapbEdgeFilter_VER[3] = bValue;  //垂直边界
					}
					else 
					{
						if( ( (uiDepth == 1) && (rasterOrder%16%8 == 6) ) || ( (uiDepth == 0) && (rasterOrder %16 == 12) ) )
						{
							m_aapbEdgeFilter_VER[0] = bValue; m_aapbEdgeFilter_VER[2] = bValue;  //垂直边界
						}
					}
			  }
			  break;
			}
			default:
			{
			  break;
			}
		}
	}
		
	//设置滤波强度
	{
		int uiPartIdx = 0;
		for(uiPartIdx = 0; uiPartIdx < uiCurNumParts; uiPartIdx++ )
		{
			//! 对于水平边界滤波的情况，检测partition是否属于上边界  
			UInt uiBSCheck = (( uiPartIdx+uiAbsZorderIdx-(((uiPartIdx+uiAbsZorderIdx)>>2)<<2) )/2 == 0);	
			if (uiBSCheck && m_aapbEdgeFilter_HOR[uiPartIdx])  //!< m_aapbEdgeFilter的值在xSetEdgefilterMultiple中设置过  
			{
				m_aapucBS_HOR[uiPartIdx] = xGetBoundaryStrengthSingle_cos ( pcCU, iDir, uiPartIdx+uiAbsZorderIdx,m_aapucBS_HOR[uiPartIdx] );  //!< 设置滤波强度
			}
		}	
			
	}	
	
	//滤波
	{
		//xEdgeFilterLuma_VER_cos( pcLoopFilter, pcCU, uiAbsZorderIdx, iDir, iEdge, m_aapucBS_VER );   //!< 对亮度分量进行滤波  
		//TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();  //!< 重建图像（滤波前）
		//Pel* piTmpSrc    = pcPicYuvRec->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );   //!< 指向当前PU对应的重建像素的首地址  
		
		//Int  iStride = pcPicYuvRec->getStride();   //!< 图像的跨度 

		TComPicYuv* pcPicYuvRec = pcCU->m_pcPic->m_apcPicYuv[1];
		Pel* piTmpSrc    = pcPicYuvRec->m_piPicOrgY + pcPicYuvRec->m_cuOffsetY[uiCUAddr] + pcPicYuvRec->m_buOffsetY[g_auiZscanToRaster[uiAbsZorderIdx]];
		Int iStride = pcPicYuvRec->m_iPicWidth + (pcPicYuvRec->m_iLumaMarginX)<<1;
		
		Int iQP = 0;
		Int iQP_P = 0;
		Int iQP_Q = 0;
	  
		UInt  uiPelsInPart = 4;
		UInt  uiBsAbsIdx = 0, uiBs = 0;
		Int   iOffset = iStride, iSrcStep = 1;

		bool  bPartPNoFilter = false;
		bool  bPartQNoFilter = false; 
		UInt  uiPartPIdx = 0;
		UInt  uiPartQIdx = 0;
		TComDataCU* pcCUP = pcCU; 
		TComDataCU* pcCUQ = pcCU;
		Int  betaOffsetDiv2 = 0;
		Int  tcOffsetDiv2 = 0;
		UInt iIdx = 0;
	  
		Int iBitdepthScale = 1;
		Int iIndexTC = 0;
		Int iIndexB = 0;
		Int iTc = 0;
		Int iBeta = 0;
		Int iSideThreshold = 0;
		Int iThrCut = 0;
		UInt  uiBlocksInPart = 0;
		UInt iBlkIdx = 0;

		piTmpSrc += iEdge*uiPelsInPart*iStride;

		for (iIdx = 0; iIdx < 2; iIdx++ )   //!< 遍历PU中的每个partition  
		{
			//uiBsAbsIdx = xCalcBsIdx( pcCU, uiAbsZorderIdx, iDir, iEdge, iIdx);  //!< partition对应的ZScan地址  
			//uiBs = m_aapucBS_HOR[iDir][uiBsAbsIdx];
			
			uiBsAbsIdx = uiAbsZorderIdx + iIdx; //获得水平边界的ZScan地址
			uiBs = m_aapucBS_HOR[iIdx]; //对应水平边界，分别获取 m_aapucBS_HOR[0],m_aapucBS_HOR[1]		
			
			if ( uiBs )  //!< uiBs == 1 or uiBs == 2  
			{
				//iQP_Q = pcCU->getQP(uiBsAbsIdx);
				iQP_Q = pcCU->m_phQP[uiBsAbsIdx];
				uiPartQIdx = uiBsAbsIdx;
				
				// Derive neighboring PU index
				//pcCUP = (TComDataCU*)getPUAbove_cos(pcCUQ,uiPartPIdx, uiPartQIdx,false,false,false);
			    pcCUP = pcCUQ->getPUAbove(uiPartPIdx, uiPartQIdx,false,false,false);
				
				//iQP_P = pcCUP->getQP(uiPartPIdx);
				iQP_P = pcCUP->m_phQP[uiPartPIdx];
				
				iQP = (iQP_P + iQP_Q + 1) >> 1;  //!< draft (8-264)  
				//iBitdepthScale = 1 << (g_bitDepthY-8);
			  
				iIndexTC = Clip3(0, MAX_QP+DEFAULT_INTRA_TC_OFFSET, (Int)(iQP + DEFAULT_INTRA_TC_OFFSET*(uiBs-1) + (tcOffsetDiv2 << 1)));
				iIndexB = Clip3(0, MAX_QP, iQP + (betaOffsetDiv2 << 1));   //!< draft (8-267) 
			  
				iTc =  sm_tcTable[iIndexTC]*iBitdepthScale;   //!< draft (8-265) 
				iBeta = sm_betaTable[iIndexB]*iBitdepthScale;   //!< draft (8-266) 
				iSideThreshold = (iBeta+(iBeta>>1))>>3;  //!< 阈值 
				iThrCut = iTc*10;

				uiBlocksInPart = 1;  //!< 4 / 1 = 1  
				for (iBlkIdx = 0; iBlkIdx<uiBlocksInPart; iBlkIdx ++)
				{
					Pel* piSrc1 = piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+0);
					Pel* piSrc2 = piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+3);
					
					//Int dp0 = xCalcDP( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+0), iOffset);
					Int dp0 = abs( piSrc1[-iOffset*3] - 2*piSrc1[-iOffset*2] + piSrc1[-iOffset] );
					
					//Int dq0 = xCalcDQ( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+0), iOffset);  //!< draft (8-269)
					Int dq0 = abs( piSrc1[0] - 2*piSrc1[iOffset] + piSrc1[iOffset*2]);
					
					
					//Int dp3 = xCalcDP( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+3), iOffset);  //!< draft (8-270)
					Int dp3 = abs( piSrc2[-iOffset*3] - 2*piSrc2[-iOffset*2] + piSrc2[-iOffset] );
					
					//Int dq3 = xCalcDQ( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+3), iOffset);  //!< draft (8-271) 
					Int dq3 = abs( piSrc2[0] - 2*piSrc2[iOffset] + piSrc2[iOffset*2]);
					
					Int d0 = dp0 + dq0;   //!< draft (8-272)  
					Int d3 = dp3 + dq3;   //!< draft (8-273)  
					
					Int dp = dp0 + dp3;    //!< draft (8-274)  
					Int dq = dq0 + dq3;    //!< draft (8-275)  
					Int d =  d0 + d3;     //!< draft (8-276)  
					
					if (d < iBeta)
					{ 
						Pel m4 = piSrc1[0];
						Pel m3 = piSrc1[-iOffset];  //!< p0 
						Pel m7 = piSrc1[ iOffset*3];  //!< q3 
						Pel m0 = piSrc1[-iOffset*4];   //!< p3 
						Int d_strong = abs(m0-m3)+abs(m7-m4); 
						Int i = 0;
						
						//xUseStrongFiltering( iOffset, 2*d0, iBeta, iTc, piSrc1)
						bool useFlag1 = ( (d_strong < (iBeta>>3)) && ((2*d0)<(iBeta>>2)) && ( abs(m3-m4) < ((iTc*5+1)>>1)) ); 
						
						bool useFlag2 = false;
						bool sw = false;
						
						bool bFilterP = (dp < iSideThreshold);  //!< dEp  
						bool bFilterQ = (dq < iSideThreshold);   //!< dEq 
						
						m4 = piSrc2[0];
						m3 = piSrc2[-iOffset];  //!< p0 
						m7 = piSrc2[ iOffset*3];  //!< q3 
						m0 = piSrc2[-iOffset*4];   //!< p3 
						
						d_strong = abs(m0-m3)+abs(m7-m4);
						
						// xUseStrongFiltering( iOffset, 2*d3, iBeta, iTc, piSrc2);
						useFlag2 = ( (d_strong < (iBeta>>3)) && ((2*d3)<(iBeta>>2)) && ( abs(m3-m4) < ((iTc*5+1)>>1)) ); 
						
						//bool sw =  xUseStrongFiltering( iOffset, 2*d0, iBeta, iTc, piSrc1)&& xUseStrongFiltering( iOffset, 2*d3, iBeta, iTc, piSrc2);
						sw = useFlag1 && useFlag2;
					
						//! sw即strong weak，该函数用于判断最终滤波的强弱
						for (i = 0; i < 4; i++)
						{
						  //! 最终进行真正的滤波  
						//xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+i), iOffset, iTc, sw, bPartPNoFilter, bPartQNoFilter, iThrCut, bFilterP, bFilterQ);
							
						  Int delta;
						  Pel* piSrc = piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*4+i);
						  //! m0 m1 m2 m3 | m4 m5 m6 m7  
						  Pel m4  = piSrc[0];
						  Pel m3  = piSrc[-iOffset];
						  Pel m5  = piSrc[ iOffset];
						  Pel m2  = piSrc[-iOffset*2];
						  Pel m6  = piSrc[ iOffset*2];
						  Pel m1  = piSrc[-iOffset*3];
						  Pel m7  = piSrc[ iOffset*3];
						  Pel m0  = piSrc[-iOffset*4];

						  if (sw)   //!< 强滤波
						  {   //! draft (8-305) to (8-310)   
							piSrc[-iOffset]   = Clip3(m3-2*iTc, m3+2*iTc, ((m1 + 2*m2 + 2*m3 + 2*m4 + m5 + 4) >> 3));
							piSrc[0]          = Clip3(m4-2*iTc, m4+2*iTc, ((m2 + 2*m3 + 2*m4 + 2*m5 + m6 + 4) >> 3));
							piSrc[-iOffset*2] = Clip3(m2-2*iTc, m2+2*iTc, ((m1 + m2 + m3 + m4 + 2)>>2));
							piSrc[ iOffset]   = Clip3(m5-2*iTc, m5+2*iTc, ((m3 + m4 + m5 + m6 + 2)>>2));
							piSrc[-iOffset*3] = Clip3(m1-2*iTc, m1+2*iTc, ((2*m0 + 3*m1 + m2 + m3 + m4 + 4 )>>3));
							piSrc[ iOffset*2] = Clip3(m6-2*iTc, m6+2*iTc, ((m3 + m4 + m5 + 3*m6 + 2*m7 +4 )>>3));
						  }
						  else   //!< 弱滤波  
						  {
							// Weak filter 
							delta = (9*(m4-m3) -3*(m5-m2) + 8)>>4 ;   //!< draft (8-311)  

							if ( abs(delta) < iThrCut )
							{  //! draft (8-312) to (8-318) 
							  Int tc2 = iTc>>1;
							  delta = Clip3(-iTc, iTc, delta);        
							  piSrc[-iOffset] = ClipY((m3+delta));
							  piSrc[0] = ClipY((m4-delta));

							  
							  if(bFilterP)
							  {
								Int delta1 = Clip3(-tc2, tc2, (( ((m1+m3+1)>>1)- m2+delta)>>1));
								piSrc[-iOffset*2] = ClipY((m2+delta1));
							  }
							  if(bFilterQ)
							  {
								Int delta2 = Clip3(-tc2, tc2, (( ((m6+m4+1)>>1)- m5-delta)>>1));
								piSrc[ iOffset] = ClipY((m5+delta2));
							  }
							}
						  }
						  if(bPartPNoFilter)   //!< Part P不滤波，保留原值  
						  {
							piSrc[-iOffset] = m3;
							piSrc[-iOffset*2] = m2;
							piSrc[-iOffset*3] = m1;
						  }
						  if(bPartQNoFilter)  //!< Part Q不滤波，保留原值  
						  {
							piSrc[0] = m4;
							piSrc[ iOffset] = m5;
							piSrc[ iOffset*2] = m6;
						  }
						}
					}
				}
			}
		}
		//色度分量进行滤波的条件：深度为3，因为此时iEdge只能取到0，满足iEdge%4 ==0 ;
		//或深度不等于3时，对于垂直滤波等价于4x4块对应的raster所在列数为4的倍数，对于水平滤波等价于4x4块对应的raster所在行数为4的倍数
		if( (uiDepth == 3) || (rasterOrder%16%4==0) || ( (rasterOrder/16%4)==0 ) )
		{
			//iStride     = pcPicYuvRec->getCStride();  //U,V分量的跨度
			//Pel*        piTmpSrcCb     = pcPicYuvRec->getCbAddr( pcCU->getAddr(), uiAbsZorderIdx );
			//Pel*        piTmpSrcCr     = pcPicYuvRec->getCrAddr( pcCU->getAddr(), uiAbsZorderIdx );
			
				
			Pel*  piTmpSrcCb  = pcPicYuvRec->m_piPicOrgU + pcPicYuvRec->m_cuOffsetC[uiCUAddr] + pcPicYuvRec->m_buOffsetC[g_auiZscanToRaster[uiAbsZorderIdx]];
			Pel*  piTmpSrcCr  = pcPicYuvRec->m_piPicOrgV + pcPicYuvRec->m_cuOffsetC[uiCUAddr] + pcPicYuvRec->m_buOffsetC[g_auiZscanToRaster[uiAbsZorderIdx]];
			
			UInt  uiPelsInPartChroma = 2;
			
			// Vertical Position
			UInt uiEdgeNumInLCUVert = rasterOrder%16;
			UInt uiEdgeNumInLCUHor =  rasterOrder/16;
		 
			UInt chromaIdx  = 0;
			
			iStride     = (pcPicYuvRec->m_iPicWidth)>>1 + (pcPicYuvRec->m_iChromaMarginX)<<1;
		  
			if (!( (uiEdgeNumInLCUHor%4)&& iDir))
			{
				iOffset   = iStride;
				iSrcStep  = 1;
				piTmpSrcCb += iEdge*iStride*uiPelsInPartChroma;
				piTmpSrcCr += iEdge*iStride*uiPelsInPartChroma;
				 
				for (iIdx = 0; iIdx < 2; iIdx++ )
				{
					
					uiBsAbsIdx = uiAbsZorderIdx + iIdx; //获得水平边界的ZScan地址
					uiBs = m_aapucBS_HOR[iIdx]; //对应水平边界，分别获取 m_aapucBS_HOR[0],m_aapucBS_HOR[1]
						
					if ( uiBs > 1)
					{
					  //iQP_Q = pcCU->getQP( uiBsAbsIdx );
					  iQP_Q = pcCU->m_phQP[uiBsAbsIdx];
					  
					  uiPartQIdx = uiBsAbsIdx;
					  // Derive neighboring PU index
					  
					  //pcCUP = (TComDataCU*)getPUAbove_cos(pcCUQ,uiPartPIdx, uiPartQIdx,false, false, false);
					  pcCUP = pcCUQ->getPUAbove(uiPartPIdx, uiPartQIdx,false,false,false);
					  
					  //iQP_P = pcCUP->getQP(uiPartPIdx);
					  iQP_P = pcCUP->m_phQP[uiPartPIdx];
					  
					  for ( chromaIdx = 0; chromaIdx < 2; chromaIdx++ )
					  {
							//Int chromaQPOffset  = (chromaIdx == 0) ? pcCU->getSlice()->getPPS()->getChromaCbQpOffset() : pcCU->getSlice()->getPPS()->getChromaCrQpOffset();
							Int chromaQPOffset  = 0;
							Pel* piTmpSrcChroma = (chromaIdx == 0) ? piTmpSrcCb : piTmpSrcCr;
							UInt uiStep = 0;
					
							iQP = QpUV( ((iQP_P + iQP_Q + 1) >> 1) + chromaQPOffset );
							iIndexTC = Clip3(0, MAX_QP+DEFAULT_INTRA_TC_OFFSET, iQP + DEFAULT_INTRA_TC_OFFSET*(uiBs - 1) + (tcOffsetDiv2 << 1));
							iTc =  sm_tcTable[iIndexTC]*iBitdepthScale;
						
							for ( uiStep = 0; uiStep < uiPelsInPartChroma; uiStep++ )
							{
							  //xPelFilterChroma( piTmpSrcChroma + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma), iOffset, iTc , bPartPNoFilter, bPartQNoFilter);
								Int delta;
								Pel* piSrc = piTmpSrcChroma + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma);
								
								Pel m4  = piSrc[0];
								Pel m3  = piSrc[-iOffset];
								Pel m5  = piSrc[ iOffset];
								Pel m2  = piSrc[-iOffset*2];
								  
								delta = Clip3(-iTc,iTc, (((( m4 - m3 ) << 2 ) + m2 - m5 + 4 ) >> 3) );
								piSrc[-iOffset] = ClipC(m3+delta);
								piSrc[0] = ClipC(m4-delta);

								if(bPartPNoFilter)
								{
									piSrc[-iOffset] = m3;
								}
								if(bPartQNoFilter)
								{
									piSrc[0] = m4;
								}
							}
					  }
					}
				}	
			} 	
		}	
	}
}

void my_deriveLoopFilterBoundaryAvailibility(TComPic*   pPic,int ctu,bool& isLeftAvail,bool& isRightAvail,bool& isAboveAvail,bool& isBelowAvail,bool& isAboveLeftAvail,bool& isAboveRightAvail, bool& isBelowLeftAvail,bool& isBelowRightAvail)
{
	pPic->getPicSym()->deriveLoopFilterBoundaryAvailibility(ctu, isLeftAvail,isRightAvail,isAboveAvail,isBelowAvail,isAboveLeftAvail,isAboveRightAvail,isBelowLeftAvail,isBelowRightAvail);
}

//这里的codedParams和reconParams必须为引用			
void my_BlkParamsRDOinit(SAOBlkParam* &codedParams,SAOBlkParam* &reconParams,unsigned long ul_pPic,unsigned long ul_m_pcEncTop)
{
	TComPic*   pPic    = (TComPic*)(ul_pPic);
	TEncTop*   m_pcEncTop = (TEncTop*)(ul_m_pcEncTop);
	TEncSampleAdaptiveOffset*  m_pcSAO = &(m_pcEncTop->m_cEncSAO);  //获取SAO滤波的类指针，或者说句柄
	codedParams = pPic->getPicSym()->getSAOBlkParam();
	reconParams = new SAOBlkParam[m_numCTUsPic];
	m_pcSAO->m_pcRDGoOnSbacCoder->load(m_pcSAO->m_pppcRDSbacCoder[ 0 ]);	
}


void my_BlkModeRDO_recSAOParam(int ctu,SAOBlkParam &modeParam,SAOBlkParam* codedParams,SAOBlkParam* reconParams,unsigned long ul_pPic,unsigned long ul_m_pcEncTop);
{
	
	TComPic*   pic    = (TComPic*)(ul_pPic);
	TEncTop*   m_pcEncTop = (TEncTop*)(ul_m_pcEncTop);
	TEncSampleAdaptiveOffset*  m_pcSAO = m_pcEncTop->getSAO();  //获取SAO滤波的类指针，或者说句柄
	m_pcSAO->m_pcRDGoOnSbacCoder->store(m_pcSAO->m_pppcRDSbacCoder[ SAO_CABACSTATE_BLK_CUR ]);
	
    
	SAOStatData***  blkStats = m_pcSAO->m_statData;	
	
	//get merge list
	std::vector<SAOBlkParam*> mergeList;
	m_pcSAO->getMergeList(pic, ctu, reconParams, mergeList);
				
	double minCost = MAX_DOUBLE;
	double modeCost = 0;	

	bool sliceEnabled[3] = {1,1,1};
	for(Int mode=1; mode < 3; mode++)
	{
		switch(mode)
		{
			case 1:
			{
					m_pcSAO->deriveModeNewRDO(ctu, mergeList, sliceEnabled, blkStats, modeParam, modeCost, m_pcSAO->m_pppcRDSbacCoder, SAO_CABACSTATE_BLK_CUR);
			}
			break;
			case 2:
			{
					m_pcSAO->deriveModeMergeRDO(ctu, mergeList, sliceEnabled, blkStats , modeParam, modeCost, m_pcSAO->m_pppcRDSbacCoder, SAO_CABACSTATE_BLK_CUR);
			}
			break;
		}
		if(modeCost < minCost)
		{
			minCost = modeCost;
			codedParams[ctu] = modeParam;
			m_pcSAO->m_pcRDGoOnSbacCoder->store(m_pcSAO->m_pppcRDSbacCoder[ SAO_CABACSTATE_BLK_NEXT ]);
		}
	} //mode
	m_pcSAO->m_pcRDGoOnSbacCoder->load(m_pcSAO->m_pppcRDSbacCoder[ SAO_CABACSTATE_BLK_NEXT ]);
				
	//apply reconstructed offsets
	reconParams[ctu] = codedParams[ctu];
	m_pcSAO->reconstructBlkSAOParam(reconParams[ctu], mergeList);
}

void freeReconParams(SAOBlkParam*& reconParams)
{
	delete[] reconParams;	
	reconParams = NULL;
}

void SAOStatDataSet(SAOStatData*& statsDataTypes,int typeIdx,Int64* &diff, Int64* &count)
{
	//statsDataTypes = m_statData[ctu][compIdx];
	memset(statsDataTypes[typeIdx].diff, 0, sizeof(Int64)*32);
	memset(statsDataTypes[typeIdx].count, 0, sizeof(Int64)*32);
	diff    = statsDataTypes[typeIdx].diff;
	count   = statsDataTypes[typeIdx].count;
}

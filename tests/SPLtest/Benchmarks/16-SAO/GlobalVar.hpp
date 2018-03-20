#include "TEncSampleAdaptiveOffset.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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

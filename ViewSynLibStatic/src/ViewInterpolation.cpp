
#ifdef WIN32
#pragma warning(disable : 4996)
#endif

#include "yuv.h"
#include "ParameterViewInterpolation.h"
#include "ViewSynthesis.h"
#include "Visbd.h"
#include "BounaryNoiseRemoval.h"
#include "ViewInterpolation.h"


#ifndef SAFE_RELEASE_IMAGE
#define SAFE_RELEASE_IMAGE(p) { if((p)!=NULL){ cvReleaseImage(&(p)); (p)=NULL; } }
#endif

CViewInterpolation::CViewInterpolation()
{
  m_pViewSynthesisGeneral = NULL; 
  m_pViewSynthesis1D = NULL;
  
  m_pcDepthMapLeft = m_pcDepthMapRight = NULL;
  m_pcImageLeft = m_pcImageRight = NULL;
  m_pcTempYuvLeft = m_pcTempYuvRight = NULL;

  m_pSynColorLeft  = NULL;
  m_pSynColorRight = NULL;
  m_pSynDepthLeft  = NULL;
  m_pSynDepthRight = NULL;

  m_iFrameNumber = 0; //Zhejiang

#ifdef _DEBUG
  m_ucSetup = 0;
#endif
}

CViewInterpolation::~CViewInterpolation()
{
  if(m_pViewSynthesisGeneral!=NULL) delete m_pViewSynthesisGeneral; 
  if(m_pViewSynthesis1D!=NULL) delete m_pViewSynthesis1D;

  if(m_pcDepthMapLeft!=NULL)  delete m_pcDepthMapLeft;
  if(m_pcDepthMapRight!=NULL)  delete m_pcDepthMapRight;
  if(m_pcImageLeft!=m_pcTempYuvLeft && m_pcTempYuvLeft!=NULL)    delete m_pcTempYuvLeft;
  if(m_pcImageRight!=m_pcTempYuvRight && m_pcTempYuvRight!=NULL)  delete m_pcTempYuvRight;
  if(m_pcImageLeft!=NULL)      delete m_pcImageLeft;
  if(m_pcImageRight!=NULL)    delete m_pcImageRight;
  
  if(m_pSynColorLeft !=NULL) free( m_pSynColorLeft );
  if(m_pSynColorRight!=NULL) free( m_pSynColorRight);
  if(m_pSynDepthLeft !=NULL) free( m_pSynDepthLeft );
  if(m_pSynDepthRight!=NULL) free( m_pSynDepthRight );

  m_pcDepthMapLeft = m_pcDepthMapRight = NULL;
  m_pcImageLeft = m_pcImageRight = NULL;
  m_pcTempYuvLeft = m_pcTempYuvRight = NULL;
  
  m_pSynColorLeft  = NULL;
  m_pSynColorRight = NULL;
  m_pSynDepthLeft  = NULL;
  m_pSynDepthRight = NULL;

#ifdef _DEBUG
  m_ucSetup = 0;
#endif
}

bool CViewInterpolation::Init(CParameterViewInterpolation &cParameter)
{
  int iWidth, iHeight;

  m_iSynthesisMode = cParameter.getSynthesisMode();

  m_uiBoundary = cParameter.getBoundaryNoiseRemoval();

  m_uiColorSpace = cParameter.getColorSpace();
  m_uiViewBlending = cParameter.getViewBlending();

  if (m_iSynthesisMode == 0)  // General mode
  {
    m_pViewSynthesisGeneral = new CViewInterpolationGeneral(); 

    m_pViewSynthesisGeneral->SetWidth(cParameter.getSourceWidth());
    m_pViewSynthesisGeneral->SetHeight(cParameter.getSourceHeight());
    m_pViewSynthesisGeneral->SetDepthType(cParameter.getDepthType());
    m_pViewSynthesisGeneral->SetColorSpace(cParameter.getColorSpace());
    m_pViewSynthesisGeneral->SetSubPelOption(cParameter.getPrecision()); 
    m_pViewSynthesisGeneral->SetViewBlending(cParameter.getViewBlending()); 
    m_pViewSynthesisGeneral->SetBoundaryNoiseRemoval(cParameter.getBoundaryNoiseRemoval());

#ifdef POZNAN_DEPTH_BLEND
    m_pViewSynthesisGeneral->SetDepthBlendDiff(cParameter.getDepthBlendDiff()); 
#endif

#ifdef NICT_IVSRS
// NICT start  
    m_pViewSynthesisGeneral->SetIvsrsInpaint(cParameter.getIvsrsInpaint());
// NICT end
#endif

    if(!m_pViewSynthesisGeneral->InitLR(cParameter.getSourceWidth(), cParameter.getSourceHeight(), 
                                      cParameter.getPrecision(), cParameter.getDepthType(), 
                                      cParameter.getLeftNearestDepthValue(), cParameter.getLeftFarthestDepthValue(), 
                                      cParameter.getRightNearestDepthValue(), cParameter.getRightFarthestDepthValue(), 
                                      cParameter.getCameraParameterFile().c_str(), 
                                      cParameter.getLeftCameraName().c_str(), cParameter.getRightCameraName().c_str(), 
                                      cParameter.getVirtualCameraName().c_str(), 
                                      cParameter.getMat_In_Left(), cParameter.getMat_Ex_Left(), cParameter.getMat_Trans_Left(),
                                      cParameter.getMat_In_Right(), cParameter.getMat_Ex_Right(), cParameter.getMat_Trans_Right(),
                                      cParameter.getMat_In_Virtual(), cParameter.getMat_Ex_Virtual(), cParameter.getMat_Trans_Virtual()
                                      )) return false;

  }
  else  // 1D mode
  {
    m_pViewSynthesis1D = new CViewInterpolation1D();
    
    m_pViewSynthesis1D->SetFocalLength(cParameter.getFocalLength());
    m_pViewSynthesis1D->SetLTranslationLeft(cParameter.getLTranslationLeft());
    m_pViewSynthesis1D->SetLTranslationRight(cParameter.getLTranslationRight());
    m_pViewSynthesis1D->SetduPrincipalLeft(cParameter.getduPrincipalLeft());
    m_pViewSynthesis1D->SetduPrincipalRight(cParameter.getduPrincipalRight());
    m_pViewSynthesis1D->SetZnearL(cParameter.getLeftNearestDepthValue());
    m_pViewSynthesis1D->SetZfarL (cParameter.getLeftFarthestDepthValue());
    m_pViewSynthesis1D->SetZnearR(cParameter.getRightNearestDepthValue());
    m_pViewSynthesis1D->SetZfarR (cParameter.getRightFarthestDepthValue());
    m_pViewSynthesis1D->SetWidth(cParameter.getSourceWidth());
    m_pViewSynthesis1D->SetHeight(cParameter.getSourceHeight());
    m_pViewSynthesis1D->SetSubPelOption(cParameter.getPrecision());  // Modify ViSBD how to set subpel
    m_pViewSynthesis1D->SetSplattingOption(cParameter.getSplattingOption());
    m_pViewSynthesis1D->SetBoundaryGrowth(cParameter.getBoudaryGrowth());
    //m_pViewSynthesis1D->SetUpsampleRefs(cParameter.getUpsampleRefs()); // Remove it
    m_pViewSynthesis1D->SetMergingOption(cParameter.getMergingOption());
    m_pViewSynthesis1D->SetDepthThreshold(cParameter.getDepthThreshold());
    m_pViewSynthesis1D->SetHoleCountThreshold(cParameter.getHoleCountThreshold());
    m_pViewSynthesis1D->SetTemporalImprovementOption(cParameter.getTemporalImprovementOption());   //Zhejiang, May, 4
    m_pViewSynthesis1D->SetWarpEnhancementOption(cParameter.getWarpEnhancementOption());
    m_pViewSynthesis1D->SetCleanNoiseOption(cParameter.getCleanNoiseOption());
    m_pViewSynthesis1D->AllocMem();
  }

  if (m_uiBoundary && cParameter.getSplattingOption() != 1 && m_iSynthesisMode == 1) {
	  if (getBoundaryNoiseRemoval())  {
		  if (m_pViewSynthesis1D->GetSplattingOption() != 1) {
			  printf("\nWarning: When you use 1D Mode, 'Boundary Noise Removal' mode supports only \n");
			  printf("         only 'SplattingOption = 1'.\n");
			  printf("         Hence, 'SplattingOption' is changed to '1'.\n\n");
			  m_pViewSynthesis1D->SetSplattingOption(1);
		  }
	  }
  }

  m_pBoundaryNoiseRemoval = new CBoundaryNoiseRemoval(); // Boundary Noise Removal


  iWidth  = int( cParameter.getSourceWidth() );
  iHeight = int( cParameter.getSourceHeight() );

  m_pcDepthMapLeft  = new CIYuv<DepthType>(iHeight, iWidth, POZNAN_DEPTHMAP_CHROMA_FORMAT);
  m_pcDepthMapRight  = new CIYuv<DepthType>(iHeight, iWidth, POZNAN_DEPTHMAP_CHROMA_FORMAT);
  if (m_iSynthesisMode == 0) // General mode
  {
    m_pcImageLeft     = new CIYuv<ImageType>(iHeight, iWidth*cParameter.getPrecision(), 444);
    m_pcImageRight    = new CIYuv<ImageType>(iHeight, iWidth*cParameter.getPrecision(), 444);
  }
  else
  {
    m_pcImageLeft  = new CIYuv<ImageType>(iHeight, iWidth, 444);
    m_pcImageRight = new CIYuv<ImageType>(iHeight, iWidth, 444);
  }

  if(cParameter.getPrecision() != 1)
  {
    m_pcTempYuvLeft   = new CIYuv<ImageType>(iHeight, iWidth, 444);
    m_pcTempYuvRight  = new CIYuv<ImageType>(iHeight, iWidth, 444);
  }
  else
  {
    m_pcTempYuvLeft   = m_pcImageLeft;
    m_pcTempYuvRight  = m_pcImageRight;
  }

  if( m_pcDepthMapLeft==NULL || m_pcDepthMapRight==NULL || 
      m_pcImageLeft==NULL    || m_pcImageRight==NULL    ||
      m_pcTempYuvLeft==NULL  || m_pcTempYuvRight==NULL  ) return false;

  if (m_iSynthesisMode == 0) // General mode
  {
    if(!m_pcImageLeft->setUpsampleFilter(cParameter.getFilter(), cParameter.getPrecision())) return false;
    if(!m_pcImageRight->setUpsampleFilter(cParameter.getFilter(), cParameter.getPrecision())) return false;
  }
  else // 1D mode: No upsampling is done before going into Visbd
  {
    if(!m_pcImageLeft ->setUpsampleFilter(cParameter.getFilter(), 1)) return false;
    if(!m_pcImageRight->setUpsampleFilter(cParameter.getFilter(), 1)) return false;
  }
  
  // Prepare buffer for boundary noise removal:
  m_iWidth = iWidth * cParameter.getPrecision();
  m_iHeight = iHeight;
  m_pSynColorLeft  = (unsigned char*) malloc(sizeof(unsigned char)*(m_iWidth*m_iHeight*3)); // in 444
  m_pSynColorRight = (unsigned char*) malloc(sizeof(unsigned char)*(m_iWidth*m_iHeight*3)); // in 444
  m_pSynDepthLeft  = (unsigned char*) malloc(sizeof(unsigned char)*(m_iWidth*m_iHeight)); // in 400
  m_pSynDepthRight = (unsigned char*) malloc(sizeof(unsigned char)*(m_iWidth*m_iHeight)); // in 400
  
  if (m_pSynColorLeft == NULL || m_pSynColorRight == NULL ||
      m_pSynDepthLeft == NULL || m_pSynDepthRight == NULL)
      return false;

#ifdef _DEBUG
  m_ucSetup = 1;
#endif

  return true;
}

bool CViewInterpolation::SetReferenceImage(int iLeft, CIYuv<ImageType> *pcYuv)
{
#ifdef _DEBUG
  if(m_ucSetup==0) return false;
#endif

  if(iLeft)
  {
    if(m_uiColorSpace)
      m_pcTempYuvLeft->setData444_inIBGR(pcYuv); // RGB
    else
      m_pcTempYuvLeft->setData444_inIYUV(pcYuv); // YUV
    m_pcImageLeft->upsampling(m_pcTempYuvLeft);
  }
  else
  {
    if(m_uiColorSpace)
      m_pcTempYuvRight->setData444_inIBGR(pcYuv); // RGB
    else
      m_pcTempYuvRight->setData444_inIYUV(pcYuv); // YUV
    m_pcImageRight->upsampling(m_pcTempYuvRight);
  }
  
#ifdef _DEBUG
  m_ucSetup=1;
#endif

  return true;
}

/*
 * \brief
 * The main interface function to perform view interpolation
 *   to be called from the application
 *
 * \param pSynYuvBuffer
 *   Store the synthesized picture into the buffer
 *
 * \return
 *    true: if succeed;
 *    false: if fails.
 */
 
bool CViewInterpolation::DoViewInterpolation( CIYuv<ImageType>* pSynYuvBuffer )
{
  /*
	ImageType*** RefLeft  = m_pcImageLeft->getData();
	ImageType*** RefRight = m_pcImageRight->getData();
	DepthType** RefDepthLeft  = m_pcDepthMapLeft->Y;
	DepthType** RefDepthRight = m_pcDepthMapRight->Y;
  //*/

  bool ret = false;
  if (m_iSynthesisMode == 0) // General mode
    ret = xViewInterpolationGeneralMode( pSynYuvBuffer );
  else if(m_iSynthesisMode == 1) // 1-D mode
    ret = xViewInterpolation1DMode( pSynYuvBuffer );
    
  if (ret == false)
    return ret;
     
  return ret;
}

bool CViewInterpolation::xViewInterpolationGeneralMode(CIYuv<ImageType>* pSynYuvBuffer)
{
  ImageType*** RefLeft  = m_pcImageLeft->getData();
  ImageType*** RefRight = m_pcImageRight->getData();
  DepthType** RefDepthLeft  = m_pcDepthMapLeft->Y;
  DepthType** RefDepthRight = m_pcDepthMapRight->Y;  
  
  if ( 0 != m_pViewSynthesisGeneral->DoOneFrameGeneral(RefLeft, RefRight, RefDepthLeft, RefDepthRight, pSynYuvBuffer) )
    return false;

  if (getBoundaryNoiseRemoval())  {
    CIYuv<ImageType> pRefLeft;
	  CIYuv<ImageType> pRefRight;
	  CIYuv<DepthType> pRefDepthLeft;
	  CIYuv<DepthType> pRefDepthRight;
	  CIYuv<HoleType> pRefHoleLeft;
	  CIYuv<HoleType> pRefHoleRight;
	  int Width = m_iWidth/m_pViewSynthesisGeneral->GetInterpolatedLeft()->GetPrecision();
	  m_pBoundaryNoiseRemoval->SetViewBlending(m_uiViewBlending);
	  m_pBoundaryNoiseRemoval->SetColorSpace(m_uiColorSpace);

	  if(!pRefLeft.resize(m_iHeight, Width, 444))      return false;
	  if(!pRefRight.resize(m_iHeight, Width, 444))     return false;
	  if(!pRefDepthLeft.resize(m_iHeight, Width, POZNAN_DEPTHMAP_CHROMA_FORMAT)) return false;
	  if(!pRefDepthRight.resize(m_iHeight, Width, POZNAN_DEPTHMAP_CHROMA_FORMAT))return false;
	  if(!pRefHoleLeft.resize(m_iHeight, Width, POZNAN_DEPTHMAP_CHROMA_FORMAT))  return false;
	  if(!pRefHoleRight.resize(m_iHeight, Width, POZNAN_DEPTHMAP_CHROMA_FORMAT)) return false;

	  xFileConvertingforGeneralMode(&pRefLeft, &pRefRight, &pRefDepthLeft, &pRefDepthRight, &pRefHoleLeft, &pRefHoleRight);
      xBoundaryNoiseRemoval(&pRefLeft, &pRefRight, &pRefDepthLeft, &pRefDepthRight, &pRefHoleLeft, &pRefHoleRight, pSynYuvBuffer, false/*General Mode*/);
  }

  return true;
}

/*
 * To perform 1-D view interpolation
 *
 * \Output:
 *    All the color images below are in 444, depth images are 400
 * \param pSynYuvBuffer
 *    To store the synthesis result.
 * \member m_pSynColorLeft
 *    To store the synthesis color component from left ref w/o hole filling
 * \member m_pSynColorRight
 *    To store the synthesis color component from right ref w/o hole filling
 * \member m_pSynDepthLeft
 *    To store the synthesis depth component from left ref w/o hole filling
 * \member m_pSynDepthRight
 *    To store the synthesis depth component from right ref w/o hole filling
 *
 * Return:
 *    true:  if succeed;
 *    false: if fails.
 */
bool CViewInterpolation::xViewInterpolation1DMode( CIYuv<ImageType>* pSynYuvBuffer )
{
  ImageType* RefLeft  = m_pcImageLeft->getBuffer();
  ImageType* RefRight = m_pcImageRight->getBuffer();
  DepthType* RefDepthLeft  = m_pcDepthMapLeft->getBuffer();
  DepthType* RefDepthRight = m_pcDepthMapRight->getBuffer();

  m_pViewSynthesis1D->SetFrameNumber( getFrameNumber()); //Zhejiang
  if ( 0 != m_pViewSynthesis1D->DoOneFrame(RefLeft, RefRight, RefDepthLeft, RefDepthRight, pSynYuvBuffer->getBuffer()) )
    return false;

  if (getBoundaryNoiseRemoval())  {
	  CIYuv<ImageType> pRefLeft;
	  CIYuv<ImageType> pRefRight;
	  CIYuv<DepthType> pRefDepthLeft;
	  CIYuv<DepthType> pRefDepthRight;	  
	  CIYuv<HoleType> pRefHoleLeft;
	  CIYuv<HoleType> pRefHoleRight;
	  int Width        = m_pViewSynthesis1D->GetWidth();
	  int Height       = m_pViewSynthesis1D->GetHeight();
	  int SampleFactor =  m_pViewSynthesis1D->GetSubPelOption() * m_pViewSynthesis1D->GetUpsampleRefs();
	  m_pBoundaryNoiseRemoval->SetViewBlending(m_uiViewBlending);
	  m_pBoundaryNoiseRemoval->SetColorSpace(m_uiColorSpace);
	  m_pBoundaryNoiseRemoval->SetPrecision(SampleFactor);

	  if(!pRefLeft.resize(Height, Width*SampleFactor,        444)) return false;
	  if(!pRefRight.resize(Height, Width*SampleFactor,       444)) return false;
	  if(!pRefDepthLeft.resize(Height, Width*SampleFactor,   POZNAN_DEPTHMAP_CHROMA_FORMAT)) return false;
	  if(!pRefDepthRight.resize(Height, Width*SampleFactor,  POZNAN_DEPTHMAP_CHROMA_FORMAT)) return false;
	  if(!pRefHoleLeft.resize(Height, Width*SampleFactor,    400)) return false;
	  if(!pRefHoleRight.resize(Height, Width*SampleFactor,   400)) return false;

	  xFileConvertingfor1DMode(&pRefLeft, &pRefRight, &pRefDepthLeft, &pRefDepthRight, &pRefHoleLeft, &pRefHoleRight);
	  xBoundaryNoiseRemoval(&pRefLeft, &pRefRight, &pRefDepthLeft, &pRefDepthRight, &pRefHoleLeft, &pRefHoleRight, pSynYuvBuffer, true/*1D Mode*/);
  }

  return true;
}


bool CViewInterpolation::xBoundaryNoiseRemoval(CIYuv<ImageType>* pRefLeft, CIYuv<ImageType>* pRefRight, CIYuv<DepthType>* pRefDepthLeft, CIYuv<DepthType>* pRefDepthRight, CIYuv<HoleType>* pRefHoleLeft, CIYuv<HoleType>* pRefHoleRight, CIYuv<ImageType>* pSynYuvBuffer, bool SynthesisMode)
{

	switch(SynthesisMode)
	{
	case false:  // General Mode	
		m_pBoundaryNoiseRemoval->SetWidth (m_iWidth/m_pViewSynthesisGeneral->GetInterpolatedLeft()->GetPrecision());
		m_pBoundaryNoiseRemoval->SetHeight(m_iHeight);
		m_pBoundaryNoiseRemoval->SetLeftBaseLineDist(m_pViewSynthesisGeneral->GetInterpolatedLeft()->getBaselineDistance());
		m_pBoundaryNoiseRemoval->SetRightBaseLineDist(m_pViewSynthesisGeneral->GetInterpolatedRight()->getBaselineDistance());
		m_pBoundaryNoiseRemoval->SetLeftH_V2R (m_pViewSynthesisGeneral->GetInterpolatedLeft() ->GetMatH_V2R());
		m_pBoundaryNoiseRemoval->SetRightH_V2R(m_pViewSynthesisGeneral->GetInterpolatedRight()->GetMatH_V2R());
		break;
	case true:   // 1D Mode
		m_pBoundaryNoiseRemoval->SetWidth (m_pViewSynthesis1D->GetWidth());
		m_pBoundaryNoiseRemoval->SetHeight(m_pViewSynthesis1D->GetHeight());
		m_pBoundaryNoiseRemoval->SetduPrincipal(m_pViewSynthesis1D->GetduPrincipal());
		m_pBoundaryNoiseRemoval->SetFocalLength(m_pViewSynthesis1D->GetFocalLength());
		m_pBoundaryNoiseRemoval->SetLTranslationLeft(m_pViewSynthesis1D->GetLTranslation());
		m_pBoundaryNoiseRemoval->SetZfar(m_pViewSynthesis1D->GetZfar());
		m_pBoundaryNoiseRemoval->SetZnear(m_pViewSynthesis1D->GetZnear());
		break;
	}


	if ( 0 != m_pBoundaryNoiseRemoval->DoBoundaryNoiseRemoval(pRefLeft, pRefRight, pRefDepthLeft, pRefDepthRight, pRefHoleLeft, pRefHoleRight, pSynYuvBuffer, SynthesisMode))
		return false;

	/*FILE *fp = fopen("mask.yuv", "wb");
    for (j=0;j<m_iHeight*m_iWidth;j++) {
		fputc(tar_L[j], fp);				
	}
	fclose(fp);*/

	
	return true;
}

void CViewInterpolation::xFileConvertingforGeneralMode(CIYuv<ImageType>* pRefLeft, CIYuv<ImageType>* pRefRight, CIYuv<DepthType>* pRefDepthLeft, CIYuv<DepthType>* pRefDepthRight, CIYuv<HoleType>* pRefHoleLeft, CIYuv<HoleType>* pRefHoleRight)
{
  int Width, Height;
	ImageType* tar_L_image, *tar_R_image;
	ImageType* org_L_image, *org_R_image;
  DepthType* tar_L_depth, *tar_R_depth;
	DepthType* org_L_depth, *org_R_depth;
  HoleType* tar_L_hole, *tar_R_hole;
	HoleType* org_L_hole, *org_R_hole;
	Width = m_iWidth/m_pViewSynthesisGeneral->GetInterpolatedLeft()->GetPrecision();
	Height= m_iHeight;


	// Color Image with hole
	org_L_image = (ImageType*)m_pViewSynthesisGeneral->GetSynLeftWithHole()->imageData;
	org_R_image = (ImageType*)m_pViewSynthesisGeneral->GetSynRightWithHole()->imageData;
	tar_L_image = pRefLeft ->getBuffer();
	tar_R_image = pRefRight->getBuffer();
	memcpy(tar_L_image, org_L_image, Height*Width*3*sizeof(ImageType));
	memcpy(tar_R_image, org_R_image, Height*Width*3*sizeof(ImageType));

	org_L_depth = (DepthType*)m_pViewSynthesisGeneral->GetSynDepthLeftWithHole()->imageData;
	org_R_depth = (DepthType*)m_pViewSynthesisGeneral->GetSynDepthRightWithHole()->imageData;
	tar_L_depth = pRefDepthLeft ->getBuffer();
	tar_R_depth = pRefDepthRight->getBuffer();
	memcpy(tar_L_depth, org_L_depth, Height*Width*sizeof(DepthType));
	memcpy(tar_R_depth, org_R_depth, Height*Width*sizeof(DepthType));

	org_L_hole = (HoleType*)m_pViewSynthesisGeneral->GetSynHoleLeft() ->imageData;
	org_R_hole = (HoleType*)m_pViewSynthesisGeneral->GetSynHoleRight()->imageData;
	tar_L_hole = pRefHoleLeft ->getBuffer();
	tar_R_hole = pRefHoleRight->getBuffer();
	memcpy(tar_L_hole, org_L_hole, Height*Width*sizeof(HoleType));
	memcpy(tar_R_hole, org_R_hole, Height*Width*sizeof(HoleType));

}

void CViewInterpolation::xFileConvertingfor1DMode( CIYuv<ImageType>* pRefLeft, CIYuv<ImageType>* pRefRight, CIYuv<DepthType>* pRefDepthLeft, CIYuv<DepthType>* pRefDepthRight,  CIYuv<HoleType>* pRefHoleLeft,  CIYuv<HoleType>* pRefHoleRight)
{
	int Width, Height, UpSampleFactor;
	//CPictureResample resmple;
	ImageType* tar_L, *tar_R;
	UpSampleFactor =  m_pViewSynthesis1D->GetSubPelOption() * m_pViewSynthesis1D->GetUpsampleRefs();
	Width = m_pViewSynthesis1D->GetWidth()*UpSampleFactor;
	Height= m_pViewSynthesis1D->GetHeight();

	//CIYuv pRefSrc, pRefDst;
	//pRefSrc.resize(Height, Width, 444);
	//pRefDst.resize(Height, Width, 444);


	// Left Synthesized Image
	tar_L = pRefLeft->getBuffer();
	memcpy(&tar_L[Height*Width*0], m_pViewSynthesis1D->GetSynColorLeftY(), Height*Width);
	memcpy(&tar_L[Height*Width*1], m_pViewSynthesis1D->GetSynColorLeftU(), Height*Width);
	memcpy(&tar_L[Height*Width*2], m_pViewSynthesis1D->GetSynColorLeftV(), Height*Width);

	// Right Synthesized Image
	tar_R = pRefRight->getBuffer();
	memcpy(&tar_R[Height*Width*0], m_pViewSynthesis1D->GetSynColorRightY(), Height*Width);
	memcpy(&tar_R[Height*Width*1], m_pViewSynthesis1D->GetSynColorRightU(), Height*Width);
	memcpy(&tar_R[Height*Width*2], m_pViewSynthesis1D->GetSynColorRightV(), Height*Width);

	// Left Synthesized Depth Image
	memcpy(pRefDepthLeft->getBuffer(), m_pViewSynthesis1D->GetSynDepthLeft(), Height*Width);
    	
	// Right Synthesized Depth Image
	memcpy(pRefDepthRight->getBuffer(), m_pViewSynthesis1D->GetSynDepthRight(), Height*Width);

	m_pBoundaryNoiseRemoval->DepthMatchingWithColor(pRefDepthLeft , pRefLeft , pRefHoleLeft );
	m_pBoundaryNoiseRemoval->DepthMatchingWithColor(pRefDepthRight, pRefRight, pRefHoleRight);
}

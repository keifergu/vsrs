#ifndef _VIEW_SYNTHESIS_H_
#define _VIEW_SYNTHESIS_H_

#pragma warning(disable:4996)
#pragma warning(disable:4244)
#pragma warning(disable:4819)

#ifndef WIN32
#define BYTE unsigned char
#endif

#include <iostream>
#include <fstream>
#include "yuv.h" 
#include "version.h"

typedef unsigned char uchar;
//using namespace std;

#ifndef UInt
#define UInt    unsigned int
#endif

class CViewInterpolationGeneral 
{
public:
  CViewInterpolationGeneral          (); 
  virtual ~CViewInterpolationGeneral      (); 

  // Member function InitLR was added to class CViewInterpolationGeneral.

  bool InitLR        (  UInt uiWidth, UInt uiHeight, UInt uiPrecision, UInt uiDepthType,
                                  double dZnearL, double dZfarL, double dZnearR, double dZfarR, 
                                  const char *strCamParamFile, const char *strRefLCamID, const char *strRefRCamID, const char *strVirCamID, 
                                  double Mat_In_Left[9], double Mat_Ex_Left[9], double Mat_Trans_Left[3], 
                                  double Mat_In_Right[9], double Mat_Ex_Right[9], double Mat_Trans_Right[3], 
                                  double Mat_In_Virtual[9], double Mat_Ex_Virtual[9], double Mat_Trans_Virtual[3]); 

  // The number of arguments of Init increased.

  bool Init        (  UInt uiWidth, UInt uiHeight, UInt uiPrecision, UInt uiDepthType, 

#ifdef NICT_IVSRS
                                  // NICT start
                                  UInt uiIvsrsInpaint,  // iVSRS inpaint flag
                                  // NICT end
#endif
                                  double dZnear, double dZfar, const char *strCamParamFile, const char *strRefCamID, const char *strVirCamID, 
                                  double Mat_In_Ref[9], double Mat_Ex_Ref[9], double Mat_Trans_Ref[3], double Mat_In_Vir[9], double Mat_Ex_Vir[9], double Mat_Trans_Vir[3]); 

  void    xReleaseMemory        ();

  bool    xSynthesizeView        (ImageType ***src, DepthType **pDepthMap, int th_same_depth=5);
  bool    xSynthesizeDepth      (DepthType **pDepthMap, ImageType ***src);
  bool    xSynthesizeView_reverse    (ImageType ***src, DepthType **pDepthMap, int th_same_depth=5);

  double    getBaselineDistance      ()  { return m_dBaselineDistance; }
  double    getLeftBaselineDistance      ()  { return LeftBaselineDistance; }
  double    getRightBaselineDistance      ()  { return RightBaselineDistance; }
  IplImage*  getHolePixels        ()  { return m_imgHoles; }
  IplImage*  getSynthesizedPixels    ()  { return m_imgSuccessSynthesis; }
  IplImage*  getUnstablePixels      ()  { return m_imgMask[0]; }
  IplImage*  getVirtualImage        ()  { return m_imgVirtualImage; }
  IplImage*  getVirtualDepthMap      ()  { return m_imgVirtualDepth; }
  //Nagoya start
  ImageType**    getVirtualImageY       ()  { return m_pVirtualImageY; }
  ImageType**    getVirtualImageU       ()  { return m_pVirtualImageU; }
  ImageType**    getVirtualImageV       ()  { return m_pVirtualImageV; }
  //Nagoya end
  DepthType**    getVirtualDepth        ()  { return m_pVirtualDepth; }
  HoleType**    getNonHoles          ()  { return m_pSuccessSynthesis; }
  void SetWidth (int sWidth ) { m_uiWidth  = sWidth;  }
  void SetHeight(int sHeight) { m_uiHeight = sHeight; }
  void SetDepthType(int sDepthType) {m_uiDepthType = sDepthType; }
  void SetColorSpace(int sColorSpace) { m_uiColorSpace = sColorSpace; }
  void SetSubPelOption(int sPrecision) { m_uiPrecision = sPrecision; }

#ifdef NICT_IVSRS
// NICT start  
  void SetIvsrsInpaint(unsigned int uiIvsrsInpaint) { m_uiIvsrsInpaint = uiIvsrsInpaint; } // NICT iVSRS inpaint flag  
// NICT end
#endif

  void SetViewBlending(int sViewBlending) { ViewBlending = sViewBlending; }
  void SetBoundaryNoiseRemoval(int sBoundaryNoiseRemoval) { m_uiBoundaryNoiseRemoval = sBoundaryNoiseRemoval; }
#ifdef POZNAN_DEPTH_BLEND
  void SetDepthBlendDiff(int iDepthBlendDiff) { m_iDepthBlendDiff = iDepthBlendDiff; }
#endif

  int  DoOneFrameGeneral(ImageType*** RefLeft, ImageType*** RefRight, DepthType** RefDepthLeft, DepthType** RefDepthRight, CIYuv<ImageType>* pSynYuvBuffer);
  IplImage*  getImgSynthesizedViewLeft  ();
  IplImage*  getImgSynthesizedViewRight  ();

  // GIST added
#ifdef POZNAN_GENERAL_HOMOGRAPHY
  CvMat* GetMatH_V2R() { return m_matH_V2R; }
#else
  CvMat** GetMatH_V2R() { return m_matH_V2R; }
#endif
  CViewInterpolationGeneral* GetInterpolatedLeft ()   { return m_pcViewSynthesisLeft;}
  CViewInterpolationGeneral* GetInterpolatedRight()   { return m_pcViewSynthesisRight;}
  IplImage*  GetSynLeftWithHole      ()    { return m_imgSynLeftforBNR;   } 
  IplImage*  GetSynRightWithHole     ()    { return m_imgSynRightforBNR;  } 
  IplImage*  GetSynDepthLeftWithHole ()    { return m_imgDepthLeftforBNR; } 
  IplImage*  GetSynDepthRightWithHole()    { return m_imgDepthRightforBNR;} 
  IplImage*  GetSynHoleLeft          ()    { return m_imgHoleLeftforBNR;  } 
  IplImage*  GetSynHoleRight         ()    { return m_imgHoleRightforBNR; } 
  int        GetPrecision            ()    { return m_uiPrecision;        }

#ifdef NICT_IVSRS
// NICT start  
  unsigned int GetIvsrsInpaint       ()    { return m_uiIvsrsInpaint;     } // NICT get iVSRS inpaint flag
// NICT end
#endif

  void       SetLeftBaselineDistance( double sLeftBaselineDistance)    { LeftBaselineDistance  = sLeftBaselineDistance;  }
  void       SetRightBaselineDistance( double sRightBaselineDistance)  { RightBaselineDistance = sRightBaselineDistance; }
  // GIST end


  //Nagoya start
  ImageType* GetSynColorLeftY () { return *(m_pcViewSynthesisLeft->getVirtualImageY()); }
  ImageType* GetSynColorRightY() { return *(m_pcViewSynthesisRight->getVirtualImageY()); }
  ImageType* GetSynColorLeftU () { return *(m_pcViewSynthesisLeft->getVirtualImageU()); }
  ImageType* GetSynColorRightU() { return *(m_pcViewSynthesisRight->getVirtualImageU()); }
  ImageType* GetSynColorLeftV () { return *(m_pcViewSynthesisLeft->getVirtualImageV()); }
  ImageType* GetSynColorRightV() { return *(m_pcViewSynthesisRight->getVirtualImageV()); }
  DepthType* GetSynDepthLeft  () { return *(m_pcViewSynthesisLeft->getVirtualDepth()); }
  DepthType* GetSynDepthRight () { return *(m_pcViewSynthesisRight->getVirtualDepth()); }
  //Nagoya end

protected:

private:
  void    convertCameraParam      (CvMat *exMat_dst, CvMat *exMat_src);

  void    cvexMedian          (IplImage* dst);
  void    cvexBilateral        (IplImage* dst, int sigma_d, int sigma_c);
  void    erodebound          (IplImage* bound, int flag);
  int      median_filter_depth      (IplImage *srcDepth, IplImage *dstDepth, IplImage *srcMask,  IplImage *dstMask, int sizeX, int sizeY, bool bSmoothing);
  int      median_filter_depth_wCheck  (IplImage *srcDepth, IplImage *dstDepth, IplImage *srcMask,  IplImage *dstMask, int sizeX, int sizeY, bool bSmoothing, int th_same_plane=5);

  CViewInterpolationGeneral*  m_pcViewSynthesisLeft; 
  CViewInterpolationGeneral*  m_pcViewSynthesisRight; 

  bool    init_camera_param(double Mat_In_Ref[9], double Mat_Ex_Ref[9], double Mat_Trans_Ref[3], double Mat_In_Vir[9], double Mat_Ex_Vir[9], double Mat_Trans_Vir[3], 
                                       CvMat *mat_in[2], CvMat *mat_ex_c2w[2], CvMat *mat_proj[2]); 
  bool    init_3Dwarp(double Z_near, double Z_far, unsigned int uiDepthType, 
                                  const char *strCamParamFile, const char *strRefCamID, const char *strVirCamID, 
                                  double Mat_In_Ref[9], double Mat_Ex_Ref[9], double Mat_Trans_Ref[3], 
                                  double Mat_In_Vir[9], double Mat_Ex_Vir[9], double Mat_Trans_Vir[3]); 
  bool    init_shift          (double Z_near, double Z_far, unsigned int uiDepthType, const char *strCamParamFile, const char *strRefCamID, const char *strVirCamID);

  void    image2world_with_z      (CvMat *mat_Rc2w_invIN_from, CvMat *matEX_c2w_from, CvMat *image, CvMat *world);
#ifdef POZNAN_GENERAL_HOMOGRAPHY
  void    makeHomography        (CvMat *&matH_F2T, CvMat *&matH_T2F, double adTable[MAX_DEPTH], CvMat *matIN_from, CvMat *matEX_c2w_from, CvMat *matProj_to);
#else
  void    makeHomography        (CvMat *matH_F2T[MAX_DEPTH], CvMat *matH_T2F[MAX_DEPTH], double adTable[MAX_DEPTH], CvMat *matIN_from, CvMat *matEX_c2w_from, CvMat *matProj_to);
#endif

  bool    depthsynthesis_3Dwarp    (DepthType **pDepthMap, ImageType ***src);
  bool    depthsynthesis_3Dwarp_ipel  (DepthType **pDepthMap, ImageType ***src);

  bool    viewsynthesis_reverse_3Dwarp    (ImageType ***src, DepthType **pDepthMap, int th_same_depth);
  bool    viewsynthesis_reverse_3Dwarp_ipel  (ImageType ***src, DepthType **pDepthMap, int th_same_depth);

  double        m_dWeightLeft;
  double        m_dWeightRight;
  double        WeightLeft;
  double        WeightRight;

  unsigned int  m_uiViewBlending; 
  unsigned int  ViewBlending; 

#ifdef POZNAN_DEPTH_BLEND
  int           m_iDepthBlendDiff;
#endif

#ifdef NICT_IVSRS
// NICT start  
  unsigned int  m_uiIvsrsInpaint; // NICT iVSRS inpaint flag for set get
// NICT end
#endif

  unsigned int  m_uiWidth;
  unsigned int  m_uiHeight;
  unsigned int  m_uiPicsize;
  unsigned int  m_uiPrecision;
  unsigned int  m_uiDepthType;
  unsigned char  m_ucLeftSide;

  unsigned int  m_uiColorSpace;
  unsigned int  m_uiBoundaryNoiseRemoval;

  std::string   CameraParameterFile;
  std::string   LeftCameraName;
  std::string   RightCameraName;
  std::string   VirtualCameraName;

//  unsigned int  Width;
//  unsigned int  Height;
//  unsigned int  Picsize;
//  unsigned int  Precision;
//  unsigned int  DepthType;

//  unsigned int  ColorSpace;

  double  Mat_Ex_Left[9];
  double  Mat_Ex_Virtual[9];
  double  Mat_Ex_Right[9];
  double  Mat_In_Left[9];
  double  Mat_In_Virtual[9];
  double  Mat_In_Right[9];
  double  Mat_Trans_Left[3];
  double  Mat_Trans_Virtual[3];
  double  Mat_Trans_Right[3];

  IplImage*  m_imgBlended;          //!> Blended image
// NICT start
#ifdef NICT_IVSRS
  IplImage*  m_imgBlendedDepth;     //!> Blended depth // NICT
#endif
// NICT end
  IplImage*  m_imgInterpolatedView; //!> The final image buffer to be output
  IplImage*  getImgInterpolatedView    ()    { return m_imgInterpolatedView; }

  double  m_dBaselineDistance;
  double  LeftBaselineDistance;
  double  RightBaselineDistance;

  DepthType**  m_pVirtualDepth;
  HoleType**  m_pSuccessSynthesis;

  //Nagoya start
  ImageType** m_pVirtualImageY;
  ImageType** m_pVirtualImageU;
  ImageType** m_pVirtualImageV;
  //Nagoya end

  IplImage*      m_imgVirtualImage;
  IplImage*      m_imgVirtualDepth;
  IplImage*      m_imgSuccessSynthesis;
  IplImage*      m_imgHoles;
  IplImage*      m_imgBound;
  IplImage*      m_imgMask[5];
  IplImage*      m_imgTemp[5];
  IplImage*      m_imgDepthTemp[5];
  IplConvKernel*    m_pConvKernel;

#ifdef POZNAN_GENERAL_HOMOGRAPHY
  CvMat*  m_matH_R2V;
  CvMat*  m_matH_V2R;

  double m_dTableD2Z[MAX_DEPTH];

#ifdef POZNAN_DEPTH_PROJECT2COMMON
  double m_dInvZNearMinusInvZFar;
  double m_dInvZfar;
#endif
#else
  CvMat*  m_matH_R2V[MAX_DEPTH];
  CvMat*  m_matH_V2R[MAX_DEPTH];
#endif

  int*  m_aiTableDisparity_ipel;
  int*  m_aiTableDisparity_subpel;

  bool (CViewInterpolationGeneral::*m_pFunc_ViewSynthesisReverse) (ImageType ***src, DepthType **pDepthMap, int th_same_depth) ; 
  bool (CViewInterpolationGeneral::*m_pFunc_DepthSynthesis) (DepthType **pDepthMap, ImageType ***src) ;

  // GIST added
  IplImage*      m_imgSynLeftforBNR;
  IplImage*      m_imgSynRightforBNR;
  IplImage*      m_imgDepthLeftforBNR;
  IplImage*      m_imgDepthRightforBNR;
  IplImage*      m_imgHoleLeftforBNR;  
  IplImage*      m_imgHoleRightforBNR;
  // GIST end

#ifdef _DEBUG
  unsigned char    m_ucSetup;
#endif
  
};

#endif

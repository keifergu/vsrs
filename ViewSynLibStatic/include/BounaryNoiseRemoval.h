/*
* This software module BNR(Boundary Noise Removal) was originally developed by 
* Gwangju Institute of Science and Technology (GIST) 
* in the course of development of the ISO/IEC JTC1/SC29 WG 11 (MPEG) 3D Video for reference 
* purposes and its performance may not have been optimized.
*
* Those intending to use this software module in products are advised that its use may infringe 
* existing patents. ISO/IEC have no liability for use of this software module or modifications thereof.
*
* Assurance that the originally developed software module can be used
*   (1) in the ISO/IEC JTC1/SC29 WG 11 (MPEG) 3D Video once the it is adopted to be used as reference 
*       software; and
*   (2) to develop the codec for ISO/IEC JTC1/SC29 WG 11 (MPEG) 3D Video.
*
* To the extent that GIST owns patent rights that would be required to 
* make, use, or sell the originally developed software module or portions thereof included in the ISO/IEC 
* JTC1/SC29 WG 11 (MPEG) 3D Video in a conforming product, GIST will assure the ISO/IEC that it 
* is willing to negotiate licenses under reasonable and non-discriminatory terms and conditions with 
* applicants throughout the world. 
*
* GIST  retains full right to modify and use the code for its own purpose, assign or donate the 
* code to a third party and to inhibit third parties from using the code for products that do not conform 
* to MPEG-related and/or ISO/IEC International Standards. 
*
* This copyright notice must be included in all copies or derivative works.
* Copyright (c) ISO/IEC 2009.
*
* Authors:
*      Cheon Lee,  leecheon@gist.ac.kr
*      Yo-Sung Ho, hoyo@gist.ac.kr
*/

#ifndef BOUNDARYNOISEREMOVAL_H
#define BOUNDARYNOISEREMOVAL_H

#pragma warning(disable:4996)
#pragma warning(disable:4244)
#pragma warning(disable:4819)

#ifndef WIN32
#define BYTE unsigned char
#endif

#include <iostream>
#include <fstream>

#include "yuv.h" 
#include "upsample.h"
#include "version.h"

typedef unsigned char uchar;
//using namespace std;

#ifndef UInt
#define UInt    unsigned int
#endif

#define max2(a,b)        (((a)-(b)>0)?(a):(b))
#define min2(a,b)        (((a)-(b)>0)?(b):(a))
#define guard(value, min, max)   ( max2((min), min2((max), (value)) ) )
#define BOUNDARY_WINDOW_SIZE 7
#define LEFTVIEW  0
#define RGHTVIEW  1



class CBoundaryNoiseRemoval
{
private:
  int Width;            
  int Height;
  int Precision;
  int ViewBlending;
  int ColorSpace;

  int DEPTH_TH;
  IplImage* m_imgSynWithHole;
  IplImage* m_imgBound;
  IplImage* m_imgBackBound;
  IplImage* m_imgTemp;
  IplImage* m_imgCheck;
  IplImage* m_imgExpandedHole;
  IplImage* m_imgDepth;
  IplImage* m_imgHoles;
  IplImage* m_imgCommonHole;
  IplImage* m_imgHoleOtherView;

#ifdef POZNAN_GENERAL_HOMOGRAPHY
  CvMat* matLeftH_V2R;  // for General Mode
  CvMat* matRightH_V2R;  // for General Mode
#else
  CvMat** matLeftH_V2R;  // for General Mode
  CvMat** matRightH_V2R;  // for General Mode
#endif
  double  LeftBaseLineDistance;
  double  RightBaseLineDistance;

  double FocalLength;  //!> focal length. it is assumed that all cameras share the same focal length
  double *LTranslation; //!> The camera distance from the left (0) and right (1) camera
  double *duPrincipal;  //!> The diff in principal point offset from left (0) and right (1) camera
  double *Znear;        //!> The real depth value of the nearest  pixel. 0: Left view.  1: Right view
  double *Zfar;         //!> The real depth value of the farthest pixel. 0: Left view.  1: Right view


public:
	CBoundaryNoiseRemoval();
	~CBoundaryNoiseRemoval();
	//void    getDepthThreshold(CvMat* matH_V2R[256], int Precision);
	bool    DoBoundaryNoiseRemoval(CIYuv<ImageType>* pRefLeft, CIYuv<ImageType>* pRefRight, CIYuv<DepthType>* pRefDepthLeft, CIYuv<DepthType>* pRefDepthRight, CIYuv<HoleType>* pRefHoleLeft, CIYuv<HoleType>* pRefHoleRight, CIYuv<ImageType>* pSynYuvBuffer, bool SynhtesisMode);
	void    xInit() ;

	void SetWidth             (int    iWidth)          { Width        = iWidth;        }
	void SetHeight            (int    iHieght)         { Height       = iHieght;       } 
	void SetViewBlending      (int    bViewBlending)   { ViewBlending = bViewBlending; }
	void SetColorSpace        (int    bColorSpace)     { ColorSpace   = bColorSpace;   }
	void SetPrecision         (int    iPrecision)      { Precision    = iPrecision;    }
#ifdef POZNAN_GENERAL_HOMOGRAPHY
  void SetLeftH_V2R         (CvMat *sH_V2R)          { matLeftH_V2R     = sH_V2R;    }
	void SetRightH_V2R        (CvMat *sH_V2R)          { matRightH_V2R    = sH_V2R;    }
#else
	void SetLeftH_V2R         (CvMat **sH_V2R)         { matLeftH_V2R     = sH_V2R;    }
	void SetRightH_V2R        (CvMat **sH_V2R)         { matRightH_V2R    = sH_V2R;    }
#endif
	void SetFocalLength       (double sFocalLength)    { FocalLength  = sFocalLength;  }
	void SetLTranslationLeft  (double *sLTranslation)  { LTranslation = sLTranslation; }
	void SetduPrincipal       (double *sduPrincipal)   { duPrincipal  = sduPrincipal;  }
	void SetZnear             (double *sZnear)         { Znear        = sZnear;        }
	void SetZfar              (double *sZfar)          { Zfar         = sZfar;         }

	void SetLeftBaseLineDist  (double sDist)           { LeftBaseLineDistance  = sDist; }
	void SetRightBaseLineDist  (double sDist)          { RightBaseLineDistance = sDist; }

	int  GetPrecision                  ()              { return Precision;             }       
	void calcDepthThreshold1DMode      (bool ViewID);
#ifdef POZNAN_GENERAL_HOMOGRAPHY
  void calcDepthThresholdGeneralMode (CvMat* matH_V2R);
#else
	void calcDepthThresholdGeneralMode (CvMat** matH_V2R);
#endif
  void copyImages                    (CIYuv<ImageType>* pSyn_CurrView, CIYuv<DepthType>* pSynDepth_CurrView, CIYuv<HoleType>* pSynHole_CurrView, CIYuv<HoleType>* pDepthHole_OthView);
	void getBoundaryContour            (IplImage* bound, IplImage* contour);
	bool checkFourNeighbours           (int i, int j, IplImage* check);  
	void getBackgroundContour          (IplImage* Bound, IplImage* Depth, IplImage* check_Depth, IplImage* BackBound);
	void expandedHoleforBNM            (IplImage* Depth, IplImage* Hole, IplImage* BackBound, IplImage* ExpandedHole); 
	void HoleFillingWithExpandedHole   (CIYuv<ImageType>* pSrc,   CIYuv<ImageType>* pTar,   IplImage* m_imgExpandedHole, bool SynthesisMode);
	void Blending                      (CIYuv<ImageType>* pLeft,  CIYuv<ImageType>* pRight, CIYuv<ImageType>* pSyn, bool SynthesisMode);
	void ColorFillingSmallHoleFor1DMode(CIYuv<ImageType>* pSrc,   CIYuv<ImageType>* pDst);
	void DepthFillingSmallHoleFor1DMode(CIYuv<DepthType>* pSrc,   CIYuv<DepthType>* pDst);
	void DepthMatchingWithColor        (CIYuv<DepthType>* pDepth, CIYuv<ImageType>* pColor, CIYuv<HoleType>* pDepthMask);
	void ColorHoleCleaning             (CIYuv<ImageType>* pSrc);	
  void RemainingHoleFilling_General  (CIYuv<ImageType>* pSrc);
  void RemainingHoleFilling_1DMode   (CIYuv<ImageType>* pSrc);

};

#endif

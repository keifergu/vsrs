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



#ifdef WIN32
#pragma warning(disable : 4996)
#endif

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>

#ifndef _OPEN_CV_HEADERS_
#define _OPEN_CV_HEADERS_
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <opencv\cvaux.h>
#endif

#include "BounaryNoiseRemoval.h"

CBoundaryNoiseRemoval::CBoundaryNoiseRemoval()
{
  Width = 1024;            
  Height= 768;
  DEPTH_TH = 5;
  Precision = 1;

  FocalLength = NULL;
  LTranslation= NULL;
  duPrincipal = NULL;
  Znear       = NULL;
  Zfar        = NULL;


  m_imgSynWithHole    = NULL;
  m_imgBound          = NULL;
  m_imgBackBound      = NULL;
  m_imgTemp           = NULL;
  m_imgCheck          = NULL;
  m_imgExpandedHole   = NULL;
  m_imgDepth          = NULL;
  m_imgHoles          = NULL;
  m_imgCommonHole     = NULL;
  m_imgHoleOtherView  = NULL;

}

CBoundaryNoiseRemoval::~CBoundaryNoiseRemoval()
{

  if (m_imgSynWithHole    != NULL)    { cvReleaseImage(&m_imgSynWithHole);   }
  if (m_imgBound          != NULL)    { cvReleaseImage(&m_imgBound);         }
  if (m_imgBackBound      != NULL)    { cvReleaseImage(&m_imgBackBound);     }
  if (m_imgTemp           != NULL)    { cvReleaseImage(&m_imgTemp);          }
  if (m_imgCheck          != NULL)    { cvReleaseImage(&m_imgCheck);         }
  if (m_imgExpandedHole   != NULL)    { cvReleaseImage(&m_imgExpandedHole);  }
  if (m_imgDepth          != NULL)    { cvReleaseImage(&m_imgDepth);         }
  if (m_imgHoles          != NULL)    { cvReleaseImage(&m_imgHoles);         }
  if (m_imgCommonHole     != NULL)    { cvReleaseImage(&m_imgCommonHole);    }
  if (m_imgHoleOtherView  != NULL)    { cvReleaseImage(&m_imgHoleOtherView); }
}

void CBoundaryNoiseRemoval::xInit() 
{
  if (m_imgSynWithHole    == NULL)    { m_imgSynWithHole    = cvCreateImage(cvSize(Width*Precision, Height), 8, 3);}
  if (m_imgBound          == NULL)    { m_imgBound          = cvCreateImage(cvSize(Width*Precision, Height), 8, 1);}
  if (m_imgBackBound      == NULL)    { m_imgBackBound      = cvCreateImage(cvSize(Width*Precision, Height), 8, 1);}
  if (m_imgTemp           == NULL)    { m_imgTemp           = cvCreateImage(cvSize(Width*Precision, Height), 8, 1);}
  if (m_imgCheck          == NULL)    { m_imgCheck          = cvCreateImage(cvSize(Width*Precision, Height), 8, 1);}
  if (m_imgExpandedHole   == NULL)    { m_imgExpandedHole   = cvCreateImage(cvSize(Width*Precision, Height), 8, 1);}
  if (m_imgDepth          == NULL)    { m_imgDepth          = cvCreateImage(cvSize(Width*Precision, Height), 8, 1);}
  if (m_imgHoles          == NULL)    { m_imgHoles          = cvCreateImage(cvSize(Width*Precision, Height), 8, 1);}
  if (m_imgCommonHole     == NULL)    { m_imgCommonHole     = cvCreateImage(cvSize(Width*Precision, Height), 8, 1);}
  if (m_imgHoleOtherView  == NULL)    { m_imgHoleOtherView  = cvCreateImage(cvSize(Width*Precision, Height), 8, 1);}

}

bool CBoundaryNoiseRemoval::DoBoundaryNoiseRemoval(CIYuv<ImageType>* pRefLeft, CIYuv<ImageType>* pRefRight, CIYuv<DepthType>* pRefDepthLeft, CIYuv<DepthType>* pRefDepthRight, 
                           CIYuv<HoleType>* pRefHoleLeft, CIYuv<HoleType>* pRefHoleRight, CIYuv<ImageType>* pSynYuvBuffer, bool SynhtesisMode)
{

  int i;
  HoleType *LeftHole, *RightHole;

  xInit();
  LeftHole  = pRefHoleLeft->getBuffer();
  RightHole = pRefHoleRight->getBuffer();
  cvZero(m_imgCommonHole);

  for (i=0;i<pRefHoleLeft->getWidth() * pRefHoleLeft->getHeight();i++) {
    if (LeftHole[i] == (MaxTypeValue<HoleType>()-1) && RightHole[i] == (MaxTypeValue<HoleType>()-1)) {
      m_imgCommonHole->imageData[i] = (MaxTypeValue<HoleType>()-1);
    }
  }
  

  // Left
  if (SynhtesisMode) {   

    calcDepthThreshold1DMode(LEFTVIEW);  // 1D Mode
    copyImages(pRefLeft, pRefDepthLeft, pRefHoleLeft, pRefHoleRight);
    getBoundaryContour(m_imgHoles, m_imgBound);
    getBackgroundContour(m_imgBound, m_imgDepth, m_imgHoles, m_imgBackBound);  
    expandedHoleforBNM(m_imgDepth, m_imgHoles, m_imgBackBound, m_imgTemp);  
    cvOr(m_imgHoles, m_imgTemp, m_imgExpandedHole);    
    HoleFillingWithExpandedHole(pRefRight, pRefLeft, m_imgExpandedHole, SynhtesisMode);
    RemainingHoleFilling_1DMode(pRefLeft);
  }
  else {
    calcDepthThresholdGeneralMode(matLeftH_V2R);  // General Mode
    copyImages(pRefLeft, pRefDepthLeft, pRefHoleLeft, pRefHoleRight);
    getBoundaryContour(m_imgHoles, m_imgBound);
    getBackgroundContour(m_imgBound, m_imgDepth, m_imgHoles, m_imgBackBound);  
    expandedHoleforBNM(m_imgDepth, m_imgHoles, m_imgBackBound, m_imgTemp);  
    cvOr(m_imgHoles, m_imgTemp, m_imgExpandedHole);
    HoleFillingWithExpandedHole(pRefRight, pRefLeft, m_imgExpandedHole, SynhtesisMode);
    RemainingHoleFilling_General(pRefLeft);    
  }

  // Right
  if (SynhtesisMode) { 
    calcDepthThreshold1DMode(RGHTVIEW);  // 1D Mode
    copyImages(pRefRight, pRefDepthRight, pRefHoleRight, pRefHoleLeft);  
    getBoundaryContour(m_imgHoles, m_imgBound);
    getBackgroundContour(m_imgBound, m_imgDepth, m_imgHoles, m_imgBackBound);  
    expandedHoleforBNM(m_imgDepth, m_imgHoles, m_imgBackBound, m_imgTemp);  
    cvOr(m_imgHoles, m_imgTemp, m_imgExpandedHole);
    HoleFillingWithExpandedHole(pRefLeft, pRefRight, m_imgExpandedHole, SynhtesisMode);
    RemainingHoleFilling_1DMode(pRefRight);
  }
  else {
    calcDepthThresholdGeneralMode(matLeftH_V2R);  // General Mode
    copyImages(pRefRight, pRefDepthRight, pRefHoleRight, pRefHoleLeft);  
    getBoundaryContour(m_imgHoles, m_imgBound);
    getBackgroundContour(m_imgBound, m_imgDepth, m_imgHoles, m_imgBackBound);  
    expandedHoleforBNM(m_imgDepth, m_imgHoles, m_imgBackBound, m_imgTemp);  
    cvOr(m_imgHoles, m_imgTemp, m_imgExpandedHole);
    HoleFillingWithExpandedHole(pRefLeft, pRefRight, m_imgExpandedHole, SynhtesisMode);
    RemainingHoleFilling_General(pRefRight);
  }

  Blending(pRefLeft, pRefRight, pSynYuvBuffer, SynhtesisMode);

  return false;
}





void CBoundaryNoiseRemoval::RemainingHoleFilling_General(CIYuv<ImageType>* pSrc)
{
  int i, j, tWidth, tHeight, CountHole;
  bool isValidLeft, isValidRight, isValid_Y, isValid_U, isValid_V, isComHole;
  ImageType* buffer;
  tWidth = pSrc->getWidth();
  tHeight= pSrc->getHeight();
  buffer = pSrc->getBuffer();

  for (j=0;j<tHeight;j++) {
    for (i=0;i<tWidth;i++) {

      buffer[j*tWidth*3 + i*3 + 0] ? isValid_Y = true : isValid_Y = false;
      buffer[j*tWidth*3 + i*3 + 1] ? isValid_U = true : isValid_U = false;
      buffer[j*tWidth*3 + i*3 + 2] ? isValid_V = true : isValid_V = false;      
      m_imgCommonHole->imageData[j*tWidth + i] ? isComHole = true : isComHole = false; 
      if (!isValid_Y || !isValid_U || !isValid_V || isComHole) {
        buffer[j*tWidth*3 + i*3 + 0] = 0;
        buffer[j*tWidth*3 + i*3 + 1] = 0;
        buffer[j*tWidth*3 + i*3 + 2] = 0;

      }
    }
  }

  CountHole = 0;
  for (j=0;j<tHeight;j++) {
    for (i=0;i<tWidth;i++) {
      if (buffer[j*tWidth*3 + i*3] == 0) {
        CountHole++;
      }
    }
  }

  while (CountHole) {
    for (j=0;j<tHeight;j++) {
      for (i=0;i<tWidth;i++) {

        if(i==0  && buffer[j*tWidth*3 + i*3 + 0] == 0) {
          isValidLeft  = false;
          buffer[j*tWidth*3 + (i+1)*3 + 0] && buffer[j*tWidth*3 + (i+1)*3 + 1] && buffer[j*tWidth*3 + (i+1)*3 + 2] ? isValidRight = true : isValidRight = false;          

          if (!isValidLeft && isValidRight) {
            buffer[j*tWidth*3 + i*3 + 0] = buffer[j*tWidth*3 + (i+1)*3 + 0];
            buffer[j*tWidth*3 + i*3 + 1] = buffer[j*tWidth*3 + (i+1)*3 + 1];
            buffer[j*tWidth*3 + i*3 + 2] = buffer[j*tWidth*3 + (i+1)*3 + 2];
            CountHole--;
          }
        }
        else if (i==tWidth-1 && buffer[j*tWidth*3 + i*3 + 0] == 0) {
          buffer[j*tWidth*3 + (i-1)*3 + 0] && buffer[j*tWidth*3 + (i-1)*3 + 1] && buffer[j*tWidth*3 + (i-1)*3 + 2] ? isValidLeft  = true : isValidLeft  = false;
          isValidRight = false;

          if (isValidLeft && !isValidRight) {
            buffer[j*tWidth*3 + i*3 + 0] = buffer[j*tWidth*3 + (i-1)*3 + 0];
            buffer[j*tWidth*3 + i*3 + 1] = buffer[j*tWidth*3 + (i-1)*3 + 1];
            buffer[j*tWidth*3 + i*3 + 2] = buffer[j*tWidth*3 + (i-1)*3 + 2];
            CountHole--;
          }
        }
        else if (buffer[j*tWidth*3 + i*3 + 0] == 0) {
          buffer[j*tWidth*3 + (i-1)*3 + 0] ? isValidLeft  = true : isValidLeft  = false;
          buffer[j*tWidth*3 + (i+1)*3 + 0] ? isValidRight = true : isValidRight = false;
          if (isValidLeft && isValidRight) {
            buffer[j*tWidth*3 + i*3 + 0] = guard((buffer[j*tWidth*3 + (i-1)*3 + 0] + buffer[j*tWidth*3 + (i+1)*3 + 0])/2 +0.5, 0, MaxTypeValue<ImageType>()-1);
            buffer[j*tWidth*3 + i*3 + 1] = guard((buffer[j*tWidth*3 + (i-1)*3 + 1] + buffer[j*tWidth*3 + (i+1)*3 + 1])/2 +0.5, 0, MaxTypeValue<ImageType>()-1);
            buffer[j*tWidth*3 + i*3 + 2] = guard((buffer[j*tWidth*3 + (i-1)*3 + 2] + buffer[j*tWidth*3 + (i+1)*3 + 2])/2 +0.5, 0, MaxTypeValue<ImageType>()-1);
            CountHole--;
          }
          else if (isValidLeft && !isValidRight) {
            buffer[j*tWidth*3 + i*3 + 0] = buffer[j*tWidth*3 + (i-1)*3 + 0];
            buffer[j*tWidth*3 + i*3 + 1] = buffer[j*tWidth*3 + (i-1)*3 + 1];
            buffer[j*tWidth*3 + i*3 + 2] = buffer[j*tWidth*3 + (i-1)*3 + 2];
            CountHole--;
          }
          else if (!isValidLeft && isValidRight) {
            buffer[j*tWidth*3 + i*3 + 0] = buffer[j*tWidth*3 + (i+1)*3 + 0];
            buffer[j*tWidth*3 + i*3 + 1] = buffer[j*tWidth*3 + (i+1)*3 + 1];
            buffer[j*tWidth*3 + i*3 + 2] = buffer[j*tWidth*3 + (i+1)*3 + 2];
            CountHole--;
          }
        }
      }
    }
  }

  //FILE *fp = fopen("pSrc.yuv", "wb");
  //fwrite(pSrc->getBuffer(), 1, tWidth*tHeight*3, fp);
  //fclose(fp);


}


void CBoundaryNoiseRemoval::RemainingHoleFilling_1DMode(CIYuv<ImageType>* pSrc)
{
  int i, j, tWidth, tHeight, CountHole;
  bool isValidLeft, isValidRight, isValid_Y, isValid_U, isValid_V;
  ImageType* buffer;
  tWidth = pSrc->getWidth();
  tHeight= pSrc->getHeight();
  buffer = pSrc->getBuffer();

  for (j=0;j<tHeight;j++) {
    for (i=0;i<tWidth;i++) {

      buffer[j*tWidth*3 + i*3 + 0] ? isValid_Y = true : isValid_Y = false;
      buffer[j*tWidth*3 + i*3 + 1] ? isValid_U = true : isValid_U = false;
      buffer[j*tWidth*3 + i*3 + 2] ? isValid_V = true : isValid_V = false;  	  
      if (!isValid_Y || !isValid_U || !isValid_V) {
        buffer[j*tWidth*3 + i*3 + 0] = 0;
        buffer[j*tWidth*3 + i*3 + 1] = 0;
        buffer[j*tWidth*3 + i*3 + 2] = 0;
      }
    }
  }

  CountHole = 0;
  for (j=0;j<tHeight;j++) {
    for (i=0;i<tWidth;i++) {
      if (buffer[ j*tWidth + i  ] == 0) {
        CountHole++;
      }
    }
  }

  while (CountHole) {
    for (j=0;j<tHeight;j++) {
      for (i=0;i<tWidth;i++) {
        if      (i==0        && buffer[tWidth*tHeight*0 + j*tWidth + i  ] == 0) {
          isValidLeft  = false;
          buffer[tWidth*tHeight*0 + j*tWidth + i+1] ? isValidRight = true : isValidRight = false;          

          if (!isValidLeft && isValidRight) {
            buffer[tWidth*tHeight*0 + j*tWidth + i] = buffer[tWidth*tHeight*0 + j*tWidth + i+1];
            buffer[tWidth*tHeight*1 + j*tWidth + i] = buffer[tWidth*tHeight*1 + j*tWidth + i+1];
            buffer[tWidth*tHeight*2 + j*tWidth + i] = buffer[tWidth*tHeight*2 + j*tWidth + i+1];
            CountHole--;
          }
        }
        else if (i==tWidth-1 && buffer[tWidth*tHeight*0 + j*tWidth + i  ] == 0) {
          buffer[tWidth*tHeight*0 + j*tWidth + i-1] ? isValidLeft  = true : isValidLeft  = false;
          isValidRight = false;

          if (isValidLeft && !isValidRight) {
            buffer[tWidth*tHeight*0 + j*tWidth + i] = buffer[tWidth*tHeight*0 + j*tWidth + i-1];
            buffer[tWidth*tHeight*1 + j*tWidth + i] = buffer[tWidth*tHeight*1 + j*tWidth + i-1];
            buffer[tWidth*tHeight*2 + j*tWidth + i] = buffer[tWidth*tHeight*2 + j*tWidth + i-1];
            CountHole--;
          }
        }
        else if (buffer[tWidth*tHeight*0 + j*tWidth + i  ] == 0) {
          buffer[tWidth*tHeight*0 + j*tWidth + i-1] ? isValidLeft  = true : isValidLeft  = false;
          buffer[tWidth*tHeight*0 + j*tWidth + i+1] ? isValidRight = true : isValidRight = false;          

          if (isValidLeft && isValidRight) {
            buffer[tWidth*tHeight*0 + j*tWidth + i] = guard((buffer[tWidth*tHeight*0 + j*tWidth + i-1] + buffer[tWidth*tHeight*0 + j*tWidth + i+1])/2 +0.5, 0, MaxTypeValue<ImageType>()-1);
            buffer[tWidth*tHeight*1 + j*tWidth + i] = guard((buffer[tWidth*tHeight*1 + j*tWidth + i-1] + buffer[tWidth*tHeight*1 + j*tWidth + i+1])/2 +0.5, 0, MaxTypeValue<ImageType>()-1);
            buffer[tWidth*tHeight*2 + j*tWidth + i] = guard((buffer[tWidth*tHeight*2 + j*tWidth + i-1] + buffer[tWidth*tHeight*2 + j*tWidth + i+1])/2 +0.5, 0, MaxTypeValue<ImageType>()-1);
            CountHole--;
          }
          else if (isValidLeft && !isValidRight) {
            buffer[tWidth*tHeight*0 + j*tWidth + i] = buffer[tWidth*tHeight*0 + j*tWidth + i-1];
            buffer[tWidth*tHeight*1 + j*tWidth + i] = buffer[tWidth*tHeight*1 + j*tWidth + i-1];
            buffer[tWidth*tHeight*2 + j*tWidth + i] = buffer[tWidth*tHeight*2 + j*tWidth + i-1];
            CountHole--;
          }
          else if (!isValidLeft && isValidRight) {
            buffer[tWidth*tHeight*0 + j*tWidth + i] = buffer[tWidth*tHeight*0 + j*tWidth + i+1];
            buffer[tWidth*tHeight*1 + j*tWidth + i] = buffer[tWidth*tHeight*1 + j*tWidth + i+1];
            buffer[tWidth*tHeight*2 + j*tWidth + i] = buffer[tWidth*tHeight*2 + j*tWidth + i+1];
            CountHole--;
          }
        }


      }
    }
  }

}


void CBoundaryNoiseRemoval::Blending(CIYuv<ImageType>* pLeft,CIYuv<ImageType>* pRight, CIYuv<ImageType>* pSyn, bool SynhtesisMode)
{
  int i, j;
  double WeightForLeft, WeightForRight;
  WeightForLeft = WeightForRight = 0.0;

  // Calculating Weighting Factors
  if (SynhtesisMode) {  // 1D Mode
    if (LTranslation[LEFTVIEW] <= LTranslation[RGHTVIEW]) {
      WeightForLeft  = fabs(1.0 - (fabs(LTranslation[LEFTVIEW])/(fabs(LTranslation[LEFTVIEW]) + fabs(LTranslation[RGHTVIEW]))));
      WeightForRight = fabs(1.0 - (fabs(LTranslation[RGHTVIEW])/(fabs(LTranslation[LEFTVIEW]) + fabs(LTranslation[RGHTVIEW]))));
    }
    else {
      WeightForLeft  = fabs(1.0 - (fabs(LTranslation[RGHTVIEW])/(fabs(LTranslation[LEFTVIEW]) + fabs(LTranslation[RGHTVIEW]))));
      WeightForRight = fabs(1.0 - (fabs(LTranslation[LEFTVIEW])/(fabs(LTranslation[LEFTVIEW]) + fabs(LTranslation[RGHTVIEW]))));
    }

  }
  else {                // General Mode
    if (ViewBlending) {
      if (LeftBaseLineDistance <= RightBaseLineDistance) {
        WeightForLeft  = 1.0;
        WeightForRight = 0.0;
      }
      else {
        WeightForLeft  = 0.0;
        WeightForRight = 1.0;
      }
    }
    else {
      if (LeftBaseLineDistance <= RightBaseLineDistance) {
        WeightForLeft  = fabs(1.0 - (fabs(LeftBaseLineDistance) /(fabs(LeftBaseLineDistance) + fabs(RightBaseLineDistance))));
        WeightForRight = fabs(1.0 - (fabs(RightBaseLineDistance)/(fabs(LeftBaseLineDistance) + fabs(RightBaseLineDistance))));
      }
      else {
        WeightForLeft  = fabs(1.0 - (fabs(RightBaseLineDistance)/(fabs(LeftBaseLineDistance) + fabs(RightBaseLineDistance))));
        WeightForRight = fabs(1.0 - (fabs(LeftBaseLineDistance) /(fabs(LeftBaseLineDistance) + fabs(RightBaseLineDistance))));
      }
    }

  }



  //if (ViewBlending) {  // No Blending

  //  if (SynhtesisMode) {  // 1D Mode
  //    if (LTranslation[LEFTVIEW] <= LTranslation[RGHTVIEW]) {
  //      WeightForLeft  = 1.0;
  //      WeightForRight = 0.0;
  //    }
  //    else {
  //      WeightForLeft  = 0.0;
  //      WeightForRight = 1.0;
  //    }
  //  }
  //  else {
  //    if (LeftBaseLineDistance <= RightBaseLineDistance) {
  //      WeightForLeft  = 1.0;
  //      WeightForRight = 0.0;
  //    }
  //    else {
  //      WeightForLeft  = 0.0;
  //      WeightForRight = 1.0;
  //    }
  //  }
  //}
  //else {               // Blend with both synthesized images
  //  if (SynhtesisMode) {  // 1D Mode
  //    if (LTranslation[LEFTVIEW] <= LTranslation[RGHTVIEW]) {
  //      WeightForLeft  = fabs(1.0 - (fabs(LTranslation[LEFTVIEW])/(fabs(LTranslation[LEFTVIEW]) + fabs(LTranslation[RGHTVIEW]))));
  //      WeightForRight = fabs(1.0 - (fabs(LTranslation[RGHTVIEW])/(fabs(LTranslation[LEFTVIEW]) + fabs(LTranslation[RGHTVIEW]))));
  //    }
  //    else {
  //      WeightForLeft  = fabs(1.0 - (fabs(LTranslation[RGHTVIEW])/(fabs(LTranslation[LEFTVIEW]) + fabs(LTranslation[RGHTVIEW]))));
  //      WeightForRight = fabs(1.0 - (fabs(LTranslation[LEFTVIEW])/(fabs(LTranslation[LEFTVIEW]) + fabs(LTranslation[RGHTVIEW]))));
  //    }
  //  }
  //  else {                // General Mode
  //    if (LeftBaseLineDistance <= RightBaseLineDistance) {
  //      WeightForLeft  = fabs(1.0 - (fabs(LeftBaseLineDistance) /(fabs(LeftBaseLineDistance) + fabs(RightBaseLineDistance))));
  //      WeightForRight = fabs(1.0 - (fabs(RightBaseLineDistance)/(fabs(LeftBaseLineDistance) + fabs(RightBaseLineDistance))));
  //    }
  //    else {
  //      WeightForLeft  = fabs(1.0 - (fabs(RightBaseLineDistance)/(fabs(LeftBaseLineDistance) + fabs(RightBaseLineDistance))));
  //      WeightForRight = fabs(1.0 - (fabs(LeftBaseLineDistance) /(fabs(LeftBaseLineDistance) + fabs(RightBaseLineDistance))));
  //    }
  //  }
  //}

  CIYuv<ImageType> temp1, temp2;
  IplImage *TempImage;
  CPictureResample<ImageType> Resampling;
  ImageType* LeftBuffer, *RightBuffer, *SrcBuffer, *DstBuffer;

  if (SynhtesisMode) {   // 1D Mode
    int tWidth, tHeight;
    ImageType *Src, *Dst;
    tWidth = Width*Precision;
    tHeight= Height;
    temp1.resize(tHeight, tWidth, 444);
    temp2.resize( Height,  Width, 444);

    // Blending
    LeftBuffer  = pLeft ->getBuffer();
    RightBuffer = pRight->getBuffer();
    Dst         = temp1.getBuffer();
    for (i=0;i<tWidth*tHeight*3;i++) {
      Dst[i] = (ImageType)guard( (WeightForLeft* LeftBuffer[i] + WeightForRight*RightBuffer[i] + 0.5), 0, MaxTypeValue<ImageType>()-1);
    }

    // DownSampling
    SrcBuffer  = temp1.getBuffer();
    Dst        = temp2.getBuffer();

    if (Precision == 1) {
      memcpy(Dst, SrcBuffer, tWidth*tHeight*3);
    }
    else {   // Half-pel, Quarter-pel
      Resampling.DownsampleView(&Dst[Width*Height*0], &SrcBuffer[tWidth*tHeight*0], tWidth, Height, Precision);  // Y
      Resampling.DownsampleView(&Dst[Width*Height*1], &SrcBuffer[tWidth*tHeight*1], tWidth, Height, Precision);  // U
      Resampling.DownsampleView(&Dst[Width*Height*2], &SrcBuffer[tWidth*tHeight*2], tWidth, Height, Precision);  // V
    }


    SrcBuffer  = temp2.getBuffer();
    Dst        = pSyn->getBuffer();
    memcpy(Dst, SrcBuffer, Width*Height);
    Dst        = &Dst      [Width*Height];
    SrcBuffer  = &SrcBuffer[Width*Height];
    for (j=0;j<Height/2;j++) {
      for (i=0;i<Width/2;i++) {
        Dst[j*Width/2+i] = SrcBuffer[(j*2)*Width+(i*2)];
      }
    }
    Dst        = &Dst      [Width*Height/4];
    SrcBuffer  = &SrcBuffer[Width*Height  ];
    for (j=0;j<Height/2;j++) {
      for (i=0;i<Width/2;i++) {
        Dst[j*Width/2+i] = SrcBuffer[(j*2)*Width+(i*2)];
      }
    }


  }
  else {  // General Mode
    temp1.resize(Height, Width, 444);
    DstBuffer  = temp1.getBuffer();
    LeftBuffer = pLeft->getBuffer();
    RightBuffer= pRight->getBuffer();

    for (i=0;i<Width*Height*3;i++) {
      DstBuffer[i] = (ImageType)guard((double)WeightForLeft* (double)LeftBuffer[i] + (double)WeightForRight*(double)RightBuffer[i], 0, MaxTypeValue<ImageType>()-1);
    }
    TempImage = cvCreateImage(cvSize(Width, Height), 8, 3);
    if (ColorSpace) {   // BGR
      memcpy(TempImage->imageData, temp1.getBuffer(), Width*Height*3);
      pSyn->setDataFromImgBGR(TempImage);
    }
    else {              // YUV
      memcpy(TempImage->imageData, temp1.getBuffer(), Width*Height*3);
      pSyn->setDataFromImgYUV(TempImage);
    }
  }

}

void CBoundaryNoiseRemoval::calcDepthThreshold1DMode(bool ViewID)
{

  int Low, SumOfGap, index, GapCount, Start_D, End_D, z;
  double posStart, posEnd, GapWidth, dk;

  index = 0;
  Low = 0;
  GapCount = 0;
  SumOfGap = 0;
  posStart = posEnd = 0.0;
  GapWidth = 0.0;
  Start_D = End_D = 0;

  for (index=1;index<MAX_DEPTH;index++)  {
    while (GapWidth < 3) {
      if (++index > (MAX_DEPTH-1)) {
        break;
      }
      z = 1.0 / ( (index/(MaxTypeValue<DepthType>()-1)) * (1/Znear[ViewID] - 1/Zfar[ViewID]) + (1/Zfar[ViewID]) );
      posEnd = (FocalLength * LTranslation[ViewID] / z) - duPrincipal[ViewID];
      GapWidth = fabs(posEnd - posStart);
      End_D = index;         
    }
    SumOfGap += abs(End_D - Start_D);
    GapCount++;
    Start_D = End_D;
    posStart = posEnd;
    GapWidth = fabs(posEnd - posStart);
  }
  DEPTH_TH = (int)(SumOfGap/GapCount + 0.5);  
}

#ifdef POZNAN_GENERAL_HOMOGRAPHY
void CBoundaryNoiseRemoval::calcDepthThresholdGeneralMode(CvMat* matH_V2R)
#else
void CBoundaryNoiseRemoval::calcDepthThresholdGeneralMode(CvMat** matH_V2R)
#endif
{

  int Low, SumOfGap, index, GapCount, Start_D, End_D;
  float posStart, posEnd, GapWidth;  
  index = 0;
  Low = 0;
  GapCount = 0;
  SumOfGap = 0;
  posStart = posEnd = 0.0;
  GapWidth = 0.0;
#ifdef POZNAN_GENERAL_HOMOGRAPHY
  CvMat* m = cvCreateMat(4, 1, CV_64F);
  CvMat* mv = cvCreateMat(4, 1, CV_64F);
  cvmSet(mv, 0, 0, 0);
  cvmSet(mv, 1, 0, 0);
  cvmSet(mv, 2, 0, 1);
  cvmSet(mv, 2, 0, 1);
  cvmMul(matH_V2R, mv, m);
#else
  CvMat* m = cvCreateMat(3, 1, CV_64F);
  CvMat* mv = cvCreateMat(3, 1, CV_64F);
  cvmSet(mv, 0, 0, 0);
  cvmSet(mv, 1, 0, 0);
  cvmSet(mv, 2, 0, 1);
  cvmMul(matH_V2R[0], mv, m);
#endif
  posStart = m->data.db[0] * Precision / m->data.db[2] + 0.5;
  posEnd   = m->data.db[0] * Precision / m->data.db[2] + 0.5;
  GapWidth = fabs(posEnd - posStart);
  Start_D = End_D = 0;

  for (index=1;index<MAX_DEPTH;index++)  {
    while (GapWidth < 3) {
      if (++index > (MAX_DEPTH-1)) {
        break;
      }
#ifdef POZNAN_GENERAL_HOMOGRAPHY
      cvmSet(mv, 3, 0, 1.0/index);//TableD2Z
      cvmMul(matH_V2R, mv, m);
#else
      cvmMul(matH_V2R[index], mv, m);
#endif
      posEnd = m->data.db[0] * Precision / m->data.db[2] + 0.5;
      GapWidth = fabs(posEnd - posStart);
      End_D = index;         
    }
    SumOfGap += abs(End_D - Start_D);
    GapCount++;
    Start_D = End_D;
    posStart = posEnd;
    GapWidth = fabs(posEnd - posStart);
  }
  DEPTH_TH = (int)(SumOfGap/GapCount + 0.5);  
}


void CBoundaryNoiseRemoval::HoleFillingWithExpandedHole(CIYuv<ImageType>* pSrc, CIYuv<ImageType>* pTar, IplImage* m_imgExpandedHole, bool SynthesisMode)
{
  int i, j, tWidth, tHeight;
  BYTE* Src_buffer, *Tar_buffer;

  Src_buffer = pSrc->getBuffer();
  Tar_buffer = pTar->getBuffer();

  if (SynthesisMode) {     // 1D Mode
    tWidth = Width*Precision;
    tHeight= Height;
    for (j=0;j<tHeight;j++) {     // Y
      for (i=0;i<tWidth;i++) {
        if ((uchar)m_imgExpandedHole->imageData[j*tWidth +i] == MAX_HOLE-1 && (uchar)m_imgCommonHole->imageData[j*tWidth +i] == 0) {
          Tar_buffer[tWidth*tHeight*0 + j*tWidth + i] = Src_buffer[tWidth*tHeight*0 + j*tWidth + i];
          Tar_buffer[tWidth*tHeight*1 + j*tWidth + i] = Src_buffer[tWidth*tHeight*1 + j*tWidth + i];
          Tar_buffer[tWidth*tHeight*2 + j*tWidth + i] = Src_buffer[tWidth*tHeight*2 + j*tWidth + i];
          m_imgExpandedHole->imageData[j*tWidth +i] = 0;
        }
      }
    }

  }
  else {                 // General Mode
    for (j=0;j<Height;j++) {
      for (i=0;i<Width;i++) {

        if ((uchar)m_imgExpandedHole->imageData[j*Width +i] == MAX_HOLE-1) {
          Tar_buffer[j*Width*3 + i*3 + 0] = Src_buffer[j*Width*3 + i*3 + 0];
          Tar_buffer[j*Width*3 + i*3 + 1] = Src_buffer[j*Width*3 + i*3 + 1];
          Tar_buffer[j*Width*3 + i*3 + 2] = Src_buffer[j*Width*3 + i*3 + 2];
          m_imgExpandedHole->imageData[j*Width +i] = 0;
        }
      }
    }

  }
}

void CBoundaryNoiseRemoval::copyImages(CIYuv<ImageType>* pSyn_CurrView, CIYuv<DepthType>* pSynDepth_CurrView, CIYuv<HoleType>* pSynHole_CurrView, CIYuv<HoleType>* pDepthHole_OthView)
{
  int i, j, width, height;
  ImageType *org_buffer_image;
  char *tar_buffer_image;

  DepthType *org_buffer_depth;
  char *tar_buffer_depth;

  HoleType *org_buffer_hole;
  char *tar_buffer_hole;

  cvZero(m_imgSynWithHole);
  cvZero(m_imgDepth);
  cvZero(m_imgHoles);
  cvZero(m_imgHoleOtherView);

  width = m_imgHoles->width;
  height= m_imgHoles->height;

  org_buffer_image = pSyn_CurrView->getBuffer();
  tar_buffer_image = m_imgSynWithHole->imageData;
  for (i=0;i<width*height*3; i++) {
    tar_buffer_image[i] = org_buffer_image[i];
  }

  org_buffer_depth = pSynDepth_CurrView->getBuffer();
  tar_buffer_depth = m_imgDepth->imageData;
  for (i=0;i<width*height; i++) {
    tar_buffer_depth[i] = org_buffer_depth[i];
  }

  org_buffer_hole = pSynHole_CurrView->getBuffer();
  tar_buffer_hole = m_imgHoles->imageData;
  for (i=0;i<width*height; i++) {
    tar_buffer_hole[i] = org_buffer_hole[i];
  }

  org_buffer_hole = pDepthHole_OthView->getBuffer();
  tar_buffer_hole = m_imgHoleOtherView->imageData;
  for (i=0;i<width*height; i++) {
    tar_buffer_hole[i] = org_buffer_hole[i];
  }

}


void CBoundaryNoiseRemoval::getBoundaryContour(IplImage* bound, IplImage* contour) 
{
  int i, j, k, width, height, Hole_width, posLeft, posRight;
  width = bound->width;    
  height= bound->height;

  for (j=0;j<height;j++){
    for (i=0;i<width;i++){
      if (bound->imageData[j*width+i] && checkFourNeighbours(i, j, bound)){
        contour->imageData[j*width+i] = 255;        
      }      
      else {
        contour->imageData[j*width+i] = 0;
      }
    }
  }
}

bool CBoundaryNoiseRemoval::checkFourNeighbours(int i, int j, IplImage* check) 
{
  int width = check->width;
  int height= check->height;

  // left (i-1, j)
  if (i-1 > 0 && check->imageData[j*width+i] && !check->imageData[j*width+i-1]){
    return true;
  }


  // right (i+1, j)
  if (i+1 < width -1 && check->imageData[j*width+i] && !check->imageData[j*width+i+1]){
    return true;
  }

  // up (i, j-1)
  if (j-1 > 0 && check->imageData[j*width+i] && !check->imageData[(j-1)*width+i]){
    return true;
  }

  // down (i, j+1)
  if (j+1 < height -1 && check->imageData[j*width+i] && !check->imageData[(j+1)*width+i]){
    return true;
  }

  return false;
}


void CBoundaryNoiseRemoval::getBackgroundContour(IplImage* Bound, IplImage* Depth, IplImage* check_Depth, IplImage* BackBound) 
{

  int i, j, k, width, height;
  int posStart, posEnd, Left_D, Right_D;  
  bool bDiff;

  width = Bound->width;
  height= Bound->height;  
  cvZero(BackBound);

  for (j=0;j<height;j++){
    for (i=0;i<width;i++){      

      if (i-1<0 || i+1 > width-1){
        continue;
      }

      if ((uchar)Bound->imageData[j*width+i]==255) {
        posStart = i;
        Left_D = (uchar)Depth->imageData[j*width+i-1];
        i = posStart;
        while ((uchar)check_Depth->imageData[j*width+i] && i < width){
          i++;                    
        }

        Right_D= (uchar)Depth->imageData[j*width+i+1];
        posEnd = i;

        abs(Left_D- Right_D) > DEPTH_TH ? bDiff = true : bDiff = false;

        if (abs(posStart - posEnd) <= 3) {
          continue;
        }

        if (bDiff && Left_D < Right_D){
          BackBound->imageData[j*width+posStart] = 255;
        }
        else if (bDiff && Left_D > Right_D){
          BackBound->imageData[j*width+posEnd] = 255;
        }
        else {
          for (k=posStart;k<posEnd;k++){
            Bound->imageData[j*width+k] ? BackBound->imageData[j*width+k] = 255 : BackBound->imageData[j*width+k] = 0;                
          }
        }
      }
    }
  }

}


void CBoundaryNoiseRemoval::expandedHoleforBNM(IplImage* Depth, IplImage* Hole, IplImage* BackBound, IplImage* ExpandedHole) 
{
  int i, j, x, y, width, height, min_x, min_y, max_x, max_y, depth_back;  

  width = Depth->width;
  height= Depth->height;
  cvZero(ExpandedHole);

  for (j=0;j<height;j++){
    for (i=0;i<width;i++){

      if ((uchar)BackBound->imageData[j*width + i] == 255){

        if (i-1<0 || i+1 > width-1 || j-1<0 || j+1 > height-1){
          continue;
        }

        min_x = guard(i - BOUNDARY_WINDOW_SIZE, 0, width-1);
        max_x = guard(i + BOUNDARY_WINDOW_SIZE, 0, width-1);
        min_y = guard(j - BOUNDARY_WINDOW_SIZE, 0, height-1);
        max_y = guard(j + BOUNDARY_WINDOW_SIZE, 0, height-1);

        if ((uchar)Hole->imageData[j*width + i] == 0){
          depth_back = (uchar)Depth->imageData[j*width + i];
        }
        else if ((uchar)Hole->imageData[j*width + i+1] == 0){
          depth_back = (uchar)Depth->imageData[j*width + i+1];
        }
        else if ((uchar)Hole->imageData[j*width + i-1] == 0){
          depth_back = (uchar)Depth->imageData[j*width + i-1];
        }
        else if ((uchar)Hole->imageData[(j+1)*width + i] == 0){
          depth_back = (uchar)Depth->imageData[(j+1)*width + i];
        }
        else if ((uchar)Hole->imageData[(j-1)*width + i] == 0){
          depth_back = (uchar)Depth->imageData[(j-1)*width + i];
        }
        else {
          continue;
        }

        for (y=min_y;y<=max_y;y++){
          for (x=min_x;x<=max_x;x++){

            if (x==1 && y ==154) {
               int aa = 0;
            }

            if ((uchar)Hole->imageData[y*width + x]==0 
              && abs((uchar)Depth->imageData[y*width+x] - depth_back) < DEPTH_TH
              && m_imgHoleOtherView->imageData[y*width+x] == 0) {
              ExpandedHole->imageData[y*width+x] = 255;
            } 
            else {
              ExpandedHole->imageData[y*width+x] = 0;
            }

          }
        }
      }
    }
  }

}

void CBoundaryNoiseRemoval::ColorFillingSmallHoleFor1DMode(CIYuv<ImageType>* pSrc, CIYuv<ImageType>* pDst) 
{
  int tWidth, tHeight,i, j;
  bool isAvailableLeft, isAvailableRight, isAvailableCurrent;
  ImageType *Src_Y, *Src_U, *Src_V;
  ImageType *Tar_Y, *Tar_U, *Tar_V;

  tWidth     = pSrc->getWidth();
  tHeight    = pSrc->getHeight();

  Src_Y = pSrc->getBuffer();
  Src_U = &Src_Y[tWidth*tHeight];
  Src_V = &Src_Y[tWidth*tHeight*2];

  Tar_Y = pDst->getBuffer();
  Tar_U = &Tar_Y[tWidth*tHeight];
  Tar_V = &Tar_Y[tWidth*tHeight*2];

  for (j=0;j<tHeight;j++) {
    for (i=1;i<tWidth-1;i++) {

      Src_Y[j*tWidth + i  ] && Src_U[j*tWidth + i  ] && Src_V[j*tWidth + i  ] ? isAvailableCurrent = true : isAvailableCurrent = false;
      Src_Y[j*tWidth + i-1] && Src_U[j*tWidth + i-1] && Src_V[j*tWidth + i-1] ? isAvailableLeft    = true : isAvailableLeft    = false;
      Src_Y[j*tWidth + i+1] && Src_U[j*tWidth + i+1] && Src_V[j*tWidth + i+1] ? isAvailableRight   = true : isAvailableRight   = false;
      if (isAvailableCurrent) {
                Tar_Y[j*tWidth + i  ] = (BYTE)guard((BYTE)Src_Y[j*tWidth + i  ], 0, MAX_LUMA-1);
        Tar_U[j*tWidth + i  ] = (BYTE)guard((BYTE)Src_U[j*tWidth + i  ], 0, MAX_LUMA-1);
        Tar_V[j*tWidth + i  ] = (BYTE)guard((BYTE)Src_V[j*tWidth + i  ], 0, MAX_LUMA-1);
      }
      else if (isAvailableLeft && isAvailableRight && !isAvailableCurrent) {
        Tar_Y[j*tWidth + i  ] = (BYTE)guard(((BYTE)Src_Y[j*tWidth + i-1] + (BYTE)Src_Y[j*tWidth + i+1] + 0.5)/2, 0, MAX_LUMA-1);
        Tar_U[j*tWidth + i  ] = (BYTE)guard(((BYTE)Src_U[j*tWidth + i-1] + (BYTE)Src_U[j*tWidth + i+1] + 0.5)/2, 0, MAX_LUMA-1);
        Tar_V[j*tWidth + i  ] = (BYTE)guard(((BYTE)Src_V[j*tWidth + i-1] + (BYTE)Src_V[j*tWidth + i+1] + 0.5)/2, 0, MAX_LUMA-1);
      }
      else {
        Tar_Y[j*tWidth + i  ] = 0;
        Tar_U[j*tWidth + i  ] = 0;
        Tar_V[j*tWidth + i  ] = 0;
      }
    }
  }

}

void CBoundaryNoiseRemoval::ColorHoleCleaning(CIYuv<ImageType>* pSrc)
{
  int i, j, tWidth, tHeight;
  bool isYAvailable, isUAvailable, isVAvailable;  
  ImageType *Src_Y, *Src_U, *Src_V;
  tWidth     = pSrc->getWidth();
  tHeight    = pSrc->getHeight();
  Src_Y = pSrc->getBuffer();
  Src_U = Src_Y + tWidth*tHeight;
  Src_V = Src_U + tWidth*tHeight;

  for (j=0;j<tHeight;j++) {
    for (i=0;i<tWidth;i++) {
      Src_Y[j*tWidth + i] == 0 ? isYAvailable = false : isYAvailable = true;
      Src_U[j*tWidth + i] == 0 ? isUAvailable = false : isUAvailable = true;
      Src_V[j*tWidth + i] == 0 ? isVAvailable = false : isVAvailable = true;

      if (!isYAvailable || !isUAvailable || !isVAvailable) {
        Src_Y[j*tWidth + i] = 0;
        Src_U[j*tWidth + i] = 0;
        Src_V[j*tWidth + i] = 0;
      }
    }
  }


}

void CBoundaryNoiseRemoval::DepthFillingSmallHoleFor1DMode(CIYuv<DepthType>* pSrc, CIYuv<DepthType>* pDst) 
{
  int tWidth, tHeight,i, j;
  bool isAvailableLeft, isAvailableRight, isAvailableCurrent;
  DepthType *Src_Y, *Tar_Y;

  tWidth     = pSrc->getWidth();
  tHeight    = pSrc->getHeight();

  Src_Y = pSrc->getBuffer();
  Tar_Y = pDst->getBuffer();

  for (j=0;j<tHeight;j++) {
    for (i=1;i<tWidth-1;i++) {

      Src_Y[j*tWidth + i  ] ? isAvailableCurrent = true : isAvailableCurrent = false;
      Src_Y[j*tWidth + i-1] ? isAvailableLeft    = true : isAvailableLeft    = false;
      Src_Y[j*tWidth + i+1] ? isAvailableRight   = true : isAvailableRight   = false;
      if (isAvailableCurrent) {
                Tar_Y[j*tWidth + i  ] = (BYTE)guard((BYTE)Src_Y[j*tWidth + i  ], 0, MAX_DEPTH-1);
      }
      else if (isAvailableLeft && isAvailableRight && !isAvailableCurrent) {
        Tar_Y[j*tWidth + i  ] = (BYTE)guard(((BYTE)Src_Y[j*tWidth + i-1] + (BYTE)Src_Y[j*tWidth + i+1] + 0.5)/2, 0, MAX_DEPTH-1);
      }
      else {
        Tar_Y[j*tWidth + i  ] = 0;
      }
    }
  }

}


void CBoundaryNoiseRemoval::DepthMatchingWithColor(CIYuv<DepthType>* pDepth, CIYuv<ImageType>* pColor, CIYuv<HoleType>* pDepthMask)
{
  int i, j, width, height;
  bool isAVailableColor, isAVailableDepth, isAVailableDepth_L, isAVailableDepth_R;
  width = pDepth->getWidth();
  height= pDepth->getHeight();
  ImageType *Y, *U, *V; 
  DepthType *D;
  HoleType *D_MASK;
  DepthType D_C, D_L, D_R;

  Y      = pColor->getBuffer();
  U      = &Y[width*height];
  V      = &Y[width*height*2];
  D      = pDepth->getBuffer();
  D_MASK = pDepthMask->getBuffer();

  // Make a hole referring to the color information
  for (j=0;j<height;j++) {
    for (i=0;i<width;i++) {
      Y[j*width+i] && U[j*width+i] && V[j*width+i] ? isAVailableColor   = true : isAVailableColor   = false;
      if (!isAVailableColor) { 
        D     [j*width+i] = 0;
        D_MASK[j*width+i] = MAX_HOLE-1;
      }
      else {
        D_MASK[j*width+i] = 0;
      }
    }
  }

  // Fill in a depth hole referring to the adjacent depth information
  for (j=0;j<height;j++) {
    for (i=1;i<width-1;i++) {
      Y[j*width+i]   && U[j*width+i] && V[j*width+i] ? isAVailableColor   = true : isAVailableColor   = false;
      D[j*width+i]                                   ? isAVailableDepth   = true : isAVailableDepth   = false;
      D[j*width+i-1]                                 ? isAVailableDepth_L = true : isAVailableDepth_L = false;
      D[j*width+i+1]                                 ? isAVailableDepth_R = true : isAVailableDepth_R = false;
      D_C = D[j*width+i+0];
      D_L = D[j*width+i-1];
      D_R = D[j*width+i+1];

            if (isAVailableColor && !isAVailableDepth) {
        if (isAVailableDepth_L && isAVailableDepth_R) {
          D     [j*width+i] = (BYTE)guard((D_L+D_R+0.5)/2, 0, MAX_DEPTH-1);
          D_MASK[j*width+i] = 0;
        }
        else if (isAVailableDepth_L) {
          D     [j*width+i] = D_L;
          D_MASK[j*width+i] = 0;
        }
        else if (isAVailableDepth_R) {
          D     [j*width+i] = D_R;
          D_MASK[j*width+i] = 0;
        }
      }
    }
  }
}

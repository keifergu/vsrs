/*
 *
 * View Synthesis using general mode
 *
 */

#include "ViewSynthesis.h"

#ifndef _OPEN_CV_HEADERS_
#define _OPEN_CV_HEADERS_
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <opencv\cvaux.h>
#endif
#include "opencv2/photo/photo.hpp"


 //#pragma comment(lib, "cv.lib")
 //#pragma comment(lib, "cxcore.lib")
 //#pragma comment(lib, "highgui.lib")
 //#pragma comment(lib, "cvaux.lib")


#define ZERO 0.00001

#ifndef SAFE_RELEASE_IMAGE
#define SAFE_RELEASE_IMAGE(p) { if((p)!=NULL){ cvReleaseImage(&(p)); (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE_MAT
#define SAFE_RELEASE_MAT(p) { if((p)!=NULL){ cvReleaseMat(&(p)); (p)=NULL; } }
#endif
#ifndef SAFE_FREE
#define SAFE_FREE(p) { if((p)!=NULL){ free(p); (p)=NULL; } }
#endif

int descending(const BYTE *a, const BYTE *b)
{
  if (*a < *b) return 1;
  if (*a > *b) return -1;
  return 0;
}

int ascending(const BYTE *a, const BYTE *b)
{
  if (*a < *b) return -1;
  if (*a > *b) return 1;
  return 0;
}

//Class name CViewSynthesis was changed to CViewInterpolationGeneral. 

CViewInterpolationGeneral::CViewInterpolationGeneral()
{
  int i;

  m_uiPrecision = 1;

  m_imgVirtualImage = m_imgVirtualDepth = NULL;
  m_imgSuccessSynthesis = m_imgHoles = m_imgBound = NULL;
  m_pConvKernel = NULL;
  m_pVirtualDepth = NULL;
  m_pSuccessSynthesis = NULL;
  //Nagoya start
  m_pVirtualImageY = m_pVirtualImageU = m_pVirtualImageV = NULL;
  //Nagoya end
  m_aiTableDisparity_ipel = m_aiTableDisparity_subpel = NULL;
  m_pcViewSynthesisLeft = NULL;
  m_pcViewSynthesisRight = NULL;

  m_imgBlended = m_imgInterpolatedView = NULL;
#ifdef NICT_IVSRS
  m_imgBlendedDepth = NULL; // NICT
#endif

  for (i = 0; i < 5; i++)
  {
    m_imgTemp[i] = NULL;
    m_imgDepthTemp[i] = NULL;
    m_imgMask[i] = NULL;
  }

#ifdef POZNAN_GENERAL_HOMOGRAPHY
  m_matH_R2V = m_matH_V2R = NULL;
#else
  for (i = 0; i < MAX_DEPTH; i++)
  {
    m_matH_R2V[i] = m_matH_V2R[i] = NULL;
  }
#endif

  // GIST added
  if (m_imgSynLeftforBNR != NULL) { m_imgSynLeftforBNR = NULL; }
  if (m_imgSynRightforBNR != NULL) { m_imgSynRightforBNR = NULL; }
  if (m_imgDepthLeftforBNR != NULL) { m_imgDepthLeftforBNR = NULL; }
  if (m_imgDepthRightforBNR != NULL) { m_imgDepthRightforBNR = NULL; }
  if (m_imgHoleLeftforBNR != NULL) { m_imgHoleLeftforBNR = NULL; }
  if (m_imgHoleRightforBNR != NULL) { m_imgHoleRightforBNR = NULL; }

}

CViewInterpolationGeneral::~CViewInterpolationGeneral()
{
  xReleaseMemory();
}

void CViewInterpolationGeneral::xReleaseMemory()
{
  int i;

  SAFE_RELEASE_IMAGE(m_imgVirtualImage);
  SAFE_RELEASE_IMAGE(m_imgVirtualDepth);
  SAFE_RELEASE_IMAGE(m_imgSuccessSynthesis);
  SAFE_RELEASE_IMAGE(m_imgHoles);
  SAFE_RELEASE_IMAGE(m_imgBound);
  SAFE_RELEASE_IMAGE(m_imgBlended);
#ifdef NICT_IVSRS
  SAFE_RELEASE_IMAGE(m_imgBlendedDepth); // NICT
#endif
  SAFE_RELEASE_IMAGE(m_imgInterpolatedView);
  if (m_pcViewSynthesisLeft != NULL)    delete m_pcViewSynthesisLeft;
  if (m_pcViewSynthesisRight != NULL)  delete m_pcViewSynthesisRight;

  m_pcViewSynthesisLeft = m_pcViewSynthesisRight = NULL;

  for (i = 0; i < 5; i++)
  {
    SAFE_RELEASE_IMAGE(m_imgTemp[i]);
    SAFE_RELEASE_IMAGE(m_imgDepthTemp[i]);
    SAFE_RELEASE_IMAGE(m_imgMask[i]);
  }

  if (m_pConvKernel != NULL)
  {
    cvReleaseStructuringElement(&m_pConvKernel);
    m_pConvKernel = NULL;
  }

  SAFE_FREE(m_pVirtualDepth);
  //Nagoya start
  SAFE_FREE(m_pVirtualImageY);
  SAFE_FREE(m_pVirtualImageU);
  SAFE_FREE(m_pVirtualImageV);
  //Nagoya end
  SAFE_FREE(m_pSuccessSynthesis);

#ifdef POZNAN_GENERAL_HOMOGRAPHY
  SAFE_RELEASE_MAT(m_matH_R2V);
  SAFE_RELEASE_MAT(m_matH_V2R);
#else
  for (i = 0; i < MAX_DEPTH; i++)
  {
    SAFE_RELEASE_MAT(m_matH_R2V[i]);
    SAFE_RELEASE_MAT(m_matH_V2R[i]);
  }
#endif

  SAFE_FREE(m_aiTableDisparity_ipel);
  SAFE_FREE(m_aiTableDisparity_subpel);

  // GIST added
  SAFE_RELEASE_IMAGE(m_imgSynLeftforBNR);
  SAFE_RELEASE_IMAGE(m_imgSynRightforBNR);
  SAFE_RELEASE_IMAGE(m_imgDepthLeftforBNR);
  SAFE_RELEASE_IMAGE(m_imgDepthRightforBNR);
  SAFE_RELEASE_IMAGE(m_imgHoleLeftforBNR);
  SAFE_RELEASE_IMAGE(m_imgHoleRightforBNR);

}

void CViewInterpolationGeneral::convertCameraParam(CvMat *exMat_dst, CvMat *exMat_src)
{
  int i, j;
  double val;

  for (i = 0; i < 3; i++)
  {
    for (j = 0; j < 3; j++)
    {
      //cvmSet(exMat_dst, i, j, cvmGet(exMat_src, j, i));
      cvmSet(exMat_dst, i, j, cvmGet(exMat_src, i, j));
    }
  }

  for (i = 0; i < 3; i++)
  {
    val = 0.0;
    for (j = 0; j < 3; j++)
    {
      val -= cvmGet(exMat_dst, i, j)*cvmGet(exMat_src, j, 3);
    }
    cvmSet(exMat_dst, i, 3, val);
  }
}

void CViewInterpolationGeneral::cvexMedian(IplImage* dst)
{
  IplImage* buf = cvCloneImage(dst);
  cvSmooth(buf, dst, CV_MEDIAN);
  cvReleaseImage(&buf);
}

void CViewInterpolationGeneral::cvexBilateral(IplImage* dst, int sigma_d, int sigma_c)
{
  IplImage* buf = cvCloneImage(dst);
  cvSmooth(buf, dst, CV_BILATERAL, sigma_d, sigma_c);
  cvReleaseImage(&buf);
}

void CViewInterpolationGeneral::erodebound(IplImage* bound, int flag)
{
  int width = bound->width;
  int height = bound->height;
  uchar *ub = (uchar *)bound->imageData;

  if (flag)
  {
    for (int j = 0; j < height; j++)
    {
      for (int i = 0; i < width; i++)
      {
        int l = i + j * width;
        if (ub[l] == 255)
        {
          int ll = l;
          while ((ub[ll] == 255) && (i < width))
          {
            ub[ll] = 0;
            ll++;
            i++;
          }
          ub[ll - 1] = 255;
        }
      }
    }
  }
  else
  {
    for (int j = 0; j < height; j++)
    {
      for (int i = 0; i < width; i++)
      {
        int l = i + j * width;
        if (ub[l] == 255)
        {
          int ll = l;
          while ((ub[ll] == 255) && (i < width))
          {
            ub[ll] = 0;
            ll++;
            i++;
          }
          ub[l] = 255;
        }
      }
    }
  }
}

// Member function InitLR was added to class CViewInterpolationGeneral.
// Intrinsic and Extrinsic parameters of left, right, and virtual cameras are set throught this function.
// Mat_In_Left[9], Mat_In_Right[9], Mat_In_Virtual[9] : Intrinsic matrices of left, right, and virtual cameras
// Mat_Ex_Left[9], Mat_Ex_Right[9], Mat_Ex_Virtual[9] : Extrinsic matrices of left, right, and virtual cameras
// Mat_Trans_Left[3], Mat_Trans_Right[3], Mat_Trans_Virtual[3] : Translation vectors of left, right, and virtual cameras

bool CViewInterpolationGeneral::InitLR(UInt uiWidth, UInt uiHeight, UInt uiPrecision,
  UInt uiDepthType, double dZnearL, double dZfarL, double dZnearR, double dZfarR,
  const char *strCamParamFile, const char *strRefLCamID, const char *strRefRCamID, const char *strVirCamID,
  double Mat_In_Left[9], double Mat_Ex_Left[9], double Mat_Trans_Left[3],
  double Mat_In_Right[9], double Mat_Ex_Right[9], double Mat_Trans_Right[3],
  double Mat_In_Virtual[9], double Mat_Ex_Virtual[9], double Mat_Trans_Virtual[3])
{
  m_pcViewSynthesisLeft = new CViewInterpolationGeneral();
  m_pcViewSynthesisRight = new CViewInterpolationGeneral();
  if (m_pcViewSynthesisLeft == NULL || m_pcViewSynthesisRight == NULL) return false;

  // The number of arguments of Init increased.
  // Intrinsic and Extrinsic parameters of (left or right) and virtual cameras are set throught this function.

  if (!m_pcViewSynthesisLeft->Init(uiWidth, uiHeight, uiPrecision, uiDepthType,
#ifdef NICT_IVSRS
    // NICT start  
    m_uiIvsrsInpaint,
    // NICT end
#endif
    dZnearL, dZfarL, strCamParamFile, strRefLCamID, strVirCamID,
    Mat_In_Left, Mat_Ex_Left, Mat_Trans_Left,
    Mat_In_Virtual, Mat_Ex_Virtual, Mat_Trans_Virtual
  ))  return false;
  if (!m_pcViewSynthesisRight->Init(uiWidth, uiHeight, uiPrecision, uiDepthType,
#ifdef NICT_IVSRS
    // NICT start  
    m_uiIvsrsInpaint,
    // NICT end
#endif
    dZnearR, dZfarR, strCamParamFile, strRefRCamID, strVirCamID,
    Mat_In_Right, Mat_Ex_Right, Mat_Trans_Right,
    Mat_In_Virtual, Mat_Ex_Virtual, Mat_Trans_Virtual
  ))  return false;

  double dTotalBaseline;
  m_dWeightLeft = m_pcViewSynthesisRight->getBaselineDistance();
  m_dWeightRight = m_pcViewSynthesisLeft->getBaselineDistance();
  dTotalBaseline = m_dWeightLeft + m_dWeightRight;
  m_dWeightLeft /= dTotalBaseline;
  m_dWeightRight /= dTotalBaseline;

  m_imgBlended = cvCreateImage(cvSize(uiWidth, uiHeight), 8, 3);
  // NICT start
#ifdef NICT_IVSRS
  m_imgBlendedDepth = cvCreateImage(cvSize(uiWidth, uiHeight), sizeof(DepthType) * 8, 1); // NICT
#endif
// NICT end
  m_imgInterpolatedView = cvCreateImage(cvSize(uiWidth, uiHeight), 8, 3);
  m_imgMask[2] = cvCreateImage(cvSize(uiWidth, uiHeight), 8, 1);
  m_imgMask[3] = cvCreateImage(cvSize(uiWidth, uiHeight), 8, 1);
  m_imgMask[4] = cvCreateImage(cvSize(uiWidth, uiHeight), 8, 1);

  return true;
}

bool CViewInterpolationGeneral::Init(UInt uiWidth, UInt uiHeight, UInt uiPrecision, UInt uiDepthType,
#ifdef NICT_IVSRS
  // NICT start  
  UInt uiIvsrsInpaint,
  // NICT end
#endif
  double dZnear, double dZfar,
  const char *strCamParamFile, const char *strRefCamID, const char *strVirCamID,
  double Mat_In_Ref[9], double Mat_Ex_Ref[9], double Mat_Trans_Ref[3],
  double Mat_In_Vir[9], double Mat_Ex_Vir[9], double Mat_Trans_Vir[3])
{
  double Z_near, Z_far;
  unsigned int h, pos;
  //Nagoya start
  unsigned w;
  //Nagoya end

  xReleaseMemory();

  Z_near = dZnear;    //z_near value from camera or the origin of 3D space
  Z_far = dZfar;    //z_far value from camera or the origin of 3D space

  m_uiPrecision = uiPrecision;
  if (m_uiPrecision != 1 && m_uiPrecision != 2 && m_uiPrecision != 4)
  {
    fprintf(stderr, "Illegal Precision setting\n");
    return false;
  }

#ifdef NICT_IVSRS
  // NICT start
  m_uiIvsrsInpaint = uiIvsrsInpaint; // NICT iVSRS inpaint flag
// NICT end
#endif

  m_uiHeight = uiHeight;
  m_uiWidth = uiWidth;
  m_uiPicsize = m_uiHeight*m_uiWidth;
  m_uiDepthType = uiDepthType;

  m_imgVirtualImage = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(ImageType) * 8, 3);
  m_imgVirtualDepth = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(DepthType) * 8, 1);
  m_imgSuccessSynthesis = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(HoleType) * 8, 1);
  m_imgHoles = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(HoleType) * 8, 1);
  m_imgBound = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(HoleType) * 8, 1);
  m_imgMask[0] = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(HoleType) * 8, 1);
  m_imgMask[1] = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(HoleType) * 8, 1);
  m_imgTemp[0] = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(ImageType) * 8, 1);
  m_imgTemp[1] = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(ImageType) * 8, 1);
  m_imgDepthTemp[0] = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(DepthType) * 8, 1);
  m_imgDepthTemp[1] = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(DepthType) * 8, 1);

  if ((m_pVirtualDepth = (DepthType **)malloc(m_uiHeight * sizeof(DepthType *))) == NULL) return false;
  for (h = pos = 0; h < m_uiHeight; h++, pos += m_imgVirtualDepth->widthStep)
  {
    m_pVirtualDepth[h] = (DepthType *) &(m_imgVirtualDepth->imageData[pos]);
  }

  // Nagoya start
  if ((m_pVirtualImageY = (ImageType **)malloc(m_uiHeight * sizeof(ImageType *))) == NULL
    || (m_pVirtualImageU = (ImageType **)malloc(m_uiHeight * sizeof(ImageType *))) == NULL
    || (m_pVirtualImageV = (ImageType **)malloc(m_uiHeight * sizeof(ImageType *))) == NULL) return false;

  for (int j = 0;j < m_uiHeight;j++) {
    if ((m_pVirtualImageY[j] = (ImageType *)malloc(m_imgSuccessSynthesis->widthStep * sizeof(ImageType))) == NULL
      || (m_pVirtualImageU[j] = (ImageType *)malloc(m_imgSuccessSynthesis->widthStep * sizeof(ImageType))) == NULL
      || (m_pVirtualImageV[j] = (ImageType *)malloc(m_imgSuccessSynthesis->widthStep * sizeof(ImageType))) == NULL) return false;
  }

  for (h = 0; h < m_uiHeight; h++)
    for (w = 0; w < m_uiWidth; w++)
    {
      m_pVirtualImageY[h][w] = (ImageType)(m_imgVirtualImage->imageData[(h*m_uiWidth + w) * 3]);
      m_pVirtualImageU[h][w] = (ImageType)(m_imgVirtualImage->imageData[(h*m_uiWidth + w) * 3 + 1]);
      m_pVirtualImageV[h][w] = (ImageType)(m_imgVirtualImage->imageData[(h*m_uiWidth + w) * 3 + 2]);
    }
  // Nagoya end

  if ((m_pSuccessSynthesis = (HoleType **)malloc(m_uiHeight * sizeof(HoleType *))) == NULL) return false;
  for (h = pos = 0; h < m_uiHeight; h++, pos += m_imgSuccessSynthesis->widthStep)
  {
    m_pSuccessSynthesis[h] = (HoleType *) &(m_imgSuccessSynthesis->imageData[pos]);
  }

  if (uiPrecision == 1)
  {
    m_pFunc_DepthSynthesis = &CViewInterpolationGeneral::depthsynthesis_3Dwarp_ipel;
    m_pFunc_ViewSynthesisReverse = &CViewInterpolationGeneral::viewsynthesis_reverse_3Dwarp_ipel;
  }
  else
  {
    m_pFunc_DepthSynthesis = &CViewInterpolationGeneral::depthsynthesis_3Dwarp;
    m_pFunc_ViewSynthesisReverse = &CViewInterpolationGeneral::viewsynthesis_reverse_3Dwarp;

  }
  return init_3Dwarp(Z_near, Z_far, uiDepthType, strCamParamFile, strRefCamID, strVirCamID,
    Mat_In_Ref, Mat_Ex_Ref, Mat_Trans_Ref, Mat_In_Vir, Mat_Ex_Vir, Mat_Trans_Vir);

  return true;
}

bool CViewInterpolationGeneral::init_camera_param(double Mat_In_Ref[9], double Mat_Ex_Ref[9], double Mat_Trans_Ref[3], double Mat_In_Vir[9], double Mat_Ex_Vir[9], double Mat_Trans_Vir[3],
  CvMat *mat_in[2], CvMat *mat_ex_c2w[2], CvMat *mat_proj[2])
{
  int i, j;
  CvMat *mat_ex_w2c;

  for (i = 0; i < 3; i++)
  {
    for (j = 0; j < 3;j++)
    {
      mat_in[0]->data.db[j + 3 * i] = *(Mat_In_Ref + 3 * i + j);
      mat_in[1]->data.db[j + 3 * i] = *(Mat_In_Vir + 3 * i + j);
    }
  }
  for (i = 0; i < 3; i++)
  {
    for (j = 0; j < 3;j++)
    {
      mat_ex_c2w[0]->data.db[j + 4 * i] = *(Mat_Ex_Ref + 3 * i + j);
      mat_ex_c2w[1]->data.db[j + 4 * i] = *(Mat_Ex_Vir + 3 * i + j);
    }
  }
  for (i = 0; i < 3; i++)
  {
    mat_ex_c2w[0]->data.db[3 + 4 * i] = *(Mat_Trans_Ref + i);
    mat_ex_c2w[1]->data.db[3 + 4 * i] = *(Mat_Trans_Vir + i);
  }

  if (mat_proj != NULL)
  {
    mat_ex_w2c = cvCreateMat(3, 4, CV_64F);
    for (i = 0; i < 2; i++)
    {
      convertCameraParam(mat_ex_w2c, mat_ex_c2w[i]);
      cvmMul(mat_in[i], mat_ex_w2c, mat_proj[i]);  // Proj = inMat_c2i * exMat_w2c
      cvmCopy(mat_ex_w2c, mat_ex_c2w[i]);
    }
    cvReleaseMat(&mat_ex_w2c);
  }

  return true;
}

void CViewInterpolationGeneral::image2world_with_z(CvMat *mat_Rc2w_invIN_from, CvMat *matEX_c2w_from, CvMat *image, CvMat *world)
{
  CvMat *temp;
  double s;

  temp = cvCreateMat(3, 1, CV_64F);

  cvmMul(mat_Rc2w_invIN_from, image, temp);

  s = (cvmGet(world, 2, 0) - cvmGet(matEX_c2w_from, 2, 3)) / cvmGet(temp, 2, 0);

  cvmSet(world, 0, 0, s*cvmGet(temp, 0, 0) + cvmGet(matEX_c2w_from, 0, 3));
  cvmSet(world, 1, 0, s*cvmGet(temp, 1, 0) + cvmGet(matEX_c2w_from, 1, 3));

  cvReleaseMat(&temp);
}

void CViewInterpolationGeneral::makeHomography(
#ifdef POZNAN_GENERAL_HOMOGRAPHY
  CvMat *&matH_F2T, CvMat *&matH_T2F,
#else
  CvMat *matH_F2T[MAX_DEPTH], CvMat *matH_T2F[MAX_DEPTH],
#endif
  double adTable[MAX_DEPTH], CvMat *matIN_from, CvMat *matEX_c2w_from, CvMat *matProj_to)
  // NICT end
{
  int i, j, k;
  double val, u, v;
  //CvMat *matIN_inv_from, *mat_Rc2w_invIN_from;
  CvMat *matInvProj_from;
  CvMat *matProj_from;
  CvMat *matProjX_from;
  CvMat *matH;

  CvMat *src_points = cvCreateMat(4, 2, CV_64F);
  CvMat *dst_points = cvCreateMat(4, 2, CV_64F);
  CvMat *image = cvCreateMat(3, 1, CV_64F);
  CvMat *world = cvCreateMat(4, 1, CV_64F);

  matProj_from = cvCreateMat(3, 4, CV_64F);
  cvmMul(matIN_from, matEX_c2w_from, matProj_from);  // Proj = inMat_c2i * exMat_w2c
  matProjX_from = cvCreateMat(4, 4, CV_64F);
  for (int y = 0;y < 3;y++)
    for (int x = 0;x < 4;x++)
      cvmSet(matProjX_from, y, x, cvmGet(matProj_from, y, x));
  for (int x = 0;x < 3;x++)
    cvmSet(matProjX_from, 3, x, 0.0);
  cvmSet(matProjX_from, 3, 3, 1.0);

  matInvProj_from = cvCreateMat(4, 4, CV_64F);
  cvmInvert(matProjX_from, matInvProj_from);

  matH = cvCreateMat(3, 4, CV_64F);
  cvmMul(matProj_to, matInvProj_from, matH);

  cvReleaseMat(&matInvProj_from);
  cvReleaseMat(&matProj_from);
  cvReleaseMat(&matProjX_from);

#ifdef POZNAN_GENERAL_HOMOGRAPHY

  matH_F2T = cvCreateMat(4, 4, CV_64F);
  matH_T2F = cvCreateMat(4, 4, CV_64F);

  for (int y = 0;y < 3;y++)
    for (int x = 0;x < 4;x++)
      cvmSet(matH_F2T, y, x, cvmGet(matH, y, x));
  for (int x = 0;x < 3;x++)
    cvmSet(matH_F2T, 3, x, 0.0);
  cvmSet(matH_F2T, 3, 3, 1.0);

  cvmInvert(matH_F2T, matH_T2F);
#else

  for (i = 0; i < MAX_DEPTH; i++)
  {
    matH_F2T[i] = cvCreateMat(3, 3, CV_64F);
    matH_T2F[i] = cvCreateMat(3, 3, CV_64F);

    cvmSet(world, 2, 0, 1.0);
    cvmSet(world, 3, 0, 1.0 / adTable[i]);

    // (u, v) = (0, 0)
    u = 0.0; v = 0.0;
    cvmSet(world, 0, 0, u);
    cvmSet(world, 1, 0, v);
    cvmSet(image, 0, 0, u);
    cvmSet(image, 1, 0, v);
    cvmSet(image, 2, 0, 1.0);
    //image2world_with_z(mat_Rc2w_invIN_from, matEX_c2w_from, image, world);
    //cvmMul(matProj_to, world, image);
    cvmMul(matH, world, image);
    cvmSet(src_points, 0, 0, u);
    cvmSet(src_points, 0, 1, v);
    cvmSet(dst_points, 0, 0, cvmGet(image, 0, 0) / cvmGet(image, 2, 0));
    cvmSet(dst_points, 0, 1, cvmGet(image, 1, 0) / cvmGet(image, 2, 0));

    // (u, v) = (0, height-1)
    u = 0.0; v = m_uiHeight - 1;
    cvmSet(world, 0, 0, u);
    cvmSet(world, 1, 0, v);
    cvmSet(image, 0, 0, u);
    cvmSet(image, 1, 0, v);
    cvmSet(image, 2, 0, 1.0);
    //image2world_with_z(mat_Rc2w_invIN_from, matEX_c2w_from, image, world);
//    cvmMul(matProj_to, world, image);
    cvmMul(matH, world, image);
    cvmSet(src_points, 1, 0, u);
    cvmSet(src_points, 1, 1, v);
    cvmSet(dst_points, 1, 0, cvmGet(image, 0, 0) / cvmGet(image, 2, 0));
    cvmSet(dst_points, 1, 1, cvmGet(image, 1, 0) / cvmGet(image, 2, 0));

    // (u, v) = (width-1, height-1)
    u = m_uiWidth - 1; v = m_uiHeight - 1;
    cvmSet(world, 0, 0, u);
    cvmSet(world, 1, 0, v);
    cvmSet(image, 0, 0, u);
    cvmSet(image, 1, 0, v);
    cvmSet(image, 2, 0, 1.0);
    //    image2world_with_z(mat_Rc2w_invIN_from, matEX_c2w_from, image, world);
    //    cvmMul(matProj_to, world, image);
    cvmMul(matH, world, image);
    cvmSet(src_points, 2, 0, u);
    cvmSet(src_points, 2, 1, v);
    cvmSet(dst_points, 2, 0, cvmGet(image, 0, 0) / cvmGet(image, 2, 0));
    cvmSet(dst_points, 2, 1, cvmGet(image, 1, 0) / cvmGet(image, 2, 0));

    // (u, v) = (width-1, 0)
    u = m_uiWidth - 1; v = 0.0;
    cvmSet(world, 0, 0, u);
    cvmSet(world, 1, 0, v);
    cvmSet(image, 0, 0, u);
    cvmSet(image, 1, 0, v);
    cvmSet(image, 2, 0, 1.0);
    //    image2world_with_z(mat_Rc2w_invIN_from, matEX_c2w_from, image, world);
    //    cvmMul(matProj_to, world, image);
    cvmMul(matH, world, image);
    cvmSet(src_points, 3, 0, u);
    cvmSet(src_points, 3, 1, v);
    cvmSet(dst_points, 3, 0, cvmGet(image, 0, 0) / cvmGet(image, 2, 0));
    cvmSet(dst_points, 3, 1, cvmGet(image, 1, 0) / cvmGet(image, 2, 0));

    cvFindHomography(src_points, dst_points, matH_F2T[i]);
    cvInvert(matH_F2T[i], matH_T2F[i], CV_LU);
    //cvFindHomography(dst_points, src_points, matH_T2F[i]);  
    //cvFindHomography(src_points, dst_points, matH_T2F[i]);  
    //cvInvert(matH_T2F[i], matH_F2T[i], CV_LU);
  }
#endif
  cvReleaseMat(&src_points);
  cvReleaseMat(&dst_points);
  cvReleaseMat(&image);
  cvReleaseMat(&world);
  cvReleaseMat(&matH);
  //cvReleaseMat(&matIN_inv_from);
  //cvReleaseMat(&mat_Rc2w_invIN_from);
}

bool CViewInterpolationGeneral::init_3Dwarp(double Z_near, double Z_far, unsigned int uiDepthType,
  const char *strCamParamFile, const char *strRefCamID, const char *strVirCamID,
  double Mat_In_Ref[9], double Mat_Ex_Ref[9], double Mat_Trans_Ref[3],
  double Mat_In_Vir[9], double Mat_Ex_Vir[9], double Mat_Trans_Vir[3])
{
  int i;

  CvMat* lMat_in[2]; //intrinsic parameter of left camera 3x3 matrix
  CvMat* lMat_ex_c2w[2]; //extrinsic parameter of left camera 3x4 matrix
  CvMat* lMat_proj_w2i[2]; // projection matrix from world to image

  double tableD2Z[MAX_DEPTH];
  double distance, temp1, temp2;

  for (i = 0; i < 2; i++)
  {
    lMat_in[i] = cvCreateMat(3, 3, CV_64F); //intrinsic parameter of camera (3x3 matrix)
    lMat_ex_c2w[i] = cvCreateMat(3, 4, CV_64F); //extrinsic parameter of camera (3x4 matrix)
    lMat_proj_w2i[i] = cvCreateMat(3, 4, CV_64F); // projection matrix
  }

  if (!init_camera_param(Mat_In_Ref, Mat_Ex_Ref, Mat_Trans_Ref, Mat_In_Vir, Mat_Ex_Vir, Mat_Trans_Vir, lMat_in, lMat_ex_c2w, lMat_proj_w2i))
  {
    for (i = 0; i < 2; i++)
    {
      cvReleaseMat(&lMat_in[i]);
      cvReleaseMat(&lMat_ex_c2w[i]);
      cvReleaseMat(&lMat_proj_w2i[i]);
    }
    return false;
  }

  int kernel[49] = { 0,0,1,1,1,0,0,
                  0,1,1,1,1,1,0,
                  1,1,1,1,1,1,1,
                  1,1,1,1,1,1,1,
                  1,1,1,1,1,1,1,
                  0,1,1,1,1,1,0,
                  0,0,1,1,1,0,0 };
  m_pConvKernel = cvCreateStructuringElementEx(7, 7, 3, 3, CV_SHAPE_CUSTOM, kernel);

  temp1 = 1.0 / Z_near - 1.0 / Z_far;
  temp2 = 1.0 / Z_far;

#ifdef POZNAN_DEPTH_PROJECT2COMMON
  m_dInvZNearMinusInvZFar = temp1;
  m_dInvZfar = temp2;
#endif

  switch (uiDepthType)
  {
  case 0:
    for (i = 0; i < MAX_DEPTH; i++)
    {
      distance = 1.0 / (double(i)*temp1 / (MAX_DEPTH - 1.0) + temp2);
      //tableD2Z[i] = cvmGet(lMat_ex_c2w[0], 2, 2) * distance + cvmGet(lMat_ex_c2w[0], 2, 3);
      tableD2Z[i] = cvmGet(lMat_ex_c2w[0], 2, 2) * distance + Mat_Trans_Ref[2];
#ifdef POZNAN_GENERAL_HOMOGRAPHY
      m_dTableD2Z[i] = tableD2Z[i];
#endif
    }
    break;
  case 1:
    for (i = 0; i < MAX_DEPTH; i++)
    {
      tableD2Z[i] = 1.0 / (double(i)*temp1 / (MAX_DEPTH - 1.0) + temp2);
#ifdef POZNAN_GENERAL_HOMOGRAPHY
      m_dTableD2Z[i] = tableD2Z[i];
#endif
    }
    break;
  default:
    return false;
  }

  makeHomography(m_matH_R2V, m_matH_V2R, tableD2Z, lMat_in[0], lMat_ex_c2w[0], lMat_proj_w2i[1]);

  // need to be modify (currently only the special case is supported)
  // All cameras must be placed on the x-axis and their direction be the z-axis (either positive or negative)
  if (cvmGet(lMat_ex_c2w[0], 0, 3) < cvmGet(lMat_ex_c2w[1], 0, 3))
    m_ucLeftSide = 1;
  else
    m_ucLeftSide = 0;

  m_dBaselineDistance = pow((lMat_ex_c2w[0]->data.db[3] - lMat_ex_c2w[1]->data.db[3]), 2.0) +
    pow((lMat_ex_c2w[0]->data.db[7] - lMat_ex_c2w[1]->data.db[7]), 2.0) +
    pow((lMat_ex_c2w[0]->data.db[11] - lMat_ex_c2w[1]->data.db[11]), 2.0);
  m_dBaselineDistance = sqrt(m_dBaselineDistance);

  for (i = 0; i < 2; i++)
  {
    cvReleaseMat(&lMat_in[i]);
    cvReleaseMat(&lMat_ex_c2w[i]);
    cvReleaseMat(&lMat_proj_w2i[i]);
  }

  return true;
}

bool CViewInterpolationGeneral::xSynthesizeView(ImageType ***src, DepthType **pDepthMap, int th_same_depth)
{
  if ((this->*m_pFunc_DepthSynthesis)(pDepthMap, src)) //src
  {
    return (this->*m_pFunc_ViewSynthesisReverse)(src, pDepthMap, th_same_depth);
  }

  return false;
}

bool CViewInterpolationGeneral::xSynthesizeDepth(DepthType **pDepthMap, ImageType ***src)
{
  return (this->*m_pFunc_DepthSynthesis)(pDepthMap, src);
}

bool CViewInterpolationGeneral::xSynthesizeView_reverse(ImageType ***src, DepthType **pDepth, int th_same_depth)
{
  return (this->*m_pFunc_ViewSynthesisReverse)(src, pDepth, th_same_depth);
}

bool CViewInterpolationGeneral::depthsynthesis_3Dwarp(DepthType **pDepthMap, ImageType ***src)
{
  int h, w, u, v;
  int window_size = 3;

#ifdef POZNAN_GENERAL_HOMOGRAPHY
  CvMat* m = cvCreateMat(4, 1, CV_64F);
  CvMat* mv = cvCreateMat(4, 1, CV_64F);
#else
  CvMat* m = cvCreateMat(3, 1, CV_64F);
  CvMat* mv = cvCreateMat(3, 1, CV_64F);
#endif

  cvZero(m_imgVirtualDepth);
  cvZero(m_imgSuccessSynthesis);

  for (h = 0; h < m_uiHeight; h++)
  {
    for (w = 0; w < m_uiWidth; w++)
    {
#ifdef POZNAN_GENERAL_HOMOGRAPHY
      cvmSet(m, 0, 0, w);
      cvmSet(m, 1, 0, h);
      cvmSet(m, 2, 0, 1.0);
      cvmSet(m, 3, 0, 1.0 / m_dTableD2Z[pDepthMap[h][w]]);
      cvmMul(m_matH_R2V, m, mv);
#else
      cvmSet(m, 0, 0, w);
      cvmSet(m, 1, 0, h);
      cvmSet(m, 2, 0, 1);
      cvmMul(m_matH_R2V[pDepthMap[h][w]], m, mv);
#endif
      u = mv->data.db[0] / mv->data.db[2] + 0.5;
      v = mv->data.db[1] / mv->data.db[2] + 0.5;

      int iDepth = pDepthMap[h][w];

#ifdef POZNAN_DEPTH_PROJECT2COMMON
      double dInvZ = mv->data.db[3] / mv->data.db[2];
      iDepth = (dInvZ - m_dInvZfar)*(MAX_DEPTH - 1) / m_dInvZNearMinusInvZFar + 0.5;
#endif
      if (u >= 0 && u < m_uiWidth && v >= 0 && v < m_uiHeight && m_pVirtualDepth[v][u] <= iDepth)
      {
        m_pVirtualDepth[v][u] = iDepth;
        m_pSuccessSynthesis[v][u] = MAX_HOLE - 1;
      }
    }
  }

#if 0 // original median filter
  median_filter_depth(m_imgVirtualDepth, m_imgTemp[0], m_imgSuccessSynthesis, m_imgMask[0], 1, 1, true);
  median_filter_depth(m_imgTemp[0], m_imgVirtualDepth, m_imgMask[0], m_imgSuccessSynthesis, 1, 1, true);
#else
  cvNot(m_imgSuccessSynthesis, m_imgHoles); // m_imgHoles express holes before smoothing
  cvSmooth(m_imgHoles, m_imgTemp[0], CV_MEDIAN, window_size); // m_imgTemp[0] express holes after smoothing
  cvAnd(m_imgSuccessSynthesis, m_imgTemp[0], m_imgMask[0]); // holes which were not holes before smoothing
  cvCopy(m_imgHoles, m_imgTemp[0], m_imgMask[0]); // m_imgTemp[0] express holes before 2nd smoothing

  cvNot(m_imgTemp[0], m_imgSuccessSynthesis); // m_imgSuccessSynthesis express non-holes before 2nd smoothing
  cvSmooth(m_imgTemp[0], m_imgHoles, CV_MEDIAN, window_size); // m_imgHoles express holes after 2nd smoothing
  cvAnd(m_imgSuccessSynthesis, m_imgHoles, m_imgMask[1]); // holes which were not holes before 2nd smoothing
  cvCopy(m_imgTemp[0], m_imgHoles, m_imgMask[1]); // m_imgHoles express holes after 2nd smoothing

//cvSaveImage("1.bmp", m_imgVirtualDepth);
  cvSmooth(m_imgVirtualDepth, m_imgDepthTemp[1], CV_MEDIAN, window_size); // 1st 3x3 median
  cvCopy(m_imgVirtualDepth, m_imgDepthTemp[1], m_imgMask[0]);
  //cvSaveImage("2.bmp", m_imgTemp[1]);
  cvSmooth(m_imgDepthTemp[1], m_imgVirtualDepth, CV_MEDIAN, window_size); // 2nd 3x3 median
  cvCopy(m_imgDepthTemp[1], m_imgVirtualDepth, m_imgMask[1]);
  //cvSaveImage("3.bmp", m_imgVirtualDepth);

  cvSmooth(m_imgHoles, m_imgTemp[0], CV_MEDIAN, window_size); // m_imgTemp[0] express holes after smoothing
  cvAnd(m_imgSuccessSynthesis, m_imgTemp[0], m_imgMask[0]); // holes which were not holes before smoothing
  cvCopy(m_imgHoles, m_imgTemp[0], m_imgMask[0]); // m_imgTemp[0] express holes before 2nd smoothing

  cvNot(m_imgTemp[0], m_imgSuccessSynthesis); // m_imgSuccessSynthesis express non-holes before 2nd smoothing
  cvSmooth(m_imgTemp[0], m_imgHoles, CV_MEDIAN, window_size); // m_imgHoles express holes after 2nd smoothing
  cvAnd(m_imgSuccessSynthesis, m_imgHoles, m_imgMask[1]); // holes which were not holes before 2nd smoothing
  cvCopy(m_imgTemp[0], m_imgHoles, m_imgMask[1]); // m_imgHoles express holes after 2nd smoothing

  cvSmooth(m_imgVirtualDepth, m_imgDepthTemp[1], CV_MEDIAN, window_size); // 3rd 3x3 median
  cvCopy(m_imgVirtualDepth, m_imgDepthTemp[1], m_imgMask[0]);
  //cvSaveImage("4.bmp", m_imgTemp[1]);
  cvSmooth(m_imgDepthTemp[1], m_imgVirtualDepth, CV_MEDIAN, window_size); // 4th 3x3 median
  cvCopy(m_imgDepthTemp[1], m_imgVirtualDepth, m_imgMask[1]);
  //cvSaveImage("5.bmp", m_imgVirtualDepth);


  cvNot(m_imgHoles, m_imgSuccessSynthesis);
#endif

  cvReleaseMat(&m);
  cvReleaseMat(&mv);

  return true;
}

bool CViewInterpolationGeneral::viewsynthesis_reverse_3Dwarp(ImageType ***src, DepthType **pDepthMap, int th_same_depth)
{
  int ptv, u, v;
  int h, w;
  int maxWidth = m_uiWidth*m_uiPrecision;

#ifdef POZNAN_GENERAL_HOMOGRAPHY
  CvMat* m = cvCreateMat(4, 1, CV_64F);
  CvMat* mv = cvCreateMat(4, 1, CV_64F);
#else
  CvMat* m = cvCreateMat(3, 1, CV_64F);
  CvMat* mv = cvCreateMat(3, 1, CV_64F);
#endif

  cvZero(m_imgVirtualImage);

  for (h = 0; h < m_uiHeight; h++)
  {
    for (w = 0; w < m_uiWidth; w++)
    {
      if (m_pSuccessSynthesis[h][w] == 0) continue;

      ptv = w + h * m_uiWidth;
#ifdef POZNAN_GENERAL_HOMOGRAPHY
      cvmSet(mv, 0, 0, w);
      cvmSet(mv, 1, 0, h);
      cvmSet(mv, 2, 0, 1.0);
      cvmSet(mv, 3, 0, 1.0 / m_dTableD2Z[m_pVirtualDepth[h][w]]);

      cvmMul(m_matH_V2R, mv, m);
#else
      cvmSet(mv, 0, 0, w);
      cvmSet(mv, 1, 0, h);
      cvmSet(mv, 2, 0, 1);

      cvmMul(m_matH_V2R[m_pVirtualDepth[h][w]], mv, m);
#endif
      u = m->data.db[0] * m_uiPrecision / m->data.db[2] + 0.5;
      v = m->data.db[1] / m->data.db[2] + 0.5;
      //      if(u>=0 && u<maxWidth && v>=0 && v<height && byte_abs[pDepthMap[v][u/m_uiPrecision]-m_pVirtualDepth[h][w]]<th_same_depth)
      if (u >= 0 && u < maxWidth && v >= 0 && v < m_uiHeight)
      {
        m_imgVirtualImage->imageData[ptv * 3] = src[0][v][u];
        m_imgVirtualImage->imageData[ptv * 3 + 1] = src[1][v][u];
        m_imgVirtualImage->imageData[ptv * 3 + 2] = src[2][v][u];
      }
      else
      {
        m_pSuccessSynthesis[h][w] = 0;
      }
    }
  }
  cvReleaseMat(&m);
  cvReleaseMat(&mv);

  cvNot(m_imgSuccessSynthesis, m_imgHoles); // pixels which couldn't be synthesized
#ifdef NICT_IVSRS
// NICT start
  if (m_uiIvsrsInpaint == 1)
  {
    cvDilate(m_imgHoles, m_imgHoles); // simple dilate with 3x3 mask
    cvNot(m_imgHoles, m_imgSuccessSynthesis);
  }
  else
  {
#endif
    cvCopy(m_imgHoles, m_imgBound);
    erodebound(m_imgBound, m_ucLeftSide);              // background-side boundary of holes
    cvDilate(m_imgBound, m_imgBound, m_pConvKernel, 1);  // dilate by using circle-shape kernel
    cvOr(m_imgHoles, m_imgBound, m_imgMask[0]);    // pixels which want to be modified by other synthesized images
#ifdef NICT_IVSRS
  }
#endif
  //cvSaveImage("6.bmp", m_imgVirtualImage);
  //cvSaveImage("7.bmp", m_imgSuccessSynthesis);

  return true;
}

int CViewInterpolationGeneral::median_filter_depth(IplImage *srcDepth, IplImage *dstDepth,
  IplImage *srcMask, IplImage *dstMask, int sizeX, int sizeY, bool bSmoothing)
{
  int num;
  int h, w, i, j, ret = 0;
  int size = (2 * sizeX + 1)*(2 * sizeY + 1);
  int th = size / 2;
  BYTE buf[25];
  BYTE *pSrcDepth[5], *pDstDepth[5], *pSrcMask[5], *pDstMask[5];

#ifdef _DEBUG
  if (sizeY > 2 || size > 25) return -1;
#endif

  for (h = 0; h < m_uiHeight; h++)
  {
    for (i = -sizeY; i <= sizeY; i++)
    {
      if (h + i < 0 || h + i >= m_uiHeight) continue;
      pSrcDepth[i + sizeY] = (BYTE *) &(srcDepth->imageData[(h + i)*srcDepth->widthStep]);
      pDstDepth[i + sizeY] = (BYTE *) &(dstDepth->imageData[(h + i)*srcDepth->widthStep]);
      pSrcMask[i + sizeY] = (BYTE *) &(srcMask->imageData[(h + i)*srcDepth->widthStep]);
      pDstMask[i + sizeY] = (BYTE *) &(dstMask->imageData[(h + i)*srcDepth->widthStep]);
    }

    for (w = 0; w < m_uiWidth; w++)
    {
      pDstDepth[sizeY][w] = pSrcDepth[sizeY][w];
      pDstMask[sizeY][w] = pSrcMask[sizeY][w];

      if (pSrcMask[sizeY][w] != 0 && !bSmoothing) continue;

      num = 0;
      for (i = -sizeY; i <= sizeY; i++)
      {
        if (h + i < 0 || h + i >= m_uiHeight) continue;
        for (j = -sizeX; j <= sizeX; j++)
        {
          if (w + j < 0 || w + j >= m_uiWidth) continue;
          if (pSrcMask[i + sizeY][w + j] == 0) continue;

          buf[num] = pSrcDepth[i + sizeY][w + j];
          num++;
        }
      }
      if (num > th)
      {
        qsort(buf, num, sizeof(BYTE), (int(*)(const void*, const void*))descending);
        num /= 2;
        pDstDepth[sizeY][w] = buf[num];
        pDstMask[sizeY][w] = MaxTypeValue<HoleType>() - 1;
        ret++;
      }
    }
  }
  return ret;
}

int CViewInterpolationGeneral::median_filter_depth_wCheck(IplImage *srcDepth, IplImage *dstDepth,
  IplImage *srcMask, IplImage *dstMask,
  int sizeX, int sizeY, bool bSmoothing, int th_same_plane)
{
  int num;
  int h, w, i, j, ret = 0;
  int size = (2 * sizeX + 1)*(2 * sizeY + 1);
  int th = size / 2;
  BYTE buf[25];
  BYTE *pSrcDepth[5], *pDstDepth[5], *pSrcMask[5], *pDstMask[5];

#ifdef _DEBUG
  if (sizeY > 2 || size > 25) return -1;
#endif

  for (h = 0; h < m_uiHeight; h++)
  {
    for (i = -sizeY; i <= sizeY; i++)
    {
      if (h + i < 0 || h + i >= m_uiHeight) continue;
      pSrcDepth[i + sizeY] = (BYTE *) &(srcDepth->imageData[(h + i)*srcDepth->widthStep]);
      pDstDepth[i + sizeY] = (BYTE *) &(dstDepth->imageData[(h + i)*srcDepth->widthStep]);
      pSrcMask[i + sizeY] = (BYTE *) &(srcMask->imageData[(h + i)*srcDepth->widthStep]);
      pDstMask[i + sizeY] = (BYTE *) &(dstMask->imageData[(h + i)*srcDepth->widthStep]);
    }

    for (w = 0; w < m_uiWidth; w++)
    {
      pDstDepth[sizeY][w] = pSrcDepth[sizeY][w];
      pDstMask[sizeY][w] = pSrcMask[sizeY][w];

      if (pSrcMask[sizeY][w] != 0 && !bSmoothing) continue;

      num = 0;
      for (i = -sizeY; i <= sizeY; i++)
      {
        if (h + i < 0 || h + i >= m_uiHeight) continue;
        for (j = -sizeX; j <= sizeX; j++)
        {
          if (w + j < 0 || w + j >= m_uiWidth) continue;
          if (pSrcMask[i + sizeY][w + j] == 0) continue;

          buf[num] = pSrcDepth[i + sizeY][w + j];
          num++;
        }
      }
      if (num > th)
      {
        qsort(buf, num, sizeof(BYTE), (int(*)(const void*, const void*))descending);
        num /= 2;

        if (abs(int(pDstDepth[sizeY][w]) - int(buf[num])) < th_same_plane)
        {
          pDstDepth[sizeY][w] = buf[num];
          pDstMask[sizeY][w] = MaxTypeValue<HoleType>() - 1;
          ret++;
        }
      }
    }
  }
  return ret;
}

bool CViewInterpolationGeneral::depthsynthesis_3Dwarp_ipel(DepthType **pDepthMap, ImageType ***src) //Why this is not modyfied like other for new hole filling?? <-- Since it is not useful.
{
  int h, w, u, v;
  int sigma_d = 20;
  int sigma_c = 50;

#ifdef POZNAN_GENERAL_HOMOGRAPHY
  CvMat* m = cvCreateMat(4, 1, CV_64F);
  CvMat* mv = cvCreateMat(4, 1, CV_64F);
#else
  CvMat* m = cvCreateMat(3, 1, CV_64F);
  CvMat* mv = cvCreateMat(3, 1, CV_64F);
#endif

  cvZero(m_imgVirtualDepth);
  cvZero(m_imgSuccessSynthesis);

  for (h = 0; h < m_uiHeight; h++)
  {
    for (w = 0; w < m_uiWidth; w++)
    {
#ifdef POZNAN_GENERAL_HOMOGRAPHY
      cvmSet(m, 0, 0, w);
      cvmSet(m, 1, 0, h);
      cvmSet(m, 2, 0, 1.0);
      cvmSet(m, 3, 0, 1.0 / m_dTableD2Z[pDepthMap[h][w]]);
      cvmMul(m_matH_R2V, m, mv);
#else
      cvmSet(m, 0, 0, w);
      cvmSet(m, 1, 0, h);
      cvmSet(m, 2, 0, 1);
      cvmMul(m_matH_R2V[pDepthMap[h][w]], m, mv);
#endif
      u = mv->data.db[0] / mv->data.db[2] + 0.5;
      v = mv->data.db[1] / mv->data.db[2] + 0.5;
      if (u >= 0 && u < m_uiWidth && v >= 0 && v < m_uiHeight && m_pVirtualDepth[v][u] <= pDepthMap[h][w])
      {
        m_pVirtualDepth[v][u] = pDepthMap[h][w];
        m_pSuccessSynthesis[v][u] = MAX_HOLE - 1;
      }
    }
  }
  //cvSaveImage("5a.bmp", m_imgVirtualDepth);
  //cvSaveImage("6a.bmp", m_imgSuccessSynthesis);  
  cvexMedian(m_imgVirtualDepth);
  cvexBilateral(m_imgVirtualDepth, sigma_d, sigma_c);
  cvexMedian(m_imgSuccessSynthesis);
  cvNot(m_imgSuccessSynthesis, m_imgHoles); // pixels which couldn't be synthesized    
  cvReleaseMat(&m);
  cvReleaseMat(&mv);
  //cvSaveImage("5.bmp", m_imgVirtualDepth);
  //cvSaveImage("6.bmp", m_imgSuccessSynthesis);
  //cvSaveImage("7.bmp", m_imgHoles);
  return true;
}

bool CViewInterpolationGeneral::viewsynthesis_reverse_3Dwarp_ipel(ImageType ***src, DepthType **pDepthMap, int th_same_depth)
{
  int ptv, u, v;
  int h, w;

#ifdef POZNAN_GENERAL_HOMOGRAPHY
  CvMat* m = cvCreateMat(4, 1, CV_64F);
  CvMat* mv = cvCreateMat(4, 1, CV_64F);
#else
  CvMat* m = cvCreateMat(3, 1, CV_64F);
  CvMat* mv = cvCreateMat(3, 1, CV_64F);
#endif
  cvZero(m_imgVirtualImage);

  for (h = 0; h < m_uiHeight; h++)
  {
    for (w = 0; w < m_uiWidth; w++)
    {
#ifdef POZNAN_GENERAL_HOMOGRAPHY
      cvmSet(mv, 0, 0, w);
      cvmSet(mv, 1, 0, h);
      cvmSet(mv, 2, 0, 1.0);
      cvmSet(mv, 2, 0, 1.0 / m_dTableD2Z[m_pVirtualDepth[h][w]]);

      cvmMul(m_matH_V2R, mv, m);
#else
      cvmSet(mv, 0, 0, w);
      cvmSet(mv, 1, 0, h);
      cvmSet(mv, 2, 0, 1);

      cvmMul(m_matH_V2R[m_pVirtualDepth[h][w]], mv, m);
#endif
      u = m->data.db[0] / m->data.db[2] + 0.5;
      v = m->data.db[1] / m->data.db[2] + 0.5;
      if (u >= 0 && u < m_uiWidth && v >= 0 && v < m_uiHeight)
      {
        ptv = w + h * m_uiWidth;
        m_imgVirtualImage->imageData[ptv * 3] = src[0][v][u];
        m_imgVirtualImage->imageData[ptv * 3 + 1] = src[1][v][u];
        m_imgVirtualImage->imageData[ptv * 3 + 2] = src[2][v][u];
      }
    }
  }
  cvReleaseMat(&m);
  cvReleaseMat(&mv);

  // NICT start
#ifdef NICT_IVSRS
  if (m_uiIvsrsInpaint == 1)
  {
    cvDilate(m_imgHoles, m_imgHoles); // simple dilate with 3x3 mask
    cvNot(m_imgHoles, m_imgSuccessSynthesis);
  }
  else
  {
#endif
    cvCopy(m_imgHoles, m_imgBound);
    erodebound(m_imgBound, m_uiDepthType);
    cvDilate(m_imgBound, m_imgBound);
    cvDilate(m_imgBound, m_imgBound);
    cvOr(m_imgHoles, m_imgBound, m_imgMask[0]);
#ifdef NICT_IVSRS
  }
#endif
  // NICT end

  //cvSaveImage("5.bmp", m_imgVirtualImage);
  return true;
}

int CViewInterpolationGeneral::DoOneFrameGeneral(ImageType*** RefLeft, ImageType*** RefRight, DepthType** RefDepthLeft, DepthType** RefDepthRight, CIYuv<ImageType> *pSynYuvBuffer)
{
  //#ifdef _DEBUG
  //  if(m_ucSetup!=3) return false;
  //#endif
  ImageType*** pRefLeft = RefLeft;
  ImageType*** pRefRight = RefRight;
  DepthType** pRefDepthLeft = RefDepthLeft;
  DepthType** pRefDepthRight = RefDepthRight;

  if (!m_pcViewSynthesisLeft->xSynthesizeView(pRefLeft, pRefDepthLeft))  return false;
  if (!m_pcViewSynthesisRight->xSynthesizeView(pRefRight, pRefDepthRight)) return false;

  // GIST added
  if (m_imgSynLeftforBNR == NULL) { m_imgSynLeftforBNR = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(ImageType) * 8, 3); }
  if (m_imgSynRightforBNR == NULL) { m_imgSynRightforBNR = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(ImageType) * 8, 3); }
  if (m_imgDepthLeftforBNR == NULL) { m_imgDepthLeftforBNR = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(DepthType) * 8, 1); }
  if (m_imgDepthRightforBNR == NULL) { m_imgDepthRightforBNR = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(DepthType) * 8, 1); }
  if (m_imgHoleLeftforBNR == NULL) { m_imgHoleLeftforBNR = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(HoleType) * 8, 1); }
  if (m_imgHoleRightforBNR == NULL) { m_imgHoleRightforBNR = cvCreateImage(cvSize(m_uiWidth, m_uiHeight), sizeof(HoleType) * 8, 1); }
  cvCopy(m_pcViewSynthesisLeft->getVirtualImage(), m_imgSynLeftforBNR);
  cvCopy(m_pcViewSynthesisRight->getVirtualImage(), m_imgSynRightforBNR);
  cvCopy(m_pcViewSynthesisLeft->getVirtualDepthMap(), m_imgDepthLeftforBNR);
  cvCopy(m_pcViewSynthesisRight->getVirtualDepthMap(), m_imgDepthRightforBNR);
  cvCopy(m_pcViewSynthesisLeft->getHolePixels(), m_imgHoleLeftforBNR);
  cvCopy(m_pcViewSynthesisRight->getHolePixels(), m_imgHoleRightforBNR);
  // GIST end

  // pixels which will be replaced by pixels synthesized from right view
#ifdef NICT_IVSRS
  // NICT start
  if (m_uiIvsrsInpaint == 1)
  {
    cvAnd(m_pcViewSynthesisLeft->getHolePixels(), m_pcViewSynthesisRight->getSynthesizedPixels(), m_imgMask[3]); // NICT use same hole mask
  }
  else
  {
    // NICT end
#endif
    cvAnd(m_pcViewSynthesisLeft->getUnstablePixels(), m_pcViewSynthesisRight->getSynthesizedPixels(), m_imgMask[3]); // Left dilated Mask[0] * Right Success -> Left Mask[3] // dilated hole fillable by Right 
#ifdef NICT_IVSRS
  }
#endif

  if (ViewBlending == 1)
  {
    if (m_dWeightLeft >= m_dWeightRight)  // if closer to Left
    {
      cvCopy(m_pcViewSynthesisRight->getVirtualImage(), m_pcViewSynthesisLeft->getVirtualImage(), m_imgMask[3]);  // Right * Mask[3] -> Left // dilated hole may left
    }
    else                               // if closer to Right
    {
      cvCopy(m_pcViewSynthesisRight->getVirtualImage(), m_pcViewSynthesisLeft->getVirtualImage(), m_pcViewSynthesisRight->getSynthesizedPixels()); // Right VirtualImage * success -> Left VirtualImage
    }
  }
  else {
    cvCopy(m_pcViewSynthesisRight->getVirtualImage(), m_pcViewSynthesisLeft->getVirtualImage(), m_imgMask[3]); // Right VirtualImage * hole fillabl by Right -> Left VirtualImage // Left hole is filled by Right
#ifdef NICT_IVSRS
// NICT start
    if (m_uiIvsrsInpaint == 1)
    {
      cvCopy(m_pcViewSynthesisRight->getVirtualDepthMap(), m_pcViewSynthesisLeft->getVirtualDepthMap(), m_imgMask[3]); // Right VirtualDepth * hole fillabl by Right -> Left VirtualDepth //NICT
    }
    // NICT end
#endif
  }

  // pixels which will be replaced by pixels synthesized from left view
#ifdef NICT_IVSRS
  // NICT start
  if (m_uiIvsrsInpaint == 1)
  {
    cvAnd(m_pcViewSynthesisRight->getHolePixels(), m_pcViewSynthesisLeft->getSynthesizedPixels(), m_imgMask[4]); // NICT use same hole mask
  }
  else
  {
    // NICT end
#endif
    cvAnd(m_pcViewSynthesisRight->getUnstablePixels(), m_pcViewSynthesisLeft->getSynthesizedPixels(), m_imgMask[4]); // Right dilated Mask[0] * Left Success -> Mask[4] // dilated hole fillable by Left
#ifdef NICT_IVSRS
  }
#endif


  if (ViewBlending == 1)
  {
    if (m_dWeightLeft <= m_dWeightRight) // if closer to Right
    {
      cvCopy(m_pcViewSynthesisLeft->getVirtualImage(), m_pcViewSynthesisRight->getVirtualImage(), m_imgMask[4]); // Left VirtualImage * Mask[4] -> Right VirtualImage
    }
    else                              // if close to Left
    {
      cvCopy(m_pcViewSynthesisLeft->getVirtualImage(), m_pcViewSynthesisRight->getVirtualImage(), m_pcViewSynthesisLeft->getSynthesizedPixels()); // Left VirtualImage * success -> Right VirtualImage
    }
  }
  else {
    cvCopy(m_pcViewSynthesisLeft->getVirtualImage(), m_pcViewSynthesisRight->getVirtualImage(), m_imgMask[4]); // Left VirtualImage * hole fillable by Left ->Right VirtualImage
#ifdef NICT_IVSRS
  // NICT start
    if (m_uiIvsrsInpaint == 1)
    {
      cvCopy(m_pcViewSynthesisLeft->getVirtualDepthMap(), m_pcViewSynthesisRight->getVirtualDepthMap(), m_imgMask[4]); // Left VirtualDedpth * hole fillable by Left ->Right VirtualDepth // NICT
    }
    // NICT end
#endif
  }

  // pixels which couldn't be synthesized from both left and right -> inpainting
  cvAnd(m_pcViewSynthesisLeft->getHolePixels(), m_pcViewSynthesisRight->getHolePixels(), m_imgMask[2]); // Left Hole * Right Hole -> Mask[2] // common hole, 

#ifdef POZNAN_DEPTH_BLEND
#define GETUCHAR(x,ptr) (((unsigned char*)x)[ptr])
  IplImage *DepthLeft = m_pcViewSynthesisLeft->getVirtualDepthMap();
  IplImage *DepthRight = m_pcViewSynthesisRight->getVirtualDepthMap();
  IplImage *ImageLeft = m_pcViewSynthesisLeft->getVirtualImage();
  IplImage *ImageRight = m_pcViewSynthesisRight->getVirtualImage();
  IplImage *SynthesizedLeft = m_pcViewSynthesisLeft->getHolePixels();
  IplImage *SynthesizedRight = m_pcViewSynthesisRight->getHolePixels();
  for (int h = 0; h < m_uiHeight; h++)
  {
    for (int w = 0; w < m_uiWidth; w++)
    {
      int ptv = w + h * m_uiWidth;
      m_imgBlended->imageData[ptv * 3 + 0] = 0;
      m_imgBlended->imageData[ptv * 3 + 1] = 0;
      m_imgBlended->imageData[ptv * 3 + 2] = 0;
      if (m_imgMask[2]->imageData[ptv] != 0) continue;
      // NICT start
      if ((abs(((DepthType*)DepthLeft->imageData[ptv]) - ((DepthType*)DepthRight->imageData[ptv])) < m_iDepthBlendDiff)) // left and right are close to each other (NICT)
// NICT end
      {
        ((ImageType*)m_imgBlended->imageData)[ptv * 3 + 0] = CLIP3((((ImageType*)ImageLeft->imageData)[ptv * 3 + 0] * m_dWeightLeft + ((ImageType*)ImageRight->imageData)[ptv * 3 + 0] * m_dWeightRight) / (m_dWeightLeft + m_dWeightRight), 0, MAX_LUMA - 1);
        ((ImageType*)m_imgBlended->imageData)[ptv * 3 + 1] = CLIP3((((ImageType*)ImageLeft->imageData)[ptv * 3 + 1] * m_dWeightLeft + ((ImageType*)ImageRight->imageData)[ptv * 3 + 1] * m_dWeightRight) / (m_dWeightLeft + m_dWeightRight), 0, MAX_LUMA - 1);
        ((ImageType*)m_imgBlended->imageData)[ptv * 3 + 2] = CLIP3((((ImageType*)ImageLeft->imageData)[ptv * 3 + 2] * m_dWeightLeft + ((ImageType*)ImageRight->imageData)[ptv * 3 + 2] * m_dWeightRight) / (m_dWeightLeft + m_dWeightRight), 0, MAX_LUMA - 1);
        // NICT start
#ifdef NICT_IVSRS
        if (m_uiIvsrsInpaint == 1)
        {
          ((DepthType*)m_imgBlendedDepth->imageData)[ptv] = CLIP3((((DepthType*)DepthLeft->imageData)[ptv] * m_dWeightLeft + ((DepthType*)DepthRight->imageData)[ptv] * m_dWeightRight) / (m_dWeightLeft + m_dWeightRight), 0, MAX_DEPTH - 1);
        }
#endif
        // NICT end	  
      }
      // NICT start
      else if ((((DepthType*)DepthLeft->imageData[ptv]) > ((DepthType*)DepthRight->imageData[ptv]))) //Fix to compare z // left is nearer (NICT)
// NICT end
      {
        m_imgBlended->imageData[ptv * 3 + 0] = ImageLeft->imageData[ptv * 3 + 0];
        m_imgBlended->imageData[ptv * 3 + 1] = ImageLeft->imageData[ptv * 3 + 1];
        m_imgBlended->imageData[ptv * 3 + 2] = ImageLeft->imageData[ptv * 3 + 2];
        // NICT start
#ifdef NICT_IVSRS
        if (m_uiIvsrsInpaint == 1)
        {
          m_imgBlendedDepth->imageData[ptv] = DepthLeft->imageData[ptv];
        }
# endif
        // NICT end	  
      }
      else /*if((m_imgMask[3]->imageData[ptv]!=0))*/ //Fix should be mixed together // Right is closer
      {
        m_imgBlended->imageData[ptv * 3 + 0] = ImageRight->imageData[ptv * 3 + 0];
        m_imgBlended->imageData[ptv * 3 + 1] = ImageRight->imageData[ptv * 3 + 1];
        m_imgBlended->imageData[ptv * 3 + 2] = ImageRight->imageData[ptv * 3 + 2];
        // NICT start
#ifdef NICT_IVSRS
        if (m_uiIvsrsInpaint == 1)
        {
          m_imgBlendedDepth->imageData[ptv] = DepthRight->imageData[ptv];
        }
# endif
        // NICT end	  
      }
    }
  }
  //m_imgBlended

#else 
  cvAddWeighted(m_pcViewSynthesisLeft->getVirtualImage(), m_dWeightLeft, m_pcViewSynthesisRight->getVirtualImage(), m_dWeightRight, 0, m_imgBlended); // Left VImage * LWeight + Rigt VImage * RWeight -> Blended
#endif

#ifdef NICT_IVSRS
  if (m_uiIvsrsInpaint == 1)
  {
    int hptv, ptv, var;
    bool holeflag, inpaintflag, lflag, mflag, rflag, filterflag;
    bool llflag, lmflag, mrflag, rrflag;
    int leftptv, rightptv, delta, midptv, lptv, mptv, rptv, refptv, varptv;
    int llptv, lmptv, mrptv, rrptv;
    int ref;
    int refdepth;
    int v, leftw, rightw, middle, left, right;

    for (int h = 0; h < m_uiHeight; h++)
    {
      holeflag = false;
      inpaintflag = false;
      hptv = h * m_uiWidth;
      for (int w = 0; w < m_uiWidth; w++)
      {
        ptv = w + hptv;
        if (m_imgMask[2]->imageData[ptv] != 0) // hole
        {
          if (w == 0) // hole start at 0
          {
            holeflag = true;
            leftw = w - 1;
            leftptv = ptv - 1;
            refdepth = MAX_DEPTH - 1; // set left depth(255) to ref depth
          }
          else if (w == m_uiWidth - 1) // hole end at W
          {
            inpaintflag = true;
            rightw = w + 1;
            rightptv = ptv + 1;
            // check hole start
            if (holeflag == false) // 1 pel hole
            {
              leftw = w - 1;
              leftptv = ptv - 1;
              refptv = leftptv;
              refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv]; // set left depth to ref depth
            }// else, left was set before
          }
          else if (holeflag == false) // hole start at middle
          {
            holeflag = true;
            leftw = w - 1;
            leftptv = ptv - 1;
            refptv = leftptv;
            refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv]; // set left depth to ref depth
          } // else, middle in hole, do nothing
        } // hole end
        else // Mask[2] = 0 not hole
        {
          if (holeflag == true) // hole end
          {
            holeflag = false;
            inpaintflag = true;
            rightw = w;
            rightptv = ptv;
            if (refdepth > (DepthType)m_imgBlendedDepth->imageData[rightptv]) // set right depth to ref depth
            {
              refptv = rightptv;
              refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
            }
          } // else, middle in not hole, do nothing
        } // not hole end

        // inpaint
        if (inpaintflag == true)
        {
          inpaintflag = false;
          // search top ref
          middle = (leftw + rightw) >> 1;
          left = middle;
          right = m_uiWidth - 1 - middle;
          midptv = (leftptv + rightptv) >> 1;
          mptv = midptv;

          lflag = mflag = rflag = false;
          llflag = lmflag = mrflag = rrflag = false;

          if (h < m_uiHeight * 2 / 3) // upper half, search sky
          {
            for (v = 1; v <= h; v++) // sesarch top
            {
              mptv -= m_uiWidth;
              if (mflag == false && m_imgMask[2]->imageData[mptv] == 0) // not hole 
              {
                mflag = true;
                if (refdepth > (DepthType) m_imgBlendedDepth->imageData[mptv])
                {
                  refptv = mptv;
                  refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
                }
              } // else hole, do nothing

              delta = v << 1;
              if (delta > left) llflag = true;
              llptv = mptv - delta;
              if (llflag == false && m_imgMask[2]->imageData[llptv] == 0) // not hole 
              {
                llflag = true;
                if (refdepth > (DepthType)m_imgBlendedDepth->imageData[llptv])
                {
                  refptv = llptv;
                  refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
                }
              } // else hole, do nopthing

              delta = v;
              if (delta > left) lflag = true;
              lptv = mptv - delta;
              if (lflag == false && m_imgMask[2]->imageData[lptv] == 0) // not hole 
              {
                lflag = true;
                if (refdepth > (DepthType)m_imgBlendedDepth->imageData[lptv])
                {
                  refptv = lptv;
                  refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
                }
              } // else hole, do nopthing

              delta = v >> 1;
              if (delta > left) lmflag = true;
              lmptv = mptv - delta;
              if (lmflag == false && m_imgMask[2]->imageData[lmptv] == 0) // not hole 
              {
                lmflag = true;
                if (refdepth > (DepthType)m_imgBlendedDepth->imageData[lmptv])
                {
                  refptv = lmptv;
                  refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
                }
              } // else hole, do nopthing

              delta = v >> 1;
              if (delta > right) mrflag = true;
              mrptv = mptv + delta;
              if (mrflag == false && m_imgMask[2]->imageData[mrptv] == 0) // not hole 
              {
                mrflag = true;
                if (refdepth > (DepthType)m_imgBlendedDepth->imageData[mrptv])
                {
                  refptv = mrptv;
                  refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
                }
              } // else hole, do nothing

              delta = v;

              if (delta > right) rflag = true;
              rptv = mptv + delta;
              if (rflag == false && m_imgMask[2]->imageData[rptv] == 0) // not hole 
              {
                rflag = true;
                if (refdepth > (DepthType)m_imgBlendedDepth->imageData[rptv])
                {
                  refptv = rptv;
                  refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
                }
              } // else hole, do nothing

              delta = v << 1;
              if (delta > right) rrflag = true;
              rrptv = mptv + delta;
              if (rrflag == false && m_imgMask[2]->imageData[rrptv] == 0) // not hole 
              {
                rrflag = true;
                if (refdepth > (DepthType)m_imgBlendedDepth->imageData[rrptv])
                {
                  refptv = rrptv;
                  refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
                }
              } // else hole, do nothing

              if (llflag == true && lflag == true && lmflag == true && mflag == true && mrflag == true && rflag == true && rrflag == true) break; // stop search
            } // v end
          } // else lower half, do followings

            // check bottom half, search farthest object
          mptv = midptv;
          lflag = mflag = rflag = false;
          llflag = lmflag = mrflag = rrflag = false;

          for (v = 1; v < m_uiHeight - h; v++) // sesarch bottom
          {
            mptv += m_uiWidth;
            if (mflag == false && m_imgMask[2]->imageData[mptv] == 0) // not hole 
            {
              mflag = true;
              if (refdepth > (DepthType) m_imgBlendedDepth->imageData[mptv])
              {
                refptv = mptv;
                refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
              }
            }

            delta = v << 1;
            if (delta > left) llflag = true;
            llptv = mptv - delta;

            if (llflag == false && m_imgMask[2]->imageData[llptv] == 0) // not hole 
            {
              llflag = true;
              if (refdepth > (DepthType)m_imgBlendedDepth->imageData[llptv])
              {
                refptv = llptv;
                refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
              }
            }

            delta = v;
            if (delta > left) lflag = true;
            lptv = mptv - delta;
            if (lflag == false && m_imgMask[2]->imageData[lptv] == 0) // not hole 
            {
              lflag = true;
              if (refdepth > (DepthType)m_imgBlendedDepth->imageData[lptv])
              {
                refptv = lptv;
                refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
              }
            }

            delta = v >> 1;
            if (delta > left) lmflag = true;
            lmptv = mptv - delta;
            if (lmflag == false && m_imgMask[2]->imageData[lmptv] == 0) // not hole 
            {
              lmflag = true;
              if (refdepth > (DepthType)m_imgBlendedDepth->imageData[lmptv])
              {
                refptv = lmptv;
                refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
              }
            }

            delta = v >> 1;
            if (delta > right) mrflag = true;
            mrptv = mptv + delta;
            if (mrflag == false && m_imgMask[2]->imageData[mrptv] == 0) // not hole 
            {
              mrflag = true;
              if (refdepth > (DepthType)m_imgBlendedDepth->imageData[mrptv])
              {
                refptv = mrptv;
                refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
              }
            }

            delta = v;
            if (delta > right) rflag = true;
            rptv = mptv + delta;
            if (rflag == false && m_imgMask[2]->imageData[rptv] == 0) // not hole 
            {
              rflag = true;
              if (refdepth > (DepthType)m_imgBlendedDepth->imageData[rptv])
              {
                refptv = rptv;
                refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
              }
            }

            delta = v << 1;
            if (delta > right) rrflag = true;
            rrptv = mptv + delta;
            if (rrflag == false && m_imgMask[2]->imageData[rrptv] == 0) // not hole 
            {
              rrflag = true;
              if (refdepth > (DepthType)m_imgBlendedDepth->imageData[rrptv])
              {
                refptv = rrptv;
                refdepth = (DepthType)m_imgBlendedDepth->imageData[refptv];
              }
            }

            if (llflag == true && lflag == true && lmflag == true && mflag == true && mrflag == true && rflag == true && rrflag == true) break;
          } // v for bottom half

          // inpaint
      //		if(refdepth == 255) continue;	// do nothing	
          ref = refptv * 3;
          for (varptv = leftptv + 1; varptv < rightptv; varptv++)
          {
            var = varptv * 3;
            m_imgBlended->imageData[var] = m_imgBlended->imageData[ref];
            m_imgBlended->imageData[var + 1] = m_imgBlended->imageData[ref + 1];
            m_imgBlended->imageData[var + 2] = m_imgBlended->imageData[ref + 2];
          }
        } // else no inpaintflag, do nothing

      } // for w
    } // for h

    // NICT  cvInpaint(m_imgBlended, m_imgMask[2], m_imgInterpolatedView, 5, CV_INPAINT_NS); // inpaint
    cvErode(m_imgMask[2], m_imgMask[2]); // use pre-inpainted pixels for smoothing
    cvInpaint(m_imgBlended, m_imgMask[2], m_imgInterpolatedView, 3, CV_INPAINT_TELEA); // NICT use small kernel // smooth pre-inpainted area

    //  cvSaveImage("7.bmp", m_imgInterpolatedView); // NICT  

    cvOr(m_imgMask[3], m_imgMask[4], m_imgMask[2]); // filled hole mask

    // NICT Edge filter start
    // Horizontal edge detect
    int varm6, varm5, varm4, varm3, varm2, varm1, var1, var2, var3, var4, var5;

    for (int h = 2; h < m_uiHeight - 1; h++)
    {
      holeflag = false;
      filterflag = false;
      hptv = h *  m_uiWidth;

      for (int w = 2; w < m_uiWidth - 1; w++)
      {
        ptv = w + hptv;
        if (holeflag == false && m_imgMask[2]->imageData[ptv] != 0) // filled hole edge
        {
          holeflag = true;
          if (abs((DepthType)m_imgBlendedDepth->imageData[ptv - 2] - (DepthType)m_imgBlendedDepth->imageData[ptv + 1]) < m_iDepthBlendDiff)
          {
            filterflag = true;
          }
          else
          {
            filterflag = false;
          }
        }
        else if (holeflag == true && m_imgMask[2]->imageData[ptv] == 0) // filled hole edge
        {
          holeflag = false;
          // NICT start
          //			if(abs((unsigned char)m_imgBlendedDepth->imageData[ptv-2] - (unsigned char)m_imgBlendedDepth->imageData[ptv+1]) < m_iDepthBlendDiff)
          if (abs((DepthType)m_imgBlendedDepth->imageData[ptv - 2] - (DepthType)m_imgBlendedDepth->imageData[ptv + 1]) < m_iDepthBlendDiff)
            // NICT end
            filterflag = true;
          else filterflag = false;
        }

        // Horizontal filtering
        if (filterflag == true)
        {
          filterflag = false;
          var = ptv * 3;
          varm6 = var - 6, varm5 = var - 5, varm4 = var - 4, varm3 = var - 3, varm2 = var - 2, varm1 = var - 1, var1 = var + 1, var2 = var + 2, var3 = var + 3, var4 = var + 4, var5 = var + 5;

          m_imgInterpolatedView->imageData[var] = ((unsigned char)m_imgInterpolatedView->imageData[varm6] + (unsigned char)m_imgInterpolatedView->imageData[var3]) >> 1;
          m_imgInterpolatedView->imageData[var1] = ((unsigned char)m_imgInterpolatedView->imageData[varm5] + (unsigned char)m_imgInterpolatedView->imageData[var4]) >> 1;
          m_imgInterpolatedView->imageData[var2] = ((unsigned char)m_imgInterpolatedView->imageData[varm4] + (unsigned char)m_imgInterpolatedView->imageData[var5]) >> 1;

          m_imgInterpolatedView->imageData[varm3] = ((unsigned char)m_imgInterpolatedView->imageData[varm6] + (unsigned char)m_imgInterpolatedView->imageData[var]) >> 1;
          m_imgInterpolatedView->imageData[varm2] = ((unsigned char)m_imgInterpolatedView->imageData[varm5] + (unsigned char)m_imgInterpolatedView->imageData[var1]) >> 1;
          m_imgInterpolatedView->imageData[varm1] = ((unsigned char)m_imgInterpolatedView->imageData[varm4] + (unsigned char)m_imgInterpolatedView->imageData[var2]) >> 1;

          m_imgInterpolatedView->imageData[var3] = ((unsigned char)m_imgInterpolatedView->imageData[var] + (unsigned char)m_imgInterpolatedView->imageData[var3]) >> 1;
          m_imgInterpolatedView->imageData[var4] = ((unsigned char)m_imgInterpolatedView->imageData[var1] + (unsigned char)m_imgInterpolatedView->imageData[var4]) >> 1;
          m_imgInterpolatedView->imageData[var5] = ((unsigned char)m_imgInterpolatedView->imageData[var2] + (unsigned char)m_imgInterpolatedView->imageData[var5]) >> 1;

        }
      }
    }

    // Vertical edge detect
    for (int w = 2; w < m_uiWidth - 1; w++)
    {
      holeflag = false;
      filterflag = false;
      for (int h = 2; h < m_uiHeight - 1; h++)
      {
        ptv = w + h *  m_uiWidth;
        if (holeflag == false && m_imgMask[2]->imageData[ptv] != 0) // filled hole edge
        {
          holeflag = true;
          // NICT start
          //			if(abs((unsigned char)m_imgBlendedDepth->imageData[ptv-2*m_uiWidth] - (unsigned char)m_imgBlendedDepth->imageData[ptv+m_uiWidth]) < m_iDepthBlendDiff)
          if (abs((DepthType)m_imgBlendedDepth->imageData[ptv - 2 * m_uiWidth] - (DepthType)m_imgBlendedDepth->imageData[ptv + m_uiWidth]) < m_iDepthBlendDiff)
            filterflag = true;
          else filterflag = false;
        }
        else if (holeflag == true && m_imgMask[2]->imageData[ptv] == 0) // filled hole edge
        {
          holeflag = false;
          // NICT start
          //			if(abs((unsigned char)m_imgBlendedDepth->imageData[ptv-2*m_uiWidth] - (unsigned char)m_imgBlendedDepth->imageData[ptv+m_uiWidth]) < m_iDepthBlendDiff)
          if (abs((DepthType)m_imgBlendedDepth->imageData[ptv - 2 * m_uiWidth] - (DepthType)m_imgBlendedDepth->imageData[ptv + m_uiWidth]) < m_iDepthBlendDiff)
            // NICT end
            filterflag = true;
          else filterflag = false;
        }

        // Vertical filtering
        if (filterflag == true)
        {
          filterflag = false;
          var = ptv * 3;
          varm6 = var - 6 * m_uiWidth, varm5 = varm6 + 1, varm4 = varm6 + 2, varm3 = var - 3 * m_uiWidth, varm2 = varm3 + 1, varm1 = varm3 + 2, var1 = var + 1, var2 = var + 2, var3 = var + 3 * m_uiWidth, var4 = var3 + 1, var5 = var3 + 2;

          m_imgInterpolatedView->imageData[var] = ((unsigned char)m_imgInterpolatedView->imageData[varm6] + (unsigned char)m_imgInterpolatedView->imageData[var3]) >> 1;
          m_imgInterpolatedView->imageData[var1] = ((unsigned char)m_imgInterpolatedView->imageData[varm5] + (unsigned char)m_imgInterpolatedView->imageData[var4]) >> 1;
          m_imgInterpolatedView->imageData[var2] = ((unsigned char)m_imgInterpolatedView->imageData[varm4] + (unsigned char)m_imgInterpolatedView->imageData[var5]) >> 1;

          m_imgInterpolatedView->imageData[varm3] = ((unsigned char)m_imgInterpolatedView->imageData[varm6] + (unsigned char)m_imgInterpolatedView->imageData[var]) >> 1;
          m_imgInterpolatedView->imageData[varm2] = ((unsigned char)m_imgInterpolatedView->imageData[varm5] + (unsigned char)m_imgInterpolatedView->imageData[var1]) >> 1;
          m_imgInterpolatedView->imageData[varm1] = ((unsigned char)m_imgInterpolatedView->imageData[varm4] + (unsigned char)m_imgInterpolatedView->imageData[var2]) >> 1;

          m_imgInterpolatedView->imageData[var3] = ((unsigned char)m_imgInterpolatedView->imageData[var] + (unsigned char)m_imgInterpolatedView->imageData[var3]) >> 1;
          m_imgInterpolatedView->imageData[var4] = ((unsigned char)m_imgInterpolatedView->imageData[var1] + (unsigned char)m_imgInterpolatedView->imageData[var4]) >> 1;
          m_imgInterpolatedView->imageData[var5] = ((unsigned char)m_imgInterpolatedView->imageData[var2] + (unsigned char)m_imgInterpolatedView->imageData[var5]) >> 1;

        }
      }
    }
    // NICT start
  }  // IvsrsInpaint = true
  else
  {
#endif
    //cvSaveImage("Mask2.bmp",m_imgMask[2]);
    cvSet(m_imgBlended, CV_RGB(0, 128, 128), m_imgMask[2]);
    cvInpaint(m_imgBlended, m_imgMask[2], m_imgInterpolatedView, 5, CV_INPAINT_NS);
    //inpaint(m_imgBlended, m_imgMask[2], m_imgInterpolatedView, 5, INPAINT_NS); 
#ifdef NICT_IVSRS
  }
#endif
// NICT end

  if (m_uiColorSpace) {
    pSynYuvBuffer->setDataFromImgBGR(m_imgInterpolatedView);
  }
  else
    pSynYuvBuffer->setDataFromImgYUV(m_imgInterpolatedView);

  //#ifdef _DEBUG
  //  m_ucSetup-=2;
  //#endif

  return 0;
}

IplImage*  CViewInterpolationGeneral::getImgSynthesizedViewLeft()
{
#ifdef _DEBUG
  if (m_ucSetup != 3) return NULL;
#endif

  return m_pcViewSynthesisLeft->getVirtualImage();
}

IplImage*  CViewInterpolationGeneral::getImgSynthesizedViewRight()
{
#ifdef _DEBUG
  if (m_ucSetup != 3) return NULL;
#endif

  return m_pcViewSynthesisRight->getVirtualImage();
}

//======================================created by Ying Chen =====================================
//===============Tampere University of Technology (TUT)/Nokia Research Center (NRC)===============

#ifdef WIN32
#pragma warning(disable : 4996)
#endif

#include "ParameterBase.h"
#include "ParameterViewInterpolation.h"

#include <string> 

#ifndef MSYS_WIN32
#define stricmp strcasecmp
#endif

#define equal(a,b)  (!stricmp((a),(b)))


//using namespace std;

/* **********
 *    Implementation of CCameraParameters
 * ********** */
CCameraParameters::CCameraParameters()
{
  int i, j;
  
  for (i = 0; i < 3; i++)
  {
    for (j = 0; j < 3; j++)
    {
      m_fIntrinsicMatrix[j][i] = 0.0;
      m_fExtrinsicMatrix[j][i] = 0.0;
    }
    m_fTranslationVector[i] = 0.0;
  }
}

CCameraParameters& CCameraParameters::operator = (CCameraParameters& src)
{
  int i, j;
  
  for (i = 0; i < 3; i++)
  {
    for (j = 0; j < 3; j++)
    {
      m_fIntrinsicMatrix[j][i] = src.m_fIntrinsicMatrix[j][i];
      m_fExtrinsicMatrix[j][i] = src.m_fExtrinsicMatrix[j][i];
    }
    m_fTranslationVector[i] = src.m_fTranslationVector[i];
  }
  
  return (*this);
}


/* **********
 *    Implementation of CParameterViewInterpolation
 * ********** */
CParameterViewInterpolation::CParameterViewInterpolation()
: m_uiDepthType          ( 0    )
, m_uiSourceWidth        ( 0    )
, m_uiSourceHeight        ( 0    )
, m_uiStartFrame                ( 0     ) 
, m_uiNumberOfFrames      ( 0    )
, m_dLeftNearestDepthValue    ( 0.0  )
, m_dLeftFarthestDepthValue    ( 0.0  )
, m_dRightNearestDepthValue    ( 0.0  )
, m_dRightFarthestDepthValue  ( 0.0  )
, m_cCameraParameterFile    (    )
, m_cLeftCameraName        (    )
, m_cRightCameraName      (    )
, m_cVirtualCameraName        (    )
, m_cLeftViewImageName      (    )
, m_cLeftDepthMapName      (    )
, m_cRightViewImageName      (    )
, m_cRightDepthMapName      (    )
, m_cOutputVirViewImageName    (    )
//, m_cVirtualViewImageName    (    )
#ifdef NICT_IVSRS
// NICT start  
 , m_uiIvsrsInpaint           ( 1   )  // NICT add iVSRS inpaint flag m_uiIvsrsInpaint to CParameterViewInterpolation class
// NICT end
#endif
, m_iSplattingOption (2)    //!> 0: Disable splatting; 1: Enable splatting; 2: Splatting only along boundaries
, m_iBoundaryGrowth(40)     //!> Only useful for SplattingOption 2
, m_iMergingOption(2)       //!> 0: Z_buffer, 1: camera distance weighting. 2: Z_buffer + hole counting + camera distance weighting
, m_iDepthThreshold(75)     //!> Only useful for MergingOption 2
, m_iHoleCountThreshold(30) //!> Only useful for MergingOption 2
, m_iTemporalImprovementOption(1) //0:disable; 1: enable, Zhejiang, May, 4
, m_iWarpEnhancementOption(0) //0:disable; 1: enable
, m_iCleanNoiseOption(0)    //0:diable; 1: enable
{
}

CParameterViewInterpolation::~CParameterViewInterpolation()
{
  release();
}

/*
 * \brief
 *    Initilize the parameters
 *
 * \return
 *    -1: Fail
 *     1: Succeed
 */

Int
CParameterViewInterpolation::Init( Int argc, Char**  argv )
{
  if ( argc < 2 ) return -1;
  
  std::string cFilename = argv[1] ; 

  setup();

  if(xReadFromFile( cFilename )!=1) return -1;
  if(xReadFromCommandLine(argc, argv)!=1) return -1;
#if 1
  xPrintParam();
#endif
  release();
  
  if(xReadCameraParameters() != 1) return -1;
  
  if(xValidation() != 1) return -1;

  return 1;  
}

Int
CParameterViewInterpolation::xPrintUsage( Char **argv )
{
  printf("\n supported options:\n\n");
  printf(" Parameter File Name\n\n");
  printf("\n");
  return 1;
}

UInt
CParameterViewInterpolation::setup()
{
  UInt uiParLnCount=0;

  m_pCfgLines[uiParLnCount++] = new ConfigLineUInt("DepthType",                             & m_uiDepthType,               0 );

  m_pCfgLines[uiParLnCount++] = new ConfigLineUInt("SourceWidth"   ,                        & m_uiSourceWidth ,            1280 );
  m_pCfgLines[uiParLnCount++] = new ConfigLineUInt("SourceHeight",                          & m_uiSourceHeight,            960 );
  m_pCfgLines[uiParLnCount++] = new ConfigLineUInt("TotalNumberOfFrames",                   & m_uiNumberOfFrames,          300 );
  m_pCfgLines[uiParLnCount++] = new ConfigLineUInt("StartFrame",                            & m_uiStartFrame,              0 ); // DT
  
  m_pCfgLines[uiParLnCount++] = new ConfigLineDbl ("LeftNearestDepthValue",                 & m_dLeftNearestDepthValue,    0.0 );
  m_pCfgLines[uiParLnCount++] = new ConfigLineDbl ("LeftFarthestDepthValue",                & m_dLeftFarthestDepthValue,   0.0 );
  m_pCfgLines[uiParLnCount++] = new ConfigLineDbl ("RightNearestDepthValue",                & m_dRightNearestDepthValue,   0.0 );
  m_pCfgLines[uiParLnCount++] = new ConfigLineDbl ("RightFarthestDepthValue",               & m_dRightFarthestDepthValue,  0.0 );
  
  m_pCfgLines[uiParLnCount++] = new ConfigLineStr ("CameraParameterFile",                   & m_cCameraParameterFile,      "cam_param.txt" );
  m_pCfgLines[uiParLnCount++] = new ConfigLineStr ("LeftCameraName",                        & m_cLeftCameraName,           "param_dog38");
  m_pCfgLines[uiParLnCount++] = new ConfigLineStr ("VirtualCameraName",                     & m_cVirtualCameraName,        "param_dog39");
  m_pCfgLines[uiParLnCount++] = new ConfigLineStr ("RightCameraName",                       & m_cRightCameraName,          "param_dog41");
  m_pCfgLines[uiParLnCount++] = new ConfigLineStr ("LeftViewImageName",                     & m_cLeftViewImageName,        "dog038.yuv");
  m_pCfgLines[uiParLnCount++] = new ConfigLineStr ("RightViewImageName",                    & m_cRightViewImageName,       "dog041.yuv");
  m_pCfgLines[uiParLnCount++] = new ConfigLineStr ("LeftDepthMapName",                      & m_cLeftDepthMapName,         "depth_dog038.yuv" );
  m_pCfgLines[uiParLnCount++] = new ConfigLineStr ("RightDepthMapName",                     & m_cRightDepthMapName,        "depth_dog041.yuv" );
  //m_pCfgLines[uiParLnCount++] = new ConfigLineStr ("ReferenceVirtualViewImageName",         & m_cVirtualViewImageName,     "dog039.yuv" );
  m_pCfgLines[uiParLnCount++] = new ConfigLineStr ("OutputVirtualViewImageName",            & m_cOutputVirViewImageName,   "dog_virtual039.yuv");
  m_pCfgLines[uiParLnCount++] = new ConfigLineUInt("ColorSpace",            & m_uiColorSpace ,            0 );
  m_pCfgLines[uiParLnCount++] = new ConfigLineUInt("Precision",             & m_uiPrecision ,             2 );
  m_pCfgLines[uiParLnCount++] = new ConfigLineUInt("Filter",                & m_uiFilter ,                1 );
  m_pCfgLines[uiParLnCount++] = new ConfigLineUInt("SynthesisMode",         & m_uiSynthesisMode ,         0 );
  m_pCfgLines[uiParLnCount++] = new ConfigLineUInt("BoundaryNoiseRemoval",  & m_uiBoundaryNoiseRemoval ,  0 );  
  m_pCfgLines[uiParLnCount++] = new ConfigLineUInt("ViewBlending",          & m_uiViewBlending,           0 ); 
#ifdef POZNAN_DEPTH_BLEND
  m_pCfgLines[uiParLnCount++] = new ConfigLineInt ("DepthBlendDifference",  & m_iDepthBlendDiff,          5 ); 
#endif
#ifdef NICT_IVSRS
// NICT start  
  m_pCfgLines[uiParLnCount++] = new ConfigLineUInt("IvsrsInpaint",          & m_uiIvsrsInpaint,             1 );  // NICT relate iVSRS inpaint flag from cfg file to its instance
// NICT end
#endif
  
  // 1-D synthesis mode, algorithm parameters
  m_pCfgLines[uiParLnCount++] = new ConfigLineInt ("SplattingOption",       & m_iSplattingOption,         2 ); 
  m_pCfgLines[uiParLnCount++] = new ConfigLineInt ("BoundaryGrowth",        & m_iBoundaryGrowth,          40 ); 
  m_pCfgLines[uiParLnCount++] = new ConfigLineInt ("MergingOption",         & m_iMergingOption,           2 ); 
  m_pCfgLines[uiParLnCount++] = new ConfigLineInt ("DepthThreshold",        & m_iDepthThreshold,          75 ); 
  m_pCfgLines[uiParLnCount++] = new ConfigLineInt ("HoleCountThreshold",    & m_iHoleCountThreshold,      30 ); 
  m_pCfgLines[uiParLnCount++] = new ConfigLineInt ("TemporalImprovementOption",    & m_iTemporalImprovementOption,      1 ); //Zhejiang, July 16
  m_pCfgLines[uiParLnCount++] = new ConfigLineInt ("WarpEnhancementOption", & m_iWarpEnhancementOption,    1  );
  m_pCfgLines[uiParLnCount++] = new ConfigLineInt ("CleanNoiseOption",      & m_iCleanNoiseOption,         1  );

  m_pCfgLines[uiParLnCount] = NULL;

  return uiParLnCount;
}

/*
 * \brief
 *    Read the camera parameters from file to memory
 * \return
 *    1: Succeed
 *    0: Fail
 */
UInt
CParameterViewInterpolation::xReadCameraParameters()
{
  int   i;
  const char* cameraId[3];
  char id[255];  // To store the cam/view id temporally
  int read=0;
  double gomi[2];
  int   found;
  FILE *fp;
  
  if((fp = fopen(m_cCameraParameterFile.c_str(), "rt"))==NULL)
  {
    fprintf(stderr, "Can't open camera parameter file: %s\n", m_cCameraParameterFile.c_str());
    return 0;
  }
  
  cameraId[0] = m_cLeftCameraName.c_str();
  cameraId[1] = m_cVirtualCameraName.c_str();
  cameraId[2] = m_cRightCameraName.c_str();

  for (i = 0; i < 3; i++)
  {
    if( fseek(fp, 0, SEEK_SET) ) return 0;

    found = 0;
    while( fscanf(fp, "%s", id) == 1 )
    {
      read = 0;
      if( strcmp(cameraId[i], id) == 0 )
      {
        read += fscanf(fp, "%lf %lf %lf", &m_camParam[i].m_fIntrinsicMatrix[0][0], &m_camParam[i].m_fIntrinsicMatrix[0][1], &m_camParam[i].m_fIntrinsicMatrix[0][2]);
        read += fscanf(fp, "%lf %lf %lf", &m_camParam[i].m_fIntrinsicMatrix[1][0], &m_camParam[i].m_fIntrinsicMatrix[1][1], &m_camParam[i].m_fIntrinsicMatrix[1][2]);
        read += fscanf(fp, "%lf %lf %lf", &m_camParam[i].m_fIntrinsicMatrix[2][0], &m_camParam[i].m_fIntrinsicMatrix[2][1], &m_camParam[i].m_fIntrinsicMatrix[2][2]);
        read += fscanf(fp, "%lf %lf", &gomi[0], &gomi[1]);
        read += fscanf(fp, "%lf %lf %lf %lf", &m_camParam[i].m_fExtrinsicMatrix[0][0], &m_camParam[i].m_fExtrinsicMatrix[0][1], &m_camParam[i].m_fExtrinsicMatrix[0][2], &m_camParam[i].m_fTranslationVector[0] );
        read += fscanf(fp, "%lf %lf %lf %lf", &m_camParam[i].m_fExtrinsicMatrix[1][0], &m_camParam[i].m_fExtrinsicMatrix[1][1], &m_camParam[i].m_fExtrinsicMatrix[1][2], &m_camParam[i].m_fTranslationVector[1] );
        read += fscanf(fp, "%lf %lf %lf %lf", &m_camParam[i].m_fExtrinsicMatrix[2][0], &m_camParam[i].m_fExtrinsicMatrix[2][1], &m_camParam[i].m_fExtrinsicMatrix[2][2], &m_camParam[i].m_fTranslationVector[2] );
        if(read!=23) return 0;
        found = 1;
      }
    }
    
    if (found == 0)
    {
      printf("Camera \"%s\" is not found in the camera parameter file.\n", cameraId[i]);
      return 0;
    }
  }

  return 1;
}


/*
 * \brief
 *    Check whether the config parameters are valid or not
 * \return
 *    1: Succeed
 *    0: Fail
 */
UInt
CParameterViewInterpolation::xValidation()
{
  if (m_uiSynthesisMode == 1) // 1D mode
  {
    if (m_uiDepthType == 1)
    {
      printf("Warning: The depth levels must be measured against the cameras. If the world coordinate system does not\n");
      printf("originate from the cameras, synthesis results may be incorrect.\n");
    }
    if (m_uiColorSpace != 0)
    {
      printf("Error: The input format must be in YUV 420 with 1D synthesis mode, for the time being.\n");
      printf("       Check ColorSpace.\n");
      return 0;
    }
  }

  // More checks for general mode:
  if (m_uiSynthesisMode == 0) // general mode
  {
  }
  
  return 1;
}

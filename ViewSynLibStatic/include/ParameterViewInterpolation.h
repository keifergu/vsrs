//======================================created by Ying Chen =====================================
//===============Tampere University of Technology (TUT)/Nokia Research Center (NRC)===============

#ifndef AFX_PARAMETER_VIEW_INTERPOLATION_H
#define AFX_PARAMETER_VIEW_INTERPOLATION_H

#include "iostream"
#include "string"

#include "ParameterBase.h"
#include "version.h"

//using namespace std;


/*
 * \brief
 *    Store the parameters of one camera
 *
 */
class CCameraParameters
{
  public:
    double m_fIntrinsicMatrix[3][3];
    double m_fExtrinsicMatrix[3][3];
    double m_fTranslationVector[3];
    
    CCameraParameters();
    ~CCameraParameters() {};
    
    CCameraParameters& operator = (CCameraParameters& src);
};

/*
 * Read parameter values from config file
 */

class CParameterViewInterpolation : public ParameterBase
{

public:
  CParameterViewInterpolation();
  virtual ~CParameterViewInterpolation();

  Int     Init( Int argc, Char** argv);
  
  Void    setDepthType                ( UInt ui )        { m_uiDepthType           = ui; }
  Void    setSourceWidth              ( UInt ui )        { m_uiSourceWidth         = ui; }
  Void    setSourceHeight             ( UInt ui )        { m_uiSourceHeight        = ui; }
  Void    setNumberOfFrames           ( UInt ui )        { m_uiNumberOfFrames      = ui; }
  Void    setStartFrame        ( UInt ui )        { m_uiStartFrame      = ui; }

  Void    setLeftNearestDepthValue  ( Double d  )         { m_dLeftNearestDepthValue    = d;  }
  Void    setLeftFarthestDepthValue  ( Double d  )         { m_dRightFarthestDepthValue  = d;  }
  Void    setRightNearestDepthValue  ( Double d  )         { m_dLeftNearestDepthValue    = d;  }
  Void    setRightFarthestDepthValue  ( Double d  )         { m_dRightFarthestDepthValue  = d;  }
  
  Void    setCameraParameterFile      ( std::string s )      { m_cCameraParameterFile  = s;  }
  
  Void    setLeftCameraName      ( std::string s )      { m_cLeftCameraName      = s;  }
  Void    setVirtualCameraName    ( std::string s )      { m_cVirtualCameraName  = s;  }
  Void    setRightCameraName      ( std::string s )      { m_cRightCameraName    = s;  }

  Void    setLeftViewImageName    ( std::string s )      { m_cLeftViewImageName    = s;  }
  Void    setRightViewImageName    ( std::string s )      { m_cRightViewImageName    = s;  }
  Void    setLeftDepthMapName      ( std::string s )      { m_cLeftDepthMapName      = s;  }  
  Void    setRightDepthMapName    ( std::string s )      { m_cRightDepthMapName    = s;  }
  //Void    setVirtualViewImageName    ( std::string s )      { m_cVirtualViewImageName  =  s;  }
  
  Void    setOutputVirViewImageName   ( std::string s )   { m_cOutputVirViewImageName = s;  }
  Void    setColorSpace        (UInt ui)      { m_uiColorSpace          = ui; }
  Void    setPrecision        (UInt ui)      { m_uiPrecision            = ui; }
  Void    setFilter          (UInt ui)      { m_uiFilter              = ui; }

#ifdef NICT_IVSRS
// NICT start  
  Void    setIvsrsInpaint   (UInt ui )      { m_uiIvsrsInpaint        = ui; }
// NICT end
#endif

  Void    setSynthesisMode      (UInt ui)      { m_uiSynthesisMode        = ui; }

  UInt    setBoundaryNoiseRemoval    (UInt ui)      { m_uiBoundaryNoiseRemoval    = ui; }

  Void    setViewBlending      (UInt ui)      { m_uiViewBlending      = ui; } 

#ifdef POZNAN_DEPTH_BLEND
  Void    setDepthBlendDiff    (Int i)        { m_iDepthBlendDiff     = i; };
#endif

  UInt    getDepthType              ()      { return m_uiDepthType;      }
  UInt    getSourceWidth            ()      { return m_uiSourceWidth;    }
  UInt    getSourceHeight           ()      { return m_uiSourceHeight;   }
  UInt    getNumberOfFrames         ()      { return m_uiNumberOfFrames; }
  UInt    getStartFrame             ()      { return m_uiStartFrame; }

  Double  getLeftNearestDepthValue  ()        { return m_dLeftNearestDepthValue;  }
  Double  getLeftFarthestDepthValue  ()        { return m_dLeftFarthestDepthValue;  }
  Double  getRightNearestDepthValue  ()        { return m_dRightNearestDepthValue;  }
  Double  getRightFarthestDepthValue  ()        { return m_dRightFarthestDepthValue;  }
  
  const std::string   getCameraParameterFile    ()    { return m_cCameraParameterFile ;  }

  const std::string   getLeftCameraName         ()    { return m_cLeftCameraName;      }
  const std::string   getVirtualCameraName      ()    { return m_cVirtualCameraName;  }
  const std::string   getRightCameraName        ()    { return m_cRightCameraName;    }

  const std::string   getLeftViewImageName      ()    { return m_cLeftViewImageName;    }
  const std::string   getRightViewImageName     ()    { return m_cRightViewImageName;    }
  const std::string   getLeftDepthMapName       ()    { return m_cLeftDepthMapName;      }  
  const std::string   getRightDepthMapName      ()    { return m_cRightDepthMapName;    }
  //const std::string   getVirtualViewImageName    ()   { return m_cVirtualViewImageName;  }
  
  const std::string   getOutputVirViewImageName ()    { return m_cOutputVirViewImageName ;  }
  
  Double getFocalLength() { return m_camParam[0].m_fIntrinsicMatrix[0][0]; } // Take the focal length from the left camera. We assume all the three cameras share the same focal length
  Double getLTranslationLeft()  { return m_camParam[1].m_fTranslationVector[0]  - m_camParam[0].m_fTranslationVector[0]; } // Tx(Syn) - Tx(Left)
  Double getLTranslationRight() { return m_camParam[1].m_fTranslationVector[0]  - m_camParam[2].m_fTranslationVector[0]; } // Tx(Syn) - Tx(Right)
  Double getduPrincipalLeft()   { return m_camParam[1].m_fIntrinsicMatrix[0][2] - m_camParam[0].m_fIntrinsicMatrix[0][2]; } // uxSyn - uxLeft;
  Double getduPrincipalRight()  { return m_camParam[1].m_fIntrinsicMatrix[0][2] - m_camParam[2].m_fIntrinsicMatrix[0][2]; } // uxSyn - uxRight;

  Double *getMat_Ex_Left() {return &m_camParam[0].m_fExtrinsicMatrix[0][0];}
  Double *getMat_Ex_Virtual() {return &m_camParam[1].m_fExtrinsicMatrix[0][0];}
  Double *getMat_Ex_Right() {return &m_camParam[2].m_fExtrinsicMatrix[0][0];}
  Double *getMat_In_Left() {return &m_camParam[0].m_fIntrinsicMatrix[0][0];}
  Double *getMat_In_Virtual() {return &m_camParam[1].m_fIntrinsicMatrix[0][0];}
  Double *getMat_In_Right() {return &m_camParam[2].m_fIntrinsicMatrix[0][0];}
  Double *getMat_Trans_Left() {return &m_camParam[0].m_fTranslationVector[0];}
  Double *getMat_Trans_Virtual() {return &m_camParam[1].m_fTranslationVector[0];}
  Double *getMat_Trans_Right() {return &m_camParam[2].m_fTranslationVector[0];}

  Double getLeftBaselineDistance() {return m_dLeftBaselineDistance; }
  Double getRightBaselineDistance() {return m_dRightBaselineDistance; }

  // Access algorithm parameter for 1D mode
  Int    getSplattingOption()     { return m_iSplattingOption; }
  Int    getBoudaryGrowth()       { return m_iBoundaryGrowth; }
  Int    getMergingOption()       { return m_iMergingOption; }  // Merging /Blending option for 1D mode
  Int    getDepthThreshold()      { return m_iDepthThreshold; }
  Int    getHoleCountThreshold()  { return m_iHoleCountThreshold; }
  Int    getTemporalImprovementOption()  { return m_iTemporalImprovementOption; } //Zhejiang, May,4
  Int    getWarpEnhancementOption()  { return m_iWarpEnhancementOption; }
  Int    getCleanNoiseOption()    { return m_iCleanNoiseOption; } 

  UInt    getColorSpace        ()      { return m_uiColorSpace; }

  UInt    getPrecision        ()      { return m_uiPrecision; }
  UInt    getFilter          ()      { return m_uiFilter; }

#ifdef NICT_IVSRS
// NICT start  
  UInt    getIvsrsInpaint    ()      { return m_uiIvsrsInpaint; }
// NICT end
#endif

  UInt    getSynthesisMode      ()      { return m_uiSynthesisMode; }
  UInt    getBoundaryNoiseRemoval    ()      { return m_uiBoundaryNoiseRemoval; }
  UInt    getViewBlending        ()    { return m_uiViewBlending; } //Blending option for General mode

#ifdef POZNAN_DEPTH_BLEND
  Int     getDepthBlendDiff      ()    { return m_iDepthBlendDiff; }
#endif

private:
  UInt    setup            ();
  UInt    xReadCameraParameters();  // Read camera parameters from file to mem
  UInt    xValidation();            // Check whether the inputs are valid
  Double m_dLeftBaselineDistance;
  Double m_dRightBaselineDistance;

protected:
  Int     xPrintUsage         ( Char**  argv );

protected:
  UInt            m_uiDepthType;
  UInt            m_uiSourceWidth;
  UInt            m_uiSourceHeight;
  UInt            m_uiNumberOfFrames;
  UInt            m_uiStartFrame; 

  Double          m_dLeftNearestDepthValue;
  Double          m_dLeftFarthestDepthValue;
  Double          m_dRightNearestDepthValue;
  Double          m_dRightFarthestDepthValue;
  
  std::string     m_cCameraParameterFile;
  
  std::string     m_cLeftCameraName;
  std::string     m_cRightCameraName;
  std::string     m_cVirtualCameraName;

  std::string     m_cLeftViewImageName;
  std::string     m_cLeftDepthMapName;
  std::string     m_cRightViewImageName;
  std::string     m_cRightDepthMapName;

  //std::string     m_cVirtualViewImageName;
  std::string     m_cOutputVirViewImageName;

  UInt      m_uiColorSpace;

  UInt      m_uiPrecision;
  UInt      m_uiFilter;

#ifdef NICT_IVSRS
// NICT start  
  UInt    m_uiIvsrsInpaint; // Type of inpaing used 0...conventional 1...NICT improved one
// NICT end
#endif

  UInt      m_uiSynthesisMode;
  UInt      m_uiBoundaryNoiseRemoval;
  UInt      m_uiViewBlending; 

#ifdef POZNAN_DEPTH_BLEND
  Int       m_iDepthBlendDiff;
#endif

  // Algorithm parameters for 1-D view synthesis mode
  //Int  m_iUpsampleRefs;        //!> 0: No upsampling for ref pictures; 1: Upsample ref pictures  >>>> Hide this parameter from config file
  
  Int  m_iSplattingOption;     //!> 0: Disable splatting; 1: Enable splatting; 2: Splatting only along boundaries
  Int  m_iBoundaryGrowth;      //!> Only useful for SplattingOption 2
  Int  m_iMergingOption;       //!> 0: Z_buffer, 1: camera distance weighting. 2: Z_buffer + hole counting + camera distance weighting
  Int  m_iDepthThreshold;      //!> Only useful for MergingOption 2
  Int  m_iHoleCountThreshold;  //!> Only useful for MergingOption 2
  Int  m_iTemporalImprovementOption;  //0: Disable; 1; Enable    Zhejiang,May,4
  Int  m_iWarpEnhancementOption;
  Int  m_iCleanNoiseOption;    //0: Disable; 1; Enable    
  
  // Camera parameters
  CCameraParameters m_camParam[3]; //!> 0: Left, 1: Center, 2: Right
};

#endif 

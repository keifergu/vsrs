//======================================created by Ying Chen =====================================
//===============Tampere University of Technology (TUT)/Nokia Research Center (NRC)===============

#ifndef AFX_PARAMETER_BASE_H
#define AFX_PARAMETER_BASE_H

#include "iostream"
#include "string"

#define UInt    unsigned int
#define Bool    bool
#define Double  double 
#define Int     int
#define Char    char
#define Void    void

#define MAX_CONFIG_PARAMS 256

//using namespace std ;

class ConfigLineBase
{
protected:
  ConfigLineBase(Char* pcTag, UInt uiType ) : m_cTag( pcTag ), m_uiType( uiType ) {}
  ConfigLineBase() {}
public:
  virtual ~ConfigLineBase() {}
  std::string&  getTag () { return m_cTag; }
  virtual Void  setVar ( std::string& rcValue ) = 0;
  virtual Void  fprintVar  ( FILE *fp ) = 0;
protected:
  std::string m_cTag;
  UInt m_uiType;
};

class ConfigLineStr : public ConfigLineBase
{
public:
  ConfigLineStr( Char* pcTag, std::string* pcPar, Char* pcDefault );
  Void setVar( std::string& pvValue );
  Void fprintVar( FILE *fp );
protected:
  std::string* m_pcPar;
};

class ConfigLineDbl : public ConfigLineBase
{
public:
  ConfigLineDbl( Char* pcTag, Double* pdPar, Double pdDefault );
  Void setVar( std::string& pvValue );
  Void fprintVar( FILE *fp );
protected:
  Double* m_pdPar;
};

class ConfigLineInt : public ConfigLineBase
{
public:
  ConfigLineInt( Char* pcTag, Int* piPar, Int piDefault );
  Void setVar( std::string& pvValue);
  Void fprintVar( FILE *fp );
protected:
  Int* m_piPar;
};

class ConfigLineUInt : public ConfigLineBase
{
public:
  ConfigLineUInt( Char* pcTag, UInt* puiPar, UInt puiDefault );
  Void setVar( std::string& pvValue);
  Void fprintVar( FILE *fp );
protected:
  UInt* m_puiPar;
};

class ConfigLineChar : public ConfigLineBase
{
public:
  ConfigLineChar( Char* pcTag, Char* pcPar, Char pcDefault );
  Void setVar( std::string& pvValue );
  Void fprintVar( FILE *fp );
protected:
  Char* m_pcPar;
};


class ParameterBase
{
public:
  ParameterBase                ();
  virtual  ~ParameterBase      () {};

  virtual Int  Init          ( Int argc, Char** argv) = 0;

  Int          xReadLine        ( FILE* hFile, std::string* pacTag );
  Int          xReadFromFile      ( std::string& rcFilename );

  Int          xReadFromCommandLine  (Int argc, Char **argv);
  Int          xReadCommandLine    ( char *buf, std::string* pacTag );
  
  Void         xPrintParam        ();

protected:
  virtual UInt setup   () = 0;
  Void         release ();

protected:
  ConfigLineBase*  m_pCfgLines[MAX_CONFIG_PARAMS];
};

#endif 

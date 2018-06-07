//======================================created by Ying Chen =====================================
//===============Tampere University of Technology (TUT)/Nokia Research Center (NRC)===============

#ifdef WIN32
#pragma warning(disable : 4996)
#endif

#include "ParameterBase.h"

#include <string> 

#ifndef MSYS_WIN32
#define stricmp strcasecmp
#endif

#define equal(a,b)  (!stricmp((a),(b)))


//using namespace std;

ConfigLineStr::ConfigLineStr( Char* pcTag, std::string* pcPar, Char* pcDefault ) : ConfigLineBase( pcTag, 1 ), m_pcPar( pcPar )
{
  *m_pcPar = pcDefault;
}

Void
ConfigLineStr::setVar( std::string& pvValue )
{
  *m_pcPar = pvValue;
}

Void
ConfigLineStr::fprintVar( FILE *fp )
{
  fprintf(fp, "%s : %s\n", m_cTag.c_str(), m_pcPar->c_str());
}

ConfigLineDbl::ConfigLineDbl( Char* pcTag, Double* pdPar, Double pdDefault ) :  ConfigLineBase( pcTag, 2 ), m_pdPar( pdPar )
{
  *m_pdPar = pdDefault;
}

Void
ConfigLineDbl::setVar( std::string& pvValue )
{
  *m_pdPar = atof( pvValue.c_str() );
}

Void
ConfigLineDbl::fprintVar( FILE *fp )
{
  fprintf(fp, "%s : %f\n", m_cTag.c_str(), *m_pdPar);
}

ConfigLineInt::ConfigLineInt( Char* pcTag, Int* piPar, Int piDefault ) : ConfigLineBase( pcTag, 3 ), m_piPar( piPar )
{
  *m_piPar = piDefault;
}

Void
ConfigLineInt::setVar( std::string& pvValue)
{
  *m_piPar = atoi( pvValue.c_str() );
}

Void
ConfigLineInt::fprintVar( FILE *fp )
{
  fprintf(fp, "%s : %d\n", m_cTag.c_str(), *m_piPar);
}

ConfigLineUInt::ConfigLineUInt( Char* pcTag, UInt* puiPar, UInt puiDefault ) : ConfigLineBase( pcTag, 4 ), m_puiPar( puiPar )
{
  *m_puiPar = puiDefault;
}

Void
ConfigLineUInt::setVar( std::string& pvValue)
{
  *m_puiPar = atoi( pvValue.c_str() );
}

Void
ConfigLineUInt::fprintVar( FILE *fp )
{
  fprintf(fp, "%s : %d\n", m_cTag.c_str(), *m_puiPar);
}

ConfigLineChar::ConfigLineChar( Char* pcTag, Char* pcPar, Char pcDefault ) : ConfigLineBase( pcTag, 5 ), m_pcPar( pcPar )
{
  *m_pcPar = pcDefault;
}

Void
ConfigLineChar::setVar( std::string& pvValue )
{
  *m_pcPar = (Char)atoi( pvValue.c_str() );
}

Void
ConfigLineChar::fprintVar( FILE *fp )
{
  fprintf(fp, "%s : %c\n", m_cTag.c_str(), *m_pcPar);
}

ParameterBase::ParameterBase()
{
  for (int i = 0; i < MAX_CONFIG_PARAMS; i++) // DT
    m_pCfgLines[i] = NULL;
}

Void
ParameterBase::release()
{
  UInt uiParLnCount = 0;
  while (m_pCfgLines[uiParLnCount] != NULL)
  {
    delete m_pCfgLines[uiParLnCount];
    m_pCfgLines[uiParLnCount] = NULL;
    uiParLnCount++;
  }
}

Void
ParameterBase::xPrintParam()
{
  UInt uiParLnCount = 0;
  while (m_pCfgLines[uiParLnCount] != NULL)
  {
    m_pCfgLines[uiParLnCount]->fprintVar(stdout);
    uiParLnCount++;
  }
}

Int
ParameterBase::xReadFromFile( std::string& rcFilename )
{
  std::string acTags[4];
  UInt        uiParLnCount = 0;
  UInt        uiLayerCnt   = 0;

  FILE *f = fopen( rcFilename.c_str(), "r");
  if( NULL == f )
  { 
    printf( "failed to open %s parameter file\n", rcFilename.c_str() );
    return -1;
  }

  while (!feof(f))
  {
    if ( xReadLine( f, acTags ) !=1) return -1;
    if ( acTags[0].empty() )
    {
      continue;
    }
    for (UInt ui=0; m_pCfgLines[ui] != NULL; ui++)
    {
//      printf("%s %s \n", acTags[0].c_str(), m_pCfgLines[ui]->getTag().c_str());
      if( acTags[0] == m_pCfgLines[ui]->getTag() )
      {
         m_pCfgLines[ui]->setVar( acTags[1] );
         break;
      }
    }
  }

  fclose( f );
  return 1; 
}

Int
ParameterBase::xReadFromCommandLine(Int argc, Char **argv)
{
  std::string acTags[4];
  Int i;
  UInt ui;
  
  for(i=2; i<argc; i++)
  {
    if ( xReadCommandLine(argv[i], acTags) != 1 )
    {
      printf("Unknown argument [%s]\n", argv[i]);
      continue;
    }
    if ( acTags[0].empty() )
    {
      continue;
    }
    for (ui=0; m_pCfgLines[ui] != NULL; ui++)
    {
      if( acTags[0] == m_pCfgLines[ui]->getTag() )
      {
         m_pCfgLines[ui]->setVar( acTags[1] );
         break;
      }
    }
    if(m_pCfgLines[ui] == NULL)
    {
      printf("Unknown parameter [%s]\n", acTags[0].c_str());
    }
  }

  return 1;
}

Int 
ParameterBase::xReadLine( FILE* hFile, std::string* pacTag )
{
  if ( pacTag == NULL) return -1;

  Int  n;
  UInt uiTagNum = 0;
  Bool          bComment  = false;
  std::string*  pcTag     = &pacTag[0];

  for( n = 0; n < 4; n++ )
  {
    pacTag[n] = "";
  }

  for( n = 0; ; n++ )
  {
    Char cChar = (Char) fgetc( hFile );
    if ( cChar == '\n' || feof( hFile ) ) return 1 ; 
    if   ( cChar == '#' )
    {
      bComment = true;
    }
    if( ! bComment )
    {
      if ( cChar == '\t' || cChar == ' ' ) // white space
      {
        if ( uiTagNum == 3 ) return -1 ;
        if( ! pcTag->empty() )
        {
          uiTagNum++;
          pcTag = &pacTag[uiTagNum]; 
        }
    }
    else
    {
      *pcTag += cChar;
     }
    }
  }
  return 1;
}

Int 
ParameterBase::xReadCommandLine( char *buf, std::string* pacTag )
{
  if ( pacTag == NULL) return -1;

  Int  n;
  UInt uiTagNum = 0;
  Bool          bComment  = false;
  std::string*  pcTag     = &pacTag[0];

  for( n = 0; n < 4; n++ )
  {
    pacTag[n] = "";
  }

  for(n=0; buf[n]!='\0' && buf[n]!='#'; n++)
  {
    if(buf[n]=='=')
    {
      if(uiTagNum==1) return -1;
      uiTagNum++;
      pcTag = &pacTag[uiTagNum]; 
      continue;
    }
    *pcTag += buf[n];
  }

  if(pcTag->empty() || uiTagNum==0) return -1;

  return 1;
}

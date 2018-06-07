/*
* View Synthesis Reference Software
* ****************************************************************************
* Copyright (c) 2011/2013 Poznan University of Technology
*
* Address: 
*   Poznan University of Technology, 
*   Polanka 3 Street, Poznan, Poland
*
* Authors:
*   Krysztof Wegner     <kwegner@multimedia.edu.pl>
*   Olgierd Stankiewicz <ostank@multimedia.edu.pl>
*
* You may use this Software for any non-commercial purpose, subject to the
* restrictions in this license. Some purposes which can be non-commercial are
* teaching, academic research, and personal experimentation. You may also
* distribute this Software with books or other teaching materials, or publish
* the Software on websites, that are intended to teach the use of the 
* Software.
*
* Reference to the following source document:
*
* Takanori Senoh, Kenji Yamamoto, Nobuji Tetsutani, Hiroshi Yasuda, Krzysztof Wegner, 
* "View Synthesis Reference Software (VSRS) 4.2 with improved inpainting and hole filing"
* ISO/IEC JTC1/SC29/WG11 MPEG2017/M40xx April 2017, Hobart, Australia
*
* are required in all documents that report any usage of the software.

* You may not use or distribute this Software or any derivative works in any
* form for commercial purposes. Examples of commercial purposes would be
* running business operations, licensing, leasing, or selling the Software, or
* distributing the Software for use with commercial products.
* ****************************************************************************
*/
//#include <string.h>
#include <time.h>

#include "version.h"
#include "yuv.h"
#include "ParameterViewInterpolation.h"
#include "ViewInterpolation.h"

#ifndef WIN32
#define BYTE unsigned char
#endif

int main(int argc , char *argv[]) 
{
  unsigned int n;

  CParameterViewInterpolation  cParameter;

  CViewInterpolation cViewInterpolation;
  CIYuv<ImageType> yuvBuffer;

#ifdef OUTPUT_COMPUTATIONAL_TIME
  clock_t start, finish, first;
  first = start = clock();
#endif

  printf("View Synthesis Reference Software (VSRS), Version %.1f\n", VERSION);
  printf("     - MPEG-I Visual, April 2017\n\n");
  
  if ( cParameter.Init( argc, argv ) != 1 ) return 0;

  if(!cViewInterpolation.Init(cParameter)) return 10;

  if(!yuvBuffer.resize(cParameter.getSourceHeight(), cParameter.getSourceWidth(), 420)) return 2;

  FILE *fin_view_r, *fin_view_l, *fin_depth_r, *fin_depth_l, *fout;

  if( (fin_view_l  = fopen(cParameter.getLeftViewImageName() .c_str(), "rb"))==NULL ||
      (fin_view_r  = fopen(cParameter.getRightViewImageName().c_str(), "rb"))==NULL ||
      (fin_depth_l = fopen(cParameter.getLeftDepthMapName()  .c_str(), "rb"))==NULL ||
      (fin_depth_r = fopen(cParameter.getRightDepthMapName() .c_str(), "rb"))==NULL ||
      (fout = fopen(cParameter.getOutputVirViewImageName()   .c_str(), "wb"))==NULL )
  {
    fprintf(stderr, "Can't open input file(s)\n");
    return 3;
  }
 
#ifdef OUTPUT_COMPUTATIONAL_TIME
  finish = clock();
  printf( "Initialization: %.4f sec\n", (double)(finish - start) / CLOCKS_PER_SEC);
  start = finish;
#endif

  for(n = cParameter.getStartFrame(); n < cParameter.getStartFrame() + cParameter.getNumberOfFrames(); n++) 
  {
    printf("frame number = %d ", n);

    if( !cViewInterpolation.getDepthBufferLeft() ->readOneFrame(fin_depth_l, n) || 
        !cViewInterpolation.getDepthBufferRight()->readOneFrame(fin_depth_r, n)  ) break;
    printf(".");

    cViewInterpolation.setFrameNumber( n - cParameter.getStartFrame()); // Zhejiang

    if(!yuvBuffer.readOneFrame(fin_view_l, n)) break;
    if(!cViewInterpolation.SetReferenceImage(1, &yuvBuffer)) break;
    printf(".");

    if(!yuvBuffer.readOneFrame(fin_view_r, n)) break;
    if(!cViewInterpolation.SetReferenceImage(0, &yuvBuffer)) break;
    printf(".");

    if(!cViewInterpolation.DoViewInterpolation( &yuvBuffer )) break;
    printf("."); 
    
    if(!yuvBuffer.writeOneFrame(fout)) break;

#ifdef OUTPUT_COMPUTATIONAL_TIME
    finish = clock();
    printf("->End (%.4f sec)\n", (double)(finish - start) / CLOCKS_PER_SEC);
    start = finish;
#else
    printf("->End\n");
#endif

  } // for n

  fclose(fout);
  fclose(fin_view_l);
  fclose(fin_view_r);
  fclose(fin_depth_l);
  fclose(fin_depth_r);
 
#ifdef OUTPUT_COMPUTATIONAL_TIME
  finish = clock();
  printf("Total: %.4f sec\n", ((double)(finish-first))/((double)CLOCKS_PER_SEC));
#endif

  return 0;
}


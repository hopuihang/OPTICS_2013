#ifndef TMJ_Scanfile_h
#define TMJ_Scanfile_h
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FL/fl_ask.H"

class Scanfile
{
   private:
      int  num_lines;
      char *sections; 

      struct Stacktype                // Stack information in ScanFile (Stackinfos im Scanfile)
      {
         int     num_slices;
         double  slice_thickness;
         double  slice_gap;
         double  fov_width;
         double  fov_height;
         int     img_width;
         int     img_height;
         double  offc_ap;
         double  offc_lr;
         double  offc_cc;
         double  ang_ap;
         double  ang_lr;
         double  ang_cc;
      };

      struct Referencetype           //Reference information in Scanfile.( Referenceinfos im Scanfile)
      {
         char    name[80];
         int     diameter;
         char    type;
         int     stack;
         double  pos_x;
         double  pos_y;
         double  pos_z;
         double  cal_x;
         double  cal_y;
         double  cal_z;
      };

      struct Ledtype                // LED Data.(Led Daten)
      {
         char    name[80];
         double  cal_x;
         double  cal_y;
         double  cal_z;
      };

      struct Movementtype           // (Movement/motion data) Transaction Data (Bewegungsdaten)
      {
         char   path[256];
         char   base[256];
         int    num_rec;
         char   max_leds[256];
         int    max_nr;
         char   max_val[256];
         char   ref_leds[256];
         int    ref_nr;
         char   ref_val[256];
      };

      int    scanwhere(char *text);
      void   scanget(char *text, int i, char c, int anz_words, char *resultat);

   public:
      char   name[256];
      char   firstname[256];
      char   anatomy[256];
      char   date[256];
      char   remarks[256];
      int    num_stacks;
      int    num_references;
      int    num_leds;
      char   raw_path[256];
      char   raw_files[256];
      char   raw_images[256];
      char   img_path[256];
      char   img_base[256];
      int    img_first;
      char   scanbase[256];
      char   database[256];
	  char   scandir[256];
	  char   shortScandir[256];
	  char   movdir[256];
	  char   scanfilename[256];

      Stacktype       *scanstack, *cancelstack;
      Referencetype   scanref[3];
      Ledtype         scanled[3];
      Movementtype    scanmov;

      char   anatomyleft;     // TMJ left / right
      int    num_cancelstacks;
      int    laststacknr;
      char   recfiles[10][256];     // MRI Rohfiles

      char   refsysFile[256];
      char   refFile1[256];
      char   refFile2[256];
	  int    reffile1number,reffile2number;

      Scanfile();
      ~Scanfile();
      char read(char *filename);
      char write(char *filename, char overwrite = 1);
      void errortext(char fehler, char* meldung);
};

#endif /* TMJ_Scanfile_h */

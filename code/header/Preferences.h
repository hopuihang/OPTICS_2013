#ifndef TMJ_Preferences_h
#define TMJ_Preferences_h

#pragma once

#include <fstream>
#include <stdio.h>
#include <string.h>
#include "FL/Fl.H"
#include <FL/fl_ask.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include "TmjLinear.h"

class Preferences
{
  public:
    char      datapath[256];
    char      scanpath[256];

    char      imgcopy[256];
    char      extracode[256];

    int       ObjectWindowWidth,
              ObjectWindowHeight;
    int       ObjectWindowX,
              ObjectWindowY;
    SbVec3f   ObjectWindowColor;

	int       LEDViewerWidth,
	          LEDViewerHeight;
	int       LEDViewerX,
  	          LEDViewerY;
	SbVec3f   LEDViewerColor; //RGB
	char      socketPort[256];
	char      socketHost[256];
	char      numCameras;
	SbVec3d   camSysRot[10]; //default 0,0,0
	SbVec3d   camSysTrans[10];
	char      camSysOverlap[10];
	int       LEDerror;
	int       LEDwarningRedRed;
	int       LEDwarningRed;
	int       LEDwarningYellow;
	char      optisController[256];
	char      optisTempDir[256];
//add
	int       LEDdrawstyle[18];
	int       LEDofObjectP[18];//18*1 matrix
	int       ObjectsAndReferencesP[6];
	SbVec3d   RefLed[5][20];
    char      loadfilename[256];
//add
	int       SegViewerWidth,
	          SegViewerHeight;
	int       SegViewerX,
  	          SegViewerY;
    char      m_acStdModelPath[256];
    char      m_cRecTiefe; //(accuracy of 3ecks area (num edge points)) genauigkeit der 3ecks fläche (anz randpunkte)
    char      m_cModelRekMode; //(Calculate new model mode) modus neu berechnen modell
	SbVec3f   SegObjColor;
	SbVec3f   SegSelObjColor;
	SbVec3f   SegPointColor;
	SbVec3f   SegNumberColor;

	int       dateType;

    int       stack_num_slices;
    double    stack_slice_thickness;
    double    stack_slice_gap;
    double    stack_fov_width;
    double    stack_fov_height;
    int       stack_img_width;
    int       stack_img_height;
    double    stack_offc_ap;
    double    stack_offc_lr;
    double    stack_offc_cc;
    double    stack_ang_ap;
    double    stack_ang_lr;
    double    stack_ang_cc;
    double    stack_mult_offc_ap;
    double    stack_mult_offc_lr;
    double    stack_mult_offc_cc;
    double    stack_mult_ang_ap;
    double    stack_mult_ang_lr;
    double    stack_mult_ang_cc;

	int       MRI_num_stacks;
	char      MRI_img_base[256];
	char      MRI_raw_images[256];
	int       MRI_first;

    Preferences();
    void        read_inifile();
    void        write_inifile();
    const char  *getdatapath();
    const char  *getscanpath();
    void        setdatapath(const char* pathname);
    void        setscanpath(const char* pathname);

    char      readViewFile(char *filename, SoPerspectiveCamera *cam,
		             SoDirectionalLight *light);
    char      writeViewFile(char *filename, SoPerspectiveCamera *cam,
		              SoDirectionalLight *light);
	void      convertDOScmd(char *cmd);
    void      RunApp(char *slFileName);
};


class TmjColorChip : public Fl_Widget {
  void draw();
public:
  uchar r,g,b;
  TmjColorChip(int X, int Y, int W, int H, char* l = NULL) : Fl_Widget(X,Y,W,H,l) {
    box(FL_ENGRAVED_FRAME);}
};


class Calibration
{
public:
	struct data
	{
		char    name[256];
        double  SphereBackX;
        double  SphereBackY;
        double  SphereBackZ;
        double  SphereFrontX;
        double  SphereFrontY;
        double  SphereFrontZ;
        double  SphereTopX;
        double  SphereTopY;
        double  SphereTopZ;
        double  LedBackX;
        double  LedBackY;
        double  LedBackZ;
        double  LedFrontX;
        double  LedFrontY;
        double  LedFrontZ;
        double  LedTopX;
        double  LedTopY;
        double  LedTopZ;
		data    *next;
	};

	data *list;

	Calibration();
	~Calibration();
	void read_file();
	void write_file();
};

class Commentlist
{
public:
	struct cdata
	{
		char    cname0[256];
		char    cname[32];
		int     ctype;
		cdata    *next;
	};
	cdata *clist;

	Commentlist();
	~Commentlist();
	void read_comfile();
};
#endif /* TMJ_Preferences_h */

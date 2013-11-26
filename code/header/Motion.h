#ifndef TMJ_Motion_h
#define TMJ_Motion_h

#include <Inventor/SbLinear.h>
#include <FL/fl_message.H>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Preferences.h"
#include "Scanfile.h"
#include "TmjLinear.h"
#include "TmjMFVec3d.h"
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoMFUShort.h>


static double coeff[] = {
                            0.015691868,
                            0.062767475,
                            0.070613409,
                            0.225392296,
                            0.251249107,
                            0.225392296,
                            0.070613409,
                            0.062767475,
                            0.015691868, 
                        };
						
struct MotionError
{
	long          step;
	int           led;
	MotionError   *next;
};

class Motion
{
   private:
      Scanfile *scnfile;
      Preferences *pref;
   public:
      SoMFVec3d    *led;
      SoMFVec3d    *oldled;
	  SoMFVec3f    *intensity;
	  SoMFVec3f    *oldintensity;
	  int          numCameras;
	  int          numCamSystems;
	  int          *numLEDs;
      SbVec3d      max_leds[6]; //6*3 matrix
      SbVec3d      ref_leds[6]; //6 groups of xyz values(6*3)matrix
      char         comment[256];
      char         flname[256];
	  long         num_steps;
	  SoMFUShort   *click;
	  SbVec3d      RefVec[3];//3*3 matrix
	  int          LEDofObject[10][18];//10*18 matrix
	  int          ObjectsAndReferences[10][6];//10*6 matrix
	  int          Speed[10];
	  int          oldLEDofObject[10][18];//10*18 matrix
	  int          oldObjectsAndReferences[10][6];//10*6 matrix
	  int          oldSpeed[10];
	  int          RSnr, RSchanged;
	  int          RSsysnr;
//060817
	  long         imp_steps;
	  SoMFVec3f    *impTraj;
	  int          numtraj;
	  char         trajcomm[10][256];
	  SbVec3d      ImpRefLed[18];
	  int          ImpObj[18];
      SbVec3d      refxyz[18]; //store ref led
	  double       freq;
      SbVec3d      refsysxyz[18]; //store reference system led

      SdMatrix mTomoToStack;
      SdMatrix mSpheresToLeds;
      SdMatrix mTomoToRef;
      SdMatrix mTomoToUVW;
      SdMatrix mTomoToXYZ;
      SdMatrix mTomoToXYZi;
      SdMatrix mTomoToJ3d;
      SdMatrix mStaticRefTrafo;
      SdMatrix mStaticMaxTrafo;
      SdMatrix mRefToJ3d;

	  SdMatrix animation_trafo[10],
		       fov_trafo[10],
			   mRef[10], mUVW[10];
	  SdMatrix animation_trafoR[10],
		       fov_trafoR[10],
			   mRefR[10], mUVWR[10];
	  SdMatrix ref_trafo;
	  SdMatrix mRef2[10],mUVW2[10];
	  SdMatrix mRefRef[10],mUVWRef[10];
	  SdMatrix mRefRefSys[10],mUVWRefSys[10];//10*camera system number
	  SdMatrix mRefImp[10],mUVWImp[10];
	  SdMatrix traj_trafo;
	  SdMatrix mObj[10];
	  SbVec3d pledvec[18];
	  double   animation_step;
	  double   old_step;

	  Motion(Preferences *p, Scanfile *s);
	  ~Motion();
	  char read_j3dfile(char *filename);
	  void ComputeMaxMean();
	  void ComputeRefMean();
	  void calculateReferences();
	  void TomoToSpheres();
	  void Vec3ToMatrix(SbVec3d &origo, SbVec3d &px, SbVec3d &pxy, SdMatrix &m);
	  void FixedXYZ(SdMatrix &rot, double a, double b, double c);
	  void SpheresToLeds();
	  void Vec2BaseTrans(SbVec3d base_from[3], SbVec3d base_to[3], SbVec3d from_origin,
		                 SbVec3d to_origin, SdMatrix &m);
	  void BaseTrans(SbVec3d from_base[3], SbVec3d to_base[3], SdMatrix &m);
	  void LowerToUpper(SbVec3d led[6], SdMatrix &m);
	  void RefToJ3d(SdMatrix &m);
	  void load_j3dfile(const char *filename);
//	  void calc_animationTrafo();
	  void Process();
	  void oneStepForward();
	  void oneStepBackward();
	  void resetStep();
	  void goToStep(double step);
	  SbMatrix getActualMatrix(int fov, long step, int camsysnr);
	  char getClick(char up);
      void CalcUVWtoXYZ(SbVec3d &uvw, SbVec3d &xyz);
      void CalcXYZtoUVW(SbVec3d &xyz, SbVec3d &uvw);
	  void CalcTrajPoint(SbVec3d &src, SbVec3f &dst, long step, int camsysnr);
      char read_mvmfile(char * filename);
      void load_mvmfile(const char *filename);
      void calc_animationTrafo(double dStep, int objnr = -1);
	  void exportTrajectories(SbVec3d &src, char *filename, char add, char *txt, char *headertxt,
		                      int camsysnr, int objnr, int refnr, long start, long stop, int type,
							  int type2, SbVec3d *src_leds);
	  void initRealTime(int numCams);
	  void setRSnr(int nr);
      char read_trajfile(char * filename);
      void load_trajfile(const char *filename);
      void calc_animationTrafoImp(double dStep, int objnr = 1,int refnr = 0);
      void load_Refmvmfile(const char *filename);
      char read_Refmvmfile(char * filename);
      void calc_animationTrafo4(double dStep, int objnr = 1,int refnr = 0);
	  void setLedagain();
      void load_RefSysmvmfile(const char *filename);
      char read_RefSysmvmfile(char * filename);
      void calc_animationTrafoRefSys(SbVec3d *src_leds, double dStep, int objnr = 1,int refnr = 0);
      void calc_animationTrafoObj(double dStep, int ledobjnr = 0);
      void calc_animationTrafoReal(double dStep, int objnr = -1);

};

#endif // TMJ_Motion_h

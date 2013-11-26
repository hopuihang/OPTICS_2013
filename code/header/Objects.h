#ifndef TMJ_Objects_h
#define TMJ_Objects_h

#include "FL/Fl.H"
#include "FL/x.H"
#include "FL/Fl_Input.H"
#include <FL/gl.h>
#include <GL/glu.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/fl_file_chooser.H>
#include <FL/fl_message.H>

#include <Inventor/SoDB.h>
#include <Inventor/SoSceneManager.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoMaterialBinding.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoRotationXYZ.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoLabel.h>
#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoText3.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/engines/SoElapsedTime.h>
#include <Inventor/nodes/SoMatrixTransform.h>
#include <Inventor/SbLinear.h>
//#include <Inventor/SoOffscreenRenderer.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/projectors/SbSphereSheetProjector.h>

#include <iostream>
#include <fstream>
#include "TmjLinear.h"

typedef struct                  // Headerteil eines SGI Bildes
{
    unsigned short imagic;      // magic number
    unsigned short type;        // compression mode, bytes per pixel
    unsigned short dim;         // image dimension (1, 2, 3)
    unsigned short xsize;       // width of image
    unsigned short ysize;       // height of image
    unsigned short zsize;       // depth of image (1 for grey, 
                                //                 3 for RGB)
    unsigned long  min;         // min pixel value (0)
    unsigned long  max;         // max pixel value (255)
    unsigned long  wastebytes;  // not used   
    char           name[80];    // free text
    unsigned long  colormodel;  // color model (1 for grey/RGB, 
                                //              2 for colormap)
} SGIheader;

struct UserParameter
{
   SbMatrix   m;
   SbVec3f    v;
};

struct ObjectData
{
   SoSeparator       *separator;
   SoLabel           *label;
   char               oldlabel[256];
   SoTransform       *transform;
   SoDrawStyle       *drawstyle, *olddrawstyle;
   SoMaterial        *material,  *oldmaterial;
   SoMaterialBinding *matbinding;
   SoCoordinate3     *coordinates;
   SoVertexProperty  *vertexproperty;
   SoLineSet         *lineset;
   float             *minmap;
   int                numdist;
   char               pseudo,     oldpseudo;
   SbColor           *stepmap;
   int                num_vertices;
   char               animated,   oldanimated;
   SoTexture2        *kfslogo;
   int                type; // 100..117=LED Number, 1=FOV, 200-220=Trajectory, 221=Sphere of 220 (last point of trajectory)
   SbVec3d            point, oldpoint;
   long               startStep, stopStep, oldStart, oldStop;
   int                camSys, oldCamSys;
   int                motionOfObject;
   int                reference;
   SoDrawStyle       *errorStyle[3];
   ObjectData        *next;
   SbVec3d            point_leds[18];
};


class ObjectViewer : public Fl_Gl_Window
{
private:
   SbVec2s         locator;
   int             eventbut;
   UserParameter   usrpar;
   unsigned char   *pixelBuffer;

public:
   SoSceneManager         *scenemanager;
   SoSeparator            *mainRoot;
   SoSeparator            *modelRoot;
   SoSeparator            *drawingRoot;
   SoGroup                *animatedGroup, *notAnimatedGroup;
   SoTransform            *animation;
   SoSeparator            *allTMJs;
   SoPerspectiveCamera    *perspCamera;
   SoDirectionalLight     *headlight;
   float                   backgroundcolor[3];
   ObjectData             *objlist;
   SbSphereSheetProjector *sphereSheet;
   SbVec3f                 locator3D;
   SbPlane                 focalplane;
   int                     refflag;
//   int                     appRefflag;
   int                     appflag;
   int                     appTF1flag;
   int                     appTF2flag;
   int                     appTF3flag;
   int                     appcoordflag;
   int                     appledflag;

private:
   void draw();
   int  handle(int event);
   void btnproc();
   void getChildren(SoSeparator *root, char *text);
   void initObject(char *labelstring, int drawstyle);
public:
   ObjectViewer(int x, int y, int w, int h, const char *l=0);//l = 0
   ~ObjectViewer();
   void     delAll();
   void     reshape(int w, int h);
   char     load(char * selectedFile);
   char     importObject(char * selectedFile);
   void     rotateCamera(const SbRotation &rot);
   void     RotateX(float x);
   void     RotateY(float y);
   void     RotateZ(float z);
   void     spinCamera(const SbVec2f &newLocator);
   void     panCamera(const SbVec2s &newLocator);
   void     dollyCamera(float d);
   void     resetView();
   void     setCenter();
   void     orderChildren();
   void     hideSpheres();
   void     saveObjects(char *selectedFile);
   void     setLabel(ObjectData *data, char *l);
   void     getLabel(ObjectData *data, char *l);
   void     setLocator3D(const SbVec2s &locator);
   SoSeparator*  loadModel(const char * fileName);
   void     addLineObject(char *labelstring, SbVec3f *xyz, int numLines, int numPoints);
   void     addLineObject2(char *labelstring);
   void     addSphereObject(char *labelstring, SbVec3f *xyz, int numSpheres, float radius, char coordSys);
  /* void     addConeObject(char*, SbVec3f*, int, float, float, SbVec3f, float, SbVec3f);*/
  void     addCubeObject(char* labelstring, SbVec3f *xyz, int numCubes, float width, float height, float depth);
   void     addTextObject(char* labelstring, SbVec3f *xyz, int numTexts);
   /*void     addCylinderObject(char*, SbVec3f*, int, float, float, SbVec3f, float, SbVec3f);*/
   void     addAxisObject(char*, SbVec3f*, SbVec3f, float, float, SbVec3f, float, char*);
   void     addAllAxis(int type);
   void     addScaleLines();
   void     addScaleValues();
   void     saveImage(char *filename, char *format, int nr, char *imgcopy, char *extracode);
   void     writeToSGI(char *filename, int nr);
   void     clearObject(ObjectData *obj);
   void     clearObject(char *str);
   void     checkName();
};

#endif

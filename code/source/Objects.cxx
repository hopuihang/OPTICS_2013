#include "Objects.h"


ObjectViewer::ObjectViewer(int x, int y, int w, int h, const char *l) : Fl_Gl_Window(x,y,w,h,l)
{
  appflag = 0;
  appTF1flag = 0;
  appTF2flag = 0;
  appTF3flag = 0;
  appcoordflag = 0;
  appledflag = 0;

  backgroundcolor[0] = 0.019;
  backgroundcolor[1] = 0.015;
  backgroundcolor[2] = 0.356;

  objlist = NULL;
  allTMJs = NULL;
  pixelBuffer = NULL;

  SoDB::init();
  printf("Open Inventor version: %s\n",SoDB::getVersion()) ;

  mainRoot = new SoSeparator;
  mainRoot->ref();

  perspCamera = new SoPerspectiveCamera;
  mainRoot->addChild(perspCamera);
//  perspCamera->heightAngle.setValue(0.0872665);

  headlight = new SoDirectionalLight;
  mainRoot->addChild(headlight);

  drawingRoot = new SoSeparator;
  mainRoot->addChild(drawingRoot);

  notAnimatedGroup = new SoGroup;
  drawingRoot->addChild(notAnimatedGroup);
  notAnimatedGroup->setToDefaults();

  animation = new SoTransform;
  drawingRoot->addChild(animation);
  animation->setToDefaults();

  animatedGroup = new SoSeparator;
  drawingRoot->addChild(animatedGroup);
  animatedGroup->setToDefaults();

  modelRoot = new SoSeparator;
  modelRoot->ref();

  // initialize scenemanager instance

  scenemanager = new SoSceneManager;
  scenemanager->setBackgroundColor(SbColor(backgroundcolor[0],
     backgroundcolor[1], backgroundcolor[2]));
  scenemanager->activate();
  scenemanager->setSceneGraph(mainRoot);

  // init the projector class
  SbViewVolume vv;
  vv.ortho(-1, 1, -1, 1, -10, 10);
  sphereSheet = new SbSphereSheetProjector;
  sphereSheet->setViewVolume( vv );
  sphereSheet->setSphere( SbSphere( SbVec3f(0, 0, 0), .7) );

  reshape(w, h);
}

ObjectViewer::~ObjectViewer()
{
   delAll();
   drawingRoot->removeAllChildren();
   mainRoot->removeAllChildren();
   mainRoot->unref();
   modelRoot->unref();

   delete sphereSheet;
   delete scenemanager;
}

void ObjectViewer::delAll()
{
   ObjectData *tmpdata;

   if (notAnimatedGroup->getNumChildren() > 0)
      notAnimatedGroup->removeAllChildren();
   if (animatedGroup->getNumChildren() > 0) 
      animatedGroup->removeAllChildren();

   if ((allTMJs != NULL) && (allTMJs->getNumChildren() > 0))
      allTMJs->removeAllChildren() ;
   if (allTMJs != NULL)
      allTMJs->unref();
   allTMJs = NULL;

   if (modelRoot->getNumChildren() > 0) 
      modelRoot->removeAllChildren() ;

   while (objlist != NULL)
   {
	   tmpdata = objlist;
	   objlist = objlist->next;
	   tmpdata->olddrawstyle->unref();
	   tmpdata->oldmaterial->unref();
	   if (tmpdata->stepmap != NULL)
		   delete[] tmpdata->stepmap;
	   if (tmpdata->minmap != NULL)
		   delete[] tmpdata->minmap;
	   delete tmpdata;
   }
   if (pixelBuffer != NULL)
      delete[] pixelBuffer;
   pixelBuffer = NULL;
}


// Reconfigure on changes to window dimensions.
void ObjectViewer::reshape(int w, int h)
{
  scenemanager->setWindowSize(SbVec2s(w, h));
  scenemanager->setSize(SbVec2s(w, h));
  scenemanager->setViewportRegion(SbViewportRegion(w, h));
//  scenemanager->getGLRenderAction()->setTransparencyType(SoGLRenderAction::SORTED_OBJECT_BLEND); //original
  scenemanager->getGLRenderAction()->setTransparencyType(SoGLRenderAction::SCREEN_DOOR);
  scenemanager->scheduleRedraw();
}


void ObjectViewer::rotateCamera(const SbRotation &rot)
{
    // get center of rotation
    SbRotation camRot = perspCamera->orientation.getValue();//SbMatrix
    float radius = perspCamera->focalDistance.getValue();
    SbMatrix mx;
    mx = camRot;
    SbVec3f forward( -mx[2][0], -mx[2][1], -mx[2][2]);
    SbVec3f center = perspCamera->position.getValue()
	+ radius * forward;
    
    // apply new rotation to the perspCamera
    camRot = rot * camRot;
    perspCamera->orientation = camRot;
    
    // reposition perspCamera to look at pt of interest
    mx = camRot;
    forward.setValue( -mx[2][0], -mx[2][1], -mx[2][2]);
    perspCamera->position = center - radius * forward;
  
    // apply new rotation to the DirectionalLight
	headlight->direction.setValue(forward);
}


void ObjectViewer::RotateX(float x)
{
    SbVec3f axis(1, 0, 0);
    SbRotation rot(axis, x);
    rotateCamera(rot);
    redraw();
}

void ObjectViewer::RotateY(float y)
{
    SbVec3f axis(0, 1, 0);
    SbRotation rot(axis, y);
    rotateCamera(rot);
    redraw();
}

void ObjectViewer::RotateZ(float z)
{
    SbVec3f axis(0, 0, 1);
    SbRotation rot(axis, z);
    rotateCamera(rot);
    redraw();
}

void ObjectViewer::spinCamera(const SbVec2f &newLocator)
{
    // find rotation and rotate camera
    SbRotation rot;
    sphereSheet->projectAndGetRotation(newLocator, rot);
    rot.invert();
    rotateCamera(rot);
	redraw();
}


void ObjectViewer::panCamera(const SbVec2s &newLocator)
{
    // map new mouse location into the camera focal plane
    SbViewVolume    cameraVolume;
    SbLine	        line;
    SbVec3f	        newLocator3D;
    cameraVolume = perspCamera->getViewVolume(w()/float(h()));
    cameraVolume.projectPointToLine(
       SbVec2f(newLocator[0]/float(w()), (h()-newLocator[1])/float(h())), line);
    focalplane.intersect(line, newLocator3D);
    
    // move the camera by the delta 3D position amount
    perspCamera->position = perspCamera->position.getValue() + 
	(locator3D - newLocator3D);
    
    // You would think we would have to set locator3D to
    // newLocator3D here.  But we don't, because moving the camera
    // essentially makes locator3D equal to newLocator3D in the
    // transformed space, and we will project the next newLocator3D in
    // this transformed space.

	redraw();
}


void ObjectViewer::dollyCamera(float d)
{
	float focalDistance = perspCamera->focalDistance.getValue();
	float newFocalDist = focalDistance * powf(2.0, d);
	
	// finally reposition the camera
	SbMatrix mx;
	mx = perspCamera->orientation.getValue();
	SbVec3f forward(-mx[2][0], -mx[2][1], -mx[2][2]);
	perspCamera->position = perspCamera->position.getValue() + 
			   (focalDistance - newFocalDist) * forward;
	perspCamera->focalDistance = newFocalDist;
    redraw();
}


void ObjectViewer::resetView()
{
   if (allTMJs == NULL)
	   return;
   headlight->setToDefaults();
   perspCamera->setToDefaults();
//   perspCamera->heightAngle.setValue(0.0872665);
   perspCamera->viewAll(allTMJs, SbViewportRegion(w(), h()));
   perspCamera->farDistance.setValue(10000.0);
   perspCamera->nearDistance.setValue(0.1);

   redraw();
}


//int ObjectViewer::idle()
//{
//  SoDB::getSensorManager()->processTimerQueue();
//  SoDB::getSensorManager()->processDelayQueue(TRUE);
//  redraw();
//  return 1;
//}


void ObjectViewer::getLabel(ObjectData *data, char *l)
{
   SbString st;
   char     name[256], newname[256];
   int      i = 0, j = 0;

   data->label->label.get(st);
   sprintf(name, "%s", st.getString());
   while (name[i] != '\0')
   {
	   if (name[i] != '"')
	   {
		   newname[j] = name[i];
		   j++;
	   }
	   i++;
   }
   newname[j] = '\0';
   sprintf(l, "%s", newname);
}


void ObjectViewer::setLabel(ObjectData *data, char *l)
{
   char     name[256];

   sprintf(name, "\"%s\"", l);
   data->label->label.set(&name[0]);
}


void ObjectViewer::getChildren(SoSeparator *root, char *text)
{
   ObjectData     *tmplist;
   SoSeparator    *childRoot;
   SoSearchAction searchAction;
   SoPathList     pathList;
   SoPath         *path, *childPath;
   int            nr, k;
   char           st[256], stl[256];

   searchAction.setType(SoSeparator::getClassTypeId());
   searchAction.setInterest(SoSearchAction::ALL);
   searchAction.apply(root);
   pathList = searchAction.getPaths();

   nr = 0;
   while(pathList != NULL)
   {
      path = pathList[0];
      childRoot = (SoSeparator*)path->getTail();
 
      if (nr > 0)
      {
         tmplist = new ObjectData;
         tmplist->next = objlist;
         objlist = tmplist;

         objlist->separator = childRoot;
         objlist->animated = 0;
         objlist->matbinding = NULL;
		 objlist->coordinates = NULL;
		 objlist->vertexproperty = NULL;
		 objlist->lineset = NULL;
         objlist->minmap = NULL;
		 objlist->numdist = 0;
         objlist->pseudo = 0;
         objlist->oldpseudo = 0;
         objlist->stepmap = NULL;
         objlist->num_vertices = 0;
         objlist->oldanimated = 0;
         objlist->type = 0;
		 objlist->point.setValue(0.0, 0.0, 0.0);
		 objlist->startStep = 0;
		 objlist->stopStep = 0;
		 objlist->camSys = 0;
 	 	 objlist->motionOfObject = 1;
		 objlist->reference = 0;
		 objlist->kfslogo = NULL;
		 for (k=0; k<3; k++)
			 objlist->errorStyle[k] = NULL;
		 for (k=0; k<18; k++)
		 objlist->point_leds[k].setValue(0.0, 0.0, 0.0);

         searchAction.setType(SoLabel::getClassTypeId());
         searchAction.setInterest(SoSearchAction::FIRST);
         searchAction.apply(childRoot);
         childPath = searchAction.getPath();
         if (childPath != NULL)
         {
            objlist->label = (SoLabel*)childPath->getTail();
			if (text != NULL)
			{
				getLabel(objlist, &st[0]);
				sprintf(stl, "%s %s", st, text);
				setLabel(objlist, &stl[0]);
			}
         }
         else
         {
            objlist->label = new SoLabel;
            if (text == NULL)
				sprintf(st, "%s%d", "Object ", nr);
			else
				sprintf(st, "%s%d %s", "Object ", nr, text);
			setLabel(objlist, &st[0]);
            childRoot->insertChild(objlist->label, 0);
         }
         sprintf(objlist->oldlabel,"");

         searchAction.setType(SoTransform::getClassTypeId());
         searchAction.setInterest(SoSearchAction::FIRST);
         searchAction.apply(childRoot);
         childPath = searchAction.getPath();
         if (childPath != NULL)
            objlist->transform = (SoTransform*)childPath->getTail();
         else
         {
            objlist->transform = new SoTransform;
            objlist->transform->setToDefaults();
            childRoot->insertChild(objlist->transform, 1);
         }

         searchAction.setType(SoDrawStyle::getClassTypeId());
         searchAction.setInterest(SoSearchAction::FIRST);
         searchAction.apply(childRoot);
         childPath = searchAction.getPath();
         if (childPath != NULL)
            objlist->drawstyle = (SoDrawStyle*)childPath->getTail();
         else
         {
            objlist->drawstyle = new SoDrawStyle;
            objlist->drawstyle->style = SoDrawStyle::FILLED;
            childRoot->insertChild(objlist->drawstyle, 2);
         }
         objlist->olddrawstyle = new SoDrawStyle;
         objlist->olddrawstyle->ref();

         searchAction.setType(SoMaterial::getClassTypeId());
         searchAction.setInterest(SoSearchAction::FIRST);
         searchAction.apply(childRoot);
         childPath = searchAction.getPath();
         if (childPath != NULL)
            objlist->material = (SoMaterial*)childPath->getTail();
         else
         {
            objlist->material = new SoMaterial;
            objlist->material->setToDefaults();
            childRoot->insertChild(objlist->material, 3);
         }
         objlist->oldmaterial = new SoMaterial;
         objlist->oldmaterial->ref();
 
         searchAction.setType(SoMaterialBinding::getClassTypeId());
         searchAction.setInterest(SoSearchAction::FIRST);
         searchAction.apply(childRoot);
         childPath = searchAction.getPath();
         if (childPath != NULL)
		 {
            objlist->matbinding = (SoMaterialBinding*)childPath->getTail();
		 }
         else
		 {
            objlist->matbinding = new SoMaterialBinding;
            objlist->matbinding->value = SoMaterialBinding::OVERALL;
            childRoot->insertChild(objlist->matbinding, 4);
		 }

		 searchAction.setType(SoCoordinate3::getClassTypeId());
		 searchAction.setInterest(SoSearchAction::FIRST);
		 searchAction.apply(childRoot);
		 childPath = searchAction.getPath();
		 if (childPath != NULL)
			 objlist->coordinates = (SoCoordinate3*)childPath->getTail();

		 searchAction.setType(SoTexture2::getClassTypeId());
		 searchAction.setInterest(SoSearchAction::FIRST);
		 searchAction.apply(childRoot);
		 childPath = searchAction.getPath();
		 if (childPath != NULL)
			 objlist->kfslogo = (SoTexture2*)childPath->getTail();
     }
      pathList.remove(0);
      nr++;
   }
}

void ObjectViewer::checkName()
{
	char st[256], stc[256], labelst[256];
	char ok;
	int  i;
	ObjectData *tmplist;

	getLabel(objlist, &labelst[0]);
	sprintf(st, "%s", labelst);
	if (strlen(st) == 0)
		sprintf(st, "Object");
    ok = 0;
	i = 0;
    while (ok == 0)
	{
        ok = 1;
		tmplist = objlist->next;
		while (tmplist != NULL)
		{
			getLabel(tmplist, &stc[0]);
			if(strcmp(st, stc) == 0)
			{
				i++;
				sprintf(st, "%s %d", labelst, i);
				ok = 0;
			}
			tmplist = tmplist->next;
		}
	}
	setLabel(objlist, &st[0]);
}

void ObjectViewer::initObject(char *labelstring, int drawstyle)
{
	char        st[256];
	ObjectData *tmplist;
	int k;
	
    tmplist = new ObjectData;
    tmplist->next = objlist;
    objlist = tmplist;

    objlist->animated = 0;
    objlist->matbinding = NULL;
	objlist->coordinates = NULL;
	objlist->vertexproperty = NULL;
	objlist->lineset = NULL;
    objlist->minmap = NULL;
    objlist->numdist = 0;
    objlist->pseudo = 0;
    objlist->oldpseudo = 0;
    objlist->stepmap = NULL;
    objlist->num_vertices = 0;
    objlist->oldanimated = 0;
    objlist->type = 0;
    objlist->point.setValue(0.0, 0.0, 0.0);
	objlist->startStep = 0;
	objlist->stopStep = 0;
	objlist->camSys = 0;
	objlist->motionOfObject = 1;
	objlist->reference = 0;
	objlist->kfslogo = NULL;
		 for (k=0; k<3; k++)
			 objlist->errorStyle[k] = NULL;

	objlist->separator = new SoSeparator;
	modelRoot->addChild(objlist->separator);
	notAnimatedGroup->addChild(objlist->separator);

    objlist->label = new SoLabel;
    sprintf(st, "%s", labelstring);
	setLabel(objlist, &st[0]);
    objlist->separator->addChild(objlist->label);
	checkName();

    objlist->transform = new SoTransform;
    objlist->transform->setToDefaults();
    objlist->separator->addChild(objlist->transform);

    objlist->drawstyle = new SoDrawStyle;
	switch (drawstyle)
	{
	   case 0: objlist->drawstyle->style = SoDrawStyle::FILLED; break;
	   case 1: objlist->drawstyle->style = SoDrawStyle::LINES; break;
	   case 2: objlist->drawstyle->style = SoDrawStyle::POINTS; break;
	   case 3: objlist->drawstyle->style = SoDrawStyle::INVISIBLE; break;
	}
    objlist->separator->addChild(objlist->drawstyle);
    objlist->olddrawstyle = new SoDrawStyle;
    objlist->olddrawstyle->ref();

    objlist->material = new SoMaterial;
    objlist->material->setToDefaults();
    objlist->separator->addChild(objlist->material);
    objlist->oldmaterial = new SoMaterial;
    objlist->oldmaterial->ref();
}

void ObjectViewer::addLineObject(char *labelstring, SbVec3f *xyz, int numLines, int numPoints)
{
	int         i, *numvertices;

    initObject(labelstring, 1);

	objlist->vertexproperty = new SoVertexProperty;
	objlist->vertexproperty->vertex.setValues(0, numPoints, xyz);
    objlist->separator->addChild(objlist->vertexproperty);
    numvertices = new int[numLines];
	for (i=0; i<numLines; i++)
		numvertices[i] = numPoints/numLines;
	objlist->lineset = new SoLineSet;
	objlist->lineset->vertexProperty = objlist->vertexproperty;
	objlist->lineset->startIndex = 0;
	objlist->lineset->numVertices.setValues(0, numLines, numvertices);
    objlist->separator->addChild(objlist->lineset);
	delete[] numvertices;
}

void ObjectViewer::addLineObject2(char *labelstring)
{// to show the axis in each target frame
	int         i, *numvertices;
	SbVec3f x[2],y[2],z[2];
	SoMaterial       *mat;	

	x[0].setValue(0.0, 0.0, 0.0);
	x[1].setValue(20.0, 0.0, 0.0);
	y[0].setValue(0.0, 0.0, 0.0);
	y[1].setValue(0.0, 20.0, 0.0);
	z[0].setValue(0.0, 0.0, 0.0);
	z[1].setValue(0.0, 0.0, 20.0);

    initObject(labelstring, 1);

	numvertices = new int[1];
	for (i=0; i<1; i++)
		numvertices[i] = 2;

	mat = new SoMaterial; // x axis
	mat->setToDefaults();
	objlist->separator->addChild(mat);
	mat->diffuseColor.setValue(1.0, 0.0, 0.0);
	mat->emissiveColor.setValue(1.0, 0.0, 0.0);
	objlist->vertexproperty = new SoVertexProperty;
	objlist->vertexproperty->vertex.setValues(0, 2, x);
    objlist->separator->addChild(objlist->vertexproperty);
	objlist->lineset = new SoLineSet;
	objlist->lineset->vertexProperty = objlist->vertexproperty;
	objlist->lineset->startIndex = 0;
	objlist->lineset->numVertices.setValues(0, 1, numvertices);
    objlist->separator->addChild(objlist->lineset);

	mat = new SoMaterial; // y axis
	mat->setToDefaults();
	objlist->separator->addChild(mat);
	mat->diffuseColor.setValue(0.0, 1.0, 0.0);
	mat->emissiveColor.setValue(0.0, 1.0, 0.0);
	objlist->vertexproperty = new SoVertexProperty;
	objlist->vertexproperty->vertex.setValues(0, 2, y);
    objlist->separator->addChild(objlist->vertexproperty);
	objlist->lineset = new SoLineSet;
	objlist->lineset->vertexProperty = objlist->vertexproperty;
	objlist->lineset->startIndex = 0;
	objlist->lineset->numVertices.setValues(0, 1, numvertices);
    objlist->separator->addChild(objlist->lineset);

	mat = new SoMaterial; // z axis
	mat->setToDefaults();
	objlist->separator->addChild(mat);
	mat->diffuseColor.setValue(0.0, 0.0, 1.0);
	mat->emissiveColor.setValue(0.0, 0.0, 1.0);
	objlist->vertexproperty = new SoVertexProperty;
	objlist->vertexproperty->vertex.setValues(0, 2, z);
    objlist->separator->addChild(objlist->vertexproperty);
	objlist->lineset = new SoLineSet;
	objlist->lineset->vertexProperty = objlist->vertexproperty;
	objlist->lineset->startIndex = 0;
	objlist->lineset->numVertices.setValues(0, 1, numvertices);
    objlist->separator->addChild(objlist->lineset);
}

void ObjectViewer::addSphereObject(char *labelstring, SbVec3f *xyz, int numSpheres, float radius, char coordSys)
{
	int               i, numvertices;
	SoSeparator      *tmpSeparator;
	SoTranslation    *tmpTranslation;
	SoSphere         *tmpSphere;
	SbVec3f           coordLine[2];
	SoVertexProperty *vertprop;
	SoLineSet        *lineSet;
	SoMaterial       *mat;	

	numvertices = 2;
    initObject(labelstring, 0);
	objlist->material->shininess.setValue(1.0);
//	objlist->material->shininess.setValue(1.0);
	objlist->material->specularColor.setValue(1.0, 1.0, 1.0);

	for (i=0; i<numSpheres; i++)
	{
		tmpSeparator = new SoSeparator;
		objlist->separator->addChild(tmpSeparator);
		tmpTranslation = new SoTranslation;
		tmpTranslation->translation.setValue(xyz[i]);
		tmpSeparator->addChild(tmpTranslation);
		tmpSphere = new SoSphere;
		tmpSphere->radius.setValue(radius);
		tmpSeparator->addChild(tmpSphere);
		if (coordSys)
		{
			// X, rot
            objlist->errorStyle[0] = new SoDrawStyle;
            objlist->errorStyle[0]->style = SoDrawStyle::INVISIBLE;
            tmpSeparator->addChild(objlist->errorStyle[0]);
            mat = new SoMaterial;
            mat->setToDefaults();
            tmpSeparator->addChild(mat);
			mat->diffuseColor.setValue(1.0, 0.0, 0.0);
			mat->emissiveColor.setValue(1.0, 0.0, 0.0);
			coordLine[0].setValue(-12.0, 0.0, 0.0);
			coordLine[1].setValue(12.0, 0.0, 0.0);
	        vertprop = new SoVertexProperty;
	        vertprop->vertex.setValues(0, 2, &coordLine[0]);
            tmpSeparator->addChild(vertprop);
	        lineSet = new SoLineSet;
	        lineSet->vertexProperty = vertprop;
	        lineSet->startIndex = 0;
	        lineSet->numVertices.setValues(0, 1, &numvertices);
            tmpSeparator->addChild(lineSet);

			// Y, gruen
            objlist->errorStyle[1] = new SoDrawStyle;
            objlist->errorStyle[1]->style = SoDrawStyle::INVISIBLE;
            tmpSeparator->addChild(objlist->errorStyle[1]);
            mat = new SoMaterial;
            mat->setToDefaults();
            tmpSeparator->addChild(mat);
			mat->diffuseColor.setValue(0.0, 1.0, 0.0);
			mat->emissiveColor.setValue(0.0, 1.0, 0.0);
			coordLine[0].setValue(0.0, -12.0, 0.0);
			coordLine[1].setValue(0.0, 12.0, 0.0);
	        vertprop = new SoVertexProperty;
	        vertprop->vertex.setValues(0, 2, &coordLine[0]);
            tmpSeparator->addChild(vertprop);
	        lineSet = new SoLineSet;
	        lineSet->vertexProperty = vertprop;
	        lineSet->startIndex = 0;
	        lineSet->numVertices.setValues(0, 1, &numvertices);
            tmpSeparator->addChild(lineSet);

			// Z, blau
            objlist->errorStyle[2] = new SoDrawStyle;
            objlist->errorStyle[2]->style = SoDrawStyle::INVISIBLE;
            tmpSeparator->addChild(objlist->errorStyle[2]);
            mat = new SoMaterial;
            mat->setToDefaults();
            tmpSeparator->addChild(mat);
			mat->diffuseColor.setValue(0.0, 0.0, 1.0);
			mat->emissiveColor.setValue(0.0, 0.0, 1.0);
			coordLine[0].setValue(0.0, 0.0, -12.0);
			coordLine[1].setValue(0.0, 0.0, 12.0);
	        vertprop = new SoVertexProperty;
	        vertprop->vertex.setValues(0, 2, &coordLine[0]);
            tmpSeparator->addChild(vertprop);
	        lineSet = new SoLineSet;
	        lineSet->vertexProperty = vertprop;
	        lineSet->startIndex = 0;
	        lineSet->numVertices.setValues(0, 1, &numvertices);
            tmpSeparator->addChild(lineSet);
		}
	}
}


/*void ObjectViewer::addConeObject(char *labelstring, SbVec3f *xyz, int numCones, float radius, float height,SbVec3f axis, float angle, SbVec3f translation)
{
    int i;
	SoCone        *tmpCone;
	SoSeparator   *tmpSeparator;
	SoTransform   *tmpTransform;
	
    initObject(labelstring, 0);
	objlist->transform->translation.setValue(*xyz);

	for (i=0; i<numCones; i++)
	{
		tmpSeparator = new SoSeparator;
		objlist->separator->addChild(tmpSeparator);
		tmpTransform = new SoTransform;
        tmpTransform->rotation.setValue(axis, angle);
        tmpTransform->translation.setValue(translation);
		tmpSeparator->addChild(tmpTransform);
        tmpCone = new SoCone;
	    tmpCone->parts = SoCone::ALL;
	    tmpCone->bottomRadius.setValue(radius);
	    tmpCone->height.setValue(height);
	    tmpSeparator->addChild(tmpCone);
	}
}*/


void ObjectViewer::addCubeObject(char* labelstring, SbVec3f *xyz, int numCubes, float width, float height, float depth)
{
	int            i;
	SoSeparator   *tmpSeparator;
	SoTranslation *tmpTranslation;
	SoCube        *tmpCube;
	
    initObject(labelstring, 1);

	for (i=0; i<numCubes; i++)
	{
		tmpSeparator = new SoSeparator;
		objlist->separator->addChild(tmpSeparator);
		tmpTranslation = new SoTranslation;
		tmpTranslation->translation.setValue(xyz[i]);
		tmpSeparator->addChild(tmpTranslation);
		tmpCube = new SoCube;
		tmpCube->width.setValue(width);
		tmpCube->height.setValue(height);
		tmpCube->depth.setValue(depth);
		tmpSeparator->addChild(tmpCube);
	}
}


void ObjectViewer::addTextObject(char* labelstring, SbVec3f *xyz, int numTexts)
{
	int            i;
	SoSeparator   *tmpSeparator;
	SoTranslation *tmpTranslation;
	SoText3       *tmpText3;
	SoFont        *tmpFont;
	
    initObject(labelstring, 1);

	for (i=0; i<numTexts; i++)
	{
		tmpSeparator = new SoSeparator;
		objlist->separator->addChild(tmpSeparator);
		tmpTranslation = new SoTranslation;
		tmpTranslation->translation.setValue(xyz[i]);
		tmpSeparator->addChild(tmpTranslation);
		tmpFont = new SoFont;
#ifdef WIN32
		tmpFont->name.setValue("Times New Roman");
#else
		tmpFont->name.setValue("Times-Roman");
#endif
		tmpFont->size.setValue(10);
		tmpSeparator->addChild(tmpFont);
		tmpText3 = new SoText3;
		tmpText3->string = labelstring;
		tmpSeparator->addChild(tmpText3);
	}
}


/*void ObjectViewer::addCylinderObject(char* labelstring, SbVec3f *xyz, int numCylinders, float radius, float height,
									 SbVec3f axis, float angle, SbVec3f translation)
{
    int i;
	SoCylinder    *tmpCylinder;
	SoSeparator   *tmpSeparator;
	SoTransform   *tmpTransform;
	
    initObject(labelstring, 0);
	objlist->transform->translation.setValue(*xyz);

	for (i=0; i<numCylinders; i++)
	{
		tmpSeparator = new SoSeparator;
		objlist->separator->addChild(tmpSeparator);
		tmpTransform = new SoTransform;
        tmpTransform->rotation.setValue(axis, angle);
        tmpTransform->translation.setValue(translation);
		tmpSeparator->addChild(tmpTransform);
        tmpCylinder = new SoCylinder;
	    tmpCylinder->parts = SoCylinder::ALL;
	    tmpCylinder->radius.setValue(radius);
	    tmpCylinder->height.setValue(height);
	    tmpSeparator->addChild(tmpCylinder);
	}
}*/


void ObjectViewer::addAxisObject(char* labelstring, SbVec3f *xyz, SbVec3f color, float radius, float height,
									 SbVec3f axis, float angle, char *zusatz)
{
	SbVec3f        tmptrans, tmpaxis;
	SoCylinder    *tmpCylinder;
	SoCone        *tmpCone;
	SoText3       *tmpText3;
	SoFont        *tmpFont;
	SoSeparator   *tmpSeparator, *tmpSeparator2;
	SoTranslation *tmpTranslation;
	SoTransform   *tmpTransform;
	
    initObject(labelstring, 0);
    objlist->material->diffuseColor.setValue(color[0], color[1], color[2]);
    objlist->material->emissiveColor.setValue(color[0]*0.8, color[1]*0.8, color[2]*0.8);//test
	tmpSeparator2 = new SoSeparator;
	objlist->separator->addChild(tmpSeparator2);
	tmpFont = new SoFont;
#ifdef WIN32
	tmpFont->name.setValue("Times New Roman");
#else
	tmpFont->name.setValue("Times-Roman");
#endif
	tmpFont->size.setValue(10);
	tmpSeparator2->addChild(tmpFont);
	tmpTransform = new SoTransform;
	tmpTransform->translation.setValue(*xyz);
    tmpTransform->rotation.setValue(axis, angle);
	tmpSeparator2->addChild(tmpTransform);
	objlist->type = 1;


	tmptrans.setValue(0.0, 0.5*height, 0.0);
	tmpSeparator = new SoSeparator;
	tmpSeparator2->addChild(tmpSeparator);
	tmpTranslation = new SoTranslation;
	tmpTranslation->translation.setValue(tmptrans);
	tmpSeparator->addChild(tmpTranslation);
    tmpCylinder = new SoCylinder;
	tmpCylinder->parts = SoCylinder::ALL;
	tmpCylinder->radius.setValue(radius);
	tmpCylinder->height.setValue(height);
	tmpSeparator->addChild(tmpCylinder);

	tmptrans.setValue(0.0, height, 0.0);
	tmpSeparator = new SoSeparator;
	tmpSeparator2->addChild(tmpSeparator);
	tmpTranslation = new SoTranslation;
	tmpTranslation->translation.setValue(tmptrans);
	tmpSeparator->addChild(tmpTranslation);
    tmpCone = new SoCone;
	tmpCone->parts = SoCone::ALL;
	tmpCone->bottomRadius.setValue(2.0*radius);
	tmpCone->height.setValue(4.0*radius);
	tmpSeparator->addChild(tmpCone);

	tmptrans.setValue(2.0*radius, 0.0, 0.0);
	tmpSeparator = new SoSeparator;
	tmpSeparator2->addChild(tmpSeparator);
	tmpTranslation = new SoTranslation;
	tmpTranslation->translation.setValue(tmptrans);
	tmpSeparator->addChild(tmpTranslation);
	tmpText3 = new SoText3;
	tmpText3->parts = SoText3::ALL;
	tmpText3->string = labelstring;
	tmpSeparator->addChild(tmpText3);

	if (strlen(zusatz)>0)
	{
		tmptrans.setValue(4.0*radius, -radius, 0.0);
		tmpTranslation = new SoTranslation;
		tmpTranslation->translation.setValue(tmptrans);
		tmpSeparator->addChild(tmpTranslation);
		tmpFont = new SoFont;
#ifdef WIN32
		tmpFont->name.setValue("Times New Roman");
#else
		tmpFont->name.setValue("Times-Roman");
#endif
		tmpFont->size.setValue(8);
		tmpSeparator->addChild(tmpFont);
		tmpText3 = new SoText3;
		tmpText3->parts = SoText3::ALL;
		tmpText3->string = zusatz;
		tmpSeparator->addChild(tmpText3);
	}

	tmptrans.setValue(0.0, 0.0, -2.0*radius);
	tmpaxis.setValue(0.0, 1.0, 0.0);
	tmpSeparator = new SoSeparator;
	tmpSeparator2->addChild(tmpSeparator);
	tmpTransform = new SoTransform;
    tmpTransform->rotation.setValue(tmpaxis, 1.5708);
    tmpTransform->translation.setValue(tmptrans);
	tmpSeparator->addChild(tmpTransform);
	tmpText3 = new SoText3;
	tmpText3->parts = SoText3::ALL;
	tmpText3->string = labelstring;
	tmpSeparator->addChild(tmpText3);
	if (strlen(zusatz)>0)
	{
		tmptrans.setValue(4.0*radius, -radius, 0.0);
		tmpTranslation = new SoTranslation;
		tmpTranslation->translation.setValue(tmptrans);
		tmpSeparator->addChild(tmpTranslation);
		tmpFont = new SoFont;
#ifdef WIN32
		tmpFont->name.setValue("Times New Roman");
#else
		tmpFont->name.setValue("Times-Roman");
#endif
		tmpFont->size.setValue(8);
		tmpSeparator->addChild(tmpFont);
		tmpText3 = new SoText3;
		tmpText3->parts = SoText3::ALL;
		tmpText3->string = zusatz;
		tmpSeparator->addChild(tmpText3);
	}

	tmptrans.setValue(-2.0*radius, 0.0, 0.0);
	tmpaxis.setValue(0.0, 1.0, 0.0);
	tmpSeparator = new SoSeparator;
	tmpSeparator2->addChild(tmpSeparator);
	tmpTransform = new SoTransform;
    tmpTransform->rotation.setValue(tmpaxis, 3.14159);
    tmpTransform->translation.setValue(tmptrans);
	tmpSeparator->addChild(tmpTransform);
	tmpText3 = new SoText3;
	tmpText3->parts = SoText3::ALL;
	tmpText3->string = labelstring;
	tmpSeparator->addChild(tmpText3);
	if (strlen(zusatz)>0)
	{
		tmptrans.setValue(4.0*radius, -radius, 0.0);
		tmpTranslation = new SoTranslation;
		tmpTranslation->translation.setValue(tmptrans);
		tmpSeparator->addChild(tmpTranslation);
		tmpFont = new SoFont;
#ifdef WIN32
		tmpFont->name.setValue("Times New Roman");
#else
		tmpFont->name.setValue("Times-Roman");
#endif
		tmpFont->size.setValue(8);
		tmpSeparator->addChild(tmpFont);
		tmpText3 = new SoText3;
		tmpText3->parts = SoText3::ALL;
		tmpText3->string = zusatz;
		tmpSeparator->addChild(tmpText3);
	}

	tmptrans.setValue(0.0, 0.0, 2.0*radius);
	tmpaxis.setValue(0.0, 1.0, 0.0);
	tmpSeparator = new SoSeparator;
	tmpSeparator2->addChild(tmpSeparator);
	tmpTransform = new SoTransform;
    tmpTransform->rotation.setValue(tmpaxis, 4.71239);
    tmpTransform->translation.setValue(tmptrans);
	tmpSeparator->addChild(tmpTransform);
	tmpText3 = new SoText3;
	tmpText3->parts = SoText3::ALL;
	tmpText3->string = labelstring;
	tmpSeparator->addChild(tmpText3);
	if (strlen(zusatz)>0)
	{
		tmptrans.setValue(4.0*radius, -radius, 0.0);
		tmpTranslation = new SoTranslation;
		tmpTranslation->translation.setValue(tmptrans);
		tmpSeparator->addChild(tmpTranslation);
		tmpFont = new SoFont;
#ifdef WIN32
		tmpFont->name.setValue("Times New Roman");
#else
		tmpFont->name.setValue("Times-Roman");
#endif
		tmpFont->size.setValue(8);
		tmpSeparator->addChild(tmpFont);
		tmpText3 = new SoText3;
		tmpText3->parts = SoText3::ALL;
		tmpText3->string = zusatz;
		tmpSeparator->addChild(tmpText3);
	}
}


void ObjectViewer::addAllAxis(int type)
{
	SbVec3f xyz, color, axis;
	char st[8];

	if(type > 999)
		sprintf(st, "%d", type/1000);
	else
		st[0] = '\0';

	xyz.setValue(100.0, 0.0, 0.0);
	color.setValue(1.0, 0.0, 0.0);
	axis.setValue(0.0, 0.0, 1.0);
    addAxisObject("X", &xyz, color, 2.5, 25, axis, -1.5708, &st[0]);
	objlist->type = type;

	xyz.setValue(0.0, 100.0, 0.0);
	color.setValue(0.0, 1.0, 0.0);
	axis.setValue(0.0, 1.0, 0.0);
    addAxisObject("Y", &xyz, color, 2.5, 25, axis, 0.0, &st[0]);
	objlist->type = type;

	xyz.setValue(0.0, 0.0, 100.0);
	color.setValue(0.0, 0.0, 1.0);
	axis.setValue(1.0, 0.0, 0.0);
    addAxisObject("Z", &xyz, color, 2.5, 25, axis, 1.5708, &st[0]);
	objlist->type = type;
}


void ObjectViewer::addScaleLines()
{
//	SbVec3f xyz[684];
	SbVec3f xyz[693];
	float f;
	int i;

	f = 1.0;
    for(i=0; i<19; i++)
	{
		if (((i+1)%5) == 0) //Linie1
		{
			xyz[i*12+0].setValue(float(-90.0+10.0*i), -100.0, -100.0+4*f);
			xyz[i*12+1].setValue(float(-90.0+10.0*i), -100.0, -100.0);
			xyz[i*12+2].setValue(float(-90.0+10.0*i), -100.0+4*f, -100.0);
		}
		else
		{
			xyz[i*12+0].setValue(float(-90.0+10.0*i), -100.0, -100.0+f);
			xyz[i*12+1].setValue(float(-90.0+10.0*i), -100.0, -100.0);
			xyz[i*12+2].setValue(float(-90.0+10.0*i), -100.0+f, -100.0);
		}
		if (((i+1)%5) == 0) //Linie2
		{
			xyz[i*12+3].setValue(float(-90.0+10.0*i), 100.0, -100.0+4*f);
			xyz[i*12+4].setValue(float(-90.0+10.0*i), 100.0, -100.0);
			xyz[i*12+5].setValue(float(-90.0+10.0*i), 100.0-4*f, -100.0);
		}
		else
		{
			xyz[i*12+3].setValue(float(-90.0+10.0*i), 100.0, -100.0+f);
			xyz[i*12+4].setValue(float(-90.0+10.0*i), 100.0, -100.0);
			xyz[i*12+5].setValue(float(-90.0+10.0*i), 100.0-f, -100.0);
		}
		if (((i+1)%5) == 0) //Linie3
		{
			xyz[i*12+6].setValue(float(-90.0+10.0*i), -100.0, 100.0-4*f);
			xyz[i*12+7].setValue(float(-90.0+10.0*i), -100.0, 100.0);
			xyz[i*12+8].setValue(float(-90.0+10.0*i), -100.0+4*f, 100.0);
		}
		else
		{
			xyz[i*12+6].setValue(float(-90.0+10.0*i), -100.0, 100.0-f);
			xyz[i*12+7].setValue(float(-90.0+10.0*i), -100.0, 100.0);
			xyz[i*12+8].setValue(float(-90.0+10.0*i), -100.0+f, 100.0);
		}
		if (((i+1)%5) == 0) //Linie4
		{
			xyz[i*12+9].setValue(float(-90.0+10.0*i), 100.0, 100.0-4*f);
			xyz[i*12+10].setValue(float(-90.0+10.0*i), 100.0, 100.0);
			xyz[i*12+11].setValue(float(-90.0+10.0*i), 100.0-4*f, 100.0);
		}
		else
		{
			xyz[i*12+9].setValue(float(-90.0+10.0*i), 100.0, 100.0-f);
			xyz[i*12+10].setValue(float(-90.0+10.0*i), 100.0, 100.0);
			xyz[i*12+11].setValue(float(-90.0+10.0*i), 100.0-f, 100.0);
		}
	}
/*	for (i=0; i<228; i++)
	{
		xyz[228+i].setValue(xyz[i][2], xyz[i][0], xyz[i][1]);
		xyz[2*228+i].setValue(xyz[i][1], xyz[i][2], xyz[i][0]);
	}
	addLineObject("Scale Lines", &xyz[0], 228, 684);*/
		xyz[228].setValue(-100.0, -100.0, 0.0);
		xyz[229].setValue(   0.0, -100.0, 0.0);
		xyz[230].setValue( 100.0, -100.0, 0.0);
	for (i=0; i<228; i++)
	{
		xyz[231+i].setValue(xyz[i][2], xyz[i][0], xyz[i][1]);
		xyz[2*231+i].setValue(xyz[i][1], xyz[i][2], xyz[i][0]);
	}
		xyz[231+228].setValue(-100.0, -100.0, 0.0);
		xyz[231+229].setValue(   0.0, -100.0, 0.0);
		xyz[231+230].setValue( 100.0, -100.0, 0.0);
		xyz[2*231+228].setValue(-100.0, -100.0, 0.0);
		xyz[2*231+229].setValue(   0.0, -100.0, 0.0);
		xyz[2*231+230].setValue( 100.0, -100.0, 0.0);
	addLineObject("Scale Lines", &xyz[0], 231, 693);
}


void ObjectViewer::addScaleValues()
{
	SbVec3f        tmptrans, tmpaxis;
	SoText3       *tmpText3;
	SoFont        *tmpFont;
	SoSeparator   *tmpSeparator, *tmpSeparator2;
	SoTransform   *tmpTransform;
	
    initObject("Scale Values", 0);
	tmpSeparator2 = new SoSeparator;
	objlist->separator->addChild(tmpSeparator2);
	tmpFont = new SoFont;
#ifdef WIN32
	tmpFont->name.setValue("Times New Roman");
#else
	tmpFont->name.setValue("Times-Roman");
#endif
	tmpFont->size.setValue(5);
	tmpSeparator2->addChild(tmpFont);
	objlist->type = 1;

	tmpSeparator = new SoSeparator;
	tmpSeparator2->addChild(tmpSeparator);
	tmptrans.setValue(-100.0, -100.0, -100.0);
	tmpaxis.setValue(0.0, 1.0, 0.0);
	tmpTransform = new SoTransform;
    tmpTransform->rotation.setValue(tmpaxis, 4.71239);
	tmpTransform->translation.setValue(tmptrans);
	tmpSeparator->addChild(tmpTransform);
	tmpText3 = new SoText3;
	tmpText3->parts = SoText3::ALL;
	tmpText3->justification = SoText3::RIGHT;
	tmpText3->string = "-100 ";
	tmpSeparator->addChild(tmpText3);

	tmpSeparator = new SoSeparator;
	tmpSeparator2->addChild(tmpSeparator);
	tmptrans.setValue(-100.0, -100.0, -100.0);
	tmpaxis.setValue(1.0, 0.0, 0.0);
	tmpTransform = new SoTransform;
    tmpTransform->rotation.setValue(tmpaxis, 1.5708);
	tmpTransform->translation.setValue(tmptrans);
	tmpSeparator->addChild(tmpTransform);
	tmpText3 = new SoText3;
	tmpText3->justification = SoText3::RIGHT;
	tmpText3->parts = SoText3::ALL;
	tmpText3->string = "-100 ";
	tmpSeparator->addChild(tmpText3);

	tmpSeparator = new SoSeparator;
	tmpSeparator2->addChild(tmpSeparator);
	tmptrans.setValue(0.0, 0.0, 0.0);
	tmpaxis.setValue(0.0, 0.0, 1.0);
	tmpTransform = new SoTransform;
    tmpTransform->rotation.setValue(tmpaxis, 1.5708);
	tmpTransform->translation.setValue(tmptrans);
	tmpSeparator->addChild(tmpTransform);
	tmptrans.setValue(-100.0, 100.0, -100.0);
	tmpaxis.setValue(1.0, 0.0, 0.0);
	tmpTransform = new SoTransform;
    tmpTransform->rotation.setValue(tmpaxis, 3.14159);
	tmpTransform->translation.setValue(tmptrans);
	tmpSeparator->addChild(tmpTransform);
	tmpText3 = new SoText3;
	tmpText3->parts = SoText3::ALL;
	tmpText3->justification = SoText3::RIGHT;
	tmpText3->string = "-100 ";
	tmpSeparator->addChild(tmpText3);

	tmpSeparator = new SoSeparator;
	tmpSeparator2->addChild(tmpSeparator);
	tmptrans.setValue(100.0, 100.0, 100.0);
	tmpaxis.setValue(0.0, 1.0, 0.0);
	tmpTransform = new SoTransform;
    tmpTransform->rotation.setValue(tmpaxis, 4.71239);
	tmpTransform->translation.setValue(tmptrans);
	tmpSeparator->addChild(tmpTransform);
	tmpText3 = new SoText3;
	tmpText3->parts = SoText3::ALL;
	tmpText3->string = " 100";
	tmpSeparator->addChild(tmpText3);

	tmpSeparator = new SoSeparator;
	tmpSeparator2->addChild(tmpSeparator);
	tmptrans.setValue(100.0, 100.0, 100.0);
	tmpaxis.setValue(1.0, 0.0, 0.0);
	tmpTransform = new SoTransform;
    tmpTransform->rotation.setValue(tmpaxis, 1.5708);
	tmpTransform->translation.setValue(tmptrans);
	tmpSeparator->addChild(tmpTransform);
	tmpText3 = new SoText3;
	tmpText3->parts = SoText3::ALL;
	tmpText3->string = " 100";
	tmpSeparator->addChild(tmpText3);

	tmpSeparator = new SoSeparator;
	tmpSeparator2->addChild(tmpSeparator);
	tmptrans.setValue(0.0, 0.0, 0.0);
	tmpaxis.setValue(0.0, 0.0, 1.0);
	tmpTransform = new SoTransform;
    tmpTransform->rotation.setValue(tmpaxis, 1.5708);
	tmpTransform->translation.setValue(tmptrans);
	tmpSeparator->addChild(tmpTransform);
	tmptrans.setValue(100.0, -100.0, 100.0);
	tmpaxis.setValue(1.0, 0.0, 0.0);
	tmpTransform = new SoTransform;
    tmpTransform->rotation.setValue(tmpaxis, 3.14159);
	tmpTransform->translation.setValue(tmptrans);
	tmpSeparator->addChild(tmpTransform);
	tmpText3 = new SoText3;
	tmpText3->parts = SoText3::ALL;
	tmpText3->string = " 100";
	tmpSeparator->addChild(tmpText3);
}


char ObjectViewer::load(char * selectedFile)
{
   ObjectData  *tmplist;

   if (selectedFile == NULL)
	  selectedFile = fl_file_chooser("Select an Inventor (*.iv) file",
                                         "*.iv", NULL) ;

   if (selectedFile != NULL)
   {
      SoSeparator *temp = loadModel(selectedFile) ;
      if (temp != NULL)
      {
         //if root non empty, remove all children
         if (modelRoot) modelRoot->removeAllChildren() ;
         while (objlist != NULL)
         {
            tmplist = objlist;
            objlist = objlist->next;
            delete tmplist;
         }
         //add the loaded model to the root
         modelRoot->addChild(temp) ;
         getChildren(temp, NULL);
		 hideSpheres();
         setCenter();
         orderChildren();
         return 1;
      }
      else
      {
         fl_message("No objects in this file \nor no OpenInventor file.");
         return 0;
      }
   }
   return 0;
}


char ObjectViewer::importObject(char * selectedFile)
{
   char st[256];

   if (selectedFile == NULL)
	  selectedFile = fl_file_chooser("Select an Inventor (*.iv) File",
                                         "*.iv", NULL) ;

   sprintf(st, "(imported)");
   if (selectedFile != NULL)
   {
      SoSeparator *temp = loadModel(selectedFile) ;
      if (temp != NULL)
      {
         modelRoot->addChild(temp) ;
         getChildren(temp, &st[0]);
		 hideSpheres();
         setCenter();
         orderChildren();
         return 1;
      }
      else
      {
         fl_message("No objects in this file \nor no OpenInventor file.");
         return 0;
      }
   }
   return 0;
}


void ObjectViewer::setCenter()
{
    SbString s;
    ObjectData *data;
	char st[256], stl[10], stc[10], stf[10], stj[10];
	int i;

    data = objlist;
    sprintf(stc, "Condyle");
    sprintf(stf, "Fossa");
    sprintf(stl, "LED");
    sprintf(stj, "FOV");
	if ((allTMJs == NULL) || (allTMJs == modelRoot))
	{
		allTMJs = new SoSeparator;
		allTMJs->ref();
	}
	else
		allTMJs->removeAllChildren();
	while (data != NULL)
	{
        getLabel(data, &st[0]);
		i = 0;
        while ((i<255) && (st[i] != ' ') && (st[i] != '\0'))
			i++;
		st[i] = '\0';
		if (strcmp(st, stc)==0)
		{
            allTMJs->addChild(data->separator);
			data->animated = 1;
			data->oldanimated = 1;
/*			if (data->kfslogo == NULL)
			{
				data->kfslogo = new SoTexture2;
				data->separator->insertChild(data->kfslogo, 3);
				data->kfslogo->filename.setValue("KFS_Logo.gif");
			}*/
		}
		if (strcmp(st, stf)==0)
            allTMJs->addChild(data->separator);
		if (strcmp(st, stl)==0)
            allTMJs->addChild(data->separator);
		if (strcmp(st, stj)==0)
            allTMJs->addChild(data->separator);
        data = data->next;
	}

	if (allTMJs->getNumChildren() < 1)
	{
		allTMJs->unref();
		allTMJs = modelRoot;
	}

   resetView();
}


void ObjectViewer::orderChildren()
{
    ObjectData *data;

    data = objlist;
    if (notAnimatedGroup->getNumChildren() > 0)
		notAnimatedGroup->removeAllChildren() ;
    if (animatedGroup->getNumChildren() > 0) 
  	   animatedGroup->removeAllChildren() ;

	while (data != NULL)
	{
		if (data->animated > 0)
			animatedGroup->addChild(data->separator);
		else
            notAnimatedGroup->addChild(data->separator);
        data = data->next;
	}
}


void ObjectViewer::clearObject(ObjectData *obj)
{
    ObjectData *data;

    if (obj == NULL)
        return;

    if (obj == objlist)
        objlist = objlist->next;
    else
    {
        data = objlist;
        while ((data != NULL) && (data->next != obj))
            data = data->next;
        data->next = obj->next;
    }

    modelRoot->removeChild(obj->separator);
    delete obj;

	orderChildren();
}


void ObjectViewer::clearObject(char *str)
{
    char st[256], stc[256];
	ObjectData *data, *tmpdata;

	sprintf(stc, "%s", str);
    data = objlist;
    while (data != NULL)
    {
	   tmpdata = data->next;
       getLabel(data, &st[0]);
       if (strcmp(st, stc)==0)
          clearObject(data);
       data = tmpdata;
    }
}


void ObjectViewer::hideSpheres()
{
    SbString s;
    ObjectData *data;
	char stb[256], stf[256], stt[256], st[256];
	int i;

    data = objlist;
    sprintf(stb, "Back");
    sprintf(stf, "Front");
    sprintf(stt, "Top");
	while (data != NULL)
	{
        getLabel(data, &st[0]);
		i = 0;
        while ((i<255) && (st[i] != ' ') && (st[i] != '\0'))
			i++;
		st[i] = '\0';
		if ((strcmp(st, stb)==0) || (strcmp(st, stf)==0) || (strcmp(st, stt)==0)) 
		{
			data->drawstyle->style = SoDrawStyle::INVISIBLE;
		}
        data = data->next;
	}
}


void ObjectViewer::saveObjects(char *selectedFile)
{
   SbVec3f p;
   SoWriteAction writeAction;
   int i;

   if (selectedFile == NULL)
      selectedFile = 
         fl_file_chooser("Save Objects in an Inventor (*.iv) file", "*.iv",
            NULL) ;

   if (selectedFile != NULL)
   {
      writeAction.getOutput()->openFile(selectedFile);
      writeAction.getOutput()->setBinary(FALSE);
      for (i=0; i<modelRoot->getNumChildren(); i++)
         writeAction.apply(modelRoot->getChild(i));
      writeAction.getOutput()->closeFile();
   }
}


void ObjectViewer::draw()
{
  if (!valid())
  {
     glEnable(GL_DEPTH_TEST);
     glEnable(GL_LIGHTING);
     reshape(w(), h());
  }
  scenemanager->render();
}


SoSeparator* ObjectViewer::loadModel(const char * fileName)
{
	char st[256];

    SoSeparator *root = new SoSeparator ;
    SoInput myScene ;

    //try to open the file
    if (!myScene.openFile(fileName))
    {
	   sprintf(st, "Could not open %s",fileName);
       fl_message(&st[0]);
       return NULL;
    }
   
    //try to read the file
    root = SoDB::readAll(&myScene);

    if (root == NULL)
    {
	   sprintf(st, "Error reading file %s",fileName);
       fl_message(&st[0]);
       myScene.closeFile() ;
       return NULL;
    }

    //close the file
    myScene.closeFile() ;
   
	return root ;
}


void ObjectViewer::btnproc()
{
  SbVec2s newLocator;

  newLocator[0] = Fl::event_x();
  newLocator[1] = Fl::event_y();
  switch(eventbut)
  {
     case 1:
		spinCamera(SbVec2f((newLocator[0])/float(w()), (h() - (newLocator[1]))/float(h())));
        break;
     case 2:
		dollyCamera((float)(locator[1] - newLocator[1]) / 40.0);
        break;
     case 3:
		panCamera(newLocator);
        break;
  }
  locator[0] = newLocator[0];
  locator[1] = newLocator[1];
}

void ObjectViewer::setLocator3D(const SbVec2s &locator)
{
   // Figure out the focal plane
   SbMatrix mx;
   mx = perspCamera->orientation.getValue();
   SbVec3f forward(-mx[2][0], -mx[2][1], -mx[2][2]);
   SbVec3f fp = perspCamera->position.getValue() + 
   forward * perspCamera->focalDistance.getValue();
   focalplane = SbPlane(forward, fp);
		    
   // map mouse starting position onto the panning plane
   SbViewVolume    cameraVolume;
   SbLine	        line;
   cameraVolume = perspCamera->getViewVolume(float(w())/float(h()));
   cameraVolume.projectPointToLine(
      SbVec2f(locator[0]/float(w()), (h()-locator[1])/float(h())), line);
   focalplane.intersect(line, locator3D);
}

int ObjectViewer::handle(int event)
{
  switch(event)
  {
     case FL_PUSH:
         eventbut = Fl::event_button();
         locator[0] = Fl::event_x();
         locator[1] = Fl::event_y();
	     if (eventbut == 1)
		 {
			// set the sphere sheet starting point
            sphereSheet->project(SbVec2f(locator[0]/float(w()), (h() - locator[1])/float(h())));
		 }
		 if (eventbut == 3)
			 setLocator3D(locator);
         return 1;
     case FL_DRAG:
         btnproc();
         return 1;
     case FL_RELEASE:    
         btnproc();
         return 1;
     default:
         // pass other events to the base class...
         return Fl_Gl_Window::handle(event);
  }
}


void ObjectViewer::saveImage(char *filename, char *format, int nr,
							 char *imgcopy, char *extracode)
{
	char cmd[256], nrstring[256], st[256];
	int j;
	char fname[256];
 	
	if (pixelBuffer != NULL)
		delete[] pixelBuffer;
    pixelBuffer = new unsigned char[h()*w()*3];

    redraw();
    glFinish();
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w(), h(), GL_RGB,
                 GL_UNSIGNED_BYTE, (GLvoid *)pixelBuffer);

    sprintf(fname, "%s", filename);
    j = 0;
	while (fname[j] != '\0')
		if (fname[j] == '.')
			fname[j] = '\0';
		else
			j++;

    sprintf(nrstring, "");
    if (nr >= 0)
    { 
	   if (nr < 10)
	      sprintf(nrstring, "%s%d", "000", nr);
	   if ((nr < 100) && (nr > 9))
	      sprintf(nrstring, "%s%d", "00", nr);
	   if ((nr < 1000) && (nr > 99))
	      sprintf(nrstring, "%s%d", "0", nr);
	   if ((nr < 10000) && (nr > 999))
	      sprintf(nrstring, "%d", nr);
    }
    sprintf(st, "%s%s", fname, nrstring);

    writeToSGI(&st[0], nr);
    if (strcmp("sgi", format)!=0)
	{
#ifndef WIN32
        sprintf(cmd, "%s %s.sgi %s%s.%s", imgcopy, st, extracode, st, format);
        printf("%s\n", cmd);
        j = system(cmd);
        sprintf(cmd, "/sbin/rm %s.sgi", fname);
        printf("%s\n", cmd);
        j = system(cmd);
#else
		sprintf(fname, "%s", st);
		j = 0;
		while (fname[j] != '\0')
		{
			if (fname[j] == '/')
				fname[j] = '\\';
			j++;
		}
//		drive = 0;
//		while ((drive<5)&&(fname[drive]!=':')) // Ist fname analog zu "C:\...."
//			drive++;
//		if (drive>=5) 
//           sprintf(cmd, "%s %s%s.sgi %s%s%s.%s", imgcopy, windrive, fname,
//			       extracode, windrive, fname, format);
//		else
           sprintf(cmd, "%s %s.sgi %s%s.%s", imgcopy, fname,
			       extracode, fname, format);
        j = system(cmd);
        printf("%s\n", cmd);
//		if (drive>=5) 
//           sprintf(cmd, "del %s%s.sgi", windrive, fname);
//		else
           sprintf(cmd, "del %s.sgi", fname);
        j = system(cmd);
        printf("%s\n", cmd);
#endif
	}
}


void ObjectViewer::writeToSGI(char *filename, int nr)
{
   int             i, j;
   unsigned char  *image;
   char            st[256];
   FILE           *writefile;
   SGIheader       header;

   image = new unsigned char[3*w()*h()];
   for(i=0; i<(w()*h()); i++)
      for(j=0; j<3; j++)
         image[(j*w()*h())+i] = pixelBuffer[(3*i)+j];  // RGBRGBRGB -> RRRGGGBBB

   for(i=0; i<80; i++)
	   header.name[i] = '\0';

#ifndef WIN32
   header.imagic = 0732;
   header.type = 1;
   header.dim = 3;
   header.xsize = (unsigned short)w();
   header.ysize = (unsigned short)h();
   header.zsize = 3;
   header.min = 0;
   header.max = 255;
   header.wastebytes = 0;
   sprintf(header.name, "%s", "no name");
   header.colormodel = 0;
#else
   header.imagic = 0xDA01;
   header.type = 0x0100;
   header.dim = 0x0300;
   header.xsize = (unsigned short)((256*(w()%256))+(w()/256));
   header.ysize = (unsigned short)((256*(h()%256))+(h()/256));
   header.zsize = 0x0300;
   header.min = 0;
   header.max = 0xFF000000;
   header.wastebytes = 0;
   sprintf(header.name, "%s", "no name");
   header.colormodel = 0x00000000;
#endif

   sprintf(st, "%s.sgi", filename);
   writefile = fopen(st, "wb");
   fwrite((char*) &header, sizeof(SGIheader), 1, writefile);
   for (i=sizeof(SGIheader); i<512; i++)
       fwrite("\0", 1, 1, writefile);
   fwrite(image, 3*w()*h(), 1, writefile);
   fclose(writefile);

   delete[] image;
}



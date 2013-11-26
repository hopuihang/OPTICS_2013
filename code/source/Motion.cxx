#include "Motion.h"

Motion::Motion(Preferences *p, Scanfile *s)
{
	int i, j;

	pref = p; //pointer pref
	scnfile = s; //pointer scnfile
	led = NULL;
	intensity = NULL;
	click = NULL;
	for (i=0; i<3; i++) //repeat 3 times
		RefVec[i].setValue(0.0, 0.0, 0.0); //set RefVec to 3*3 0 matrix

	mTomoToStack.makeIdentity();//do makeIdentity() to all
	mSpheresToLeds.makeIdentity();
    mTomoToRef.makeIdentity();
    mTomoToUVW.makeIdentity();
    mTomoToXYZ.makeIdentity();
    mTomoToXYZi.makeIdentity();
    mTomoToJ3d.makeIdentity();
    mStaticRefTrafo.makeIdentity();
    mStaticMaxTrafo.makeIdentity();
    mRefToJ3d.makeIdentity();

	for (i=0; i<10; i++) //from 0 to 9
	{
	    animation_trafo[i].makeIdentity(); // do makeIdentity to all the matrix from 1st to 10th row.
	    fov_trafo[i].makeIdentity();
	    mRef[i].makeIdentity();
	    mUVW[i].makeIdentity();
	    mRef2[i].makeIdentity();
	    mUVW2[i].makeIdentity();
	    animation_trafoR[i].makeIdentity();
	    fov_trafoR[i].makeIdentity();
	    mRefR[i].makeIdentity();
	    mUVWR[i].makeIdentity();
	}


    animation_step = 0;
	old_step = animation_step;//initialize old_step=0

    num_steps = 0;
	numCameras = 3;
	numCamSystems = 1;//default number of Camera System = 1

	numLEDs = NULL;//default no LEDs


	for (i=0; i<10; i++)//from 0 to 9
	{
		for (j=0; j<18; j++) //Each LED
		{
			LEDofObject[i][j] = pref->LEDofObjectP[j];  //LEDofObjectP[j] assigned to LEDofObject[i][j]
			oldLEDofObject[i][j] = pref->LEDofObjectP[j]; //Same value assigned to oldLEDofObject[i][j]
		}

		for (j=0; j<6; j++) //Each Object
		{
			ObjectsAndReferences[i][j] = pref->ObjectsAndReferencesP[j]; //ObjectsAndReferencesP[j] from pointer pref, assigned to ObjectsAndReferences[i][j]
			oldObjectsAndReferences[i][j] = pref->ObjectsAndReferencesP[j];//same value assigned to oldObjectsAndReferences[i][j]
		}

/*		for (j=0; j<18; j++)
		{
			LEDofObject[i][j] = j/3;
			oldLEDofObject[i][j] = j/3;
		}

		for (j=0; j<6; j++)
		{
			if ((j%2) == 0)
			{
				ObjectsAndReferences[i][j] = -1;
				oldObjectsAndReferences[i][j] = -1;
			}
			else
			{
				ObjectsAndReferences[i][j] = j-1;
				oldObjectsAndReferences[i][j] = j-1;
			}
		}*/



		Speed[i] = 1; //set matrix Speed[0~9] = 1
		oldSpeed[i] = 1; //set matrix oldSpeed[0-9] = 1
	}


	RSnr = -1;
	RSchanged = 0;
    RSsysnr = -1;
}


Motion::~Motion() //delete pointer led,oldled,intensity,oldintensity,and click
{
	int i;

    if (led != NULL)
	{
		for (i=0; i<(numCamSystems*18); i++)
		{
			led[i].deleteValues(0, -1);
			intensity[i].deleteValues(0, -1);
		}
		delete[] led;
		delete[] oldled;
		delete[] intensity;
		delete[] oldintensity;
		led = NULL;
		intensity = NULL;
	}
	if (click != NULL)
	{
		for (i=0; i<numCamSystems; i++)
			click[i].deleteValues(0, -1);
		delete[] click;
		click = NULL;
	}
}


char Motion::read_j3dfile(char *filename)//return type:char
{
    unsigned char  *buf;
    int            fileSize;
    FILE           *readfile;
    int            i, j;
    long           pos;
    unsigned short word[3];
    short          val;
    char           raw_header[198];
	char           st[256];
	MotionError    *err2, *err1 = NULL;
	SbVec3d        vd;

    if ((readfile = fopen(filename, "rb")) == NULL) //if cannnot open file
    {
		sprintf(st, "Error opening file %s", filename); //output "Error opening file XXX"
        fl_message(st);
        return 0;
    }

    // read header
    // -----------

    fread(raw_header, 198, 1, readfile); //header size :198 bytes

    // we only need the comment
    // ------------------------
    strncpy(comment, raw_header, 40);
    comment[40] = '\0';

    // read motion data
    // ----------------
    fseek(readfile, 0, SEEK_END);
    fileSize = ftell(readfile);
    num_steps = (fileSize - 198) / 36;
    buf = new unsigned char[fileSize-198];
    fseek(readfile, 198, SEEK_SET); //streams open in binary mode
    fread((char*) buf, fileSize-198, 1, readfile);
    fclose(readfile);

     if (led != NULL)
	{
		for (i=0; i<(numCamSystems*18); i++)
		{
			led[i].deleteValues(0, -1);// from start,delete all values (-1 means all values deleted)
			oldled[i].deleteValues(0, -1);
			intensity[i].deleteValues(0, -1);
			oldintensity[i].deleteValues(0, -1);
		}
		delete[] led;
		delete[] oldled;
		delete[] intensity;
		led = NULL;
		oldled = NULL;
		intensity = NULL;
		oldintensity = NULL;
	}
	if (click != NULL)
	{
		for (i=0; i<numCamSystems; i++)
			click[i].deleteValues(0, -1);
		delete[] click;
		click = NULL;
	}

	numCamSystems = 1;
	numCameras = 3;
	if (numLEDs != NULL)
		delete[] numLEDs;
	numLEDs = new int[1];
	numLEDs[0] = 6;
	led = new SoMFVec3d[18];
	oldled = new SoMFVec3d[18];
	intensity = new SoMFVec3f[18];
	oldintensity = new SoMFVec3f[18];
	click = new SoMFUShort[1];

    for (i=0; i<18; i++)
    {
		led[i].deleteValues(0, -1);
		led[i].insertSpace(0, num_steps);
		oldled[i].deleteValues(0, -1);
		oldled[i].insertSpace(0, num_steps);
		intensity[i].deleteValues(0, -1);
		intensity[i].insertSpace(0, num_steps);
		oldintensity[i].deleteValues(0, -1);
		oldintensity[i].insertSpace(0, num_steps);
    }
    click[0].deleteValues(0, -1);
    click[0].insertSpace(0, num_steps);

    // calculate LED positions
    // -----------------------
    for (pos=0; pos<num_steps; pos++) //each step
    {
        click[0].set1Value(pos, 0);// click = (step number, 0)
        for (j=0; j<6; j++)//each LEDs
		{
	        for (i=0; i<3; i++)//each coordinate.??

#ifndef WIN32

	           word[i] = 256*buf[(36*pos + 6*j)+2*i+1] 
			       + buf[(36*pos + 6*j)+2*i];   //swap bytes
#else
	           memcpy(&word[i], &buf[(36*pos + 6*j)+2*i], 2); //copy bytes
#endif

	        // The following code extracts the LED coordinates from the raw
	        // data. It is based on the Pascal-Software of Mesqui.


	        // ------------------------------------------------------------
	        for (i=0; i<3; i++)//each coordinate??
	        {
	            if (word[i] != 0x8000)                 //( if the value is valid)   falls (if) Wert(value) gueltig(valid) 
				{				                        
		            if ((j==0)&&(i==0)&&((word[i]&0x4000)!=0)) // (if Bit[14] =1) falls Bit[14] = 1, first i, first j,(Each LED)
       			       click[0].set1Value(pos, 1);	   //(i.e. crack)  d.h. Knacken [Set click = (step_number,1)

                    word[i] = word[i] & 0x3FFF;        // Bit[14] = Bit[15] = 0 (11,12*1)
       		        if ((word[i] & 0x2000) != 0)       // falls (if) Bit[13] = 1 
  		               word[i] = word[i] | 0xC000;     // Bit[14] = Bit[15] = 1 
					
                    val = word[i]; //val is short
		            vd = led[j][pos]; //vd = led[led number][step number]
                    vd[i] = 13.0*double(val)/1000.0;   // CCD-units -> mm (camera units convert to mm unit)

					
					led[j].set1Value(pos, vd); //vd is in mm unit.
					oldled[j].set1Value(pos, vd);
					intensity[j].set1Value(pos, 255, 255, 255);
					oldintensity[j].set1Value(pos, 255, 255, 255);
                } 
 
                else //if Errors
				{
					err2 = new MotionError;
					err2->step = pos;
					err2->led = j;
					err2->next = err1;
					err1 = err2;
					vd = led[j][pos];
                    vd[i] = 10000.0; 
					led[j].set1Value(pos, vd);
					oldled[j].set1Value(pos, vd);
					intensity[j].set1Value(pos, 0);
					oldintensity[j].set1Value(pos, 0);
				}
            }
       }
    }


    delete[] buf;
	if (err1 != NULL)//if Error
	{
		while (err1 != NULL)
		{
			err2 = err1;
			err1 = err1->next;

			pos = err2->step + 1;
			while ((pos < num_steps) &&
				   ((led[err2->led][pos][0] == 10000.0) ||
                    (led[err2->led][pos][1] == 10000.0) ||
				    (led[err2->led][pos][2] == 10000.0)))
				pos++;
			if (pos >= num_steps) // Last step unusable -> shorten (Letzter Schritt unbrauchbar -> kuerzen)
			{
	            for (i=0; i<6; i++)
				   led[i].deleteValues(pos-1, 1);
				   oldled[i].deleteValues(pos-1, 1);
				num_steps--;
				delete err2;
			}
			else
			{
				pos = err2->step - 1;
				while ((pos >= 0) &&
					   ((led[err2->led][pos][0] == 10000.0) ||
		                (led[err2->led][pos][1] == 10000.0) ||
					    (led[err2->led][pos][2] == 10000.0)))
					pos--;
				if (pos < 0)  // Alle Schritte am Anfang unbrauchbar -> kuerzen(All steps in the early unusable - shorten>)
				{
	                for (i=0; i<6; i++)
						led[i].deleteValues(0, err2->step+1);
						oldled[i].deleteValues(0, err2->step+1);
					num_steps -= (err2->step+1);
					while (err2 != NULL) // Restliche Fehlermeldungen loeschen (remaining errors delete)
					{
						delete err2;
						err2 = err1;
						if (err1 != NULL)
							err1 = err1->next;
					}
				}
				else  // Werte interpolieren (Values interpolated)
				{
					led[err2->led].set1Value(err2->step, (led[err2->led][pos]+((err2->step-pos)*led[err2->led][err2->step+1]))/
						(err2->step-pos+1));
					oldled[err2->led].set1Value(err2->step, (led[err2->led][pos]+((err2->step-pos)*led[err2->led][err2->step+1]))/
						(err2->step-pos+1));
					intensity[err2->led].set1Value(err2->step, 255, 255, 255);
					oldintensity[err2->led].set1Value(err2->step, 255, 255, 255);
					delete err2;
				}
			}
		}
	}

	return 1;
}


void Motion::ComputeMaxMean()
{
    long i, pos;

    for (i=0; i<6; i++)//Each Object
    {
       max_leds[i].setValue(0.0, 0.0, 0.0);
       for (pos=0; pos<num_steps; pos++)
          max_leds[i] += led[i][pos];
       max_leds[i] /= double(num_steps);
	}
}

void Motion::ComputeRefMean()
{
    long i, pos;

    for (i=0; i<6; i++)//each Object
    {
       ref_leds[i].setValue(0.0, 0.0, 0.0);//Reset Each Object
       for (pos=0; pos<num_steps; pos++)//Each Step
          ref_leds[i] += led[i][pos];//led = [object number][step number][x,y,z coordinates] get sum of led[i][pos] (sum of x,y,z coordinate values)
       ref_leds[i] /= double(num_steps);//get average value of ref_leds coordinates of Each Object
	}
}


void Motion::FixedXYZ(SdMatrix &rot, double a, double b, double c)//direction cosine (in rad)
{
    rot[0][0] = cos(c)*cos(b);  
    rot[0][1] = cos(c)*sin(b)*sin(a) - sin(c)*cos(a);
    rot[0][2] = cos(c)*sin(b)*cos(a) + sin(c)*sin(a);
	rot[0][3] = 0.0;
    rot[1][0] = sin(c)*cos(b);
    rot[1][1] = sin(c)*sin(b)*sin(a) + cos(c)*cos(a);
    rot[1][2] = sin(c)*sin(b)*cos(a) - cos(c)*sin(a);
	rot[1][3] = 0.0;
    rot[2][0] = - sin(b);
    rot[2][1] = cos(b)*sin(a);
    rot[2][2] = cos(b)*cos(a);
	rot[2][3] = 0.0;
	rot[3][0] = 0.0;
	rot[3][1] = 0.0;
	rot[3][2] = 0.0;
	rot[3][3] = 1.0;
}


void Motion::Vec3ToMatrix(SbVec3d &origo, SbVec3d &px, SbVec3d &pxy, SdMatrix &m)//argument(LED1,2,3,output matrix m)
{
   SbVec3d base[3]; //3*3 matrix

   base[0] = px - origo;
   base[1] = pxy - origo;
   base[2] = base[0].cross(base[1]);//base[0] cross base[1](confirm the direction of z axis)
   base[1] = base[2].cross(base[0]);//confirm the direction of y axis
   base[0].normalize();//gets unit vector
   base[1].normalize();
   base[2].normalize();

   m[0][0] = base[0][0];//gets transformation matrix 4*4
   m[0][1] = base[0][1];
   m[0][2] = base[0][2];
   m[1][0] = base[1][0];
   m[1][1] = base[1][1];
   m[1][2] = base[1][2];
   m[2][0] = base[2][0];
   m[2][1] = base[2][1];
   m[2][2] = base[2][2];
   base[0] *= -1.0;
   base[1] *= -1.0;
   base[2] *= -1.0;
   m[0][3] = base[0].dot(origo);
   m[1][3] = base[1].dot(origo);
   m[2][3] = base[2].dot(origo);
   m[3][0] = 0.0;
   m[3][1] = 0.0;
   m[3][2] = 0.0;
   m[3][3] = 1.0;
}


void Motion::TomoToSpheres()
{
    SdMatrix       rot;
    int            i;
    int            stack;
    SbVec3d        ResVec[3];

//    if (NumReferences() < 3)
//        return cTRAFO_ID;

    // transform references to global MR coordinate system
    // --------------------------------------------------- 



    stack = scnfile->scanref[0].stack - 1;  
    FixedXYZ(rot, scnfile->scanstack[stack].ang_ap / 180.0 * 3.14159, 
                  scnfile->scanstack[stack].ang_cc / 180.0 * 3.14159,
                  scnfile->scanstack[stack].ang_lr / 180.0 * 3.14159);
    rot[0][3] = scnfile->scanstack[stack].offc_ap;
    rot[1][3] = scnfile->scanstack[stack].offc_cc;
    rot[2][3] = scnfile->scanstack[stack].offc_lr;
    for (i=0; i<3; i++)
    {
       RefVec[i][0] = scnfile->scanref[i].pos_x;
       RefVec[i][1] = scnfile->scanref[i].pos_y;
       RefVec[i][2] = scnfile->scanref[i].pos_z;
    }

    // compute transformation from MR to reference system
    // --------------------------------------------------
    for (i=0; i<3; i++)
    {
       rot.multMatrixVec(RefVec[i], ResVec[i]);
       RefVec[i] = ResVec[i];
    }
    Vec3ToMatrix(RefVec[0], RefVec[1], RefVec[2], mTomoToStack);
}


void Motion::Vec2BaseTrans(SbVec3d base_from[3], SbVec3d base_to[3],
     SbVec3d from_origin, SbVec3d to_origin, SdMatrix &m)
{
   SbVec3d v1;

   m[0][0] = base_from[0].dot(base_to[0]);
   m[0][1] = base_from[1].dot(base_to[0]);
   m[0][2] = base_from[2].dot(base_to[0]);
   m[1][0] = base_from[0].dot(base_to[1]);
   m[1][1] = base_from[1].dot(base_to[1]);
   m[1][2] = base_from[2].dot(base_to[1]);
   m[2][0] = base_from[0].dot(base_to[2]);
   m[2][1] = base_from[1].dot(base_to[2]);
   m[2][2] = base_from[2].dot(base_to[2]);
   v1 = from_origin - to_origin;
   m[0][3] = v1.dot(base_to[0]);
   m[1][3] = v1.dot(base_to[1]);
   m[2][3] = v1.dot(base_to[2]);
   m[3][0] = 0.0;
   m[3][1] = 0.0;
   m[3][2] = 0.0;
   m[3][3] = 1.0;
}


void Motion::BaseTrans(SbVec3d from_base[3], SbVec3d to_base[3], SdMatrix &m)
{
   SbVec3d base_from[3];//3*3 matrix
   SbVec3d base_to[3];//3*3 matrix
   SbVec3d v1;
   SbVec3d v2;

   v1 = from_base[1] - from_base[0];
   v2 = from_base[2] - from_base[0];
   base_from[0] = v1;
   base_from[2] = v1.cross(v2);
   base_from[0].normalize();
   base_from[2].normalize();
   base_from[1] = base_from[2].cross(base_from[0]);
   v1 = to_base[1] - to_base[0];
   v2 = to_base[2] - to_base[0];
   base_to[0] = v1;
   base_to[2] = v1.cross(v2);
   base_to[0].normalize();
   base_to[2].normalize();
   base_to[1] = base_to[2].cross(base_to[0]);

   Vec2BaseTrans(base_from, base_to, from_base[0], to_base[0], m);
}


void Motion::SpheresToLeds()
/*--------------------------------*/
{
    int      i;
    SbVec3d  base_ref[3];
    SbVec3d  base_xyz[3];
    SbVec3d  led_top, led_front, led_back;
    SbVec3d  v1, v2, origin;

    for (i=0; i<3; i++)
    {
       base_ref[i][0] = scnfile->scanref[i].cal_x;
       base_ref[i][1] = scnfile->scanref[i].cal_y;
       base_ref[i][2] = scnfile->scanref[i].cal_z;
    }

    for (i=0; i<3; i++)
    {
       if (strcmp(scnfile->scanled[i].name, "Top") == 0)
       {
          led_top[0] = scnfile->scanled[i].cal_x;
          led_top[1] = scnfile->scanled[i].cal_y;
          led_top[2] = scnfile->scanled[i].cal_z;
       }
       if (strcmp(scnfile->scanled[i].name, "Front") == 0)
       {
          led_front[0] = scnfile->scanled[i].cal_x;
          led_front[1] = scnfile->scanled[i].cal_y;
          led_front[2] = scnfile->scanled[i].cal_z;
       }
       if (strcmp(scnfile->scanled[i].name, "Back") == 0)
       {
          led_back[0] = scnfile->scanled[i].cal_x;
          led_back[1] = scnfile->scanled[i].cal_y;
          led_back[2] = scnfile->scanled[i].cal_z;
       }
    }
    v1 = led_back - led_front; 
    v2 = led_top - led_front; 
    origin = v1 * (v1.dot(v2)/v1.dot(v1));
    origin += led_front; 
    base_xyz[0] = origin;
    base_xyz[1] = led_front;
    base_xyz[2] = v1.cross(v2);
    base_xyz[2] += origin;
    BaseTrans(base_ref, base_xyz, mSpheresToLeds);
}


void Motion::LowerToUpper(SbVec3d led[6], SdMatrix &m)
{
   SbVec3d baseXYZ[3], baseUVW[3];
   SbVec3d originXYZ, originUVW, v1, v2;

    // determine the orthonormal basis of the XYZ-system
    // expressed in the global, spatial invariant system
    // formed by the cameras.
    // ------------------------------------------------- 
   v1 = led[1] - led[0];
   v2 = led[2] - led[0];
   originXYZ = v1*((v1.dot(v2))/(v1.dot(v1))) + led[0];
   baseXYZ[0] = -v1;
   baseXYZ[0].normalize();
   baseXYZ[1] = v2.cross(baseXYZ[0]);
   baseXYZ[1].normalize();
   baseXYZ[2] = baseXYZ[0].cross(baseXYZ[1]);

    // determine the orthonormal basis of the UVW-system
    // expressed in the global, spatial invariant system
    // formed by the cameras.
    // -------------------------------------------------
   
   originUVW = led[5];
   baseUVW[0] = led[3] - led[5];
   baseUVW[0].normalize();
   baseUVW[2] = baseUVW[0].cross(led[4] - led[5]);
   baseUVW[2].normalize();
   baseUVW[1] = baseUVW[2].cross(baseUVW[0]);

    // calculate transformation from the UVW- to the XYZ-system
    // -------------------------------------------------------- 
   Vec2BaseTrans(baseUVW, baseXYZ, originUVW, originXYZ, m);
}


void Motion::RefToJ3d(SdMatrix &m)
{
   SdMatrix uvw_to_j3d;
   SdMatrix j3d_to_uvw;

   Vec3ToMatrix(max_leds[5], max_leds[3], max_leds[4], j3d_to_uvw);
   uvw_to_j3d = mStaticRefTrafo.inverse();
   m = j3d_to_uvw.inverse();
   m *= uvw_to_j3d;
}


void Motion::calculateReferences()
{
   char max_name[256], ref_name[256];

   if (scnfile->scanmov.max_nr < 10)
      sprintf(max_name, "%s/%s/%s.0%d", pref->getdatapath(),
         scnfile->scanmov.path, scnfile->scanmov.base,
         scnfile->scanmov.max_nr);
   else
      sprintf(max_name, "%s/%s/%s.%d", pref->getdatapath(),
         scnfile->scanmov.path, scnfile->scanmov.base,
         scnfile->scanmov.max_nr);
   if (scnfile->scanmov.ref_nr < 10)
      sprintf(ref_name, "%s/%s/%s.0%d", pref->getdatapath(),
         scnfile->scanmov.path, scnfile->scanmov.base,
         scnfile->scanmov.ref_nr);
   else
      sprintf(ref_name, "%s/%s/%s.%d", pref->getdatapath(),
         scnfile->scanmov.path, scnfile->scanmov.base,
         scnfile->scanmov.ref_nr);

   read_j3dfile(max_name);
   ComputeMaxMean();
   read_j3dfile(ref_name);
   ComputeRefMean();

   TomoToSpheres();
   SpheresToLeds();
   mTomoToRef = mSpheresToLeds;
   mTomoToRef *= mTomoToStack;
   LowerToUpper(ref_leds, mStaticRefTrafo); 
   mTomoToUVW = mStaticRefTrafo.inverse();
   mTomoToUVW *= mTomoToRef;
   LowerToUpper(max_leds, mStaticMaxTrafo); 
   mTomoToXYZ = mStaticMaxTrafo;
   mTomoToXYZ *= mTomoToUVW;
   mTomoToXYZi = mTomoToXYZ.inverse();
   RefToJ3d(mRefToJ3d);
   mTomoToJ3d = mRefToJ3d;
   mTomoToJ3d *= mTomoToRef;
}


void Motion::Process()
/*------------------------*/
{
    long       step, l, k;
    SbVec3d    *temp;
    double     sum;

    temp = new SbVec3d[num_steps];
    for (l=0; l<6; l++)//Each object
    {
        // smooth beginning of sequence
        // ----------------------------
        for (step=0; step<4; step++)
        {
            temp[step].setValue(0.0, 0.0, 0.0);
            sum = 0.0;
            for (k= (-4); k<=4; k++)
            {
                if (step+k >= 0)
                {
                    temp[step] += coeff[k+4] * led[l][step+k];
                    sum += coeff[k+4];
                }
            }
            temp[step] /= sum;
        }

        // smooth end of sequence
        // ----------------------
        for (step=num_steps-1; step>=num_steps-4; step--)
        {
            temp[step].setValue(0.0, 0.0, 0.0);
            sum = 0.0;
            for (k= (-4); k<=4; k++)
            {
                if (step+k < num_steps)
                {
                    temp[step] += coeff[k+4] * led[l][step+k];
                    sum += coeff[k+4];
                }
            }
            temp[step] /= sum;
        }

        // smooth rest
        // -----------
        sum = 0.0;
        for (k=0; k<=2*4; k++)
            sum += coeff[k];

        for (step=4; step<num_steps-4; step++)
        {
            temp[step].setValue(0.0, 0.0, 0.0);
            for (k= (-4); k<=4; k++)
               temp[step] += coeff[k+4] * led[l][step+k];
            temp[step] /= sum;
        }

        for (step=0; step<num_steps; step++)
            led[l].set1Value(step, temp[step]);
    }
    delete[] temp;
}


void Motion::load_j3dfile(const char *filename)
{
   int j, i;
//   long step;
   SdMatrix m1, m2;
   SbVec3d  tmpled[6];
   char st[256], cmd[256];

   animation_step = 0;
   old_step = 0;

   sprintf(st, "%s", filename);
   sprintf(flname, "%s", filename);
   j = 0;
   while (st[j] != ' ')
     j++;
   st[j] = '\0';
   sprintf(cmd, "%s/%s/%s", pref->getdatapath(),
           scnfile->scanmov.path, st);
   if (read_j3dfile(cmd) == 0)
   {
	   // alles zurücksetzen
	   return;
   }

   for (i=0; i<3; i++)
	   Process();
   calc_animationTrafo(0);
/*   animation_trafo = new SdMatrix[num_steps];
   for (step=0; step<num_steps; step++)
   {
      for (j=0; j<6; j++)
         tmpled[j] = led[j][step];
      LowerToUpper(tmpled, m1);
      m2 = m1;
      m2 *= mTomoToUVW;
      animation_trafo[step] = mTomoToXYZi;
      animation_trafo[step] *= m2;
   }*/
}


void Motion::oneStepForward()
{
	double plus;
	int i, j;

	j = 1;
	for (i=0; i<10; i++)
	{
		if (((j==2) && (Speed[i]==3)) ||
			((j==3) && (Speed[i]==2)))
			j = 6;
		if (Speed[i] > j)
			j = Speed[i];
	}
	old_step = animation_step;
	plus = 1.0 / double(j);
	animation_step += plus;
    if (animation_step >= num_steps)
		animation_step = 0;
}


void Motion::oneStepBackward()
{
	double minus;
	int i, j;

	j = 1;
	for (i=0; i<10; i++)
	{
		if (((j==2) && (Speed[i]==3)) ||
			((j==3) && (Speed[i]==2)))
			j = 6;
		if (Speed[i] > j)
			j = Speed[i];
	}
	old_step = animation_step;
	minus = 1.0 / double(j);
	animation_step -= minus;
    if (animation_step < 0)
		animation_step = num_steps-1+ double(double(j-1.0)/double(j));
	if ((animation_step - floor(animation_step)) > 0.9) // (round, circle) runden
		animation_step = floor(animation_step)+1.0;
}


SbMatrix Motion::getActualMatrix(int fov, long step, int camsysnr)
{
	SbMatrix m;
	SdMatrix md;

    if (step < 0)
       calc_animationTrafo(animation_step);
	else
       calc_animationTrafo(step);
	if (fov)
	{
        md = fov_trafo[camsysnr].inverse();
        md *= animation_trafo[camsysnr];
        md.setSbMatrix(m);
	}
	else
		animation_trafo[camsysnr].setSbMatrix(m);
	return m;
}


char Motion::getClick(char up)
{
	int i;
	char c = 0;

    if ((old_step == animation_step) || (up == 0))
		return 0;
	i = old_step;
    while (i != animation_step)
	{
	   if (up>0)
		   i++;
	   else
		   i--;
	   if (i >= num_steps)
		   i = 0;
	   if (i < 0)
		   i = num_steps;
       if (click[0][i] > 0)
	   {
		   c = 1;
		   i = animation_step; //a crack is enough (ein Knacken genuegt)
	   }
	}
	return c;
}


void Motion::resetStep()
{
	animation_step = 0;
	old_step = animation_step;
}


void Motion::goToStep(double step)
{
	old_step = animation_step;
	animation_step = step;
    if (animation_step < 0)
		animation_step = 0;
    if (animation_step >= num_steps)
		animation_step = num_steps-0.11; // slightly less than num_steps (etwas weniger als num_steps)
}


void Motion::CalcUVWtoXYZ(SbVec3d &uvw, SbVec3d &xyz)//matrix.function(input vec3d,out put vec3d)
{
   SdMatrix mUVWToTomo;

   mUVWToTomo = mTomoToUVW.inverse();//this is a transfomation matrix
   mUVWToTomo.multMatrixVec(uvw, xyz);//multiply mUVWToTomo with this matrix,out put xyz
}


void Motion::CalcXYZtoUVW(SbVec3d &xyz, SbVec3d &uvw)//matrix.function(input vec3d,out put vec3d)
{
   mTomoToUVW.multMatrixVec(xyz, uvw);//multiply mTomoToUVW with xyz, output uvw
}


void Motion::CalcTrajPoint(SbVec3d &src, SbVec3f &dst, long step, int camsysnr)
{
   SbVec3d tmp;

   if ((step > (num_steps-1)) || (step < 0))
	   tmp.setValue(src[0], src[1], src[2]);
   else
   {
	   calc_animationTrafo(step);
	   animation_trafo[camsysnr].multMatrixVec(src, tmp);
   }
   dst.setValue((float)tmp[0], (float)tmp[1], (float)tmp[2]);
}


void Motion::initRealTime(int numCams)
{
	int i;

    if (led != NULL)//reset LEDs
	{
		for (i=0; i<(numCamSystems*18); i++)
		{
			led[i].deleteValues(0, -1);
			oldled[i].deleteValues(0, -1);
			intensity[i].deleteValues(0, -1);
			oldintensity[i].deleteValues(0, -1);
		}
		delete[] led;
		delete[] oldled;
		delete[] intensity;
		delete[] oldintensity;
		led = NULL;
		oldled = NULL;
		intensity = NULL;
		oldintensity = NULL;
	}
	if (click != NULL)//reset Click
	{
		for (i=0; i<numCamSystems; i++)
			click[i].deleteValues(0, -1);
		delete[] click;
		click = NULL;
	}

	numCameras = numCams;
	numCamSystems = numCams/3;

	if (numLEDs != NULL)          //reset numLEDs
		delete[] numLEDs;

	numLEDs = new int[numCamSystems];           //re-assign memory

	for (i=0; i<numCamSystems; i++)
	{
	    numLEDs[i] = 6;
	}

	led = new SoMFVec3d[numCamSystems*18];//maximum number of LED is 18
	oldled = new SoMFVec3d[numCamSystems*18];
	intensity = new SoMFVec3f[numCamSystems*18];
	oldintensity = new SoMFVec3f[numCamSystems*18];
	click = new SoMFUShort[numCamSystems];

	num_steps = 0;
	animation_step = 0;
}


char Motion::read_mvmfile(char * filename)
{
	FILE    *readfile;
	char    st[256];
	unsigned char c, d, e, f;
	long    fileSize, pos;
	int     i, j, k;
	int     imap,ibit,irest;
	double  dimap,dmap;
	SbVec3d xyz[18];
	SbVec3f intens[18];

	sprintf(pref->loadfilename, filename);	//060815 save load filename

    if ((readfile = fopen(filename, "rb")) == NULL)
    {
		sprintf(st, "Error opening file %s", filename);
        fl_message(st);
        return 0;
    }

    if (led != NULL) //reset
	{
		for (i=0; i<(numCamSystems*18); i++)
		{
			led[i].deleteValues(0, -1);
			oldled[i].deleteValues(0, -1);
			intensity[i].deleteValues(0, -1);
			oldintensity[i].deleteValues(0, -1);
		}
		delete[] led;
		delete[] oldled;
		delete[] intensity;
		delete[] oldintensity;
		led = NULL;
		oldled = NULL;
		intensity = NULL;
		oldintensity = NULL;
	}
	if (click != NULL)
	{
		for (i=0; i<numCamSystems; i++)
			click[i].deleteValues(0, -1);
		delete[] click;
		click = NULL;
	}

    // read header
    // -----------
	fseek(readfile, 0, SEEK_SET);
    fread(comment, 256, 1, readfile);
	comment[255] = '\0';

    fseek(readfile, 0, SEEK_END);
    fileSize = ftell(readfile);

	numCameras = 0;
	fseek(readfile, 1024, SEEK_SET);
	if (fileSize >= 8704)
		k = 30;
	else
		k = (fileSize-1024)/256;
	for (i=0; i<k; i++)
	{
		fread(&c, 1, 1, readfile);
		if (c > numCameras)
			numCameras = c;
		for (j=1; j<256; j++)
			fread(&c, 1, 1, readfile);
	}
    num_steps = (fileSize - 1024) / (numCameras*256);

	numCamSystems = numCameras/3;
	led = new SoMFVec3d[numCamSystems*18];
	oldled = new SoMFVec3d[numCamSystems*18];
	intensity = new SoMFVec3f[numCamSystems*18];
	oldintensity = new SoMFVec3f[numCamSystems*18];
	click = new SoMFUShort[numCamSystems];
    for (i=0; i<(numCamSystems*18); i++)
	{
        led[i].insertSpace(0, num_steps);
        oldled[i].insertSpace(0, num_steps);
        intensity[i].insertSpace(0, num_steps);
        oldintensity[i].insertSpace(0, num_steps);
	}
    for (i=0; i<numCamSystems; i++)
		click[i].insertSpace(0, num_steps);

	if (numLEDs != NULL)
		delete[] numLEDs;
	numLEDs = new int[numCamSystems];
	for (i=0; i<numCamSystems; i++)
	    numLEDs[i] = 0;
	fseek(readfile, 1024, SEEK_SET);
    for (pos=0; pos<num_steps; pos++)
    {
		for (i=0; i<18; i++)
		{
			xyz[i].setValue(-120.0, -120.0, -120.0);
			intens[i].setValue(0, 0, 0);
		}
		for (i=0; i<numCameras; i++)
		{
			click[i/3].set1Value(pos, 0);
			fread(&c, 1, 1, readfile);		//camera number(xyz)
			fread(&c, 1, 1, readfile);		//data length(bit)
			ibit = c;
			fread(&c, 1, 1, readfile);		//data number(step)1
			fread(&c, 1, 1, readfile);		//data number(step)2
			fread(&c, 1, 1, readfile);		//data number(step)3
			fread(&c, 1, 1, readfile);		//data number(step)4
			fread(&c, 1, 1, readfile);		//mapping(devided number)1
			fread(&d, 1, 1, readfile);		//mapping(devided number)2
			imap = 256 * c;
			imap +=d;
			if(ibit==32)			//for 32bit recording data
			{
				   dimap = float(imap) * 65536.0;
				   dmap  = 32768.0 / float(imap);
				   irest = 98;
			}else if(ibit==16)		//for 16bit recording data
			{
				   dimap = float(imap);
				   dmap  = 32768.0 / float(imap);
				   irest = 62;

			}else if(ibit==0)		//for 16bit sample data
			{
				   dimap = 320.0;
				   dmap  = 102.4;
				   irest = 62;

			}

			for(j=0; j<18; j++)
			{
				if(ibit==32)
				{
			   fread(&c, 1, 1, readfile);
			   fread(&d, 1, 1, readfile);
			   fread(&e, 1, 1, readfile);
			   fread(&f, 1, 1, readfile);
			   if ((c>0)||(d>0)||(e>0)||(f>0))
			   {
					xyz[j][i%3] = 256 * 256* 256 * float(c);
					xyz[j][i%3] += 256 * 256 * float(d);
					xyz[j][i%3] += 256 * float(e);
					xyz[j][i%3] += float(f);
					xyz[j][i%3] /= dimap;
					xyz[j][i%3] -= dmap;

					/*xyz[j][i%3] = 256 * 256* 256 * float(c);
					xyz[j][i%3] += 256 * 256 * float(d);
					xyz[j][i%3] += 256 * float(e);
					xyz[j][i%3] += float(f);
					xyz[j][i%3] /= 13107200.0;
					xyz[j][i%3] -= float(163.84);*/

					/*xyz[j][i%3] = 256 * c;
					xyz[j][i%3] += d;
					xyz[j][i%3] /= 320.0;
					xyz[j][i%3] -= 102.4;*/
					if (pos == 0)
						numLEDs[i/3]++;
			   }
				}else if((ibit==16)||(ibit==0))
				{
			   fread(&c, 1, 1, readfile);
			   fread(&d, 1, 1, readfile);
			   if ((c>0)||(d>0))
			   {
					xyz[j][i%3] = 256 * c;
					xyz[j][i%3] += d;
					xyz[j][i%3] /= dimap;
					xyz[j][i%3] -= dmap;
					if (pos == 0)
						numLEDs[i/3]++;
			   }
				}

			}
			for(j=0; j<18; j++)
			{
			   fread(&c, 1, 1, readfile);
			   intens[j][i%3] = float(c);
			}
			for(j=irest; j<256; j++)
//			for(j=62; j<256; j++)
//			for(j=98; j<256; j++)
			{
			   fread(&c, 1, 1, readfile);
			}
			if ((i%3) == 2)
			{
				for (j=0; j<18; j++)
				{
					led[((i/3)*18)+j].set1Value(pos, xyz[j]);
					oldled[((i/3)*18)+j].set1Value(pos, xyz[j]);
					xyz[j].setValue(-120.0, -120.0, -120.0);
					intensity[((i/3)*18)+j].set1Value(pos, intens[j]);
					oldintensity[((i/3)*18)+j].set1Value(pos, intens[j]);
					intens[j].setValue(0, 0, 0);

				}
			}
		}
	}
    fclose(readfile);
	for (i=0; i<numCamSystems; i++)
	   numLEDs[i] /= 3;
	return 1;
}


void Motion::load_mvmfile(const char *filename)
{
   int j;
   char st[256], cmd[256];

   sprintf(st, "%s", filename);
   sprintf(flname, "%s", filename);
   j = 0;
   while (st[j] != ' ')
     j++;
   st[j] = '\0';
   sprintf(cmd, "%s/%s/%s", pref->getdatapath(),
           scnfile->scanmov.path, st);
   if (read_mvmfile(cmd) == 0)
   {
	   //(Set back to reset everything?) alles zurücksetzen zuruecksetzen?
	   return;
   }

/*   Process();
   Process();
   Process();*/

   calc_animationTrafo(0);

//   calc_animationTrafo();
}




/*
void Motion::calc_animationTrafo()
{
   SdMatrix m1, m2, mfirst, mref, muvw;
   long step;
   SbVec3d tmpled[6];

   if (animation_trafo != NULL)
      delete[] animation_trafo;
   animation_trafo = NULL;
   animation_step = 0;
   old_step = 0;
   animation_trafo = new SdMatrix[num_steps];
   if (fov_trafo != NULL)
      delete[] fov_trafo;
   fov_trafo = NULL;
   fov_trafo = new SdMatrix[num_steps];
   for (step=0; step<num_steps; step++)
   {
	  tmpled[3] = led[0][step];
	  tmpled[4] = led[1][step];
	  tmpled[5] = led[2][step];
	  tmpled[0].setValue(0.0, 0.0, 0.0);
	  tmpled[1].setValue(1.0, 0.0, 0.0);
	  tmpled[2].setValue(0.0, 1.0, 0.0);
	  BaseTrans(&tmpled[0], &tmpled[3], m2);
	  tmpled[3] = led[3][step];
	  tmpled[4] = led[4][step];
	  tmpled[5] = led[5][step];
	  tmpled[0].setValue(0.0, 0.0, 0.0);
	  tmpled[1].setValue(1.0, 0.0, 0.0);
	  tmpled[2].setValue(0.0, 1.0, 0.0);
	  BaseTrans(&tmpled[0], &tmpled[3], m1);
	  if (step == 0)
	  {
		  mref = m2.inverse();
          fov_trafo[step].makeIdentity();

		  muvw = m1;
          animation_trafo[step].makeIdentity();
	  }
	  else
	  {
          fov_trafo[step] = mref;
          fov_trafo[step] *= m2;
          animation_trafo[step] = fov_trafo[step];
          animation_trafo[step] *= m1.inverse();
          animation_trafo[step] *= muvw;
	  }
   }
}*/

void Motion::calc_animationTrafo(double dStep, int objnr)
{
   SdMatrix m1, m2;
   SbVec3d tmpled[6];
   int camsysnr, i, j;
   SbVec3d testvec, tmpvec;
   SbVec3d ledvec[18];
   long step;
   int tmpRSnr1, tmpRSnr2;

   if (RSchanged > 0)  // Zuerst Matrizen zu Step 0 berechnen!
	   //At first a matrix is calcurated based upon step 0.
   {
	   RSchanged = 0;
	   calc_animationTrafo(0.0, objnr);
   }

   step = (long)dStep;
   if (num_steps == 0)//reset to default
   {
      for (camsysnr=0; camsysnr<numCamSystems; camsysnr++)
	  {
	 	  mRef[camsysnr].makeIdentity();
	      mUVW[camsysnr].makeIdentity();
	      fov_trafo[camsysnr].makeIdentity();
	      animation_trafo[camsysnr].makeIdentity();
	  }
	  return;
   }

   testvec.setValue(-120.0, -120.0, -120.0);
   for (camsysnr=0; camsysnr<numCamSystems; camsysnr++)
   {
	   if (pref->camSysOverlap[camsysnr] > 0)
	   {
		   m1.makeIdentity();
		   FixedXYZ(m1, pref->camSysRot[camsysnr][0]/180.0*3.14159,
			  pref->camSysRot[camsysnr][1]/180.0*3.14159,
			  pref->camSysRot[camsysnr][2]/180.0*3.14159);//unit:rad
		   m1[0][3] = pref->camSysTrans[camsysnr][0];
		   m1[1][3] = pref->camSysTrans[camsysnr][1];
		   m1[2][3] = pref->camSysTrans[camsysnr][2];
		   m2 = m1.inverse();
	   }
	   for (i=0; i<18; i++)
	   {
		   ledvec[i] = led[camsysnr*18+i][step];
		   if ((camsysnr == 0) && (ledvec[i] == testvec))
		   {
			   j = 1;
			   while (j<numCamSystems)
			   {
				   if (pref->camSysOverlap[j] > 0)
				   {
					   tmpvec = led[j*18+i][step];
					   if (tmpvec != testvec)
					   {
						   m1.makeIdentity();
						   FixedXYZ(m1, pref->camSysRot[j][0]/180.0*3.14159,
							  pref->camSysRot[j][1]/180.0*3.14159,
							  pref->camSysRot[j][2]/180.0*3.14159);
						   m1[0][3] = pref->camSysTrans[j][0];
						   m1[1][3] = pref->camSysTrans[j][1];
						   m1[2][3] = pref->camSysTrans[j][2];
						   m1.multMatrixVec(tmpvec, ledvec[i]);
						   if (step == 0)
						      led[camsysnr*18+i].set1Value(step, ledvec[i]);
						   j = numCamSystems;
					   }
				   }
				   j++;
			   }
		   }
		   if ((camsysnr > 0) && (ledvec[i] == testvec) && (pref->camSysOverlap[camsysnr] > 0))
		   {
			   tmpvec = led[i][step];
			   if (tmpvec != testvec)
			   {
				   m2.multMatrixVec(tmpvec, ledvec[i]);
				   if (step == 0)
				      led[camsysnr*18+i].set1Value(step, ledvec[i]);
			   }
		   }
	   }
	   if (Speed[camsysnr] == 2)
	   {
		   if ((dStep-step)>=0.5)
		   {
			   for (i=0; i<6; i++)
				   ledvec[i] = ledvec[i+9];
		   }
	   }
	   if (Speed[camsysnr] == 3)
	   {
		   if (((dStep-step)>=(1.0/3.0)) && ((dStep-step)<(2.0/3.0)))
		   {
			   for (i=0; i<6; i++)
				   ledvec[i] = ledvec[i+6];
		   }
		   if ((dStep-step)>=(2.0/3.0))
		   {
			   for (i=0; i<6; i++)
				   ledvec[i] = ledvec[i+12];
		   }
	   }
	   if((ledvec[0] != testvec) &&  // aendern!
		  (ledvec[1] != testvec) &&
		  (ledvec[2] != testvec) &&
		  (ledvec[3] != testvec) &&
		  (ledvec[4] != testvec) &&
		  (ledvec[5] != testvec))
	   {
		   if (RSnr == -1)  // Referenz
			   tmpRSnr1 = 0;
		   else
			   tmpRSnr1 = RSnr;
		   if ((objnr >= 0) && (objnr != tmpRSnr1)) // Objekt
			   tmpRSnr2 = objnr;
		   else
		   {
			   if (RSnr == 1)
				   tmpRSnr2 = 1;
//				   tmpRSnr2 = 0;
			   else
				   tmpRSnr2 = 0;
//				   tmpRSnr2 = 1;
		   }
		   i=0;
		   j=0;
		   while ((i<18) && (j<3))
		   {
			   if (LEDofObject[camsysnr][i] == tmpRSnr1)
			   {
				   tmpled[3+j] = ledvec[i];
				   j++;
			   }
			   i++;
		   }
//		   tmpled[3] = ledvec[0];
//		   tmpled[4] = ledvec[1];
//		   tmpled[5] = ledvec[2];
		   tmpled[0].setValue(0.0, 0.0, 0.0);
		   tmpled[1].setValue(1.0, 0.0, 0.0);
		   tmpled[2].setValue(0.0, 1.0, 0.0);
		   BaseTrans(&tmpled[0], &tmpled[3], m2);
		   i=0;
		   j=0;
		   while ((i<18) && (j<3))
		   {
			   if (LEDofObject[camsysnr][i] == tmpRSnr2)
			   {
				   tmpled[3+j] = ledvec[i];
				   j++;
			   }
			   i++;
		   }
//		   tmpled[3] = ledvec[3];
//		   tmpled[4] = ledvec[4];
//		   tmpled[5] = ledvec[5];
		   tmpled[0].setValue(0.0, 0.0, 0.0);
		   tmpled[1].setValue(1.0, 0.0, 0.0);
		   tmpled[2].setValue(0.0, 1.0, 0.0);
		   BaseTrans(&tmpled[0], &tmpled[3], m1);
		   if (step == 0)
		   {
		 	  mRef[camsysnr] = m2.inverse();
		      fov_trafo[camsysnr].makeIdentity();
		      mUVW[camsysnr] = m1;
		      animation_trafo[camsysnr].makeIdentity();
		   }
		   else
		   {
		      fov_trafo[camsysnr] = mRef[camsysnr];
		      fov_trafo[camsysnr] *= m2;
		      animation_trafo[camsysnr] = fov_trafo[camsysnr];
		      animation_trafo[camsysnr] *= m1.inverse();
		      animation_trafo[camsysnr] *= mUVW[camsysnr];
		   }
	   }
	   else
	   {
		   if (step == 0)
		   {
		 	  mRef[camsysnr].makeIdentity();
		      mUVW[camsysnr].makeIdentity();
		      fov_trafo[camsysnr].makeIdentity();
		      animation_trafo[camsysnr].makeIdentity();
		   }
		   else
		   {
		      fov_trafo[camsysnr].makeIdentity();
		      animation_trafo[camsysnr].makeIdentity();
		   }
	   }
   }
}


void Motion::setRSnr(int nr)
{
	RSnr = nr;
	RSchanged = 1;
}


void Motion::exportTrajectories(SbVec3d &src, char *filename, char add, char *txt, char *headertxt, 
								int camsysnr, int objnr, int refnr, long start, long stop, int type,int type2, SbVec3d *src_leds)
{
	FILE *writefile;
	SbVec3d dst;
	SbVec3d dst1;
	char st[256], fname[256];
	int i,  j, tmplednr[3];
    int oldRSnr;
	SbVec3d vd1;
	SbVec3d tmp1;
	SdMatrix md;
	SdMatrix m1;

    sprintf(fname, "%s", filename);
    i = 0;
	while (fname[i] != '\0')
	{
		if (fname[i] == '.')
			fname[i] = '\0';
		else
			i++;
	}
	strcat(fname, ".xls");

	if (add)
	{
		if ((writefile = fopen(fname, "ab")) == NULL)
		{
			sprintf(st, "Error opening file %s", filename);
			fl_message(st);
			return;
		}
	}
	else
	{
		if ((writefile = fopen(fname, "wb")) == NULL)
		{
			sprintf(st, "Error opening file %s", filename);
			fl_message(st);
			return;
		}
	}

	oldRSnr = RSnr;
	setRSnr(refnr);

	if(!add)
	{		
		fwrite(pref->loadfilename, strlen(pref->loadfilename), 1, writefile);
		fwrite("\n", 1, 1, writefile);
		fwrite(headertxt, strlen(headertxt), 1, writefile);
		fwrite("\r\n", 2, 1, writefile);
		if(oldRSnr==-1)sprintf(st, "All trajectory is the motion relative to FOV");//FOV	
		else sprintf(st, "All trajectory is the motion relative to Object%d",oldRSnr+1);//Object	
		fwrite(st, strlen(st), 1, writefile);
		fwrite("\r\n", 2, 1, writefile);
//		fwrite("\r\n\r\n", 4, 1, writefile);
		sprintf(st, "%d\r\n", stop+1);
		fwrite(st, strlen(st), 1, writefile);
//write object1 and 2
			for (i=0; i<3;i++)tmplednr[i] = 0;
			i=0; j=0;
			while ((i<18) && (j<3))
			{
			   if (LEDofObject[camsysnr][i] == 0)
			   {
				   tmplednr[j] = i;
				   j++;
			   }
			   i++;
			}
			for (i=0; i<3; i++)
			{	
				sprintf(st, "Object1LED\t%d\t:\t%.5f\t%.5f\t%.5f\r\n", tmplednr[i]+1, led[camsysnr*18+tmplednr[i]][0][0],
					    led[camsysnr*18+tmplednr[i]][0][1], led[camsysnr*18+tmplednr[i]][0][2]);
				fwrite(st, strlen(st), 1, writefile);
			}
			for (i=0; i<3;i++)tmplednr[i] = 0;
			i=0; j=0;
			while ((i<18) && (j<3))
			{
			   if (LEDofObject[camsysnr][i] == 1)
			   {
				   tmplednr[j] = i;
				   j++;
			   }
			   i++;
			}
			for (i=0; i<3; i++)
			{	
				sprintf(st, "Object2LED\t%d\t:\t%.5f\t%.5f\t%.5f\r\n", tmplednr[i]+1, led[camsysnr*18+tmplednr[i]][0][0],
					    led[camsysnr*18+tmplednr[i]][0][1], led[camsysnr*18+tmplednr[i]][0][2]);
				fwrite(st, strlen(st), 1, writefile);
			}	
		fwrite("\r\n", 2, 1, writefile);
	}
	if(strlen(txt)>0)
	{
		fwrite(txt,strlen(txt),1,writefile);
		fwrite("\t\t\tX\tY\tZ\t", 9, 1, writefile);
		sprintf(st, "%d",type2);
		fwrite(st,strlen(st),1,writefile);
		fwrite("\r\n", 2, 1, writefile);
		if ((type == 220))
		{		//extra points
			sprintf(st, "The trajectory is defined by the motion of Object%d\r\n",objnr+1);
			fwrite(st, strlen(st), 1, writefile);
			for (i=0; i<3;i++)
				tmplednr[i] = 0;
			i=0;
			j=0;
			while ((i<18) && (j<3))
			{
			   if (LEDofObject[camsysnr][i] == objnr)
			   {
				   tmplednr[j] = i;
				   j++;
			   }
			   i++;
			}
			for (i=0; i<3; i++)
			{
					sprintf(st, "LED\t%d\t:\t%.5f\t%.5f\t%.5f\r\n", tmplednr[i]+1, led[camsysnr*18+tmplednr[i]][0][0],
						    led[camsysnr*18+tmplednr[i]][0][1], led[camsysnr*18+tmplednr[i]][0][2]);
				fwrite(st, strlen(st), 1, writefile);
			}
		setRSnr(oldRSnr);//
		if(RSnr==-1)sprintf(st, "relative to FOV\r\n");//FOV	
		else sprintf(st, "relative to Object%d\r\n",RSnr+1);//Object	
			fwrite(st, strlen(st), 1, writefile);
			for (i=0; i<3;i++)
				tmplednr[i] = 0;
			i=0;
			j=0;
		if(RSnr==-1){	//FOV	
			while ((i<18) && (j<3))
			{
			   if (LEDofObject[camsysnr][i] == objnr)//060814
			   {
				   tmplednr[j] = i;
				   j++;
			   }
			   i++;
			}
		}else{
			while ((i<18) && (j<3))
			{
			   if (LEDofObject[camsysnr][i] == RSnr)//060814
			   {
				   tmplednr[j] = i;
				   j++;
			   }
			   i++;
			}
		}
			for (i=0; i<3; i++)
			{
					sprintf(st, "LED\t%d\t:\t%.5f\t%.5f\t%.5f\r\n", tmplednr[i]+1, led[camsysnr*18+tmplednr[i]][0][0],
						    led[camsysnr*18+tmplednr[i]][0][1], led[camsysnr*18+tmplednr[i]][0][2]);
				fwrite(st, strlen(st), 1, writefile);
			}
		}else{	//LEDs
			setRSnr(oldRSnr);
//			sprintf(st, "The trajectory is defined by the motion of \r\n");
			sprintf(st, "The trajectory is defined by one LED of \r\n");
			fwrite(st, strlen(st), 1, writefile);
			for (i=0; i<3;i++)
				tmplednr[i] = 0;
			i=0;
			j=0;
			while ((i<18) && (j<3))
			{
			   if (LEDofObject[camsysnr][i] == objnr)
			   {
				   tmplednr[j] = i;
				   j++;
			   }
			   i++;
			}
			for (i=0; i<3; i++)
			{
				sprintf(st, "LED\t%d\t:\t%.5f\t%.5f\t%.5f\r\n", tmplednr[i]+1, led[camsysnr*18+tmplednr[i]][0][0],
				    led[camsysnr*18+tmplednr[i]][0][1], led[camsysnr*18+tmplednr[i]][0][2]);
				fwrite(st, strlen(st), 1, writefile);
			}
		if(RSnr==-1)sprintf(st, "relative to FOV\r\n");//FOV	
		else sprintf(st, "relative to Object%d\r\n",RSnr+1);//Object	
			fwrite(st, strlen(st), 1, writefile);
			for (i=0; i<3;i++)
				tmplednr[i] = 0;
			i=0;
			j=0;
		if(RSnr==-1){	//FOV	
			while ((i<18) && (j<3))
			{
			   if (LEDofObject[camsysnr][i] == objnr)//060814
			   {
				   tmplednr[j] = i;
				   j++;
			   }
			   i++;
			}
		}else{
			while ((i<18) && (j<3))
			{
			   if (LEDofObject[camsysnr][i] == RSnr)//060814
			   {
				   tmplednr[j] = i;
				   j++;
			   }
			   i++;
			}
		}
			for (i=0; i<3; i++)
			{
				sprintf(st, "LED\t%d\t:\t%.5f\t%.5f\t%.5f\r\n", tmplednr[i]+1, led[camsysnr*18+tmplednr[i]][0][0],
				    led[camsysnr*18+tmplednr[i]][0][1], led[camsysnr*18+tmplednr[i]][0][2]);
				fwrite(st, strlen(st), 1, writefile);
			}
		}//if LEDs or extra points
//		sprintf(st, "\r\n");
		if(refnr == -1)sprintf(st, "this point belongs to FOV (absolute coordinate)\r\n"); //belonged coordinate
		if(refnr > -1)sprintf(st, "this point belongs to object %d\t\t\t%.5f\t%.5f\t%.5f\r\n",(refnr+1),src[0],src[1],src[2]); //belonged coordinate
		fwrite(st, strlen(st), 1, writefile);
	}
// change
   if ((type >= 200)&&(type < 220))	//LEDs
   {
	setRSnr(oldRSnr);
		if(RSnr==-1){	//FOV	
			for (i=start; i<=stop; i++)
			{
			vd1 = led[18*camsysnr+type-200][i];
			sprintf(st, "step\t%d\tvalue:\t%f\t%f\t%f\r\n",
				i+1, vd1[0], vd1[1], vd1[2]);
			fwrite(st, strlen(st), 1, writefile);
			}
		}else{
			for (i=start; i<=stop; i++)
			{
			vd1 = led[18*camsysnr+type-200][i];
			calc_animationTrafo(i, objnr);
			fov_trafo[camsysnr].multMatrixVec(vd1, dst);
			sprintf(st, "step\t%d\tvalue:\t%f\t%f\t%f\r\n",
				i+1, dst[0], dst[1], dst[2]);
			fwrite(st, strlen(st), 1, writefile);
			}
		}
//   }else if((type == 222)){	//extra points
   }else{	//extra points
	setRSnr(oldRSnr);

	if(refnr == -1)
	{
		if(RSnr==-1){
			setRSnr(refnr);
			for (i=start; i<=stop; i++)
			{
				calc_animationTrafo(i, objnr);
				animation_trafo[camsysnr].multMatrixVec(src, vd1);
				md = fov_trafo[camsysnr].inverse();
				md.multMatrixVec(vd1, dst);
				sprintf(st, "step\t%d\tvalue:\t%f\t%f\t%f\r\n",
					i+1, dst[0], dst[1], dst[2]);
				fwrite(st, strlen(st), 1, writefile);
			}
		}else if(RSnr == objnr){
			for (i=start; i<=stop; i++)
			{
				sprintf(st, "step\t%d\tvalue:\t%f\t%f\t%f\r\n",
					i+1, src[0], src[1], src[2]);
				fwrite(st, strlen(st), 1, writefile);
			}
		}else{
			for (i=start; i<=stop; i++)
			{
				calc_animationTrafo(i, objnr);
				animation_trafo[camsysnr].multMatrixVec(src, dst);
				sprintf(st, "step\t%d\tvalue:\t%f\t%f\t%f\r\n",
					i+1, dst[0], dst[1], dst[2]);
				fwrite(st, strlen(st), 1, writefile);
			}
		}
	}
	else if(refnr > -1)
	{
         calc_animationTrafoRefSys(&src_leds[0], 0, 1,refnr);
         m1 = mRefRefSys[0].inverse();
         m1.multMatrixVec(src, tmp1);
		if(RSnr==-1){
			setRSnr(refnr);
			for (i=start; i<=stop; i++)
			{
				calc_animationTrafo(i, objnr);
				animation_trafo[camsysnr].multMatrixVec(tmp1, vd1);
				md = fov_trafo[camsysnr].inverse();
				md.multMatrixVec(vd1, dst);
				sprintf(st, "step\t%d\tvalue:\t%f\t%f\t%f\r\n",
					i+1, dst[0], dst[1], dst[2]);
				fwrite(st, strlen(st), 1, writefile);
			}
		}else if(RSnr == objnr){
			for (i=start; i<=stop; i++)
			{
				sprintf(st, "step\t%d\tvalue:\t%f\t%f\t%f\r\n",
					i+1, tmp1[0], tmp1[1], tmp1[2]);
				fwrite(st, strlen(st), 1, writefile);
			}
		}else{
			for (i=start; i<=stop; i++)
			{
				calc_animationTrafo(i, objnr);
				animation_trafo[camsysnr].multMatrixVec(tmp1, dst);
				sprintf(st, "step\t%d\tvalue:\t%f\t%f\t%f\r\n",
					i+1, dst[0], dst[1], dst[2]);
				fwrite(st, strlen(st), 1, writefile);
			}
		}
	}
	}
	fwrite("\r\n\r\n", 2 , 1, writefile);

	fclose(writefile);
	setRSnr(oldRSnr);
}

//import trajectory
char Motion::read_trajfile(char * filename)
{
	FILE    *readfile;
	char    st[256];
	char    filename2[256],file[64];
	char    header[256];
	char    comment[256];
	char    dammy[256];
	SbVec3f      traj[18];
	int i,j,k,ii;
	long istop;

	printf("trajectory file -> %s\n",filename);
    if ((readfile = fopen(filename, "rb")) == NULL)
    {
		sprintf(st, "Error opening file %s", filename);
        fl_message(st);
        return 0;
    }
   fgets(filename2,256,readfile);
   strcpy(file,filename2);
   fgets(header,256,readfile);   //printf("%s\n",header);
   fgets(comment,256,readfile);  //printf("%s\n",comment);
   fgets(dammy,256,readfile); istop=atol(dammy); //printf("istop %d\n",istop);
   imp_steps = istop;            //printf("file number %d\n",imp_steps);

   for(i=0;i<18;i++)ImpRefLed[i].setValue(0.0, 0.0, 0.0);//initialize

   for(i=0;i<3;i++){
         fscanf(readfile,"%s",dammy);	//printf("D:%s\n",dammy);
         fscanf(readfile,"%s",dammy);	//printf("D:%s\n",dammy);
		 ii=atoi(dammy);
         ImpObj[i]=ii-1;
         fscanf(readfile,"%s",dammy);	//printf("D:%s\n",dammy);
	  for(j=0;j<3;j++){
         fscanf(readfile,"%s",dammy);  ImpRefLed[ii-1][j]=atof(dammy);
	  }
   }
   for(i=0;i<3;i++){
         fscanf(readfile,"%s",dammy);	//printf("D:%s\n",dammy);
         fscanf(readfile,"%s",dammy);	//printf("D:%s\n",dammy);
		 ii=atoi(dammy);
         ImpObj[i+3]=ii-1;
         fscanf(readfile,"%s",dammy);	//printf("D:%s\n",dammy);
	  for(j=0;j<3;j++){
         fscanf(readfile,"%s",dammy);  ImpRefLed[ii-1][j]=atof(dammy);
	  }
   }

   fgets(dammy,256,readfile);
   fgets(dammy,256,readfile);

   impTraj = new SoMFVec3f[18];

   k=0;
   while(1){
   fgets(dammy,40,readfile);     //printf("D:%s\n",dammy);
   strcpy(trajcomm[k], dammy);   //printf("comm:%s\n",trajcomm[k]);
// printf("trajcomm[]:%s\n",trajcomm[k]);
   if(feof( readfile )) break;
 for(i=0;i<9;i++){
	 fgets(dammy,256,readfile);
// printf("D:%s\n",dammy);
 }
   if(feof( readfile )) break;
   for(i=0;i<istop;i++){
      for(j=0;j<3;j++) fscanf(readfile,"%s",dammy);
      for(j=0;j<3;j++){
         fscanf(readfile,"%s",dammy);  traj[k][j]=atof(dammy);
	  }
		 impTraj[k].set1Value(i, traj[k]);
//   if(i==(0)) printf("stt:%f\t%f\t%f\n",traj[k][0],traj[k][1],traj[k][2]);
//   if(i==(istop-1)) printf("end:%f\t%f\t%f\n",traj[k][0],traj[k][1],traj[k][2]);
   }
   fgets(dammy,256,readfile);
   fgets(dammy,256,readfile);
   k++;
   }
    fclose(readfile);
    numtraj=k;
	return 1;
}
void Motion::load_trajfile(const char *filename)
{
   int j;
   char st[256], cmd[256];

   sprintf(st, "%s", filename);
   j = 0;
   while (st[j] != ' ')
     j++;
   st[j] = '\0';
   sprintf(cmd, "%s/%s/%s", pref->getdatapath(),
           scnfile->scanmov.path, st);
   if (read_trajfile(cmd) == 0)
   {
	   return;
   }
}

//calcuration matrix for imported trajectories.
void Motion::calc_animationTrafoImp(double dStep, int objnr, int refnr)
{
   SdMatrix m1, m2;
   SbVec3d tmpled[6];
   int camsysnr, i, j;
   SbVec3d testvec, tmpvec;
   SbVec3d ledvec[18];
   long step;

   step = (long)dStep;
   testvec.setValue(-120.0, -120.0, -120.0);
   for (camsysnr=0; camsysnr<numCamSystems; camsysnr++)
   {
	   for (i=0; i<18; i++)
	   {
		   ledvec[i] = ImpRefLed[i];
	   }
		 	  mRefImp[camsysnr].makeIdentity();
		      mUVWImp[camsysnr].makeIdentity();

	   if((ledvec[0] != testvec) &&
		  (ledvec[1] != testvec) &&
		  (ledvec[2] != testvec) &&
		  (ledvec[3] != testvec) &&
		  (ledvec[4] != testvec) &&
		  (ledvec[5] != testvec))
	   {

		   i=0;
		   j=0;
		   while ((i<18) && (j<3))
		   {
			   if (LEDofObject[camsysnr][i] == refnr)
			   {
				   tmpled[3+j] = ledvec[i];
				   j++;
			   }
			   i++;
		   }
		   tmpled[0].setValue(0.0, 0.0, 0.0);
		   tmpled[1].setValue(1.0, 0.0, 0.0);
		   tmpled[2].setValue(0.0, 1.0, 0.0);
		   BaseTrans(&tmpled[0], &tmpled[3], m2);
		   i=0;
		   j=0;
		   while ((i<18) && (j<3))
		   {
			   if (LEDofObject[camsysnr][i] == objnr)
			   {
				   tmpled[3+j] = ledvec[i];
//printf("imp objnr %f\t%f\t%f\n",tmpled[3+j][0],tmpled[3+j][1],tmpled[3+j][2]);
				   j++;
			   }
			   i++;
		   }
		   tmpled[0].setValue(0.0, 0.0, 0.0);
		   tmpled[1].setValue(1.0, 0.0, 0.0);
		   tmpled[2].setValue(0.0, 1.0, 0.0);
		   BaseTrans(&tmpled[0], &tmpled[3], m1);
		 	  mRefImp[camsysnr] = m2;
		      mUVWImp[camsysnr] = m1;
	   }
	   else
	   {
		 	  mRefImp[camsysnr].makeIdentity();
		      mUVWImp[camsysnr].makeIdentity();
	   }
   }
}

void Motion::load_Refmvmfile(const char *filename)
{
//   int j;
   char st[256], cmd[256];

   sprintf(st, "%s", filename);
   sprintf(cmd, "%s", filename);
/*   j = 0;
   while (st[j] != ' ')
     j++;
   st[j] = '\0';
   sprintf(cmd, "%s/%s/%s", pref->getdatapath(),
           scnfile->scanmov.path, st);*/
   printf("Load ref file: %s\n",st);
   if (read_Refmvmfile(cmd) == 0)
   {
	   return;
   }
}
char Motion::read_Refmvmfile(char * filename)
{
	FILE    *readfile;
	char    st[256];
	unsigned char c, d, e, f;
	long    fileSize, pos;
	long    numpos;
	int     i, j, k;
	int     imap,ibit,irest;
	double  dimap,dmap;
	SbVec3d xyz[18];
	SbVec3f intens[18];

	numpos=0;
    if ((readfile = fopen(filename, "rb")) == NULL)
    {
		sprintf(st, "Error opening file %s", filename);
        fl_message(st);
        return 0;
    }
// read header
	fseek(readfile, 0, SEEK_SET);
    fread(comment, 256, 1, readfile);
	comment[255] = '\0';

    fseek(readfile, 0, SEEK_END);
    fileSize = ftell(readfile);

	numCameras = 0;
	fseek(readfile, 1024, SEEK_SET);
	if (fileSize >= 8704)
		k = 30;
	else
		k = (fileSize-1024)/256;
	for (i=0; i<k; i++)
	{
		fread(&c, 1, 1, readfile);
		if (c > numCameras)
			numCameras = c;
		for (j=1; j<256; j++)
			fread(&c, 1, 1, readfile);
	}
    numpos = (fileSize - 1024) / (numCameras*256);
	numCamSystems = numCameras/3;

	fseek(readfile, 1024, SEEK_SET);

    for (pos=0; pos<numpos; pos++)
    {
		for (i=0; i<18; i++)
		{
			xyz[i].setValue(-120.0, -120.0, -120.0);
			intens[i].setValue(0, 0, 0);
		}
		for (i=0; i<numCameras; i++)
		{
			fread(&c, 1, 1, readfile);		//camera number(xyz)
			fread(&c, 1, 1, readfile);		//data length(bit)
			ibit = c;
			fread(&c, 1, 1, readfile);		//data number(step)1
			fread(&c, 1, 1, readfile);		//data number(step)2
			fread(&c, 1, 1, readfile);		//data number(step)3
			fread(&c, 1, 1, readfile);		//data number(step)4
			fread(&c, 1, 1, readfile);		//mapping(devided number)1
			fread(&d, 1, 1, readfile);		//mapping(devided number)2
			imap = 256 * c;
			imap +=d;
			if(ibit==32)			//for 32bit recording data
			{
				   dimap = float(imap) * 65536.0;
				   dmap  = 32768.0 / float(imap);
				   irest = 98;
			}else if(ibit==16)		//for 16bit recording data
			{
				   dimap = float(imap);
				   dmap  = 32768.0 / float(imap);
				   irest = 62;
			}else if(ibit==0)		//for 16bit sample data
			{
				   dimap = 320.0;
				   dmap  = 102.4;
				   irest = 62;
			}

			for(j=0; j<18; j++)
			{
				if(ibit==32)
				{
			   fread(&c, 1, 1, readfile);
			   fread(&d, 1, 1, readfile);
			   fread(&e, 1, 1, readfile);
			   fread(&f, 1, 1, readfile);
			   if ((c>0)||(d>0)||(e>0)||(f>0))
			   {
					xyz[j][i%3] = 256 * 256* 256 * float(c);
					xyz[j][i%3] += 256 * 256 * float(d);
					xyz[j][i%3] += 256 * float(e);
					xyz[j][i%3] += float(f);
					xyz[j][i%3] /= dimap;
					xyz[j][i%3] -= dmap;
			   }
				}else if((ibit==16)||(ibit==0))
				{
			   fread(&c, 1, 1, readfile);
			   fread(&d, 1, 1, readfile);
			     if ((c>0)||(d>0))
				 {
					xyz[j][i%3] = 256 * c;
					xyz[j][i%3] += d;
					xyz[j][i%3] /= dimap;
					xyz[j][i%3] -= dmap;
				 }
				}
			}
			for(j=0; j<18; j++)
			{
			   fread(&c, 1, 1, readfile);
			   intens[j][i%3] = float(c);
			}
			for(j=irest; j<256; j++)
			{
			   fread(&c, 1, 1, readfile);
			}
			if ((i%3) == 2)
			{
				for (j=0; j<18; j++)
				{
				if(pos==0)refxyz[j]=xyz[j];
				if(pos==0)printf("ref(%2d): %9.3f\t%9.3f\t%9.3f\n",(j+1),refxyz[j][0],refxyz[j][1],refxyz[j][2]);
				}
			}
		}
	}
    fclose(readfile);
	return 1;
}

void Motion::calc_animationTrafo4(double dStep, int objnr, int refnr)
{
   SdMatrix m1, m2;
   SbVec3d tmpled[6];
   int camsysnr, i, j;
   SbVec3d testvec, tmpvec;
   SbVec3d ledvec[18];
   long step;

   step = (long)dStep;
   testvec.setValue(-120.0, -120.0, -120.0);
   for (camsysnr=0; camsysnr<numCamSystems; camsysnr++)
   {
	   for (i=0; i<18; i++)
	   {
		   ledvec[i] = led[camsysnr*18+i][step];
	   }
		 	  mRef2[camsysnr].makeIdentity();
		      mUVW2[camsysnr].makeIdentity();

	   if((ledvec[0] != testvec) &&
		  (ledvec[1] != testvec) &&
		  (ledvec[2] != testvec) &&
		  (ledvec[3] != testvec) &&
		  (ledvec[4] != testvec) &&
		  (ledvec[5] != testvec))
	   {

		   i=0;
		   j=0;
		   while ((i<18) && (j<3))
		   {
			   if (LEDofObject[camsysnr][i] == refnr)
			   {
				   tmpled[3+j] = ledvec[i];
				   j++;
			   }
			   i++;
		   }
		   tmpled[0].setValue(0.0, 0.0, 0.0);
		   tmpled[1].setValue(1.0, 0.0, 0.0);
		   tmpled[2].setValue(0.0, 1.0, 0.0);
		   BaseTrans(&tmpled[0], &tmpled[3], m2);
		   i=0;
		   j=0;
		   while ((i<18) && (j<3))
		   {
			   if (LEDofObject[camsysnr][i] == objnr)
			   {
				   tmpled[3+j] = ledvec[i];
				   j++;
			   }
			   i++;
		   }
		   tmpled[0].setValue(0.0, 0.0, 0.0);
		   tmpled[1].setValue(1.0, 0.0, 0.0);
		   tmpled[2].setValue(0.0, 1.0, 0.0);
		   BaseTrans(&tmpled[0], &tmpled[3], m1);
		 	  mRef2[camsysnr] = m2;
		      mUVW2[camsysnr] = m1;
	   }
	   else
	   {
		 	  mRef2[camsysnr].makeIdentity();
		      mUVW2[camsysnr].makeIdentity();
	   }
   }
}

void Motion::setLedagain()
{
	int i,pos;
	    for (pos=0; pos<num_steps; pos++)
    {
	    for (i=0; i<3; i++)
	    {
	led[13].set1Value(pos, oldled[13][pos][0], oldled[13][pos][1], oldled[13][pos][2]);
	led[15].set1Value(pos, oldled[15][pos][0], oldled[15][pos][1], oldled[15][pos][2]);
	led[17].set1Value(pos, oldled[17][pos][0], oldled[17][pos][1], oldled[17][pos][2]);
	intensity[13].set1Value(pos, oldintensity[13][pos][0], oldintensity[13][pos][1], oldintensity[13][pos][2]);
	intensity[15].set1Value(pos, oldintensity[15][pos][0], oldintensity[15][pos][1], oldintensity[15][pos][2]);
	intensity[17].set1Value(pos, oldintensity[17][pos][0], oldintensity[17][pos][1], oldintensity[17][pos][2]);
		}
	}
}

void Motion::load_RefSysmvmfile(const char *filename)
{
//   int j;
   char st[256], cmd[256];

   sprintf(st, "%s", filename);
   sprintf(cmd, "%s", filename);
/*   j = 0;
   while (st[j] != ' ')
     j++;
   st[j] = '\0';
   sprintf(cmd, "%s/%s/%s", pref->getdatapath(),
           scnfile->scanmov.path, st);*/
   printf("Load ref sys file: %s\n",st);
   if (read_RefSysmvmfile(cmd) == 0)
   {
	   return;
   }
}
char Motion::read_RefSysmvmfile(char * filename)
{
	FILE    *readfile;
	char    st[256];
	unsigned char c, d, e, f;
	long    fileSize, pos;
	long    numpos;
	int     i, j, k;
	int     imap,ibit,irest;
	double  dimap,dmap;
	SbVec3d xyz[18];
	SbVec3f intens[18];

	numpos=0;
    if ((readfile = fopen(filename, "rb")) == NULL)
    {
		sprintf(st, "Error opening file %s", filename);
        fl_message(st);
        return 0;
    }
// read header
	fseek(readfile, 0, SEEK_SET);
    fread(comment, 256, 1, readfile);
	comment[255] = '\0';

    fseek(readfile, 0, SEEK_END);
    fileSize = ftell(readfile);

	numCameras = 0;
	fseek(readfile, 1024, SEEK_SET);
	if (fileSize >= 8704)
		k = 30;
	else
		k = (fileSize-1024)/256;
	for (i=0; i<k; i++)
	{
		fread(&c, 1, 1, readfile);
		if (c > numCameras)
			numCameras = c;
		for (j=1; j<256; j++)
			fread(&c, 1, 1, readfile);
	}
    numpos = (fileSize - 1024) / (numCameras*256);
	numCamSystems = numCameras/3;

	fseek(readfile, 1024, SEEK_SET);

    for (pos=0; pos<numpos; pos++)
    {
		for (i=0; i<18; i++)
		{
			xyz[i].setValue(-120.0, -120.0, -120.0);
			intens[i].setValue(0, 0, 0);
		}
		for (i=0; i<numCameras; i++)
		{
			fread(&c, 1, 1, readfile);		//camera number(xyz)
			fread(&c, 1, 1, readfile);		//data length(bit)
			ibit = c;
			fread(&c, 1, 1, readfile);		//data number(step)1
			fread(&c, 1, 1, readfile);		//data number(step)2
			fread(&c, 1, 1, readfile);		//data number(step)3
			fread(&c, 1, 1, readfile);		//data number(step)4
			fread(&c, 1, 1, readfile);		//mapping(devided number)1
			fread(&d, 1, 1, readfile);		//mapping(devided number)2
			imap = 256 * c;
			imap +=d;
			if(ibit==32)			//for 32bit recording data
			{
				   dimap = float(imap) * 65536.0;
				   dmap  = 32768.0 / float(imap);
				   irest = 98;
			}else if(ibit==16)		//for 16bit recording data
			{
				   dimap = float(imap);
				   dmap  = 32768.0 / float(imap);
				   irest = 62;
			}else if(ibit==0)		//for 16bit sample data
			{
				   dimap = 320.0;
				   dmap  = 102.4;
				   irest = 62;
			}

			for(j=0; j<18; j++)
			{
				if(ibit==32)
				{
			   fread(&c, 1, 1, readfile);
			   fread(&d, 1, 1, readfile);
			   fread(&e, 1, 1, readfile);
			   fread(&f, 1, 1, readfile);
			   if ((c>0)||(d>0)||(e>0)||(f>0))
			   {
					xyz[j][i%3] = 256 * 256* 256 * float(c);
					xyz[j][i%3] += 256 * 256 * float(d);
					xyz[j][i%3] += 256 * float(e);
					xyz[j][i%3] += float(f);
					xyz[j][i%3] /= dimap;
					xyz[j][i%3] -= dmap;
			   }
				}else if((ibit==16)||(ibit==0))
				{
			   fread(&c, 1, 1, readfile);
			   fread(&d, 1, 1, readfile);
			     if ((c>0)||(d>0))
				 {
					xyz[j][i%3] = 256 * c;
					xyz[j][i%3] += d;
					xyz[j][i%3] /= dimap;
					xyz[j][i%3] -= dmap;
				 }
				}
			}
			for(j=0; j<18; j++)
			{
			   fread(&c, 1, 1, readfile);
			   intens[j][i%3] = float(c);
			}
			for(j=irest; j<256; j++)
			{
			   fread(&c, 1, 1, readfile);
			}
			if ((i%3) == 2)
			{
				for (j=0; j<18; j++)
				{
				if(pos==0)refsysxyz[j]=xyz[j];
				if(pos==0)printf("refsys xyz(%2d): %9.3f\t%9.3f\t%9.3f\n",(j+1),refsysxyz[j][0],refsysxyz[j][1],refsysxyz[j][2]);
				}
			}
		}
	}
    fclose(readfile);
	return 1;
}

//calcuration matrix for reference system.
void Motion::calc_animationTrafoRefSys(SbVec3d *src_leds, double dStep, int objnr, int refnr)
{
   SdMatrix m1, m2;
   SbVec3d tmpled[6];
   int camsysnr, i, j;
   SbVec3d testvec, tmpvec;
   SbVec3d ledvec[18];
   long step;

   step = (long)dStep;//integer
   testvec.setValue(-120.0, -120.0, -120.0);
   for (camsysnr=0; camsysnr<numCamSystems; camsysnr++)
   {
	   for (i=0; i<18; i++)//Each Led
	   {
//		   ledvec[i] = refsysxyz[i];
		   ledvec[i] = src_leds[i]; //Copy src_leds to ledvec[i]
	   }
//	   printf("src_leds[6] =%f %f %f\n",ledvec[6][0],ledvec[6][1],ledvec[6][2]);

		 	  mRefRefSys[camsysnr].makeIdentity(); // initialize
		      mUVWRefSys[camsysnr].makeIdentity();

	   if((ledvec[0] != testvec) &&//all six LEDs have value
		  (ledvec[1] != testvec) &&
		  (ledvec[2] != testvec) &&
		  (ledvec[3] != testvec) &&
		  (ledvec[4] != testvec) &&
		  (ledvec[5] != testvec))
	   {

		   i=0;
		   j=0;
		   while ((i<18) && (j<3))
		   {
			   if (LEDofObject[camsysnr][i] == refnr)//i = each LED
			   {
				   tmpled[3+j] = ledvec[i];
				   j++;
			   }
			   i++;
		   }
		   tmpled[0].setValue(0.0, 0.0, 0.0);
		   tmpled[1].setValue(1.0, 0.0, 0.0);
		   tmpled[2].setValue(0.0, 1.0, 0.0);
		   BaseTrans(&tmpled[0], &tmpled[3], m2);
		   i=0;
		   j=0;
		   while ((i<18) && (j<3))
		   {
			   if (LEDofObject[camsysnr][i] == objnr)
			   {
				   tmpled[3+j] = ledvec[i];
				   j++;
			   }
			   i++;
		   }
		   tmpled[0].setValue(0.0, 0.0, 0.0);
		   tmpled[1].setValue(1.0, 0.0, 0.0);
		   tmpled[2].setValue(0.0, 1.0, 0.0);
		   BaseTrans(&tmpled[0], &tmpled[3], m1);
		 	  mRefRefSys[camsysnr] = m2;
		      mUVWRefSys[camsysnr] = m1;
	   }
	   else
	   {
		 	  mRefRefSys[camsysnr].makeIdentity();
		      mUVWRefSys[camsysnr].makeIdentity();
	   }
   }
}

//appearance of the axis of target frame
void Motion::calc_animationTrafoObj(double dStep, int ledobjnr)
{
   SdMatrix m1;
   SbVec3d tmpled[6];
   int camsysnr, i, j;
   SbVec3d testvec;
   SbVec3d ledvec[18];
   long step;

   step = (long)dStep;
   testvec.setValue(-120.0, -120.0, -120.0);
   for (camsysnr=0; camsysnr<numCamSystems; camsysnr++)
   {
	   for (i=0; i<18; i++)//each LED
	   {
		   ledvec[i] = led[camsysnr*18+i][step];
	   }
		 	  mObj[camsysnr].makeIdentity();//initialize

	   if((ledvec[0] != testvec) &&
		  (ledvec[1] != testvec) &&
		  (ledvec[2] != testvec) &&
		  (ledvec[3] != testvec) &&
		  (ledvec[4] != testvec) &&
		  (ledvec[5] != testvec))
	   {
		   i=0;
		   j=0;
		   while ((i<18) && (j<3))
		   {
			   if (LEDofObject[camsysnr][i] == ledobjnr)
			   {
				   tmpled[3+j] = ledvec[i];
				   j++;
			   }
			   i++;
		   }
		   tmpled[0].setValue(0.0, 0.0, 0.0);
		   tmpled[1].setValue(1.0, 0.0, 0.0);
		   tmpled[2].setValue(0.0, 1.0, 0.0);
		   BaseTrans(&tmpled[0], &tmpled[3], m1);
		      mObj[camsysnr] = m1;
	   }
	   else
	   {
		 	  mObj[camsysnr].makeIdentity();
	   }
   }
}

void Motion::calc_animationTrafoReal(double dStep, int objnr)
{
   SdMatrix m1, m2;
   SbVec3d tmpled[6];
   int camsysnr, i, j;
   SbVec3d testvec, tmpvec;
   SbVec3d ledvec[18];
   long step;
   int tmpRSnr1, tmpRSnr2;

   if (RSchanged > 0)	   //At first a matrix is calcurated based upon step 0.
   {
	   RSchanged = 0;
	   calc_animationTrafoReal(0.0, objnr);
   }

   step = (long)dStep;
   if (num_steps == 0)
   {
      for (camsysnr=0; camsysnr<numCamSystems; camsysnr++)
	  {
	 	  mRefR[camsysnr].makeIdentity();
	      mUVWR[camsysnr].makeIdentity();
	      fov_trafoR[camsysnr].makeIdentity();
	      animation_trafoR[camsysnr].makeIdentity();
	  }
	  return;
   }

   testvec.setValue(-120.0, -120.0, -120.0);
   for (camsysnr=0; camsysnr<numCamSystems; camsysnr++)
   {
	   if (pref->camSysOverlap[camsysnr] > 0)
	   {
		   m1.makeIdentity();
		   FixedXYZ(m1, pref->camSysRot[camsysnr][0]/180.0*3.14159,
			  pref->camSysRot[camsysnr][1]/180.0*3.14159,
			  pref->camSysRot[camsysnr][2]/180.0*3.14159);
		   m1[0][3] = pref->camSysTrans[camsysnr][0];
		   m1[1][3] = pref->camSysTrans[camsysnr][1];
		   m1[2][3] = pref->camSysTrans[camsysnr][2];
		   m2 = m1.inverse();
	   }
	   for (i=0; i<18; i++)
	   {
		   ledvec[i] = led[camsysnr*18+i][step];
		   if ((camsysnr == 0) && (ledvec[i] == testvec))
		   {
			   j = 1;
			   while (j<numCamSystems)
			   {
				   if (pref->camSysOverlap[j] > 0)
				   {
					   tmpvec = led[j*18+i][step];
					   if (tmpvec != testvec)
					   {
						   m1.makeIdentity();
						   FixedXYZ(m1, pref->camSysRot[j][0]/180.0*3.14159,
							  pref->camSysRot[j][1]/180.0*3.14159,
							  pref->camSysRot[j][2]/180.0*3.14159);
						   m1[0][3] = pref->camSysTrans[j][0];
						   m1[1][3] = pref->camSysTrans[j][1];
						   m1[2][3] = pref->camSysTrans[j][2];
						   m1.multMatrixVec(tmpvec, ledvec[i]);
						   if (step == 0)
						      led[camsysnr*18+i].set1Value(step, ledvec[i]);
						   j = numCamSystems;
					   }
				   }
				   j++;
			   }
		   }
		   if ((camsysnr > 0) && (ledvec[i] == testvec) && (pref->camSysOverlap[camsysnr] > 0))
		   {
			   tmpvec = led[i][step];
			   if (tmpvec != testvec)
			   {
				   m2.multMatrixVec(tmpvec, ledvec[i]);
				   if (step == 0)
				      led[camsysnr*18+i].set1Value(step, ledvec[i]);
			   }
		   }
	   }
	   if (Speed[camsysnr] == 2)
	   {
		   if ((dStep-step)>=0.5)
		   {
			   for (i=0; i<6; i++)
				   ledvec[i] = ledvec[i+9];
		   }
	   }
	   if (Speed[camsysnr] == 3)
	   {
		   if (((dStep-step)>=(1.0/3.0)) && ((dStep-step)<(2.0/3.0)))
		   {
			   for (i=0; i<6; i++)
				   ledvec[i] = ledvec[i+6];
		   }
		   if ((dStep-step)>=(2.0/3.0))
		   {
			   for (i=0; i<6; i++)
				   ledvec[i] = ledvec[i+12];
		   }
	   }
	   if((ledvec[0] != testvec) &&  // aendern! (if change)
		  (ledvec[1] != testvec) &&
		  (ledvec[2] != testvec) &&
		  (ledvec[3] != testvec) &&
		  (ledvec[4] != testvec) &&
		  (ledvec[5] != testvec))
	   {
		   if (RSnr == -1)  // Referenz
			   tmpRSnr1 = 0;
		   else
			   tmpRSnr1 = RSnr;
		   if ((objnr >= 0) && (objnr != tmpRSnr1)) // Objekt
			   tmpRSnr2 = objnr;
		   else
		   {
			   if (RSnr == 1)
				   tmpRSnr2 = 1;
//				   tmpRSnr2 = 0;
			   else
				   tmpRSnr2 = 0;
//				   tmpRSnr2 = 1;
		   }
		   i=0;
		   j=0;
		   while ((i<18) && (j<3))
		   {
			   if (LEDofObject[camsysnr][i] == tmpRSnr1)
			   {
				   tmpled[3+j] = ledvec[i];
				   j++;
			   }
			   i++;
		   }
		   tmpled[0].setValue(0.0, 0.0, 0.0);
		   tmpled[1].setValue(1.0, 0.0, 0.0);
		   tmpled[2].setValue(0.0, 1.0, 0.0);
		   BaseTrans(&tmpled[0], &tmpled[3], m2);
		   i=0;
		   j=0;
		   while ((i<18) && (j<3))
		   {
			   if (LEDofObject[camsysnr][i] == tmpRSnr2)
			   {
				   tmpled[3+j] = ledvec[i];
				   j++;
			   }
			   i++;
		   }
		   tmpled[0].setValue(0.0, 0.0, 0.0);
		   tmpled[1].setValue(1.0, 0.0, 0.0);
		   tmpled[2].setValue(0.0, 1.0, 0.0);
		   BaseTrans(&tmpled[0], &tmpled[3], m1);
		   if (step == 0)
		   {
		 	  mRefR[camsysnr] = m2.inverse();
		      fov_trafoR[camsysnr].makeIdentity();
		      mUVWR[camsysnr] = m1;
		      animation_trafoR[camsysnr].makeIdentity();
		   }
		   else
		   {
		      fov_trafoR[camsysnr] = mRefR[camsysnr];
		      fov_trafoR[camsysnr] *= m2;
		      animation_trafoR[camsysnr] = fov_trafoR[camsysnr];
		      animation_trafoR[camsysnr] *= m1.inverse();
		      animation_trafoR[camsysnr] *= mUVWR[camsysnr];
		   }
	   }
	   else
	   {
		   if (step == 0)
		   {
		 	  mRefR[camsysnr].makeIdentity();
		      mUVWR[camsysnr].makeIdentity();
		      fov_trafoR[camsysnr].makeIdentity();
		      animation_trafoR[camsysnr].makeIdentity();
		   }
		   else
		   {
		      fov_trafoR[camsysnr].makeIdentity();
		      animation_trafoR[camsysnr].makeIdentity();
		   }
	   }
   }
}
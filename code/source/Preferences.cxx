#include "Preferences.h"

Preferences::Preferences()
{
	int i,j;

   sprintf(m_acStdModelPath, "/");
   m_cRecTiefe = 1;
   m_cModelRekMode = 0;
   SegObjColor.setValue(0.0, 0.0, 1.0);
   SegSelObjColor.setValue(1.0, 0.0, 0.0);
   SegPointColor.setValue(1.0, 1.0, 0.0);
   SegNumberColor.setValue(0.0, 1.0, 0.0);
   SegViewerWidth = SegViewerHeight = 512;
   SegViewerX = SegViewerY = -1;

   LEDerror = 0;             //Falls Intensitaet der LED <= LEDerror, liegt ein Fehler vor
                               /*If intensity of the LED <= LEDerror, there is an error*/
   LEDwarningRedRed = 3;
   LEDwarningRed = 128;      //Warnung ab diesem Wert (und kleiner): LED wird fast nicht gesehen
                             /*Warning from this value (and smaller): LED is almost not seen*/
   LEDwarningYellow = 192;   //LED befindet sich unterhalb dieses Wertes ausserhalb des optimalen Bereiches
                              /*LED located below this value outside of the optimum range*/

	  for (i=0; i<6; i++)
	  {
		  LEDdrawstyle[i]= 0;
	  }
	  for (i=6; i<18; i++)
	  {
		  LEDdrawstyle[i]= 1;
	  }   
//LED&OBJECT char
	  		for (i=0; i<18; i++)
		{
//			LEDofObjectP[i] = i/3;
						LEDofObjectP[i] = 0;

		}
		for (i=0; i<6; i++)
		{
			if ((i%2) == 0)
			{
				ObjectsAndReferencesP[i] = -1;
			}
			else
			{
				ObjectsAndReferencesP[i] = i-1;
			}
		}
//LED&OBJECT char
//Store Reference LED data
		for (j=0; j<5; j++){
		for (i=0; i<20; i++)
		RefLed[j][i].setValue(0.0, 0.0, 0.0);
		}
//Store Reference LED data
//	  sprintf(datapath, "/database/data");
//      sprintf(scanpath, "/database/scans");
	  sprintf(datapath, "C:/database");
      sprintf(scanpath, "C:/database");
#ifdef WIN32
      sprintf(imgcopy, "C:\\Programme\\IrfanView\\i_view32.exe");
      sprintf(extracode, "/convert=");
#else
      sprintf(imgcopy, "imgcopy -ctl");
      sprintf(extracode, "");
#endif
	  ObjectWindowWidth = ObjectWindowHeight = 512;
	  ObjectWindowX = ObjectWindowY = -1;
	  ObjectWindowColor.setValue(0.019, 0.015, 0.356);
	  LEDViewerWidth = LEDViewerHeight = 512;
	  LEDViewerX = LEDViewerY = -1;
	  LEDViewerColor.setValue(0.0, 0.0, 0.0);
	  sprintf(socketPort, "1717");
	  sprintf(socketHost, "localhost");
#ifdef WIN32
	  sprintf(optisController, "C:\\OPTIS\\Controller\\controller.exe");
	  sprintf(optisTempDir, "C:/OPTIS/Controller/temp");
#else
	  sprintf(optisController, "/OPTIS/Controller/controller");
	  sprintf(optisTempDir, "/OPTIS/Controller/temp");
#endif
	  numCameras = 3;
	  for (i=0; i<10; i++)
	  {
		  camSysRot[i].setValue(0.0, 0.0, 0.0);
		  camSysTrans[i].setValue(250.0 * i, 0.0, 0.0);
		  camSysOverlap[i] = 0;
	  }
	  dateType = 1;
      stack_num_slices = 14;
      stack_slice_thickness = 2.0;
      stack_slice_gap = 0.0;
      stack_fov_width = 130.0;
      stack_fov_height = 130.0;
      stack_img_width = 256;
      stack_img_height = 256;
      stack_offc_ap = 0.0;
      stack_offc_lr = 0.0;
      stack_offc_cc = 0.0;
      stack_ang_ap = 0.0;
      stack_ang_lr = 0.0;
      stack_ang_cc = 0.0;
      stack_mult_offc_ap = -1.0;
      stack_mult_offc_lr = -1.0;
      stack_mult_offc_cc = 1.0;
      stack_mult_ang_ap = -1.0;
      stack_mult_ang_lr = -1.0;
      stack_mult_ang_cc = 1.0;
      MRI_num_stacks = 2;
      sprintf(MRI_img_base, "image_");
      sprintf(MRI_raw_images, "1-14, 15-28");
      MRI_first = 1;
}

void Preferences::read_inifile()
{
   FILE    *readfile;
   int     namesize, i;

   if ((readfile = fopen("OPTIS.ini", "rb")) != NULL)
   {
      fread((char*)&namesize, sizeof(int), 1, readfile);
      fread(&datapath[0], (namesize+1), 1, readfile);
      fread((char*)&namesize, sizeof(int), 1, readfile);
      fread(&scanpath[0], (namesize+1), 1, readfile);
      fread((char*)&namesize, sizeof(int), 1, readfile);
      fread(&m_acStdModelPath[0], (namesize+1), 1, readfile);
      fread((char*)&namesize, sizeof(int), 1, readfile);
      fread(&imgcopy[0], (namesize+1), 1, readfile);
      fread((char*)&namesize, sizeof(int), 1, readfile);
      fread(&extracode[0], (namesize+1), 1, readfile);
      fread((char*)&ObjectWindowWidth, sizeof(int), 1, readfile);
      fread((char*)&ObjectWindowHeight, sizeof(int), 1, readfile);
      fread((char*)&ObjectWindowX, sizeof(int), 1, readfile);
      fread((char*)&ObjectWindowY, sizeof(int), 1, readfile);
      fread((char*)&ObjectWindowColor, sizeof(SbVec3f), 1, readfile);
      fread((char*)&SegViewerWidth, sizeof(int), 1, readfile);
      fread((char*)&SegViewerHeight, sizeof(int), 1, readfile);
      fread((char*)&SegViewerX, sizeof(int), 1, readfile);
      fread((char*)&SegViewerY, sizeof(int), 1, readfile);
      fread((char*)&m_cRecTiefe, sizeof(char), 1, readfile);
      fread((char*)&m_cModelRekMode, sizeof(char), 1, readfile);
      fread((char*)&SegObjColor, sizeof(SbVec3f), 1, readfile);
      fread((char*)&SegSelObjColor, sizeof(SbVec3f), 1, readfile);
      fread((char*)&SegPointColor, sizeof(SbVec3f), 1, readfile);
      fread((char*)&SegNumberColor, sizeof(SbVec3f), 1, readfile);
      fread((char*)&LEDViewerWidth, sizeof(int), 1, readfile);
      fread((char*)&LEDViewerHeight, sizeof(int), 1, readfile);
      fread((char*)&LEDViewerX, sizeof(int), 1, readfile);
      fread((char*)&LEDViewerY, sizeof(int), 1, readfile);
      fread((char*)&LEDViewerColor, sizeof(SbVec3f), 1, readfile);
// add
      fread((char*)&LEDwarningRedRed, sizeof(int), 1, readfile);      
      fread((char*)&LEDwarningRed, sizeof(int), 1, readfile);      
      fread((char*)&LEDwarningYellow, sizeof(int), 1, readfile);      
	  for (i=0; i<18; i++)
	  {
      fread((char*)&LEDdrawstyle[i], sizeof(int), 1, readfile);      
	  }
	  for (i=0; i<18; i++)
	  {
      fread((char*)&LEDofObjectP[i], sizeof(int), 1, readfile);//LED char
	  }
	  for (i=0; i<6; i++)
	  {
      fread((char*)&ObjectsAndReferencesP[i], sizeof(int), 1, readfile);//OBJECT char
	  }
// add	  
	  fread((char*)&namesize, sizeof(int), 1, readfile);
      fread(&socketPort[0], (namesize+1), 1, readfile);
      fread((char*)&namesize, sizeof(int), 1, readfile);
      fread(&socketHost[0], (namesize+1), 1, readfile);
      fread((char*)&namesize, sizeof(int), 1, readfile);
      fread(&optisController[0], (namesize+1), 1, readfile);
      fread((char*)&namesize, sizeof(int), 1, readfile);
      fread(&optisTempDir[0], (namesize+1), 1, readfile);
      fread((char*)&numCameras, 1, 1, readfile);
	  for (i=0; i<10; i++)
	  {
	      fread((char*)&camSysRot[i], sizeof(SbVec3d), 1, readfile);
	      fread((char*)&camSysTrans[i], sizeof(SbVec3d), 1, readfile);
	      fread((char*)&camSysOverlap[i], 1, 1, readfile);
	  }
      fread((char*)&dateType, sizeof(int), 1, readfile);
      fread((char*)&stack_num_slices, sizeof(int), 1, readfile);
      fread((char*)&stack_slice_thickness, sizeof(double), 1, readfile);
      fread((char*)&stack_slice_gap, sizeof(double), 1, readfile);
      fread((char*)&stack_fov_width, sizeof(double), 1, readfile);
      fread((char*)&stack_fov_height, sizeof(double), 1, readfile);
      fread((char*)&stack_img_width, sizeof(int), 1, readfile);
      fread((char*)&stack_img_height, sizeof(int), 1, readfile);
      fread((char*)&stack_offc_ap, sizeof(double), 1, readfile);
      fread((char*)&stack_offc_lr, sizeof(double), 1, readfile);
      fread((char*)&stack_offc_cc, sizeof(double), 1, readfile);
      fread((char*)&stack_ang_ap, sizeof(double), 1, readfile);
      fread((char*)&stack_ang_lr, sizeof(double), 1, readfile);
      fread((char*)&stack_ang_cc, sizeof(double), 1, readfile);
      fread((char*)&stack_mult_offc_ap, sizeof(double), 1, readfile);
      fread((char*)&stack_mult_offc_lr, sizeof(double), 1, readfile);
      fread((char*)&stack_mult_offc_cc, sizeof(double), 1, readfile);
      fread((char*)&stack_mult_ang_ap, sizeof(double), 1, readfile);
      fread((char*)&stack_mult_ang_lr, sizeof(double), 1, readfile);
      fread((char*)&stack_mult_ang_cc, sizeof(double), 1, readfile);
      fread((char*)&MRI_num_stacks, sizeof(int), 1, readfile);
      fread((char*)&namesize, sizeof(int), 1, readfile);
      fread(&MRI_img_base[0], (namesize+1), 1, readfile);
      fread((char*)&namesize, sizeof(int), 1, readfile);
      fread(&MRI_raw_images[0], (namesize+1), 1, readfile);
      fread((char*)&MRI_first, sizeof(int), 1, readfile);
      fclose(readfile);
   }
}


void Preferences::write_inifile()
{
   FILE     *writefile;
   int      namesize, i;

   if ((writefile = fopen("OPTIS.ini", "wb")) == NULL)
      fl_message("Could not open OPTIS.ini");
   else
   {
      namesize = strlen(datapath);
      fwrite((char*)&namesize, sizeof(int), 1, writefile);
      fwrite(datapath, (namesize+1), 1, writefile);

      namesize = strlen(scanpath);
      fwrite((char*)&namesize, sizeof(int), 1, writefile);
      fwrite(scanpath, (namesize+1), 1, writefile);
      namesize = strlen(m_acStdModelPath);
      fwrite((char*)&namesize, sizeof(int), 1, writefile);
      fwrite(m_acStdModelPath, (namesize+1), 1, writefile);
      namesize = strlen(imgcopy);
      fwrite((char*)&namesize, sizeof(int), 1, writefile);
      fwrite(&imgcopy[0], (namesize+1), 1, writefile);
      namesize = strlen(extracode);
      fwrite((char*)&namesize, sizeof(int), 1, writefile);
      fwrite(&extracode[0], (namesize+1), 1, writefile);
      fwrite((char*)&ObjectWindowWidth, sizeof(int), 1, writefile);
      fwrite((char*)&ObjectWindowHeight, sizeof(int), 1, writefile);
      fwrite((char*)&ObjectWindowX, sizeof(int), 1, writefile);
      fwrite((char*)&ObjectWindowY, sizeof(int), 1, writefile);
      fwrite((char*)&ObjectWindowColor, sizeof(SbVec3f), 1, writefile);
      fwrite((char*)&SegViewerWidth, sizeof(int), 1, writefile);
      fwrite((char*)&SegViewerHeight, sizeof(int), 1, writefile);
      fwrite((char*)&SegViewerX, sizeof(int), 1, writefile);
      fwrite((char*)&SegViewerY, sizeof(int), 1, writefile);
      fwrite((char*)&m_cRecTiefe, sizeof(char), 1, writefile);
      fwrite((char*)&m_cModelRekMode, sizeof(char), 1, writefile);
      fwrite((char*)&SegObjColor, sizeof(SbVec3f), 1, writefile);
      fwrite((char*)&SegSelObjColor, sizeof(SbVec3f), 1, writefile);
      fwrite((char*)&SegPointColor, sizeof(SbVec3f), 1, writefile);
      fwrite((char*)&SegNumberColor, sizeof(SbVec3f), 1, writefile);
      fwrite((char*)&LEDViewerWidth, sizeof(int), 1, writefile);
      fwrite((char*)&LEDViewerHeight, sizeof(int), 1, writefile);
      fwrite((char*)&LEDViewerX, sizeof(int), 1, writefile);
      fwrite((char*)&LEDViewerY, sizeof(int), 1, writefile);
      fwrite((char*)&LEDViewerColor, sizeof(SbVec3f), 1, writefile);
// add
      fwrite((char*)&LEDwarningRedRed, sizeof(int), 1, writefile);
      fwrite((char*)&LEDwarningRed, sizeof(int), 1, writefile);
      fwrite((char*)&LEDwarningYellow, sizeof(int), 1, writefile);      
	  for (i=0; i<18; i++)
	  {
      fwrite((char*)&LEDdrawstyle[i], sizeof(int), 1, writefile);      
	  }
	  for (i=0; i<18; i++)
	  {
      fwrite((char*)&LEDofObjectP[i], sizeof(int), 1, writefile);      
	  }
	  for (i=0; i<6; i++)
	  {
      fwrite((char*)&ObjectsAndReferencesP[i], sizeof(int), 1, writefile);      
	  }
// add
	  namesize = strlen(socketPort);
      fwrite((char*)&namesize, sizeof(int), 1, writefile);
      fwrite(socketPort, (namesize+1), 1, writefile);
      namesize = strlen(socketHost);
      fwrite((char*)&namesize, sizeof(int), 1, writefile);
      fwrite(socketHost, (namesize+1), 1, writefile);
      namesize = strlen(optisController);
      fwrite((char*)&namesize, sizeof(int), 1, writefile);
      fwrite(&optisController[0], (namesize+1), 1, writefile);
      namesize = strlen(optisTempDir);
      fwrite((char*)&namesize, sizeof(int), 1, writefile);
      fwrite(&optisTempDir[0], (namesize+1), 1, writefile);
      fwrite((char*)&numCameras, 1, 1, writefile);
	  for (i=0; i<10; i++)
	  {
	      fwrite((char*)&camSysRot[i], sizeof(SbVec3d), 1, writefile);
	      fwrite((char*)&camSysTrans[i], sizeof(SbVec3d), 1, writefile);
	      fwrite((char*)&camSysOverlap[i], 1, 1, writefile);
	  }
      fwrite((char*)&dateType, sizeof(int), 1, writefile);
      fwrite((char*)&stack_num_slices, sizeof(int), 1, writefile);
      fwrite((char*)&stack_slice_thickness, sizeof(double), 1, writefile);
      fwrite((char*)&stack_slice_gap, sizeof(double), 1, writefile);
      fwrite((char*)&stack_fov_width, sizeof(double), 1, writefile);
      fwrite((char*)&stack_fov_height, sizeof(double), 1, writefile);
      fwrite((char*)&stack_img_width, sizeof(int), 1, writefile);
      fwrite((char*)&stack_img_height, sizeof(int), 1, writefile);
      fwrite((char*)&stack_offc_ap, sizeof(double), 1, writefile);
      fwrite((char*)&stack_offc_lr, sizeof(double), 1, writefile);
      fwrite((char*)&stack_offc_cc, sizeof(double), 1, writefile);
      fwrite((char*)&stack_ang_ap, sizeof(double), 1, writefile);
      fwrite((char*)&stack_ang_lr, sizeof(double), 1, writefile);
      fwrite((char*)&stack_ang_cc, sizeof(double), 1, writefile);
      fwrite((char*)&stack_mult_offc_ap, sizeof(double), 1, writefile);
      fwrite((char*)&stack_mult_offc_lr, sizeof(double), 1, writefile);
      fwrite((char*)&stack_mult_offc_cc, sizeof(double), 1, writefile);
      fwrite((char*)&stack_mult_ang_ap, sizeof(double), 1, writefile);
      fwrite((char*)&stack_mult_ang_lr, sizeof(double), 1, writefile);
      fwrite((char*)&stack_mult_ang_cc, sizeof(double), 1, writefile);
      fwrite((char*)&MRI_num_stacks, sizeof(int), 1, writefile);
      namesize = strlen(MRI_img_base);
      fwrite((char*)&namesize, sizeof(int), 1, writefile);
      fwrite(MRI_img_base, (namesize+1), 1, writefile);
      namesize = strlen(MRI_raw_images);
      fwrite((char*)&namesize, sizeof(int), 1, writefile);
      fwrite(MRI_raw_images, (namesize+1), 1, writefile);
      fwrite((char*)&MRI_first, sizeof(int), 1, writefile);
	  fclose(writefile);
   }
}


const char* Preferences::getdatapath()
{
    return &datapath[0];
}

const char* Preferences::getscanpath()
{
    return &scanpath[0];
}


void Preferences::setdatapath(const char* pathname)
{
    sprintf(datapath, "%s", pathname);
}

void Preferences::setscanpath(const char* pathname)
{
    sprintf(scanpath, "%s", pathname);
}


char Preferences::readViewFile(char *filename, SoPerspectiveCamera *cam,
		                       SoDirectionalLight *light)
{
   FILE    *readfile;
   SbVec3f axis;
   float   angle;

   if ((readfile = fopen(filename, "rb")) == NULL)
   {
      fl_message("Could not open\n%s!", filename);
	  return 0;
   }

   fread(&axis, sizeof(axis), 1, readfile);
   cam->position.setValue(axis);
   fread(&axis, sizeof(axis), 1, readfile);
   fread(&angle, sizeof(angle), 1, readfile);
   cam->orientation.setValue(axis, angle);
   fread(&angle, sizeof(angle), 1, readfile);
   cam->focalDistance.setValue(angle);
   fread(&axis, sizeof(axis), 1, readfile);
   light->direction.setValue(axis);

   fclose(readfile);
   return 1;
}


char Preferences::writeViewFile(char *filename, SoPerspectiveCamera *cam,
		                        SoDirectionalLight *light)
{
   FILE    *writefile;
   char    st[256], fname[256];
   int     j;
   SbVec3f axis;
   float   angle;

   sprintf(st, "%s", filename);
   j = 0;
   while (st[j] != '\0')
      if (st[j] == '.')
         st[j] = '\0';
      else
         j++;
   sprintf(fname, "%s.vws", st);

   if ((writefile = fopen(&fname[0], "wb")) == NULL)
   {
      fl_message("Could not open\n%s!", fname);
	  return 0;
   }

   axis = cam->position.getValue();
   fwrite(&axis, sizeof(axis), 1, writefile);
   cam->orientation.getValue(axis, angle);
   fwrite(&axis, sizeof(axis), 1, writefile);
   fwrite(&angle, sizeof(angle), 1, writefile);
   angle = cam->focalDistance.getValue();
   fwrite(&angle, sizeof(angle), 1, writefile);
   axis = light->direction.getValue();
   fwrite(&axis, sizeof(axis), 1, writefile);

   fclose(writefile);
   return 1;
}


void Preferences::convertDOScmd(char *cmd)
{
	int i = 0;;

	while ((i<255) && (cmd[i]!= '\0'))
	{
		if (cmd[i] == '/')
			cmd[i] = '\\';
		i++;
	}
}


//-----------------------------------------------------------------------
//  Starten von Programmen, Ausführen von Verknüpfungen etc.(Starting programs, run shortcuts etc)
//[ Starten von Programmen, Ausführen von Verknüpfungen etc.]

//  Parameter: Name der Auszuführenden Datei als String (kann auch Name
//             einer nicht ausführbaren Datei sein, wenn der Dateityp
//             einer Anwendung zugeordnet ist)
// [Parameter: Name the file as a string performers (can also be a non-executable file name,
//the file type is associated with an application)]
//-----------------------------------------------------------------------
void Preferences::RunApp(char *slFileName)
{
  int slResuslt = int(ShellExecute(0, "open", slFileName,
    0, 0, SW_SHOWNORMAL));

  if(slResuslt <= 32)//if errors
  {
    char slMessage[256];
    if(slResuslt == ERROR_FILE_NOT_FOUND || slResuslt == SE_ERR_FNF)
      sprintf(slMessage, "Die angegebene Datei wurde nicht gefunden !(The specified file was not found!)");
    else if(slResuslt == ERROR_PATH_NOT_FOUND || slResuslt == SE_ERR_PNF)
      sprintf(slMessage,  "Der Pfad zur Datei wurde nicht gefunden !(The path to the file was not found!)");
	else if(slResuslt == ERROR_BAD_FORMAT)
      sprintf(slMessage, "Die .EXE Datei ist ungültig oder beschädigt !(The. EXE file is invalid or corrupted!)");
    else if(slResuslt == SE_ERR_ACCESSDENIED || slResuslt == SE_ERR_SHARE)
      sprintf(slMessage, "Dateizugriff wird vom Betriebssystem verweigert !(File access is denied by the operating system!)");
    else if(slResuslt == SE_ERR_ASSOCINCOMPLETE || slResuslt == SE_ERR_NOASSOC)
      sprintf(slMessage, "Der angegebene Dateityp ist auf Ihrem Computer keiner Anwendung zugeordnet !(The given file type is associated with any application on your computer!)");
    else
      sprintf(slMessage, "Datei konnte nicht geöffnet werden !(File could not be opened!)");
	fl_message(&slMessage[0]);
  }
}


void TmjColorChip::draw() {
  if (damage()&FL_DAMAGE_ALL) draw_box();
  fl_rectf(x()+Fl::box_dx(box()),
	   y()+Fl::box_dy(box()),
	   w()-Fl::box_dw(box()),
	   h()-Fl::box_dh(box()),r,g,b);
}


Calibration::Calibration()
{
	list = NULL;
}


Calibration::~Calibration()
{
	data *d;

	while (list != NULL)
	{
		d = list;
		list = list->next;
		delete d;
	}
}


void Calibration::write_file()
{
   FILE     *writefile;
   int      namesize, anz, i, j;
   data     *d;

   d = list;
   anz = 0;
   while (d != NULL)
   {
	   anz++;
	   d = d->next;
   }

   if ((writefile = fopen("TmjViewer.cal", "wb")) == NULL)
   {
      fl_message("Could not open TmjViewer.cal");
	  return;
   }

   fwrite((char*)&anz, sizeof(int), 1, writefile);
   for (i=0; i<anz; i++)
   {
	  d = list; // Letzte Daten zuerst schreiben
	  for (j=(i+1); j<anz; j++)
		  d = d->next;
      namesize = strlen(d->name);
      fwrite((char*)&namesize, sizeof(int), 1, writefile);
      fwrite(d->name, (namesize+1), 1, writefile);
      fwrite((char*)&d->SphereBackX, sizeof(double), 1, writefile);
      fwrite((char*)&d->SphereBackY, sizeof(double), 1, writefile);
      fwrite((char*)&d->SphereBackZ, sizeof(double), 1, writefile);
      fwrite((char*)&d->SphereFrontX, sizeof(double), 1, writefile);
      fwrite((char*)&d->SphereFrontY, sizeof(double), 1, writefile);
      fwrite((char*)&d->SphereFrontZ, sizeof(double), 1, writefile);
      fwrite((char*)&d->SphereTopX, sizeof(double), 1, writefile);
      fwrite((char*)&d->SphereTopY, sizeof(double), 1, writefile);
      fwrite((char*)&d->SphereTopZ, sizeof(double), 1, writefile);
      fwrite((char*)&d->LedBackX, sizeof(double), 1, writefile);
      fwrite((char*)&d->LedBackY, sizeof(double), 1, writefile);
      fwrite((char*)&d->LedBackZ, sizeof(double), 1, writefile);
      fwrite((char*)&d->LedFrontX, sizeof(double), 1, writefile);
      fwrite((char*)&d->LedFrontY, sizeof(double), 1, writefile);
      fwrite((char*)&d->LedFrontZ, sizeof(double), 1, writefile);
      fwrite((char*)&d->LedTopX, sizeof(double), 1, writefile);
      fwrite((char*)&d->LedTopY, sizeof(double), 1, writefile);
      fwrite((char*)&d->LedTopZ, sizeof(double), 1, writefile);
	  d = d->next;
   }
   fclose(writefile);
}


void Calibration::read_file()
{
   FILE    *readfile;
   int     namesize, i, anz;
   data    *d;

   while (list != NULL)
   {
	   d = list;
	   list = list->next;
	   delete d;
   }

   if ((readfile = fopen("TmjViewer.cal", "rb")) == NULL)
   {
	  d = new data;
	  d->next = list;
	  list = d;

	  sprintf(d->name, "none");
	  d->SphereBackX = 0.0;
	  d->SphereBackY = 0.0;
	  d->SphereBackZ = 0.0;
	  d->SphereFrontX = 0.0;
	  d->SphereFrontY = 0.0;
	  d->SphereFrontZ = 0.0;
	  d->SphereTopX = 0.0;
	  d->SphereTopY = 0.0;
	  d->SphereTopZ = 0.0;
	  d->LedBackX = 0.0;
	  d->LedBackY = 0.0;
	  d->LedBackZ = 0.0;
	  d->LedFrontX = 0.0;
	  d->LedFrontY = 0.0;
	  d->LedFrontZ = 0.0;
	  d->LedTopX = 0.0;
	  d->LedTopY = 0.0;
	  d->LedTopZ = 0.0;
	  return;
   }

   fread((char*)&anz, sizeof(int), 1, readfile);
   for (i=0; i<anz; i++)
   {
	  d = new data;
	  d->next = list;
	  list = d;

      fread((char*)&namesize, sizeof(int), 1, readfile);
      fread(&d->name, (namesize+1), 1, readfile);
      fread((char*)&d->SphereBackX, sizeof(double), 1, readfile);
      fread((char*)&d->SphereBackY, sizeof(double), 1, readfile);
      fread((char*)&d->SphereBackZ, sizeof(double), 1, readfile);
      fread((char*)&d->SphereFrontX, sizeof(double), 1, readfile);
      fread((char*)&d->SphereFrontY, sizeof(double), 1, readfile);
      fread((char*)&d->SphereFrontZ, sizeof(double), 1, readfile);
      fread((char*)&d->SphereTopX, sizeof(double), 1, readfile);
      fread((char*)&d->SphereTopY, sizeof(double), 1, readfile);
      fread((char*)&d->SphereTopZ, sizeof(double), 1, readfile);
      fread((char*)&d->LedBackX, sizeof(double), 1, readfile);
      fread((char*)&d->LedBackY, sizeof(double), 1, readfile);
      fread((char*)&d->LedBackZ, sizeof(double), 1, readfile);
      fread((char*)&d->LedFrontX, sizeof(double), 1, readfile);
      fread((char*)&d->LedFrontY, sizeof(double), 1, readfile);
      fread((char*)&d->LedFrontZ, sizeof(double), 1, readfile);
      fread((char*)&d->LedTopX, sizeof(double), 1, readfile);
      fread((char*)&d->LedTopY, sizeof(double), 1, readfile);
      fread((char*)&d->LedTopZ, sizeof(double), 1, readfile);
   }
   fclose(readfile);
}

//import comment list
Commentlist::Commentlist()
{
	clist = NULL;
}

Commentlist::~Commentlist()
{
	cdata *d;

	while (clist != NULL)
	{
		d = clist;
		clist = clist->next;
		delete d;
	}
}

/*void Commentlist::read_comfile()
{
   FILE     *readfile;
   cdata    *d;
   unsigned char c;
   char     fc;
   char     cc[256];
   int      nsize[50];
   int      i,j;
   int      num_lines;

   for (i=0; i<50; i++)nsize[i]=0;

   while (clist != NULL)
   {
	   d = clist;
	   clist = clist->next;
	   delete d;
	  return;
   }

   if ((readfile = fopen("comment.txt", "rb")) == NULL)
   {
	  d = new cdata;
	  d->next = clist;
	  clist = d;
	  sprintf(d->cname, "none");
	  return;
   }

   i=0;
   num_lines = 0;
   while (! feof(readfile))
   {
      num_lines++;
      j = 0;
	  i=0;
      while ((j < 255) && (! feof(readfile)))
      {
         fread(&fc, 1, 1, readfile);
         if (fc == '\n')
		 {
		nsize[num_lines]=i;
            j = 300;
		 }
         else
		 {
			 i++;
            j++;
		 }
      }
   }
   fclose(readfile);
		nsize[num_lines]=i;
//   printf("number of the lines%d\n",num_lines);

   readfile = fopen("comment.txt", "rb");

   fread(&cc, (nsize[1]+1), 1, readfile);
   for (i=1; i<num_lines; i++)
   {
	  d = new cdata;
	  d->next = clist;
	  clist = d;

      fread(&d->cname, (nsize[i+1]-1), 1, readfile);
      fread(&c, 1, 1, readfile);
      fread(&c, 1, 1, readfile);
	  d->ctype =i-1;
//	  printf("%d  namesize %d   cname %s\n", d->ctype,nsize[i+1],d->cname);
   }
   fclose(readfile);
}*/

void Commentlist::read_comfile()
{
   FILE     *readfile;
   cdata    *d;
   unsigned char c;
   char     fc;
   char     cc[256];
   int      nsize[50];
   int      i,j;
   int      num_lines;

   for (i=0; i<50; i++)nsize[i]=0;

   while (clist != NULL)
   {
	   d = clist;
	   clist = clist->next;
	   delete d;
	  return;
   }

   if ((readfile = fopen("comment.txt", "rb")) == NULL)
   {
	  d = new cdata;
	  d->next = clist;
	  clist = d;
	  sprintf(d->cname, "none");
	  return;
   }

   i=0;
   num_lines = 0;
   while (! feof(readfile))
   {
      num_lines++;
      j = 0;
	  i=0;
      while ((j < 255) && (! feof(readfile)))
      {
         fread(&fc, 1, 1, readfile);
         if (fc == '\n')
		 {
		nsize[num_lines]=i;
            j = 300;
		 }
         else
		 {
			 i++;
            j++;
		 }
      }
   }
   fclose(readfile);
		nsize[num_lines]=i;
//   printf("number of the lines%d\n",num_lines);

   readfile = fopen("comment.txt", "rb");

   fread(&cc, (nsize[1]+1), 1, readfile);
   for (i=1; i<num_lines; i++)
   {
	  d = new cdata;
	  d->next = clist;
	  clist = d;

      fread(&d->cname, (nsize[i+1]-1), 1, readfile);
      fread(&c, 1, 1, readfile);
      fread(&c, 1, 1, readfile);
	  d->ctype =i-1;
//	  printf("%d  namesize %d   cname %s\n", d->ctype,nsize[i+1],d->cname);
   }
   fclose(readfile);
}

#include "Scanfile.h"

Scanfile::Scanfile()
{
   int i;

   sections = NULL;
   
   sprintf(name, "%s", "/0");
   sprintf(firstname, "%s", "/0");
   sprintf(anatomy, "%s", "/0");
   sprintf(date, "%s", "/0");
   sprintf(remarks, "%s", "/0");
   num_stacks = 0;
   num_references = 0;
   num_leds = 0;
   sprintf(raw_path, "%s", "/0");
   sprintf(raw_files, "%s", "/0");
   sprintf(raw_images, "%s", "/0");
   sprintf(img_path, "%s", "/0");
   sprintf(img_base, "%s", "/0");
   img_first = 1;
   sprintf(scanbase, "%s", "/0");
   sprintf(database, "%s", "/0");
   sprintf(scandir, "%s", "/0");
   sprintf(shortScandir, "%s", "/0");
   sprintf(movdir, "%s", "/0");
   sprintf(scanfilename, "%s", "/0");

   scanstack = NULL;
   cancelstack = NULL;

   anatomyleft = 1;
   for (i=0; i<10; i++)
      sprintf(recfiles[i], "%s", "leer");

/*   sprintf(refsysFile, "%s", "/0");
   sprintf(refFile1, "%s", "/0");
   sprintf(refFile2, "%s", "/0");*/
   sprintf(refsysFile, "%s", "");
   sprintf(refFile1, "%s", "");
   sprintf(refFile2, "%s", "");
   reffile1number = 0;
   reffile2number = 0;

}


Scanfile::~Scanfile()
{
   if (sections != NULL)
   {
      delete[] sections;
      sections = NULL;
   }

   if (scanstack != NULL)
   {
      delete[] scanstack;
      scanstack = NULL;
      delete[] cancelstack;
      cancelstack = NULL;
   }
}


int Scanfile::scanwhere(char *text)
{
   int i=0;
   char st[256], *tok;

   sprintf(st, "%s", "/0");
   while ((i<num_lines) && (strcmp(&st[0],text) != 0))
   {
      sprintf(st, "%s", &sections[i*256]);
      tok = strtok(&st[0], " ");
      i++;
   }

   if (i == num_lines)
      return 9999;
   else
      return i;
}


void Scanfile::scanget(char *text, int i, char c, int anz_worte, char *resultat)
{
   int j, k;
   char st[256];

   st[0] = '\0';
   while ((i<num_lines) && (strcmp(&st[0],text) != 0))
   {
      j = 0;
      while ((j<256) && (sections[i*256+j]!=' ') && (sections[i*256+j]!='=') &&
             (sections[i*256+j]!='\0'))
      {
         st[j] = sections[i*256+j];
         j++;
      }
      st[j] = '\0';
      i++;
   }

   if (i == num_lines)
      return;

   i--;
   sprintf(st, "%s", &sections[i*256]);
   j = 0;
   while (st[j] != '=')
      j++;
   j++;
   while (st[j] == ' ')
      j++;

   k = 0;
   while ((j<256) && (st[j]!='\0') && (st[j]!=c))
   {
      resultat[k] = st[j];
      k++; j++;
   } 
   resultat[k] = '\0';
}


char Scanfile::read(char *filename)
{
   int     j, i;
   FILE    *readfile;
   char    fc;
   char    myst[256];
   char    fehler;

   num_stacks = 0;
   num_references = 0;
   num_leds = 0;
   fehler = 0;

   if ((readfile = fopen(filename, "rb")) == NULL)
      return 0;

   num_lines = 0;
   while (! feof(readfile))
   {
      num_lines++;
      j = 0;
      while ((j < 255) && (! feof(readfile)))
      {
         fread(&fc, 1, 1, readfile);
         if (fc == '\n')
            j = 300;
         else
            j++;
      }
   }
   fclose(readfile);
   sections = new char[num_lines*256];

   readfile = fopen(filename, "rb");
   i = 0;
   while (i < num_lines)
   {
      j = 0;
      while (j < 255)
      {
         fread(&fc, 1, 1, readfile);
         if (fc == '\n')
         {
            sections[i*256+j] = '\0';
            j = 300;
         }
         else
         {
			if (fc != '\r')
			{
	            sections[i*256+j] = fc;
	            j++;
			}
         }
         if (j == 255)
            sections[i*256+j] = '\0';
      }
      i++;
   }
   fclose(readfile);

   i = scanwhere("[Patient]");
   if (i < num_lines)
   {
      scanget("Name", i, '*', 1, (char *)name);
      scanget("Firstname", i, '*', 1, (char *)firstname);
   }
   else
      fehler += 2;

   i = scanwhere("[Scan]");
   if (i < num_lines)
   {
      scanget("Anatomy", i, '*', 1, (char *)anatomy);
      scanget("Recording", i, '*', 2, (char *)date);
      scanget("Remarks", i, '*', 1, (char *)remarks);
   }
   else
      fehler += 4;

   i = scanwhere("[Movements]");
   if (i < num_lines)
   {
      scanget("Path", i, ' ', 1, (char *)scanmov.path);
      scanget("Base", i, '*', 1, (char *)scanmov.base);
   }
   else
      fehler += 32;

   delete[] sections;
   sections = NULL;

   sprintf(myst, "%s", filename);
   i = strlen(myst);
   while ((i >= 0) && (myst[i] != '/'))
      i--;
   sprintf(scanfilename, "%s", &myst[i+1]);
   myst[i] = '\0';
   sprintf(scandir, "%s", myst);

   if (fehler > 0)
      return fehler;
   else
      return 1;
}


char Scanfile::write(char *filename, char overwrite)
{
   int      i, j;
   FILE     *writefile;
   char     cmd[256], fname2[256];

   num_lines = 13;
   sections = new char[num_lines*256];

   sprintf(&sections[0*256], "[Patient]\r\n\0");
   sprintf(&sections[1*256], "Name                 = %s\r\n\0", name);
   sprintf(&sections[2*256], "Firstname            = %s\r\n\0", firstname);
   sprintf(&sections[3*256], "\r\n\0");
   sprintf(&sections[4*256], "[Scan]\r\n\0");
   sprintf(&sections[5*256], "Anatomy              = %s\r\n\0", anatomy);
   sprintf(&sections[6*256], "Recording date       = %s\r\n\0", date);
   sprintf(&sections[7*256], "Remarks              = %s\r\n\0", remarks);
   sprintf(&sections[8*256], "\r\n\0");
   sprintf(&sections[9*256], "[Movements]\r\n\0");
   sprintf(&sections[10*256], "Path                 = %s\r\n\0",
      scanmov.path);
   sprintf(&sections[11*256], "Base                 = %s\r\n\0",
      scanmov.base);
   sprintf(&sections[12*256], "\r\n\0");

   // Existiert File bereits?
   if ((writefile = fopen(filename, "rb")) != NULL)
   {
	   fclose(writefile);
	   if (overwrite < 1)
		   // Ueberschreiben?
		   j = fl_ask("Overwrite the existing file \n%s?", filename);
	   else
		   j = 1;
	   if (j < 1) // nein
	   {
		   delete[] sections;
		   return 0;
	   }
	   // Sicherungskopie erstellen
	   sprintf(fname2, filename);
	   i = strlen(fname2);
	   i--;
	   while ((i>0) && (fname2[i] != '.'))
		   i--;
	   if (i>0)
		   fname2[i] = '_';
#ifdef WIN32
	   sprintf(cmd, "move %s %s.bak", filename, fname2);
	   i = 0;
	   while ((i<255) && (cmd[i]!= '\0'))
	   {
		   if (cmd[i] == '/')
			   cmd[i] = '\\';
		   i++;
	   }
#else
	   sprintf(cmd, "mv %s %s.bak", filename, fname2);
#endif
	   printf("%s\n", cmd);
	   system(cmd);
   }

   if ((writefile = fopen(filename, "wb")) == NULL)
   {
      delete[] sections;
      return 0;
   }

   for (i=0; i<num_lines; i++)
   {
      j = 0;
      while (sections[i*256+j] != '\0')
      {
         fwrite((char*) &sections[i*256+j], 1, 1, writefile);
         j++;
      }
   }

   fclose(writefile);

   delete[] sections;
   sections = NULL;
   return 1;
}


void Scanfile::errortext(char fehler, char* meldung)
{
   int z = 32;

   sprintf(meldung, "\0");
   while ((fehler > 0) && (z > 1))
   {
      if ((fehler - z) >= 0)
      {
         switch (z)
         {
            case 32:
               strcat(meldung, "Missing section [Movements].\n");
               fehler -= 32;
               break;
            case 16:
               strcat(meldung, "Missing section [Images].\n");
               fehler -= 16;
               break;
            case 8:
               strcat(meldung, "Missing section [Raw].\n");
               fehler -= 8;
               break;
            case 4:
               strcat(meldung, "Missing section [Scan].\n");
               fehler -= 4;
               break;
            case 2:
               strcat(meldung, "Missing section [Patient].\n");
               fehler -= 2;
               break;
         } 
      }
      z /= 2;
   }
}



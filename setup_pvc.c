#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// ip-10.200.10.1:3260-iscsi-iqn.2002-03.com.compellent:5000d3100335b621-lun-77
// ip-10.200.10.1:3260-iscsi-iqn.2002-03.com.compellent:5000d3100335b621-lun-79
// ip-10.200.10.1:3260-iscsi-iqn.2002-03.com.compellent:5000d3100335b621-lun-81
// ip-10.200.10.1:3260-iscsi-iqn.2002-03.com.compellent:5000d3100335b621-lun-83
// ip-10.200.10.1:3260-iscsi-iqn.2002-03.com.compellent:5000d3100335b621-lun-85

FILE *thefile;
FILE *yml;

char theline[200000];

main()
{
char devicenum[256];
char devicename[256];
char *token;
char *ipaddr;
char *iqn;
char *next;
char *theend;
char *luns;
char *ptr;
int lun;
char *lunstart;
char lunhex[20];
const char colon[2] = ":";
const char dash[2] = "-";
int capacity;


        system("mkdir -p .setup_pvc/data");
        system("mkdir -p .setup_pvc/data/yml");
	system("ls /dev/disk/by-path | grep 'ip-' > .setup_pvc/data/disklist");
        system("lsscsi  --transport --lun  --size | grep 'iqn' > .setup_pvc/data/scsilist");
        thefile = fopen(".setup_pvc/data/disklist","r");
        if (thefile == NULL){
           printf("Empty file\n");
           exit(0);
           }
        while(fgets(theline,256,thefile)!=NULL){
            theend = strchr(theline,'-');
            *theend = 0;
            next = ++theend;
            ipaddr = next;
            theend = strchr(next,':');
            *theend = 0;
            next = ++theend;
            iqn = strstr(next,"iqn");
            luns = strstr(next,"-lun");
            *luns = 0;
            luns++;
            lunstart = strchr(luns,'-');
            lunstart++;
            theend = strchr(lunstart,0);
            lun = strtol(lunstart,&theend,10);
            sprintf(lunhex,"%0.4x",lun);
            capacity = getCapacity(iqn,lunhex);
            // Find the device within the iqn
            ptr = strchr(iqn,':');
            ptr++; 
            strcpy(devicenum,ptr);
            ptr = strchr(devicenum,'-');
            ptr = 0; // Terminate our devicenum copy
            sprintf(devicename,"dev%sx%d",devicenum,lun);
            if (ymlexists(devicename) == 0){
               buildyml(devicename,ipaddr,iqn,lun,capacity);
               }
            }
        fclose(thefile);
}

int getCapacity(char *iqn, char *lunhex)
{
// [10:0:0:0x0063]             disk    iqn.2002-03.com.compellent:5000d3100335b621,t,0x0  /dev/sdj   1.07GB
FILE *scsilist;
char cmdline[512];
char sizeline[512];
char *tokens[20];
const char space[2] = " ";
int tcnt;
unsigned long capacity;

   sprintf(cmdline,"grep %s .setup_pvc/data/scsilist  | grep %s > .setup_pvc/data/sizeentry",iqn,lunhex);
   system(cmdline);
   scsilist = fopen(".setup_pvc/data/sizeentry","r");
   if (scsilist == NULL){
      printf("Cannot open sizeentry\n");
      exit(-1);
      }
   fgets(sizeline,256,scsilist); 
   fclose(scsilist);
   tcnt = 0;
   tokens[tcnt++] = strtok(sizeline,space);
   while(tokens[tcnt] != NULL){
      tcnt++;
      tokens[tcnt] = strtok(NULL,space);
      }
  capacity = strtol(tokens[5],NULL,10);
  return(capacity);
   
}

int file_exists (char *filename)
{
  int result;
  struct stat buffer;
  result =  (stat (filename, &buffer) == 0);
  return(result);
}

int ymlexists(char *devname)
{
char filename[256];

      sprintf(filename,".setup_pvc/data/yml/%s.yml",devname);
      if (file_exists(filename))
         return(1);
      else return(0);
}

buildyml(char *devname,char *portal,char *iqn, int lun, int capacity)
{
// portal: 10.200.10.1
// iqn: iqn.2002-03.com.compellent:5000d3100335b621
// lun: 99
char filename[256];
char cmdline[256];

      sprintf(filename,".setup_pvc/data/yml/%s.yml",devname);
      printf("Building %s Filename: %s\n",devname,filename);
      yml = fopen(filename,"w");
      fprintf(yml,"apiVersion: v1\n");
      fprintf(yml,"kind: PersistentVolume\n");
      fprintf(yml,"metadata:\n");
      fprintf(yml,"  name: %s\n",devname);
      fprintf(yml,"spec:\n");
      fprintf(yml,"  capacity:\n");
      fprintf(yml,"    storage: %dGi\n",capacity);
      fprintf(yml,"  accessModes:\n");
      fprintf(yml,"    - ReadWriteOnce\n");
      fprintf(yml,"  iscsi:\n");
      fprintf(yml,"     targetPortal: %s\n",portal);
      fprintf(yml,"     iqn: %s\n",iqn);
      fprintf(yml,"     lun: %d\n",lun);
      fprintf(yml,"     fsType: 'ext4'\n");
      fprintf(yml,"     readOnly: false\n");
      fclose(yml);
      sprintf(cmdline,"oc create -f %s",filename);
      printf("%s\n",cmdline);
      system(cmdline);
      return;
}


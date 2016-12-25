#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
char *token;
char *ipaddr;
char *iqn;
char *next;
char *theend;
char *luns;

        system("mkdir -p .setup_pvc/data");
	system("ls /dev/disk/by-path | grep 'ip-' > .setup_pvc/data/disklist");
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
            printf("IP:%s ",ipaddr);
            iqn = strstr(next,"iqn");
            luns = strstr(next,"-lun");
            *luns = 0;
            luns++;
            printf("IQN:%s ",iqn);
            printf("lun:%s ",luns);
            }
        fclose(thefile);
}

buildyml(char *devname,char *portal,char *iqn, int lun, int capacity)
{
// portal: 10.200.10.1
// iqn: iqn.2002-03.com.compellent:5000d3100335b621
// lun: 99
char filename[256];
char cmdline[256];

      sprintf(filename,"~/.setup_pvc/%s.yml",devname);
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
      fprintf(yml,"	readOnly: false\n");
      fclose(yml);
      sprintf(cmdline,"oc create -f $volname",filename);
      system(cmdline);
      return;
}


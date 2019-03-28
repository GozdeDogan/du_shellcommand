/******************************************************************************/
/*                                                                            */  
/* Gozde DOGAN - 131044019                                                    */
/* CSE344 System Programming - Homework 1                                     */
/*                                                                            */ 
/* ONEMLI:                                                                    */
/*          Dosya boyutu icin st_blksize kullanilmistir.                      */
/*          http://man7.org/linux/man-pages/man2/fstat.2.html                 */
/*                                                                            */                            
/*          Dosyanin ozel olup olmadigi kontrolu (link, fifo ya da pipe)      */
/*          http://codewiki.wikidot.com/c:system-calls:stat sitesinden        */
/*          arastirilmistir.                                                  */
/*                                                                            */      
/*          Directory icini dolasma,  https://www.ibm.com/support/knowled     */
/*          gecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/rtoped.htm      */
/*          sitesinden arastirilmistir.                                       */ 
/*                                                                            */   
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> 
#include <errno.h>    
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH_SIZE 1024
#define DEBUG

/****************************** Global Veriables ******************************/
int iIs_z = 0;
int iFirstSize = 0;
int iIndex = 0;
char sOldFname[MAX_PATH_SIZE] = "";
int iOldSizeOfDir = 0;
/******************************************************************************/


/**************************** Function Definitions ****************************/
void doOperation(int argc, char *argv[]);
int postOrderApply(char *path, int pathfun (char *path1));
int sizepathfun (char *path);
void usage();
int isInSameDirectory(char *sString1, char *sString2);
/******************************************************************************/


/******************************** START OF MAIN *******************************/
int main(int argc, char *argv[])
{
	if((argc != 2 && argc != 3) || (argc == 3 && strcmp(argv[1], "-z") != 0))
	{
		usage();
		return 0;
	}
    doOperation(argc, argv);
    		
	return 0;
}
/********************************** END OF MAIN *******************************/

/************************** Function Declaration ******************************/

/*
 * Kullanici tarafindan girilen compiler kodu kontrol edilerek 
 * islemler gerceklestiriliyor.
 * Kullanicinin calistiriken "-z" komutunu girip girmedigi kontrol eidliyor.
 * Bunun sonucuna gore ekrana yazdirilacak seyler ve klasor boyutlarinin 
 * toplanma sekli degismektedir.
 *
 * input: 
 *  argc: a number of string in the compiler code
 *  argv: strings in the compiler code
 */
void doOperation(int argc, char *argv[])
{
    int totalSize = 0;
    
    sprintf(sOldFname, "%s", argv[argc-1]);
    
    if(argc == 3 && strcmp(argv[1], "-z") == 0)
        iIs_z = 1;
        
    if(argc == 2)
    {
        fprintf(stderr, "Output of \"buNeDu %s\" don't add subdirectory sizes:\n", argv[1]);
        totalSize = postOrderApply(argv[argc-1], sizepathfun);
        fprintf(stderr, "%d\t\t%s\n", iFirstSize, argv[argc-1]);
    }
    
    if(argc == 3)
    {
        fprintf(stderr, "Output of \"buNeDu %s %s\" gives total sizes:\n", argv[1], argv[2]);
        totalSize = postOrderApply(argv[argc-1], sizepathfun);
        fprintf(stderr, "%d\t\t%s\n", totalSize+iFirstSize, argv[argc-1]);
    }
}


/*
 * Directory icindeki directory'lerin boyutlarini buluyor.
 * Kullanici tarafindan girilen calistirma kodundaki "-z" degerinin 
 * varligina gore hesaplama yapilmakta.
 *
 * input: 
 *  path: directory name
 *  int pathfun(char *path1): function name
 * output:
 *  int: total size of directories in input directory
 */
int postOrderApply(char *path, int pathfun (char *path1))
{
    DIR *dir;
    struct dirent *dosya;																	
	struct stat status;  //Degisen fname ile birlikte status durumunu alir ve S_ISDIR fonksiyonunda kullanir

	//Variables
	int iSizeFname = 0;
	char fname[MAX_PATH_SIZE]; //dosyanin adini tutar
	static int iSizeOfTotalDir = 0;
	int iSizeOfDir = 0;
	int iPrintSize = 0;
   
    //Directory acilabiliyor mu kontrolu yaptim
    if ((dir = opendir(path)) == NULL) {
		//perror("opendir");
		fprintf(stderr, "%d\t%s\n", pathfun(path), path);
		exit(1); 
	}
	
	// input olarak girilen klasorun icindeki dosyalarin boyutlari bulundu.
	if(iIndex == 0)
	{
	    iFirstSize = pathfun(path);
	    iOldSizeOfDir = iFirstSize;
	    iIndex++;
	}

	while ((dosya = readdir(dir)) != NULL) 
	{	
	    //Dosya adi "." veya ".." olmadiginda islem yapilacak.
		if ((strcmp(dosya->d_name, ".") != 0) && (strcmp(dosya->d_name, "..") != 0 )) 
		{        
			sprintf(fname, "%s/%s", path, dosya->d_name); //dosya ismini fname'e attim
			//fprintf(stderr, "%s\n", fname);

			iSizeFname=strlen(fname);
			if( fname[iSizeFname-1] != '~'  )
			{   
			    #ifndef DEBUG
				    puts(fname);
			    #endif
				if (stat(fname, &status) == -1) 
				{                            
					perror("stat");                                  
					break;
				}
				if (S_ISDIR(status.st_mode))   //File OR Directory kontrolu
				{		                       // Directory ise islem yapilmaktadir.
				    if(iIs_z != 0) // "-z" komutunun girildiginde add subdirectory
				    {
				        iSizeOfDir = 0;
			        	iSizeOfDir = pathfun(fname);
			        	
			        	char sString[MAX_PATH_SIZE];
					    sprintf(sString, "%s", fname);
				        
					    if(isInSameDirectory(sOldFname, sString) == 1) // ayni klasor icindeki klasorlerin boyutlari toplanmamali
					        iPrintSize -= iOldSizeOfDir;
					    
					    if(iSizeOfTotalDir + iPrintSize > 0) // en icteki klasore kadar ilk deger 0 olmaliydi
				            iPrintSize -= iOldSizeOfDir;
				        
				        iSizeOfTotalDir = iSizeOfDir + postOrderApply(fname, pathfun);

					    fprintf(stderr, "%d\t\t%s\n", iSizeOfTotalDir + iPrintSize, fname);
				        
					    iOldSizeOfDir = iSizeOfDir;
					    sprintf(sOldFname, "%s", fname);
				    }
				    else // "-z girilmediginde don't add subdirectory"
				    {	
				        iSizeOfDir = 0;
				        iSizeOfDir = pathfun(fname);
				        postOrderApply(fname, pathfun);
				    	//fprintf(stderr, "2-DIR NAME: %s\n", fname);
				    	fprintf(stderr, "%d\t\t%s\n", iSizeOfDir, fname);
				    }	
				}																								
			}	
		}
	}
	
	closedir(dir);
	return iSizeOfTotalDir;
}


/*
 * Directory icindeki dosyalarin boyutlarinin toplamini buluyor.
 * link ya da fifo gibi ozel bir file'a denk gelindiginde
 * dosyanın ozel oldugu yazilarak boyutu 0 kabul ediliyor.
 *
 * input: 
 *  path: directory name
 * output:
 *  int: total size of files in directory
 */
int sizepathfun (char *path)
{
    DIR *dir;
    struct dirent *dit;
    struct stat st;

    int iSizeFilePath = 0;
    int iSizeOfFile = 0;

    char filePath[MAX_PATH_SIZE];

    if ((dir = opendir(path)) == NULL) 
    {
		//perror("opendir");
		return 0;
	}
    
    while ((dit = readdir(dir)) != NULL)
    {
        if ((strcmp(dit->d_name, ".") != 0) && (strcmp(dit->d_name, "..") != 0 )) 
		{        
			sprintf(filePath, "%s/%s", path, dit->d_name);
			//lstat kullanildi, cunku ozel dosyalar boyle gorulebilmektedir.
            if (lstat(filePath, &st) == -1) 
			{                            
				perror("stat");                                  
				break;
			}

			iSizeFilePath=strlen(filePath);
			if(filePath[iSizeFilePath-1] != '~')
			{ 
                if(!S_ISDIR(st.st_mode))
                {
			        if(S_ISLNK(st.st_mode) || S_ISFIFO(st.st_mode))
			            fprintf(stderr, "Special file\t%s\n", filePath);
		            else
		            {
                        #ifndef DEBUG
                            fprintf(stderr, "\tFile: %s --- Size: %d\n", filePath, (int)st.st_size);
                        #endif
                        iSizeOfFile += st.st_blksize;
                    }
                }
            }
        }
    }
    
    #ifndef DEBUG
        fprintf(stderr, "\n\tDir: %s --- Size: %d\n\n", path, iSizeOfFile);
    #endif
    
    closedir(dir);
    return iSizeOfFile;
}


/*
 * Iki path'in ayni directory icinde olup olamdigini buluyor.
 * Gelen iki path icindeki "/" karakteri sayisi bulundu.
 * Gelen iki path icinde de ayni degerde "/" karakteri varsa 
 * ayni directory icinde yer aldiklari dogrulanmis olur.
 *
 * input: 
 *  sString1: old directory name
 *  sString2: new directory name
 * output:
 *  int: returns 1 if they are in the same directory
 *       returns 0 if they are not in the same directory
 */
int isInSameDirectory(char *sString1, char *sString2)
{
    #ifndef DEBUG
        fprintf(stderr, "\n\nsString1: %s ---- sString2: %s\n", sString1, sString2);
    #endif
    
    const char s[2] = "/";
    int iCount1 = 0, iCount2 = 0;
    char *sToken1, *sToken2;
    
    sToken1 = strtok(sString1, s);
    while(sToken1 != NULL) 
    {
        iCount1++;
        sToken1 = strtok(NULL, s);
    }
    
    sToken2 = strtok(sString2, s);
    while(sToken2 != NULL) 
    {
        iCount2++;
        sToken2 = strtok(NULL, s);
    }
    
    #ifndef DEBUG
        fprintf(stderr, "iCount1: %d ---- iCount2:%d\n\n", iCount1, iCount2);
    #endif
    
    if(iCount1 == iCount2)
        return 1;
    return 0;    
}


/*
 * Yanlis derleme sonrasi kullaniciya dogrusu gosterilmektedir
 */
void usage()
{
    fprintf(stderr, "Usage:\n");
	fprintf(stderr, "-----------------------------------\n");
	fprintf(stderr, "\t./buNeDu \"filename\" \n \t\tOR \n\t./buNeDu -z \"filename\"\n");
	fprintf(stderr, "-----------------------------------\n");
}
/******************************************************************************/

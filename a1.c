#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define false 0
#define true 1
 

char *permissionRights(char *path)
{
	struct stat buf;
    char *permiss = malloc(sizeof(char) * 9 + 1);

    if(lstat(path, &buf) == -1)
    {
        printf("\nError: failed to obtain more details about %s\n", path);
    } 

    lstat(path, &buf);
    
    mode_t rights = buf.st_mode;

    if (rights & S_IRUSR)
       	permiss[0] = 'r';
    else
       	permiss[0] = '-';

    if (rights & S_IWUSR)
       	permiss[1] = 'w';
    else
       	permiss[1] = '-';

    if(rights & S_IXUSR)
       	permiss[2] = 'x';
    else 
       	permiss[2] = '-';

    if(rights & S_IRGRP)
       	permiss[3] = 'r';
    else
       	permiss[3] = '-';

    if(rights & S_IWGRP)
       	permiss[4] = 'w';
    else
       	permiss[4] = '-';

    if(rights & S_IXGRP)
       	permiss[5] = 'x';
    else 
       	permiss[5] = '-';

    if(rights & S_IROTH)
       	permiss[6] = 'r';
    else 
      	permiss[6] = '-';

    if(rights & S_IWOTH) 
       	permiss[7] = 'w';
    else 
       	permiss[7] = '-';

    if(rights & S_IXOTH)
       	permiss[8] = 'x';
    else
       	permiss[8] = '-';

    permiss[9] = '\0';

    free (permiss); 

    return permiss;    
}

_Bool permissionCriteria(char* permission, char*path)
{
	char *filePermission = permissionRights(path);
	if(strcmp(permission, filePermission)==0)
		return true;
	else return false;
}

_Bool nameCriteria(char* name_ends_with, char* entry)
{
	
    if (strcmp(entry+(strlen(entry)-strlen(name_ends_with)), name_ends_with) == 0) 
    	return true;
    else return false;  	
}

void parse(char *file_path)
{
	char MAGIC[4]="7ODM";
    int SECT_TYPEE[7]  = {23,88,84,23,89,20,71};
	char magic[4];
	short int header_size=0;
	int no_of_sections=0;
	int version=0;
	int no=0;

	char SECT_NAME[6];
	int SECT_TYPE = 0;
	//int SECT_OFFSET = 0;
	int SECT_SIZE = 0;

	int file;
	file = open(file_path, O_RDONLY);

	if ( file == -1)
	{
		printf("ERROR opening the file\n");
		exit(5);
	}

	lseek(file, -4, SEEK_END);
	read(file, &magic, 4);

	if(strncmp(magic,MAGIC,4) != 0)
	{
		printf("ERROR\nwrong magic\n");
		exit(6);
	}

	lseek(file, -6, SEEK_END);
	read(file, &header_size, 2);

	lseek(file, -header_size, SEEK_END);
	read(file, &version, 2);
	
	read(file, &no_of_sections, 1);

	if (version < 31 || version > 51)
	{
		printf("ERROR\nwrong version\n");
		exit(7);
	}

	if (no_of_sections < 3 || no_of_sections > 18)
	{
		printf("ERROR\nwrong sect_nr \n");
		exit(8);
	}
	
	for(int i=0; i<no_of_sections; i++)
	{
		_Bool ok=false;
		lseek(file, -header_size+3+18*i, SEEK_END);
		read(file, &SECT_NAME, 6);
		read(file, &SECT_TYPE, 4);
		//read(file, &SECT_OFFSET, 4);
		lseek(file, 4, SEEK_CUR);
		read(file, &SECT_SIZE, 4);
		
		for(int j=0; j<7; j++)
			if (SECT_TYPEE[j] == SECT_TYPE)
				ok = true;
		if (ok==true)
			no++;
	}
	 
	if(no!=no_of_sections)
	{	
		printf("ERROR\nwrong sect_types\n");
		exit(9);
	}
	else 
		if(no==no_of_sections)
		{    printf("SUCCESS\n");
			 printf("version=%d\n",version);
			 printf("nr_sections=%d\n", no_of_sections);
			for(int i=0; i<no_of_sections; i++)
			{	
				lseek(file, -header_size+3+18*i, SEEK_END);
				read(file, &SECT_NAME, 6);
				read(file, &SECT_TYPE, 4);
				//read(file, &SECT_OFFSET, 4);
				lseek(file, 4, SEEK_CUR);
				read(file, &SECT_SIZE, 4);
				printf("section%d: %s %d %d\n", i+1, SECT_NAME, SECT_TYPE, SECT_SIZE);				
			}
		}	
}


void listDir (char* dir_path, char *name_ends_with, _Bool nameC, char *permission, _Bool permissionC)
{
	DIR* directory;
    struct dirent *dirEntry;
    char name[257];
    char *path = (char *)malloc(sizeof(char) * 100);

    directory = opendir(dir_path);
    if (directory == 0) {
        printf("Error opening the directory; invalid pathname\n");
        free(path);
        exit(2);
    }
    printf("SUCCESS\n");

    while ((dirEntry = readdir(directory)) != 0) {
        
       	snprintf(name, 257, "%s/%s", dir_path, dirEntry->d_name);

        if (strcmp(dirEntry->d_name, ".")==0) 
        	continue;
        if (strcmp(dirEntry->d_name, "..")==0)
        	continue;

        if(nameC == false && permissionC == false)
        	printf("%s\n", name);

        else if (nameC == true && nameCriteria(name_ends_with, dirEntry->d_name) == true)
        	printf("%s\n", name);

        else if (permissionC == true) 
        {  
        	strcpy(path, dir_path);
        	strcat(path, "/");
        	strcat(path, dirEntry->d_name);

            if (permissionCriteria(permission, path) == true)
        		printf("%s\n", name);
        }
    }

    free(path);
    closedir(directory);
        	
}


void listDirRecursive (char* dir_path, char *name_ends_with, _Bool nameC, char *permission, _Bool permissionC)
{
	DIR* directory;
	struct dirent *dirEntry;
	char name[257];
	struct stat inode;
	char *path = (char *)malloc(sizeof(char) * 300);
	char *path1 = (char *)malloc(sizeof(char) * 300);
	_Bool crit = nameC;
	_Bool critP = permissionC;
	char* name_ends = name_ends_with;
	char* rights = permission;

	directory = opendir(dir_path);
	 if (directory == 0) {
        printf("Error opening the directory; invalid pathname\n");
        free(path);
 		exit(3);
    }
    

    while ((dirEntry = readdir(directory)) != 0) {

    	snprintf(name, 257, "%s/%s", dir_path, dirEntry->d_name);

        if (strcmp(dirEntry->d_name, ".")==0) 
        	continue;
        if (strcmp(dirEntry->d_name, "..")==0)
        	continue;

    	lstat(name, &inode);

    	if (S_ISREG(inode.st_mode))
    	{
    		
        	if(nameC == false && permissionC == false)
        		printf("%s\n", name);

        	else if (nameC == true && nameCriteria(name_ends_with, dirEntry->d_name) == true)
        		printf("%s\n", name);
        	
        	if (permissionC == true)
        		{

        			strcpy(path1, dir_path);
        			strcat(path1, "/");
        			strcat(path1, dirEntry->d_name);

            		if (permissionCriteria(permission, path1) == true)
        				printf("%s\n", name);
        		}
        		
        }
    		
    	else if (S_ISDIR(inode.st_mode))
            {
            	
                strcpy(path, dir_path);
                strcat(path, "/");
                strcat(path, dirEntry->d_name);
                
                if(nameC == false && permissionC == false)
        	    	printf("%s\n", name);

       			else if (nameC == true && nameCriteria(name_ends_with, dirEntry->d_name) == true)
       	       		printf("%s\n", name);
             	 
       		    if (permissionC == true && permissionCriteria(permission, path) == true)
        			printf("%s\n", name);
        		

                listDirRecursive(path, name_ends, crit, rights, critP);
                

            }
    }

   free(path);
   free(path1);
   closedir(directory);
   
}
int main(int argc, char **argv){
  
    char *dir_path;
    char *name_ends_with;
    char *permission;
  
    
    if(argc >= 2){

        if (strcmp(argv[1], "variant") == 0){
            printf("82817\n");
        }
        
    
        else if (strcmp(argv[1], "list")==0){
        	if (argc < 3)
        		printf("You need to specify at least the pathname\n Usage: %s list  path=<path_name>\n", argv[0]);

        	else {

        		if (argc == 3) //listare simpla
        		{

        		dir_path = argv[2]+5;
        		listDir(dir_path, "", false, "", false);

        	    }

        	    if(argc == 4) //listare recursiva sau listare simpla cu un criteriu de filtarre
        	    {
        	    	if (strcmp(argv[2], "recursive")==0) //listare recursiva
        	    	{
        	    		dir_path = argv[3]+5;
        	    		printf("SUCCESS\n");
        	    		listDirRecursive(dir_path, "", false, "", false);
        	    			
        	    	}
        	    	
        	    	else if (strncmp(argv[2], "name_ends_with=", 15)==0) //listare simpla cu criteriu de filtrare name_ends_with
        	    	{
        	    		dir_path = argv[3]+5;
        	    		name_ends_with = argv[2]+15;
        	    		listDir(dir_path, name_ends_with, true, "", false);
        	    		
        	    	}

        	    	else if(strncmp(argv[2], "permissions=", 12)==0)  //listare simpla cu criteriu de filtrate permission
        	    	{
        	    		dir_path = argv[3]+5;
        	    		permission = argv[2]+12;
        	    		listDir(dir_path, "", false, permission, true);

        	    	}
        	    }

        	    if (argc == 5) //listare recursiva cu criterii de filtrare
        	    {
        	    	if(strcmp(argv[2], "recursive")==0 && strncmp(argv[3], "name_ends_with=", 15)==0) //listare recursiva cu criteriu de filtrare name_ends_with (recursive name_ends_with)
        	    	{
        	    		dir_path = argv[4]+5;
        	    		name_ends_with = argv[3]+15;
        	    		printf("SUCCES\n");
        	    		listDirRecursive(dir_path, name_ends_with, true, "", false);
        	    	}

        	    	else if(strcmp(argv[3], "recursive")==0 && strncmp(argv[2], "name_ends_with=", 15)==0)  //listare recursiva cu criteriu de filtrare name_ends_with (name_ends_with recursive)
        	    	{

        	    		dir_path = argv[4]+5;
        	    		name_ends_with = argv[2]+15;
        	    		printf("SUCCES\n");
        	    		listDirRecursive(dir_path, name_ends_with, true, "", false);
        	   
        	    	}

        	    	else if(strcmp(argv[2], "recursive")==0 && strncmp(argv[3], "permissions=", 12)==0) //listare recursiva cu criteriu de filtrare permissions (recursive permissions)
        	    	{
        	    		dir_path = argv[4]+5;
        	    		permission = argv[3]+12;
        	    		printf("SUCCESS\n");
        	    		listDirRecursive(dir_path, "", false, permission, true);
        	    	}

        	    	else if(strcmp(argv[3], "recursive")==0 && strncmp(argv[2], "permissions=", 12)==0) //listare recursiva cu criteriu de filtrare permissions (permissions recursive)
        	    	{
        	    		dir_path = argv[4]+5;
        	    		permission = argv[2]+12;
        	    		printf("SUCCESS\n");
        	    		listDirRecursive(dir_path, "", false, permission, true);
        	    	} 
        	    }

        	}
        }

        else if (strcmp(argv[1], "parse")==0)
        {
        	dir_path=argv[2]+5;
        	parse(dir_path);
        }
    }
    else{
    	printf("\nError\n");
    	exit(1);
    }

    return 0;
}

// total is block usage of listed files

// ls -l
// - file d directory l link, etc etc
// file_permissions(owner-group-everyone)
// number of links
// owner
// group
// size
// modification date: month-day[-year] time
// filename

#include <stdio.h>
#include <dirent.h>
#include <limits.h>

#include <sys/stat.h>
#include <string.h>
#include <time.h>

#include <pwd.h>
#include <grp.h>

void fmode_symbol(mode_t st_mode, char *ftype)
{

	switch (st_mode & S_IFMT) {

		case S_IFLNK:
				*ftype = 'l';	// link
			break;

		case S_IFDIR:
				*ftype = 'd';	// directory
			break;
				
		case S_IFREG:
				*ftype = '-';	// file
			break;
		
		case S_IFSOCK:
				*ftype = 's';	// socket
			break;
		
		case S_IFBLK:
				*ftype = 'b';	// block dev
			break;

		case S_IFCHR:
				*ftype = 'c';	// char dev
			break;

		case S_IFIFO:
				*ftype = 'p';	// fifo, pipe
			break;

		default:
				*ftype = '?';
	}
}

int	fill_permis(mode_t st_mode, char *permbuf)
{
	permbuf[0] = (st_mode & S_IRUSR)? 'r' : '-';
	permbuf[1] = (st_mode & S_IWUSR)? 'w' : '-';
	permbuf[2] = (st_mode & S_IXUSR)? 'x' : '-';

	permbuf[3] = (st_mode & S_IRGRP)? 'r' : '-';
	permbuf[4] = (st_mode & S_IWGRP)? 'w' : '-';
	permbuf[5] = (st_mode & S_IXGRP)? 'x' : '-';

	permbuf[6] = (st_mode & S_IROTH)? 'r' : '-';
	permbuf[7] = (st_mode & S_IWOTH)? 'w' : '-';
	permbuf[8] = (st_mode & S_IXOTH)? 'x' : '-';

	permbuf[9] = 0;
}

#define	SIX_MONTHS	(6*30*86400)
void filetime(time_t *filetime)
{
	struct tm *ftime;
	char buffer[200];
	
	ftime = localtime(filetime);

	if (difftime(time(NULL), *filetime) > SIX_MONTHS) {
		strftime(buffer, sizeof(buffer), "%b %d %Y", ftime);
	} else {
		strftime(buffer, sizeof(buffer), "%b %d %H:%M", ftime);
	}

	printf("%s\t", buffer);
}

int main(int argc, char *argv[])
{
	DIR *curdir;
	struct dirent *rec;
    struct stat file_attr;
	int total = 0;

	char dirname[PATH_MAX] = "./";
	char fname[NAME_MAX];
	int	dnamelen;

	char attrbuf[16];

	if (argc > 1) {	// dirname in argv[1]
		strncpy(dirname, argv[1], sizeof dirname);
	}

	dnamelen = strlen(dirname);

	if ((dnamelen > 0) && (dnamelen < (sizeof dirname)-2) && (dirname[dnamelen-1] != '/')) {
		
		dirname[dnamelen++] = '/';
		dirname[dnamelen] = 0;
	}

	strncpy(fname, dirname, sizeof fname);
	curdir = opendir(dirname);

	if (curdir) {
		while (rec = readdir(curdir)) {
			if ( *((char*)rec->d_name) != '.') {	// no hidden files, . and ..

				fname[dnamelen] = 0;			// cut the directory path
				strcat(fname, rec->d_name);		// change to strncat

			    if ((lstat(fname, &file_attr)) == -1) {
			    	perror("fstat");
			    	continue;
			    }

			    fmode_symbol(file_attr.st_mode, attrbuf);
			    fill_permis(file_attr.st_mode, &attrbuf[1]);

			    printf("%s\t", attrbuf);

			    // printf("%x\t", file_attr.st_mode);

				printf("%ld\t", file_attr.st_nlink);	// number of links

				{
					struct passwd *uname = getpwuid(file_attr.st_uid);
					struct group *gname = getgrgid(file_attr.st_gid);
					
					if (uname) {
						printf("%s\t", uname->pw_name);
					} else {
						printf("%d\t", file_attr.st_uid);	// user
					}

					if (gname) {
						printf("%s\t", gname->gr_name);
					} else {
						printf("%d\t", file_attr.st_gid);	// group
					}
				}

				printf("%8ld\t", file_attr.st_size);	// size

				// printf("%ld\t", file_attr.st_mtime);	// modified time
				filetime(&file_attr.st_mtime);

				// printf("%ld\t", file_attr.st_blksize);

				printf("%s\n", rec->d_name);	// name

				total +=  file_attr.st_blocks;
				
				/*
				if ((file_attr.st_mode & S_IFMT) == S_IFREG) {
					total +=  file_attr.st_blocks;
					printf("%d\n", total);
				}*/

				
			}
		}
		printf("total %d\n", total/2);	// blocks are 512B, ls counts 1K blocks
	}

	return !curdir;

}
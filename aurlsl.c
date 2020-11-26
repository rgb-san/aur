// ls -l

// total is 1K block usage of listed files
// - file d directory l link, etc etc
// file_permissions(owner-group-everyone)
// number of links
// owner
// group
// size
// modification date: month day [year | time]
// filename

#include <ctype.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>

#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <pwd.h>
#include <grp.h>

char fmode_symbol(mode_t st_mode)
{
	char mode;

	switch (st_mode & S_IFMT) {

		case S_IFLNK:
				mode = 'l';	// link
			break;

		case S_IFDIR:
				mode = 'd';	// directory
			break;
				
		case S_IFREG:
				mode = '-';	// regular file
			break;
		
		case S_IFSOCK:
				mode = 's';	// socket
			break;
		
		case S_IFBLK:
				mode = 'b';	// block dev
			break;

		case S_IFCHR:
				mode = 'c';	// char dev
			break;

		case S_IFIFO:
				mode = 'p';	// fifo, pipe
			break;

		default:
				mode = '?';
	}

	return mode;
}

int	file_permis(mode_t st_mode, char *permbuf)
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

#define	SIX_MONTHS	(6*30*86400.0)
#define	PBUFLEN	160
char	pbuf[PBUFLEN];
int 	pblen = 0;

void print_file_time(time_t *filetime)
{
	struct tm *ftime;
	
	ftime = localtime(filetime);

	if (difftime(time(NULL), *filetime) > SIX_MONTHS) {
		pblen += strftime(pbuf+pblen, PBUFLEN-pblen, "%b %d %Y", ftime);
	} else {
		pblen += strftime(pbuf+pblen, PBUFLEN-pblen, "%b %d %H:%M", ftime);
	}
}

void print_file_stats(struct stat *file_attr, char *fname)
{
	char attrbuf[16];

	attrbuf[0] = fmode_symbol(file_attr->st_mode);
	file_permis(file_attr->st_mode, &attrbuf[1]);
	pblen += snprintf(pbuf+pblen, PBUFLEN-pblen, "%s ", attrbuf);

	pblen += snprintf(pbuf+pblen, PBUFLEN-pblen, "%2ld ", file_attr->st_nlink);	// number of links

	{
		struct passwd *uname = getpwuid(file_attr->st_uid);
		struct group *gname = getgrgid(file_attr->st_gid);
		
		if (uname) {
			pblen += snprintf(pbuf+pblen, PBUFLEN-pblen, "%s ", uname->pw_name);
		} else {
			pblen += snprintf(pbuf+pblen, PBUFLEN-pblen, "%d ", file_attr->st_uid);	// user
		}

		if (gname) {
			pblen += snprintf(pbuf+pblen, PBUFLEN-pblen, "%s ", gname->gr_name);
		} else {
			pblen += snprintf(pbuf+pblen, PBUFLEN-pblen, "%d ", file_attr->st_gid);	// group
		}
	}

	pblen += snprintf(pbuf+pblen, PBUFLEN-pblen, " %8ld ", file_attr->st_size);	// size

	print_file_time(&file_attr->st_mtime);

	pblen += snprintf(pbuf+pblen, PBUFLEN-pblen, "\t%s", fname);
}

typedef struct lpout_s {
	struct lpout_s *next;
	char	*lpbuf;
	char	*key;
} lpout_t;

lpout_t	*lpout_start = NULL;

char	*str2lower(char *str, int strlen)
{
	int i;

	for (i=0; (i<strlen) && (str[i]); i++) {
		str[i] = (char)tolower(str[i]);
	}

	return str;
}

void	lpout_add(char *lpbuf, char *key)
{
	lpout_t *pnew, *p, *pp;

	pnew = malloc(sizeof(lpout_t));

	if (!pnew)
		return;

	pnew->key = malloc(strlen(key)+1);
	if (pnew->key)
		strcpy(pnew->key, key);

	pnew->lpbuf = malloc(strlen(lpbuf)+1);
	if (pnew->lpbuf)
		strcpy(pnew->lpbuf, lpbuf);

	if (!lpout_start) {	// empty list, create element
		lpout_start = pnew;
		pnew->next = NULL;
		return;
	}

	p = lpout_start;	// start iterating

	if (strcmp(pnew->key, p->key) < 0) {	// < than head, insert at start
		pnew->next = lpout_start;
		lpout_start = pnew;
		return;
	}

	do {
		pp = p;
		p = p->next;
	} while ((p) && (strcmp(pnew->key, p->key) > 0));	// until > or end of list

	pp->next = pnew;
	pnew->next = p;

}

void	lpout_print(void)
{
	lpout_t *p = lpout_start, *np;

	while (p) {
		printf("%s\n", p->lpbuf);

		np = p->next;

		// free() nodes that already been printed
		free(p->key);
		free(p->lpbuf);
		free(p);

		p = np;
	}
}

int main(int argc, char *argv[])
{
	DIR *curdir;

	struct dirent *rec;
    struct stat file_attr;

	int bltotal = 0;

	char dirname[PATH_MAX] = "./";
	int	dnamelen;
	char fname[NAME_MAX];

	if (argc > 1) {	// dirname in argv[1]
		strncpy(dirname, argv[1], sizeof dirname);
	}

	dnamelen = strlen(dirname);

	if ((dnamelen > 0) && (dnamelen < (sizeof dirname)-2) && (dirname[dnamelen-1] != '/')) {
		
		dirname[dnamelen++] = '/';		// append trailing '/' if missed
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

			    pblen = 0;
				print_file_stats(&file_attr, rec->d_name);

				// printf("%s\n", pbuf);
				lpout_add(pbuf, str2lower(rec->d_name, strlen(rec->d_name)));
				bltotal +=  file_attr.st_blocks;
								
			}
		}
		printf("total %d\n", bltotal/2);	// blocks are 512B, ls counts 1K blocks
		lpout_print();

	} else {
		// try file name as parameter
	}

	return !curdir;

}

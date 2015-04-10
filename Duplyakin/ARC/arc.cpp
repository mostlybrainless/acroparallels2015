#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <sys/mman.h>
#include <errno.h>
#include <string>
#include <set>
#include "lzlib.h"

#if 0
              struct stat {
                  dev_t         st_dev;      /* устройство */
                  ino_t         st_ino;      /* inode */
                  mode_t        st_mode;     /* режим доступа */
                  nlink_t       st_nlink;    /* количество жестких ссылок */
                  uid_t         st_uid;      /* ID пользователя-владельца */
                  gid_t         st_gid;      /* ID группы-владельца */
                  dev_t         st_rdev;     /* тип устройства */
                                             /* (если это устройство) */
                  off_t         st_size;     /* общий размер в байтах */
                  unsigned long st_blksize;  /* размер блока ввода-вывода */
                                             /* в файловой системе */
                  unsigned long st_blocks;   /* количество выделенных блоков */
                  time_t        st_atime;    /* время последнего доступа */
                  time_t        st_mtime;    /* время последней модификации */
                  time_t        st_ctime;    /* время последнего изменения */
              };
#endif
class file_info
{
public:
    std::string	  path;
    mode_t        mode;
    off_t         size;
    time_t        atime;    /* время последнего доступа */
    time_t        mtime;    /* время последней модификации */
    time_t        ctime;    /* время последнего изменения */
    bool operator < (const file_info & f) const
    {
	return path < f.path;
    };
    bool operator == (const file_info & f) const
    {
	return path == f.path;
    };
    void assign(const std::string & s, struct stat * pst)
    {
	path = s;
	if (pst)
	{
	    mode = pst->st_mode;
	    size = pst->st_size;
	    atime = pst->st_atime;
	    mtime = pst->st_mtime;
	    ctime = pst->st_ctime;
	}
    };
    file_info (const std::string & s,struct stat * pst = NULL)
    {
	assign(s,pst);
    };
    file_info (const char * s,struct stat * pst = NULL)
    {
	assign(std::string(s),pst);
    };
    file_info (const file_info & f):
	path(f.path),
	mode(f.mode),
	size(f.size),
	atime(f.atime),
	mtime(f.mtime),
	ctime(f.ctime)
    {
    }
    file_info(FILE * fi)
    {
	int len;
	char buf[PATH_MAX];
#define GET(a) fread(&a,sizeof(a),1,fi)
	GET(len);
	fread(buf,1,len,fi);
	buf[len]=0;
	path = buf;
	GET(mode);
	GET(size);
	GET(atime);
	GET(mtime);
	GET(ctime);
#undef GET
    }
    virtual void write(FILE * fo) const
    {
	int len=path.length();
#define PUT(a) fwrite(&a,sizeof(a),1,fo)
	PUT(len);
	fwrite(path.c_str(),1,len,fo);
	PUT(mode);
	PUT(size);
	PUT(atime);
	PUT(mtime);
	PUT(ctime);
#undef PUT
    }
};

class regfile_info: public file_info
{
public:
    regfile_info (const std::string & s,struct stat * pst = NULL):
	file_info(s,pst)
    {
    };
    regfile_info (const char * s,struct stat * pst = NULL):
	file_info(s,pst)

    {
    };
    regfile_info(const regfile_info & f):
	file_info(f)
    {
    };
    regfile_info(FILE *fi):
	file_info(fi)
    {
    };
};
class linkfile_info: public file_info
{
public:
    std::string dest;
    linkfile_info (const std::string & s,struct stat * pst = NULL):
	file_info(s,pst)
    {
	char buf[PATH_MAX];
	int len=readlink(s.c_str(),buf,PATH_MAX);
	buf[len]=0;
	dest=buf;
    };
    linkfile_info (const char * s,struct stat * pst = NULL):
	file_info(s,pst)

    {
	char buf[PATH_MAX];
	readlink(s,buf,PATH_MAX);
	dest=buf;

    };
    linkfile_info(const linkfile_info & f):
	file_info(f),
	dest(f.dest)
    {
    };
    linkfile_info(FILE *fi):file_info(fi)
    {
	int len;
	char buf[PATH_MAX];
	fread(&len,sizeof(len),1,fi);
	fread(buf,1,len,fi);
	buf[len]=0;
	dest=buf;
    }
    virtual void write(FILE * fo) const
    {
	file_info::write(fo);
	int len=dest.length();
	fwrite(&len,sizeof(len),1,fo);
	fwrite(dest.c_str(),1,len,fo);
    }
};

std::set<file_info> dirs;
std::set<regfile_info> files;
std::set<linkfile_info> links;
void add_dirs(const std::string & path)
{
   char buf[PATH_MAX];
   strncpy(buf,path.c_str(),PATH_MAX);
   for (char * p=strrchr(buf,'/');p;p=strrchr(buf,'/'))
   {
       struct stat st;
       *p=0;
       lstat(buf,&st);
       dirs.insert(file_info(buf,&st));
   }
}
void add_file(const std::string & file_name)
{
    struct stat st;
    lstat(file_name.c_str(),&st);
    if (S_ISDIR(st.st_mode))
    {
	DIR * d;
	struct dirent *prd;
	dirs.insert(file_info(file_name,&st));
	d=opendir(file_name.c_str());
	for (prd=readdir(d);prd;prd=readdir(d))
	    if (strcmp(prd->d_name,".")
		&& strcmp(prd->d_name,"..") &&
		(
		    prd->d_type == DT_REG ||
		    prd->d_type == DT_DIR ||
		    prd->d_type == DT_LNK
		))
		add_file(file_name+"/"+std::string(prd->d_name));
    }

    if (S_ISLNK(st.st_mode))
    {
	add_dirs(file_name);
	links.insert(linkfile_info(file_name,&st));
    }
    if (S_ISREG(st.st_mode))
    {
	add_dirs(file_name);
	files.insert(regfile_info(file_name,&st));
    }
}
void print_all()
{
    printf("DIRS:\n");
    for (std::set<file_info>::const_iterator it=dirs.begin();it!=dirs.end();it++)
    {
	printf("%s,%d\n",it->path.c_str(),it->mode);
    }
    printf("FILES:\n");

    for (std::set<regfile_info>::const_iterator it=files.begin();it!=files.end();it++)
    {
	printf("%s,%d\n",it->path.c_str(),it->mode);
    }
    printf("LINKS:\n");
    for (std::set<linkfile_info>::const_iterator it=links.begin();it!=links.end();it++)
    {
	printf("%s->%s,%d\n",it->path.c_str(),it->dest.c_str(),it->mode);
    }

}

void save_file(FILE * fo,const std::string & path)
{
    FILE *fi=fopen(path.c_str(),"r");
    struct stat st;
    fstat(fileno(fi),&st);
    void * mem_in=mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,fileno(fi),0);
    long file_off=ftell(fo);
    fflush(fo);
    ftruncate(fileno(fo),file_off+st.st_size);
    long sc_ps=sysconf(_SC_PAGE_SIZE);
    long real_off=(file_off/sc_ps)*sc_ps;
    long off_delta=file_off-real_off;
    unsigned char * mem_out=(unsigned char *)mmap(NULL,st.st_size+off_delta,PROT_WRITE,MAP_SHARED,fileno(fo),real_off);
    if (mem_out==(void *)(-1))
	perror("Error:");
    memcpy(mem_out+off_delta,mem_in,st.st_size);
    munmap(mem_in,st.st_size);
    munmap(mem_out,st.st_size+off_delta);
    fclose(fi);
    fseek(fo,file_off+st.st_size,SEEK_SET);
}

bool restore_file(FILE * fi,const file_info & fl)
{
    FILE *fo=fopen(fl.path.c_str(),"w+");
    utimbuf ub={fl.atime,fl.mtime};
    void * mem_out=mmap(NULL,fl.size,PROT_WRITE,MAP_SHARED,fileno(fo),0);
    fchmod(fileno(fo),fl.mode&07777);
    long file_off=ftell(fi);
    ftruncate(fileno(fo),fl.size);
    long sc_ps=sysconf(_SC_PAGE_SIZE);
    long real_off=(file_off/sc_ps)*sc_ps;
    long off_delta=file_off-real_off;
    unsigned char * mem_in=(unsigned char *)mmap(NULL,fl.size+off_delta,PROT_READ,MAP_PRIVATE,fileno(fi),real_off);
    if (mem_in==(void *)(-1))
	return false;
    memcpy(mem_out,mem_in+off_delta,fl.size);
    munmap(mem_in,fl.size+off_delta);
    munmap(mem_out,fl.size);
    fclose(fo);
    utime(fl.path.c_str(),&ub);
    fseek(fi,file_off+fl.size,SEEK_SET);
}

void save_data(FILE *fo)
{
    int len;
    len = dirs.size();
    fwrite(&len,sizeof(len),1,fo);
    for (std::set<file_info>::const_iterator it=dirs.begin();it!=dirs.end();it++)
	it->write(fo);
    len = files.size();
    fwrite(&len,sizeof(len),1,fo);

    for (std::set<regfile_info>::const_iterator it=files.begin();it!=files.end();it++)
	it->write(fo);
    len = links.size();
    fwrite(&len,sizeof(len),1,fo);

    for (std::set<linkfile_info>::const_iterator it=links.begin();it!=links.end();it++)
	it->write(fo);
    for (std::set<regfile_info>::const_iterator it=files.begin();it!=files.end();it++)
	save_file(fo,it->path);
}

void restore_data(FILE *fi)
{
    int len;
    fread(&len,sizeof(len),1,fi);
    for (int i=0;i<len;i++)
	dirs.insert(file_info(fi));
    fread(&len,sizeof(len),1,fi);
    for (int i=0;i<len;i++)
	files.insert(regfile_info(fi));
    fread(&len,sizeof(len),1,fi);
    for (int i=0;i<len;i++)
	links.insert(linkfile_info(fi));
}

bool restore_files(FILE *fi)
{
    for (std::set<file_info>::const_iterator it=dirs.begin();it!=dirs.end();it++)
    {
	struct stat st;
	utimbuf ub={it->atime,it->mtime};
	if (lstat(it->path.c_str(),&st) == 0)
	{
	    if (!S_ISDIR(st.st_mode))
		return false;
	    else
		continue;
	}
	if (errno != ENOENT)
	    return false;
	if (mkdir(it->path.c_str(),it->mode & 07777) != 0)
	    return false;
	utime(it->path.c_str(),&ub);
    }
    for (std::set<regfile_info>::const_iterator it=files.begin();it!=files.end();it++)
	if (!restore_file(fi,*it))
	    return false;
    for (std::set<linkfile_info>::const_iterator it=links.begin();it!=links.end();it++)
    {
	struct stat st;
	if (lstat(it->path.c_str(),&st) == 0)
	{
	    char buf[PATH_MAX];
	    if (!S_ISLNK(st.st_mode))
		return false;
	    unlink(it->path.c_str());
	}

	if (symlink(it->dest.c_str(),it->path.c_str())!=0)
	    return false;
    }
}
void usage()
{
    printf("Usage: arc [clx] archive [files]\n");
}
FILE * unpack(std::string & out_name,const char * fname)
{
    char tmpfname[PATH_MAX];
    int fd;
    FILE * fi=fopen(fname,"r");
    if (fi == NULL)
	return NULL;
    strcpy(tmpfname,"/tmp/arcXXXXXX");
    fd = mkstemp(tmpfname);
    if (fd < 0)
	return NULL;
    FILE * fo=fdopen(fd,"w+");
    lzdecode(fo,fi);
    out_name=tmpfname;
    return fo;
}
int arc_create(int nfiles,const char *arc,char ** files)
{
    char tmpfname[PATH_MAX];
    int fd;
    strcpy(tmpfname,"/tmp/arcXXXXXX");
    fd = mkstemp(tmpfname);
    if (fd < 0)
    {
	printf("Cannot create tempfile\n");
	return 2;
    }
    FILE * f=fdopen(fd,"w+");
    for (int i=0;i<nfiles;i++)
	add_file(files[i]);
    save_data(f);
    fseek(f,0,SEEK_SET);
    FILE * fo=fopen(arc,"w");
    if (fo == NULL)
    {
	printf("Cannot create archive\n");
	fclose(f);
	unlink(tmpfname);
	return 3;
    }
    lzencode(fo,f);
    fclose(f);
    fclose(fo);
    unlink(tmpfname);
    return 0;
}

int arc_list(const char * arc)
{
    char tmpfname[PATH_MAX];
    int fd;
    FILE * fi;
    fi=fopen(arc,"r");
    if (fi == NULL)
    {
	printf("Cannot open archive\n");
	return 2;
    }
    strcpy(tmpfname,"/tmp/arcXXXXXX");
    fd = mkstemp(tmpfname);
    if (fd < 0)
    {
	printf("Cannot create tempfile\n");
	return 3;
    }
    FILE * f=fdopen(fd,"w+");
    lzdecode(f,fi);
    fclose(fi);
    fseek(f,0,SEEK_SET);
    restore_data(f);
    print_all();
    fclose(f);
    unlink(tmpfname);
    return 0;
}

int arc_unpack(const char * arc)
{
    char tmpfname[PATH_MAX];
    int fd;
    FILE * fi;
    fi=fopen(arc,"r");
    if (fi == NULL)
    {
	printf("Cannot open archive\n");
	return 2;
    }
    strcpy(tmpfname,"/tmp/arcXXXXXX");
    fd = mkstemp(tmpfname);
    if (fd < 0)
    {
	printf("Cannot create tempfile\n");
	return 3;
    }
    FILE * f=fdopen(fd,"w+");
    lzdecode(f,fi);
    fclose(fi);
    fseek(f,0,SEEK_SET);
    restore_data(f);
    restore_files(f);
    fclose(f);
    unlink(tmpfname);
    return 0;
}

int main(int argc, char ** argv)
{
    if (argc < 3)
    {
	usage();
	return 1;
    }
    switch (argv[1][0])
    {
	case 'c':
	    if (argc < 4)
	    {
		usage();
		return 1;
	    }
	    return arc_create(argc-3,argv[2],argv+3);
	case 'l':
	    return arc_list(argv[2]);
	case 'x':
	    return arc_unpack(argv[2]);
    }
    usage();
    return 1;
}

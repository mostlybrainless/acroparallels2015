#include<stdio.h>
#include<string.h>
#include<vector>
#include<errno.h>

class write_voc
{
public:
    static const int voc_size=65535;
    class writer
    {
	public:
	virtual void put(unsigned short c) = 0;
    };
protected:
    class node
    {
    public:
	unsigned short ind[256];
	void clear()
	{
	    for (int i=0;i<256;i++)
		ind[i]=voc_size;
	};
	node()
	{
	    clear();
	};
    };
    node voc[voc_size];
    writer * pwrt;
    int last_node,cur_node;
public:
    void clear_voc()
    {
        last_node=255;
        cur_node=voc_size;
        for (int i=0;i<voc_size;i++)
            voc[i].clear();
    };

    write_voc(writer * ppw)
    {
	clear_voc();
	pwrt=ppw;
    };
    void put(unsigned short ch)
    {
	unsigned short new_node;
	if (cur_node == voc_size)
	{
	    cur_node=ch;
	    return;
	}
	new_node=voc[cur_node].ind[ch];

	if ( new_node != voc_size)
	{
	    cur_node =  new_node;
	    return;
	}

	pwrt->put(cur_node);
	last_node++;
	if (last_node == voc_size )
	{
	    clear_voc();
	    pwrt->put(ch);
	    return;
	}
        voc[cur_node].ind[ch]=last_node;
	cur_node=ch;
    };
    void finish()
    {
	pwrt->put(cur_node);
	pwrt->put(voc_size);
    }
};

class read_voc
{
public:
    static const int voc_size=65535;
    class writer
    {
	public:
	virtual void put(const char *msg,int len) = 0;
    };
protected:
    class node
    {
    public:
	unsigned short parent;
	char ch;
	int len;
	void clear()
	{
	    parent=voc_size;
	    ch=0;
	    len=1;
	};
	node()
	{
	    clear();
	};
    };
    node voc[voc_size];
    writer * pwrt;
    int last_node,cur_node;
    char cur_ch;
    void clear_voc()
    {
	last_node=255;
	cur_node=voc_size;
    };

public:

    void clear()
    {
	last_node=255;
	cur_node=voc_size;
    }
    read_voc(writer * ppw)
    {
	last_node=255;
	cur_node=voc_size;
	pwrt=ppw;
	for (int i=0;i<256;i++)
	    voc[i].ch=i;
    };

    void put(unsigned short ch)
    {
	if (cur_node == voc_size)
	{
	    cur_ch=ch;
	    pwrt->put(&cur_ch,1);
	    cur_node=ch;
	    return;
	}

	if (ch <= last_node)
	{
	    int len=voc[ch].len;
	    char buf[len];
	    int i,code;
	    for (i=len-1,code=ch;i>=0;i--,code=voc[code].parent)
		buf[i]=voc[code].ch;
	    pwrt->put(buf,len);
	    cur_ch=buf[0];
	    last_node++;
	    if (last_node == voc_size )
	    {
		clear_voc();
		cur_ch=buf[0];
	    }
	    else
	    {
		voc[last_node].parent=cur_node;
		voc[last_node].len=voc[cur_node].len+1;
		voc[last_node].ch=cur_ch;
		cur_node=ch;
	    }
	    return;
	}
	else
	{
	    int len=voc[cur_node].len+1;
	    char buf[len];
	    int i,code;
	    buf[len-1]=cur_ch;
	    for (i=len-2,code=cur_node;i>=0;i--,code=voc[code].parent)
		buf[i]=voc[code].ch;
	    pwrt->put(buf,len);
	    last_node++;
	    if (last_node == voc_size )
	    {
		clear_voc();
		cur_ch=buf[0];
	    }
	    else
	    {
		voc[last_node].parent=cur_node;
		voc[last_node].ch=cur_ch;
		voc[last_node].len=len;
		cur_ch=buf[0];
		cur_node=ch;
	    }
	    return;
	}

    };
};

class file_put_16: public write_voc::writer
{
    int buf_index;
    FILE *fo;
    static const int buf_size=2048;
    unsigned short file_buf[buf_size];
public:
    class exception
    {
    public:
	int err;
	exception(int aerr):err(aerr) {};
    };
    file_put_16(FILE * f)
    {
	fo=f;
	buf_index=0;
    }
    virtual void put(unsigned short ch)
    {
	file_buf[buf_index++]=ch;
	if (buf_index == buf_size)
	{
	    fwrite(file_buf,2,buf_size,fo);
	    buf_index=0;
	}
    }
    virtual void finish()
    {
	fwrite(file_buf,2,buf_index+1,fo);
    }
};

class file_get_16
{
    FILE *fi;
    int nread,buf_index;
    bool even;
    static const int buf_size=2048;
    unsigned short file_buf[buf_size];
public:
    class exception
    {
    public:
	int err;
	exception(int aerr):err(aerr) {};
    };
    file_get_16(FILE * f)
    {
	fi=f;
	even=true;
	buf_index=0;
	nread=0;
    }
    unsigned short get()
    {
	unsigned short code;
	if (nread == 0)
	{
	    nread = fread(file_buf,2,buf_size,fi);
	    if (nread == 0)
		return 0xffff;
	    buf_index=0;
	}
	code=file_buf[buf_index++];
	nread--;
	return code;
    };
};

class file_put: public read_voc::writer
{
    FILE * fo;
public:
    file_put(FILE *ffo = stdout)
    {
	fo=ffo;
    };
    virtual void put(const char *msg,int len)
    {
	fwrite(msg,len,1,fo);
    };
};

int lzencode(FILE * fo, FILE * fi)
{
    int c;
    file_put_16 lzout(fo);
    write_voc * plzenc=new write_voc(&lzout);
    while ((c=fgetc(fi))!=-1)
	plzenc->put(c);
    plzenc->finish();
    lzout.finish();
    delete plzenc;
    return 0;
}

int lzdecode(FILE *fo, FILE *fi)
{
    int c;
    file_get_16 lzin(fi);
    file_put file_out(fo);
    read_voc * plzenc=new read_voc(&file_out);
    while (1)
    {
	c = lzin.get();
	if (c == read_voc::voc_size)
	    break;
	plzenc->put(c);
    }
    delete plzenc;
    return 0;
}



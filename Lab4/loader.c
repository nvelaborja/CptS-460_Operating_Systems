    #define BLK 1024

/*************** #include "ext2.h" ******************/
struct ext2_inode {
    u16 i_mode;                 /* File mode */
    u16 i_uid;                  /* Owner Uid */
    u32 i_size;                 /* Size in bytes */
    u32 i_atime;                /* Access time */
    u32 i_ctime;                /* Creation time */
    u32 i_mtime;                /* Modification time */
    u32 i_dtime;                /* Deletion Time */
    u16 i_gid;                  /* Group Id */
    u16 i_links_count;          /* Links count */
    u32 i_blocks;               /* Blocks count */
    u32 i_flags;                /* File flags */
    u32 dummy;
    u32 i_block[15];            /* Pointers to blocks */
    u32 pad[7];                 /* inode size MUST be 128 bytes */
};

struct ext2_group_desc
{
    u32 bg_block_bitmap;        /* Blocks bitmap block */
    u32 bg_inode_bitmap;        /* Inodes bitmap block */
    u32 bg_inode_table;         /* Inodes table block */
    u16 bg_free_blocks_count;   /* Free blocks count */
    u16 bg_free_inodes_count;   /* Free inodes count */
    u16 bg_used_dirs_count;     /* Directories count */
    u16 bg_pad;
    u32 bg_reserved[3];
};

struct ext2_dir_entry_2 {
    u32 inode;                  /* Inode number */
    u16 rec_len;                /* Directory entry length */
    u8  name_len;               /* Name length */
    u8  file_type;
    char name[255];             /* File name */
};

struct header{
    u32 ID_space; 
    u32 magic_number;           // 0x00000020
    u32 tsize;                  // code section size in bytes
    u32 dsize;                  //initialized data section size in bytes
    u32 bsize;                  // bss section size in bytes
    u32 total_size;             // total memory size, including heap
    u32 symbolTable_size;       // only if symbol table is present
};

typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;
typedef struct header           HEADER;
u16 NSEC = 2;
char buf1[BLK], buf2[BLK];      // 2 I/O buffers of 1KB each

int getblk(u16 blk, char *buf)
{
    readfd( blk / 18, (( 2 * blk ) % 36) / 18, (( 2 * blk ) % 36) % 18, buf);
}

u16 search(INODE *ip, char *name)
{
    int i; char c; DIR *dp;
    for (i=0; i<12; i++)                        // assume a DIR has at most 12 direct blocks
    {
        if ( (u16)ip->i_block[i])
        {
            getblk((u16)ip->i_block[i], buf2);
            dp = (DIR *)buf2;

            while ((char *)dp < &buf2[BLK])
            {
                c = dp->name[dp->name_len];     // save last byte
                dp->name[dp->name_len] = 0;     // make name into a string

                if (strcmp(dp->name, name) == 0)
                    return ((u16)dp->inode);

                dp->name[dp->name_len] = c;     // restore last byte
                dp = (DIR *)((char *)dp + dp->rec_len);
            }
        }
    }
    error();                                    // to error() if can't find file
}

int load(char* filename, int segment)
{
    char *cp, *name[10];
    u16 i, j, ino, blk, iblk, w[64], word;
    int nnames = 0, namelen = 0;
    int tsize, dsize, bsize;
    int segment_hremoved;
    u32 *up;
    GD *gp;
    INODE *ip;
    DIR *dp;
    HEADER *h;

    if (!tokenize(filename, name, &nnames))
    {
        printf("load: Invlaid file name %s", filename);
        return -1;
    }

    //filesystem
    getblk(2, buf1); // read blk#2 to get group descriptor 0
    gp = (GD *)buf1;
    iblk = (u16)gp->bg_inode_table; // inodes begin block

    getblk(iblk, buf1);             // read first inode block
    ip = (INODE *)buf1 + 1;         // ip->root inode #2

    // search for file
    for (i = 0; i < nnames; i++)
    {
        ino = search(ip, name[i]) - 1;
        if (ino < 0) error();       // if search() returned 0
        getblk(iblk+(ino/8), buf1); // read inode block of ino
        ip = (INODE *)buf1 + (ino % 8);
    }

    // read file header to get tsize, dsize, and bsize // pg 143
    getblk((u16)ip->i_block[0], buf2);
    h = (HEADER *)buf2; // set h as the header
    tsize = h->tsize;
    dsize = h->dsize;
    bsize = h->bsize;

    if ((u16)ip->i_block[12]) // read indirect block into buf2, if any
        getblk((u16)ip->i_block[12], buf2); // load boot image into buf2

    printf("Loading %s to segment %x ... ", filename, segment);
    setes(segment);            // load ES to loading segment

    // load direct blocks
    for (i=0; i<12; i++)
    {
        getblk((u16)ip->i_block[i], 0);
        inces(); //putc('*'); // show a * for each direct block loaded
    }

    // load indirect blocks, if any
    if ((u16)ip->i_block[12])
    {
        up = (u32 *)buf2;
        while(*up++)
        {
            getblk((u16)*up, 0);
            inces(); //putc('.');  // show a . for each ind block loaded
        }
    }

    /* load [code|data] sections of filename to memory segment */
    i =0;
    // because in hex adding 2 is moving forward 2 16-bytes
    // ex. if segment = 0x2000, adding 2 will be (2002)base 16
    segment_hremoved = segment + 2;
    while(i <= tsize + dsize)
    {
        word = get_word(segment_hremoved, i); // get word gets 2 bytes at a time
        put_word(word, segment, i);    // therefore set 2 bytes
        i+=2;
    }

    // clear bss section of loaded image to 0;
    for (i = 0; i < bsize; i += 2)
        put_word(0, segment, i + 16); // 16 because not going by bytes -> tsize + dsize = 32 bytes, put_word puts 2 bytes at a time

    setes(0x1000);  // go back to kernel mode
    printf("done\n");
    return 1;
}

/// Tokenize path and put tokens into argv, and token count in argc
int tokenize(char *path, char *argv[], int *argc)
{
  int i, nnames = 0;
  char *cp;
  cp = path;
  
  while (*cp != 0)
  {
    while (*cp == '/') 
      *cp++ = 0; 

    if (*cp != 0)
      argv[nnames++] = cp; 

    while (*cp != '/' && *cp != 0) 
      cp++;          

    if (*cp != 0)   
      *cp = 0;                   
    else 
      break; 

    cp++;
  }

  if(nnames) return 1;
  return 0;
}
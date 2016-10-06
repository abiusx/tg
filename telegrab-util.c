void file_put_contents(const char * filename,const char *content)
{
    FILE *fp=fopen(filename,"wt");
    fwrite(content,sizeof(char), strlen(content),fp);
    fclose(fp);
}
#define isutf(c) (((c)&0xC0)!=0x80)
#define file_put_content(...) file_put_contents(__VA_ARGS__);

static const u_int32_t offsetsFromUTF8[6] = {
    0x00000000UL, 0x00003080UL, 0x000E2080UL,
    0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

static const char trailingBytesForUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/* returns length of next utf-8 sequence */
int u8_seqlen(char *s)
{
    return trailingBytesForUTF8[(unsigned int)(unsigned char)s[0]] + 1;
}

/* conversions without error checking
   only works for valid UTF-8, i.e. no 5- or 6-byte sequences
   srcsz = source size in bytes, or -1 if 0-terminated
   sz = dest size in # of wide characters

   returns # characters converted
   dest will always be L'\0'-terminated, even if there isn't enough room
   for all the characters.
   if sz = srcsz+1 (i.e. 4*srcsz+4 bytes), there will always be enough space.
*/
int u8_toucs(u_int32_t *dest, int sz, char *src, int srcsz)
{
    u_int32_t ch;
    char *src_end = src + srcsz;
    int nb;
    int i=0;

    while (i < sz-1) {
        nb = trailingBytesForUTF8[(unsigned char)*src];
        if (srcsz == -1) {
            if (*src == 0)
                goto done_toucs;
        }
        else {
            if (src + nb >= src_end)
                goto done_toucs;
        }
        ch = 0;
        switch (nb) {
            /* these fall through deliberately */
        case 3: ch += (unsigned char)*src++; ch <<= 6;
        case 2: ch += (unsigned char)*src++; ch <<= 6;
        case 1: ch += (unsigned char)*src++; ch <<= 6;
        case 0: ch += (unsigned char)*src++;
        }
        ch -= offsetsFromUTF8[nb];
        dest[i++] = ch;
    }
 done_toucs:
    dest[i] = 0;
    return i;
}
/* reads the next utf-8 sequence out of a string, updating an index */
u_int32_t u8_nextchar(char *s, int *i)
{
    u_int32_t ch = 0;
    int sz = 0;

    do {
        ch <<= 6;
        ch += (unsigned char)s[(*i)++];
        sz++;
    } while (s[*i] && !isutf(s[*i]));
    ch -= offsetsFromUTF8[sz-1];

    return ch;
}

int u8_strlen(char *s)
{
    int count = 0;
    int i = 0;

    while (u8_nextchar(s, &i) != 0)
        count++;

    return count;
}

int is_rtl(u_int32_t c)
{
    if(c >= 0x5BE && c <= 0x10B7F)
    {
        if(c <= 0x85E)
        {
            if(c == 0x5BE)                        return 1;
            else if(c == 0x5C0)                   return 1;
            else if(c == 0x5C3)                   return 1;
            else if(c == 0x5C6)                   return 1;
            else if(0x5D0 <= c && c <= 0x5EA)     return 1;
            else if(0x5F0 <= c && c <= 0x5F4)     return 1;
            else if(c == 0x608)                   return 1;
            else if(c == 0x60B)                   return 1;
            else if(c == 0x60D)                   return 1;
            else if(c == 0x61B)                   return 1;
            else if(0x61E <= c && c <= 0x64A)     return 1;
            else if(0x66D <= c && c <= 0x66F)     return 1;
            else if(0x671 <= c && c <= 0x6D5)     return 1;
            else if(0x6E5 <= c && c <= 0x6E6)     return 1;
            else if(0x6EE <= c && c <= 0x6EF)     return 1;
            else if(0x6FA <= c && c <= 0x70D)     return 1;
            else if(c == 0x710)                   return 1;
            else if(0x712 <= c && c <= 0x72F)     return 1;
            else if(0x74D <= c && c <= 0x7A5)     return 1;
            else if(c == 0x7B1)                   return 1;
            else if(0x7C0 <= c && c <= 0x7EA)     return 1;
            else if(0x7F4 <= c && c <= 0x7F5)     return 1;
            else if(c == 0x7FA)                   return 1;
            else if(0x800 <= c && c <= 0x815)     return 1;
            else if(c == 0x81A)                   return 1;
            else if(c == 0x824)                   return 1;
            else if(c == 0x828)                   return 1;
            else if(0x830 <= c && c <= 0x83E)     return 1;
            else if(0x840 <= c && c <= 0x858)     return 1;
            else if(c == 0x85E)                   return 1;
        }
        else if(c == 0x200F)                      return 1;
        else if(c >= 0xFB1D)
        {
            if(c == 0xFB1D)                       return 1;
            else if(0xFB1F <= c && c <= 0xFB28)   return 1;
            else if(0xFB2A <= c && c <= 0xFB36)   return 1;
            else if(0xFB38 <= c && c <= 0xFB3C)   return 1;
            else if(c == 0xFB3E)                  return 1;
            else if(0xFB40 <= c && c <= 0xFB41)   return 1;
            else if(0xFB43 <= c && c <= 0xFB44)   return 1;
            else if(0xFB46 <= c && c <= 0xFBC1)   return 1;
            else if(0xFBD3 <= c && c <= 0xFD3D)   return 1;
            else if(0xFD50 <= c && c <= 0xFD8F)   return 1;
            else if(0xFD92 <= c && c <= 0xFDC7)   return 1;
            else if(0xFDF0 <= c && c <= 0xFDFC)   return 1;
            else if(0xFE70 <= c && c <= 0xFE74)   return 1;
            else if(0xFE76 <= c && c <= 0xFEFC)   return 1;
            else if(0x10800 <= c && c <= 0x10805) return 1;
            else if(c == 0x10808)                 return 1;
            else if(0x1080A <= c && c <= 0x10835) return 1;
            else if(0x10837 <= c && c <= 0x10838) return 1;
            else if(c == 0x1083C)                 return 1;
            else if(0x1083F <= c && c <= 0x10855) return 1;
            else if(0x10857 <= c && c <= 0x1085F) return 1;
            else if(0x10900 <= c && c <= 0x1091B) return 1;
            else if(0x10920 <= c && c <= 0x10939) return 1;
            else if(c == 0x1093F)                 return 1;
            else if(c == 0x10A00)                 return 1;
            else if(0x10A10 <= c && c <= 0x10A13) return 1;
            else if(0x10A15 <= c && c <= 0x10A17) return 1;
            else if(0x10A19 <= c && c <= 0x10A33) return 1;
            else if(0x10A40 <= c && c <= 0x10A47) return 1;
            else if(0x10A50 <= c && c <= 0x10A58) return 1;
            else if(0x10A60 <= c && c <= 0x10A7F) return 1;
            else if(0x10B00 <= c && c <= 0x10B35) return 1;
            else if(0x10B40 <= c && c <= 0x10B55) return 1;
            else if(0x10B58 <= c && c <= 0x10B72) return 1;
            else if(0x10B78 <= c && c <= 0x10B7F) return 1;
        }
    }
    return 0;
}
int u8_is_rtl(char * src)
{
    int i=0;
    u_int32_t c;
    int rtl=0;
    // printf("%s\n",src);
    while ((c=u8_nextchar(src, &i)) != 0)
    {
        rtl+=is_rtl(c);
        // printf("index %d) char:%u rtl:%d\n",i,c,rtl)  ; 
    }
    return rtl>u8_strlen(src)/2;
}
static inline void* safe_malloc(size_t n)
{
    void* p = malloc(n);
    if (!p)
      perror("malloc failure");
    return p;
}
#define xsprintf(buf,format, ...) \
    do { \
        sprintf(buf+strlen(buf),format, ##__VA_ARGS__); \
    } while(0)

void safe_mkdir(const char *dir)
{
  struct stat st = {0};
  if (stat(dir, &st) == -1) 
     mkdir(dir, 0700);
}
/**
 * Returns the extension of a filename, 
 * used to generate extensions for document files
 * @param  filename [description]
 * @return          [description]
 */
const char *get_extension(const char *filename)
{
  const char *dot = strrchr(filename, '.');
  if(!dot || dot == filename) return "dat";
  return dot + 1;
}
int file_exists(const char *filepath)
{
  FILE *fp=fopen(filepath,"r");
  fclose(fp);
  return fp!=0;
}
/**
 * allocates the result, have to release
 * 
 * @param  orig [description]
 * @param  rep  [description]
 * @param  with [description]
 * @return      [description]
 */
char *str_replace(const char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep
    int len_with; // length of with
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    if (!orig)
        return NULL;
    if (!rep)
        rep = "";
    len_rep = strlen(rep);
    if (!with)
        with = "";
    len_with = strlen(with);

    ins = (char *)orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}
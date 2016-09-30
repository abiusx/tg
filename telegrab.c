#define TELEGRAB 1
#include <sys/stat.h>
static inline void* safe_malloc(size_t n)
{
    void* p = malloc(n);
    if (!p)
      perror("malloc failure");
    return p;
}
void get_peer_name(char *user_name,tgl_peer_t *from_peer )
{
    // printf("0: %d\n",from_peer->flags);
    int peer_type=tgl_get_peer_type (from_peer->id);
    int id=tgl_get_peer_id (from_peer->id);
    if (peer_type==TGL_PEER_USER)
    {
      if ((from_peer->flags & TGLUF_DELETED)) {
        // printf("1\n");
        sprintf (user_name,"deleted user#%d",  id);
      } else if (!(from_peer->flags & TGLUF_CREATED)) {
        // printf("2\n");
        sprintf (user_name, "user#%d", id);
      } else if (!from_peer->user.first_name || !strlen (from_peer->user.first_name)) {
        // printf("3\n");
        sprintf (user_name, "%s_#%d", from_peer->user.last_name,id);
      } else if (!from_peer->user.last_name || !strlen (from_peer->user.last_name)) {
        // printf("4\n");
        sprintf (user_name, "%s_#%d", from_peer->user.first_name,id);
      } else {
        // printf("5\n");
        sprintf (user_name, "%s %s_#%d", from_peer->user.first_name, from_peer->user.last_name,id); 
      }
    }
    else if (peer_type==TGL_PEER_CHAT)
      sprintf(user_name, "%s", from_peer->chat.title);
    else if (peer_type==TGL_PEER_CHANNEL)
      sprintf(user_name, "%s", from_peer->channel.title);
    else if (peer_type==TGL_PEER_ENCR_CHAT)
      sprintf(user_name, "%s", from_peer->print_name);

}
#define xsprintf(buf,format, ...) \
    do { \
        sprintf(buf+strlen(buf),format, ##__VA_ARGS__); \
    } while(0)

int chatname2filename(char *filename,char* chatname);
void htmlspecialchars(char *input,char * output)
{
  unsigned long i = 0, j=0;

  while (input[i])
  {
      if (input[i] == '<')
      {
          memcpy(&output[j], "&lt;", 4);
          j += 4;
      } else if (input[i] == '>')
      {
          memcpy(&output[j], "&gt;", 4);
          j += 4;
      } else if (input[i] == '"')
      {
          memcpy(&output[j], "&quot;", 6);
          j += 6;
      } else if (input[i] == '&')
      {
          memcpy(&output[j], "&amp;", 5);
          j += 5;
      } else
      {
          output[j++] = input[i];
      }
      if (j > sizeof(output)-7)
      {
          break;
      }
      i++;
  }
  output[j] = 0;
}
void date2string(char *buf,long t)
{
  static char *monthes[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  struct tm *tm = localtime ((void *)&t);
  sprintf (buf, "[%02d:%02d:%02d %s %02d, %d] ", tm->tm_hour, tm->tm_min, tm->tm_sec, monthes[tm->tm_mon],tm->tm_mday, tm->tm_year + 1900);
 
}
void safe_mkdir(char *dir)
{
  struct stat st = {0};
  if (stat(dir, &st) == -1) 
     mkdir(dir, 0700);
}
/**
 * Download callback
 * @param TLSR    [description]
 * @param extra   [description]
 * @param success [description]
 * @param name    [description]
 */
static void download_callback (struct tgl_state *TLSR, void *extra, int success, const char *name) {
  if (!success) 
  {
    printf("fail load photo");
  }
  else
  {
    rename(name,(char*)extra);
    printf ("Saved to %s (%s)\n",(char*)extra, name);
  }
  if (extra) free(extra);
}
/**
 * Creates a temporary name to download a file.
 * @param  chat_name [description]
 * @param  extension [description]
 * @return           [description]
 */
char * create_download_file(const char *chat_name,const char *extension)
{
  char *file=safe_malloc(1024);
  sprintf(file,"grab/%s/",chat_name);
  safe_mkdir(file);
  xsprintf(file,"%d%d.%s",rand(),rand(),extension);
  return file;
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
void dump_message(struct tgl_message *M,struct in_ev *ev)
{
  char *buf=safe_malloc(1024*1024);
  buf[0]=0;

  tgl_peer_t *to_peer=tgl_peer_get (TLS, M->to_id);
  tgl_peer_t *from_peer=tgl_peer_get(TLS,M->from_id);
  int peer_type=tgl_get_peer_type (M->to_id);
  // printf("type:%d\n",peer_type);
  //dump filename
  char user_name[1024];
  get_peer_name(user_name,from_peer);
  
  char chat_name[1024];
  if (peer_type==TGL_PEER_USER)
  {
    char t[1024];
    if (M->flags & TGLMF_OUT) //outbound, use to_id
    {
      get_peer_name(t,to_peer); 
      sprintf(chat_name, "user_%s",t);
    }
    else
    {
      get_peer_name(t,from_peer); 
      sprintf(chat_name, "user_%s",t);
    }
  }
  else if (peer_type==TGL_PEER_CHAT || peer_type==TGL_PEER_ENCR_CHAT || peer_type==TGL_PEER_GEO_CHAT)
    sprintf(chat_name, "group_%s",to_peer->chat.title);
  else if (peer_type==TGL_PEER_CHANNEL)
    sprintf(chat_name, "channel_%s",to_peer->channel.title);
  else
    sprintf(chat_name, "unknown_%s",to_peer->chat.title);

  //date string
  date2string(buf+strlen(buf),M->date);

  //user
  xsprintf (buf, " %s: ", user_name);
  //TODO: append uniqueid after username, some people have same username


  //other message types
  if (tgl_get_peer_type (M->fwd_from_id) > 0)  //forward
  {
    xsprintf (buf, "{fwd from ");
    char t[1024];
    get_peer_name (t, tgl_peer_get (TLS, M->fwd_from_id));
    // printf("test:%s\n",t);
    xsprintf (buf, "%s ",t);
    date2string(buf+strlen(buf),M->date);
    xsprintf (buf, "} ");
  }
  if (M->reply_id) //reply
  {
    xsprintf (buf, "{reply to ");
    // tgl_message_id_t msg_id = M->permanent_id;
    // msg_id.id = M->reply_id;
    xsprintf(buf,"%d",M->reply_id);
    // struct tgl_message *N = tgl_message_get (TLS, &msg_id);
    // print_msg_id (ev, msg_id, N);
    // xsprintf(buf,"%s",print_permanent_msg_id(msg_id));

    xsprintf (buf, "} ");
  }
  if (M->flags & TGLMF_MENTION) //mention, only works when someone else mentions YOU in a group (or when someone replies you)
  {
    xsprintf (buf, "{mention} ");
  }
  if (M->message && strlen (M->message))  //the message
  {
    if (M->message && strlen (M->message)) 
      sprintf (buf+strlen(buf), " %s", M->message);
  }

  //MEDIA handling
  if (M->media.type != tgl_message_media_none) //media
  {
      struct tgl_message_media *ME=&M->media;
      xsprintf (buf, " MEDIA ");
      if (ME->type==tgl_message_media_photo) //PHOTO
      {

        if (ME->caption)
          xsprintf(buf,"caption:'%s' ",ME->caption);
        // printf("%d\n\n\n %p",ME->photo->sizes_num,M->media.photo);
        
        int max = -1;
        int maxi = 0;
        int i;
        for (i = 0; i < ME->photo->sizes_num; i++) {
          if (ME->photo->sizes[i].w + ME->photo->sizes[i].h > max) {
            max = ME->photo->sizes[i].w + ME->photo->sizes[i].h;
            maxi = i;
          }
        }          
        xsprintf(buf, "{photo num:%d, size:%d, width:%d, height:%d, type:%s}"
          ,ME->photo->sizes_num
          ,ME->photo->sizes[maxi].size
          ,ME->photo->sizes[maxi].w,ME->photo->sizes[maxi].h,ME->photo->sizes[maxi].type);

        char *image_file=create_download_file(chat_name,"jpg");
        tgl_do_load_file_location(TLS,&ME->photo->sizes[maxi].loc,download_callback,image_file);
        xsprintf(buf," saved in %s",image_file);
      }
      else if (ME->type==tgl_message_media_document ||
        ME->type==tgl_message_media_audio ||
        ME->type==tgl_message_media_video
        )
      {
        if (ME->caption)
          xsprintf(buf,"caption:'%s' ",ME->caption);
        assert (ME->document);
        xsprintf(buf,"{");
        const char *extension;
        if (ME->document->caption)
          extension=get_extension(ME->document->caption);
        else
          extension="";
        if (ME->document->flags & TGLDF_IMAGE) 
        {
          xsprintf (buf, "image");
          extension="jpg";
        }
        else if (ME->document->flags & TGLDF_AUDIO)
        {
          xsprintf (buf, "audio");
          extension="mp3";
        }
        else if (ME->document->flags & TGLDF_VIDEO)
        {
          xsprintf (buf, "video");
          extension="avi";
        }
        else if (ME->document->flags & TGLDF_STICKER)
        {
          xsprintf (buf, "sticker");
          extension="png";
        }
        else
        {
          xsprintf (buf, "document");
        }
        if (ME->document->caption && strlen (ME->document->caption))
          xsprintf (buf, " caption=%s", ME->document->caption);
        if (ME->document->mime_type) 
          xsprintf (buf, " type=%s", ME->document->mime_type);
        if (ME->document->w && ME->document->h) 
          xsprintf (buf, " size=%dx%d", ME->document->w, ME->document->h);
        if (ME->document->duration) 
          xsprintf (buf, " duration=%d", ME->document->duration);
        xsprintf (buf, " size=%d",ME->document->size);
        xsprintf(buf,"}");
        
        //save in file
        char *file=create_download_file(chat_name,extension);
        tgl_do_load_document(TLS,ME->document,download_callback,file);
        xsprintf(buf," saved in %s",file);        
      }
      else if (ME->type==tgl_message_media_document_encr)
      {
        if (ME->caption)
          xsprintf(buf,"caption:'%s' ",ME->caption);
        xsprintf(buf,"{");
        if (ME->encr_document->flags & TGLDF_IMAGE)
          xsprintf (buf, "image");
        else if (ME->encr_document->flags & TGLDF_AUDIO)
          xsprintf (buf, "audio");
        else if (ME->encr_document->flags & TGLDF_VIDEO)
          xsprintf (buf, "video");
        else if (ME->encr_document->flags & TGLDF_STICKER)
          xsprintf (buf, "sticker");
        else
          xsprintf (buf, "document");
        if (ME->encr_document->caption && strlen (ME->encr_document->caption)) 
          xsprintf (buf, " caption=%s", ME->encr_document->caption);
        if (ME->encr_document->mime_type) 
          xsprintf (buf, " type=%s", ME->encr_document->mime_type);
        if (ME->encr_document->w && ME->encr_document->h) 
          xsprintf (buf, " size=%dx%d", ME->encr_document->w, ME->encr_document->h);

        if (ME->encr_document->duration) 
          xsprintf (buf, " duration=%d", ME->encr_document->duration);
        xsprintf (buf, " size=%d",ME->encr_document->size);
        xsprintf(buf,"}");

        //save in file
        char *file=create_download_file(chat_name,get_extension(ME->caption));
        tgl_do_load_encr_document(TLS,ME->encr_document,download_callback,file);
        xsprintf(buf," saved in %s",file);        

      }
      else if (ME->type==tgl_message_media_geo)
      {
        xsprintf (buf, "{geo https://maps.google.com/?q=%.6lf,%.6lf}", ME->geo.latitude, ME->geo.longitude);
      }
      else if (ME->type==tgl_message_media_contact)
      {
        xsprintf (buf, "{contact");
        xsprintf (buf, " %s %s", ME->first_name, ME->last_name);
        xsprintf (buf, " %s", ME->phone);
        xsprintf (buf,"}");
      }
      else if (ME->type==tgl_message_media_unsupported)
      {
        xsprintf (buf, "{unsupported}");
      }
      else if (ME->type==tgl_message_media_webpage)
      {
        if (ME->caption)
          xsprintf(buf,"caption:'%s' ",ME->caption);
        xsprintf (buf, "{webpage:");
        assert (ME->webpage);
        if (ME->webpage->url) 
          xsprintf (buf, " url:'%s'", ME->webpage->url);
        if (ME->webpage->title) 
          xsprintf (buf, " title:'%s'", ME->webpage->title);
        if (ME->webpage->description) 
          xsprintf (buf, " description:'%s'", ME->webpage->description);
        if (ME->webpage->author)
          xsprintf (buf, " author:'%s'", ME->webpage->author);
        xsprintf(buf,"}");
      }
      else if (ME->type==tgl_message_media_venue)
      {
        xsprintf (buf, "{venue https://maps.google.com/?q=%.6lf,%.6lf", ME->venue.geo.latitude, ME->venue.geo.longitude);
        if (ME->venue.title) 
          xsprintf (buf, " title:'%s'", ME->venue.title);
        if (ME->venue.address) 
          xsprintf (buf, " address:'%s'", ME->venue.address);
        if (ME->venue.provider) 
          xsprintf (buf, " provider:'%s'", ME->venue.provider);
        if (ME->venue.venue_id) 
          xsprintf (buf, " id:'%s'", ME->venue.venue_id);
        xsprintf(buf,"}");
      }
      else
      {
        xsprintf(buf, "{unknown}");
      }
    // print_media (ev, &M->media);
  }  
  xsprintf (buf, "\n");


  //create folder
  safe_mkdir("grab");

  //write to file
  char filename[1024];
  chatname2filename(filename,chat_name);
  char filepath[1024];
  sprintf(filepath,"grab/%s.txt",filename);
  printf("%s %s\n",chat_name,filepath);
  FILE * f=fopen(filepath,"at+");
  if (f==NULL)
    perror("Error");
  fwrite(buf,strlen(buf),1,f);
  fclose(f);

  free(buf);
}


int chatname2filename(char *filename,char* chatname)
{
  int i,j;
  char *illegal="\\/:*?\"<>|";
  char *s=chatname;
  int flag;
  for (i=0;s[i] && i<1024; ++i)
  {
    flag=0;
    for (j=0;j<illegal[j];++j)
      if (s[i]==illegal[j])
      {

    // if ( (s[i]>='a' && s[i]<='z') || (s[i]>='A' && s[i]<='Z') || (s[i]>='0' && s[i]<='9')
    //   || s[i]==' ' || s[i]=='_' || s[i]=='-' || s[i]=='#' || s[i]=='.')
        flag=1;
        break;
      }
    if (!flag)   
      filename[i]=s[i];
    else
      filename[i]='-';
  }
  filename[i]=0;
  return 0;
} 

/*
void print_media (struct in_ev *ev, struct tgl_message_media *M) {
  assert (M);
  switch (M->type) 
  {
    case tgl_message_media_none:
      return;
    case tgl_message_media_photo:
      if (!M->photo) {
        mprintf (ev, "[photo bad]");
      } else if (M->photo->caption && strlen (M->photo->caption)) {
        mprintf (ev, "[photo %s]", M->photo->caption);
      } else {
        mprintf (ev, "[photo]");
      }
      if (M->caption) {
        mprintf (ev, " %s", M->caption);
      }
      return;
    case tgl_message_media_document:
    case tgl_message_media_audio:
    case tgl_message_media_video:
      mprintf (ev, "[");
      assert (M->document);
      if (M->document->flags & TGLDF_IMAGE) {
        mprintf (ev, "image");
      } else if (M->document->flags & TGLDF_AUDIO) {
        mprintf (ev, "audio");
      } else if (M->document->flags & TGLDF_VIDEO) {
        mprintf (ev, "video");
      } else if (M->document->flags & TGLDF_STICKER) {
        mprintf (ev, "sticker");
      } else {
        mprintf (ev, "document");
      }

      if (M->document->caption && strlen (M->document->caption)) {
        mprintf (ev, " %s:", M->document->caption);
      } else {
        mprintf (ev, ":");
      }
      
      if (M->document->mime_type) {
        mprintf (ev, " type=%s", M->document->mime_type);
      }

      if (M->document->w && M->document->h) {
        mprintf (ev, " size=%dx%d", M->document->w, M->document->h);
      }

      if (M->document->duration) {
        mprintf (ev, " duration=%d", M->document->duration);
      }
      
      mprintf (ev, " size=");
      if (M->document->size < (1 << 10)) {
        mprintf (ev, "%dB", M->document->size);
      } else if (M->document->size < (1 << 20)) {
        mprintf (ev, "%dKiB", M->document->size >> 10);
      } else if (M->document->size < (1 << 30)) {
        mprintf (ev, "%dMiB", M->document->size >> 20);
      } else {
        mprintf (ev, "%dGiB", M->document->size >> 30);
      }
      
      mprintf (ev, "]");
      
      if (M->caption) {
        mprintf (ev, " %s", M->caption);
      }

      return;
    case tgl_message_media_document_encr:
      mprintf (ev, "[");
      if (M->encr_document->flags & TGLDF_IMAGE) {
        mprintf (ev, "image");
      } else if (M->encr_document->flags & TGLDF_AUDIO) {
        mprintf (ev, "audio");
      } else if (M->encr_document->flags & TGLDF_VIDEO) {
        mprintf (ev, "video");
      } else if (M->encr_document->flags & TGLDF_STICKER) {
        mprintf (ev, "sticker");
      } else {
        mprintf (ev, "document");
      }

      if (M->encr_document->caption && strlen (M->encr_document->caption)) {
        mprintf (ev, " %s:", M->encr_document->caption);
      } else {
        mprintf (ev, ":");
      }
      
      if (M->encr_document->mime_type) {
        mprintf (ev, " type=%s", M->encr_document->mime_type);
      }

      if (M->encr_document->w && M->encr_document->h) {
        mprintf (ev, " size=%dx%d", M->encr_document->w, M->encr_document->h);
      }

      if (M->encr_document->duration) {
        mprintf (ev, " duration=%d", M->encr_document->duration);
      }
      
      mprintf (ev, " size=");
      if (M->encr_document->size < (1 << 10)) {
        mprintf (ev, "%dB", M->encr_document->size);
      } else if (M->encr_document->size < (1 << 20)) {
        mprintf (ev, "%dKiB", M->encr_document->size >> 10);
      } else if (M->encr_document->size < (1 << 30)) {
        mprintf (ev, "%dMiB", M->encr_document->size >> 20);
      } else {
        mprintf (ev, "%dGiB", M->encr_document->size >> 30);
      }
      
      mprintf (ev, "]");

      return;
    case tgl_message_media_geo:
      mprintf (ev, "[geo https://maps.google.com/?q=%.6lf,%.6lf]", M->geo.latitude, M->geo.longitude);
      return;
    case tgl_message_media_contact:
      mprintf (ev, "[contact] ");
      mpush_color (ev, COLOR_RED);
      mprintf (ev, "%s %s ", M->first_name, M->last_name);
      mpop_color (ev);
      mprintf (ev, "%s", M->phone);
      return;
    case tgl_message_media_unsupported:
      mprintf (ev, "[unsupported]");
      return;
    case tgl_message_media_webpage:
      mprintf (ev, "[webpage:");
      assert (M->webpage);
      if (M->webpage->url) {
        mprintf (ev, " url:'%s'", M->webpage->url);
      }
      if (M->webpage->title) {
        mprintf (ev, " title:'%s'", M->webpage->title);
      }
      if (M->webpage->description) {
        mprintf (ev, " description:'%s'", M->webpage->description);
      }
      if (M->webpage->author) {
        mprintf (ev, " author:'%s'", M->webpage->author);
      }
      mprintf (ev, "]");
      break;
    case tgl_message_media_venue:
      mprintf (ev, "[geo https://maps.google.com/?q=%.6lf,%.6lf", M->venue.geo.latitude, M->venue.geo.longitude);
      
      if (M->venue.title) {
        mprintf (ev, " title:'%s'", M->venue.title);
      }
      
      if (M->venue.address) {
        mprintf (ev, " address:'%s'", M->venue.address);
      }
      if (M->venue.provider) {
        mprintf (ev, " provider:'%s'", M->venue.provider);
      }
      if (M->venue.venue_id) {
        mprintf (ev, " id:'%s'", M->venue.venue_id);
      }

      mprintf (ev, "]");
      return;
      
    default:
      mprintf (ev, "x = %d\n", M->type);
      assert (0);
  }
}
*/
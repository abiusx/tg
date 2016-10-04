#define TELEGRAB 1
#include <sys/stat.h>

#include "telegrab-util.c"
/**
 * Returns a non-reliable name for a peer
 * @param user_name [description]
 * @param from_peer [description]
 */
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

void date2string(char *buf,long t)
{
  struct tm *tm = localtime ((void *)&t);
  // static char *monthes[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  // sprintf (buf, "%02d:%02d:%02d %s %02d, %d", tm->tm_hour, tm->tm_min, tm->tm_sec, monthes[tm->tm_mon],tm->tm_mday, tm->tm_year + 1900);
  sprintf (buf, "%02d:%02d:%02d %02d/%02d/%d", tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_mon,tm->tm_mday, tm->tm_year + 1900);
 
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
 * not guaranteed to be unique for a peer throughout his lifetime
 * @param title     [description]
 * @param from_peer [description]
 */
char * get_peer_title(char *title,tgl_peer_t *from_peer )
{
  char *user_name=title;
  // printf("0: %d\n",from_peer->flags);
  int peer_type=tgl_get_peer_type (from_peer->id);
  int id=tgl_get_peer_id (from_peer->id);
  if (peer_type==TGL_PEER_USER)
  {
    if ((from_peer->flags & TGLUF_DELETED)) {
      sprintf (user_name,"deleted user#%d",  id);
    } else if (!(from_peer->flags & TGLUF_CREATED)) {
      sprintf (user_name, "user#%d", id);
    } else if (!from_peer->user.first_name || !strlen (from_peer->user.first_name)) {
      sprintf (user_name, "%s", from_peer->user.last_name);
    } else if (!from_peer->user.last_name || !strlen (from_peer->user.last_name)) {
      sprintf (user_name, "%s", from_peer->user.first_name);
    } else {
      sprintf (user_name, "%s %s", from_peer->user.first_name, from_peer->user.last_name); 
    }
  }
  else if (peer_type==TGL_PEER_CHAT)
    sprintf(user_name, "%s", from_peer->chat.title);
  else if (peer_type==TGL_PEER_CHANNEL)
    sprintf(user_name, "%s", from_peer->channel.title);
  else if (peer_type==TGL_PEER_ENCR_CHAT)
    sprintf(user_name, "%s", from_peer->print_name);
  else 
    sprintf(user_name, "unknown#%d", id);
  return title;
}
/**
 * unique for the lifetime of a peer
 * @param  from_peer [description]
 * @return           [description]
 */
char * get_peer_filename(char *buf,tgl_peer_t *from_peer)
{
  int peer_type=tgl_get_peer_type (from_peer->id);
  int id=tgl_get_peer_id (from_peer->id);

  if (peer_type==TGL_PEER_USER)
  {
    sprintf(buf,"user_%d",id);
  }
  else if (peer_type==TGL_PEER_CHAT)
    sprintf(buf, "chat_%d", id);//from_peer->chat.title);
  else if (peer_type==TGL_PEER_CHANNEL)
    sprintf(buf, "channel_%d",id);// from_peer->channel.title);
  else if (peer_type==TGL_PEER_ENCR_CHAT)
    sprintf(buf, "encrypted-chat_%d",id);// from_peer->print_name);
  else
    sprintf(buf, "unknown_%d",id);// from_peer->print_name);
  return buf;
}

#include "telegrab-txt.c"
#include "telegrab-html.c"

void dump_message_html(struct tgl_message *M,struct in_ev *ev);
void dump_message(struct tgl_message *M,struct in_ev *ev)
{
  enum modes{text=0,html=1};
  enum modes mode=html;
  if (mode==text)
    dump_message_txt(M,ev);
  else if (mode==html)
    dump_message_html(M,ev);
  else
    printf("Unknown output mode!\n");
}

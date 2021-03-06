/**
 * Convert new lines to HTML <br>s
 * @param  dest [description]
 * @param  src  [description]
 * @return      [description]
 */
char *nl2br(char *dest,const char * src)
{
  char *temp=str_replace(src,"\n","<br/>");
  if (dest==NULL)
    strcpy(dest,src);
  else
    strcpy(dest,temp);
  return dest;
}
/**
 * Encode HTML special chars
 * @param  output [description]
 * @param  input  [description]
 * @return        [description]
 */
char * htmlspecialchars(char *output,const char *input)
{
  long i = 0;
  long j = 0;

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
      i++;
  }
  output[j] = 0;
  return output;
}
/**
 * The css used in all chats
 * @param filename [description]
 */
void initialize_css(const char *filename)
{
  file_put_content(filename,"\n\
    body {\n\
      background-color:#B0B8CC;\n\
      font-family:sans-serif;\n\
    }\n\
    .message { \n\
      width:auto;\n\
      margin:3px;\n\
      border-radius:25px;\n\
      display:inline-block;\n\
      position:relative;\n\
      overflow:auto;\n\
      max-width:80%;\n\
    }\n\
    .content {\n\
      padding:10px 15px 5px 20px;\n\
    }\n\
    .media {\n\
      \n\
    }\n\
    img.media {\n\
      max-width:400px;\n\
    }\n\
    .message.receive {\n\
      background-color:white;\n\
    }\n\
    .message.send {\n\
      float:right;\n\
      background-color:#EBFFD5;\n\
    }\n\
    .message_container { \n\
      clear:both; \n\
    } \n\
    .date {\n\
      float:right;\n\
      margin-left:15px;\n\
      margin-top:5px;\n\
    }\n\
    .message.receive .date {\n\
      color:#8F9CA8;\n\
    }\n\
    .message.send .date {\n\
      color:#59BE8B;\n\
    }\n\
    .clear {\n\
      claer:both;\n\
    }\n\
    .avatar {\n\
      margin:5px;\n\
      width:32px;\n\
      height:32px;\n\
      vertical-align:top;\n\
      border-radius:50%;\n\
    }\n\
    img[src='Error.src']{\n\
      display: none;\n\
    }\n\
    .system {\n\
      font-weight:bold;\n\
    }\n\
  \n\
    "
    );

}
/**
 * The javascript used in all chats
 * @param filename [description]
 */
void initialize_script(const char *filename)
{
  //TODO: show top/bottom button
  file_put_content(filename,"\
    document.addEventListener('DOMContentLoaded', function() {\n\
  var text = document.getElementById('title').innerHTML;\n\
  //var title_holder=parent.frames['top'].getElementById('title'); //SOP violation\n\
  //title_holder.innerHTML=text;\n\
}, false);\n\
    ");
}
/**
 * The frameset for chat manager
 */
void initialize_frameset()
{
  file_put_content("grab/index.html","<!DOCTYPE html>\n\
<html>\n\
<frameset rows='10%%,*'>\n\
  <frame name='top' src='_html/top.html'>\n\
  <frameset cols='25%%,75%%'>\n\
    <frame name='list' src='_html/list.html'>\n\
    <frame name='content' src='_html/home.html'>\n\
  </frameset>\n\
</frameset>\n\
</html>\n\
");
  file_put_content("grab/_html/top.html"
  ,"<!DOCTYPE html>\n\
<html>\n\
<head></head>\n\
<body>\n\
<h1 style='text-align:center;'>Telegrab</h1>\n\
<div id='title'></div>\n\
</body>\n\
</html>\n\
");
  file_put_content("grab/_html/home.html",
  "<!DOCTYPE html>\n\
<html>\n\
<head></head>\n\
<body>\n\
<p>Welcome to Telegrab! Select an item from the list on the left.</p>\n\
</body>\n\
</html>\n\
");
  file_put_content("grab/_html/list.html"
  ,"<!--total: 0 #will be updated below-->\n");
  file_put_content("grab/_html/list_style.css"
  ,"\n\
    a.list {\n\
      display:block;\n\
      height:50px;\n\
      padding:5px;\n\
      width:100%%;\n\
    }\n\
    a.list:HOVER {\n\
      background-color:#EEE;\n\
    }\n\
    object.icon {\n\
      margin-right:10px;\n\
    }\n\
    .list .icon {\n\
      width:50px;\n\
      height:50px;\n\
      border-radius:50%;\n\
      vertical-align:top;\n\
    }\n\
    ");
}
/**
 * Downloads avatar and returns user id for use
 * @param  peer [description]
 * @return      [description]
 */
int download_avatar(tgl_peer_t * peer)
{
  int id=tgl_get_peer_id(peer->id);
  safe_mkdir("grab/_html/avatars");
  char temp[1024];
  sprintf(temp,"%d",id);
  char *avatar_file=create_download_file("_html/avatars",temp,"jpg");
  if (!file_exists(avatar_file))
  {
    tgl_do_load_file_location(TLS,&peer->photo_big,download_callback,avatar_file);
    printf("Saving avatar to %s...\n",avatar_file); 
  }
  return id;
}
/**
 * Adds a single entry to the chat list, and brings it to the top
 * @param  peer  [description]
 * @param  title [description]
 * @param  file  [description]
 * @return       [description]
 */
int add_to_list(tgl_peer_t *peer, const char *title, const char *file)
{

  //FIXME: what if a channel and a user have same id? is it even possible?
  int id=tgl_get_peer_id(peer->id);

  char **lines;
  const int size=4096;
  char buf[size];
  FILE * fp=fopen("grab/_html/list.html","rt");
  fgets(buf,size,fp); //includes newline
  int total;
  sscanf(buf,"%*s %d",&total);

  lines=safe_malloc(sizeof(char*) * (total+1+1));
  lines[0]=safe_malloc(strlen(buf)+1);
  strcpy(lines[0],buf);
  int i;
  for (i=1;i<=total;++i)
  {
    fgets(buf,size,fp);
    if (!strlen(buf)) continue;
    lines[i]=safe_malloc(strlen(buf)+1);
    // printf("X%dX-%s\n",i,lines[i]);
    strcpy(lines[i],buf);
  }
  fclose (fp);
  int found=0;
  for (i=1;i<=total;++i)
  {
    int eid;
    sscanf(lines[i],"%*s %d",&eid);
    if (eid==id)
    {
      char *t=lines[i];
      lines[i]=lines[1];
      lines[1]=t;
      found=1;
      //FIXME: shift instead of swap
      break;
    }
  }
  if (!found) //new user
  {
    total++;
    //FIXME:shift
    lines[total]=lines[1];
    lines[1]=safe_malloc(size);

    //title
    char *t=safe_malloc(strlen(title)*2);
    htmlspecialchars(t,title);

    printf("Adding new title '%s'...\n",t);
    int avatar_id=download_avatar(peer);
    sprintf(lines[1],"<!--peerid: %d --><a class='list' target='content' href='../../%s'>\
<object class='icon' data='avatars/%d.jpg' type='image/jpg'>\
<img class='icon' src='avatars/default.jpg' />\
</object>%s</a>\n",avatar_id,file,avatar_id,t);
    free(t);
  }

  fp=fopen("grab/_html/list.html","wt");
 //<meta http-equiv='refresh' content='5; URL=list.html'>
  fprintf(fp,"<!--total: %d --><link rel='stylesheet' href='list_style.css' />\
    \n",total);
  for (i=1;i<=total;++i)
    fprintf(fp,"%s",lines[i]);
  fclose(fp);
  for (i=0;i<=total;++i)
    free(lines[i]);
  free(lines);
  return !found;
}
/**
 * Create a basic template for a single chat's HTML
 * @param id       [description]
 * @param filename [description]
 * @param title    [description]
 */
void initiate_html(int id,const char *filename, const char *title)
{
  safe_mkdir("grab");
  safe_mkdir("grab/_html");

  if (!file_exists("grab/index.html"))
    initialize_frameset();
  if (!file_exists("grab/_html/style.css"))
    initialize_css("grab/_html/style.css");
  if (!file_exists("grab/_html/script.js"))
    initialize_script("grab/_html/script.js");

  char buf[1024];
  FILE * fp=fopen(filename,"wt");
  htmlspecialchars(buf,title);
  fprintf(fp,"<html>\n\t<head>\n\t\t<script src='_html/script.js'></script>\
\n\t\t<link rel='stylesheet' href='_html/style.css' />\n\t</head>\n\t<body>\
<h1 id='title'>\
<a href='_html/avatars/%d.jpg' target='_blank'>\
<object class='avatar' data='_html/avatars/%d.jpg' type='image/jpg'>\
<img class='avatar' src='_html/avatars/default.jpg' />\
</object></a>%s</h1>",id,id,buf);
  fclose(fp);
}
/**
 * Dumps a message into HTML file
 * @param M  [description]
 * @param ev [description]
 */
void dump_message_html(struct tgl_message *M,struct in_ev *ev)
{
  //TODO: support media
  //FIXME: support reply-to embedding
  //FIXME: appropriate color for system messages depending on sender and message
  //FIXME: Use name only once in a batch of messages in groups
  //FIXME: some groups and all channels are not recovered in -5000, TGL_PEER_CHANNEL flag is for group.
  //  solution: supergroups are converted into channels. 
  char *html=safe_malloc(1024*1024*8);
  char *temp=safe_malloc(1024*1024);
  char *temp2=safe_malloc(1024*1024);
  html[0]=0;

  tgl_peer_t *to_peer=tgl_peer_get (TLS, M->to_id);
  tgl_peer_t *from_peer=tgl_peer_get(TLS,M->from_id);
  int peer_type=tgl_get_peer_type (M->to_id);
  tgl_peer_t * sender_peer;
  tgl_peer_t * chat_peer;
  char *operation=safe_malloc(1024);
  int is_group=0;

  //determine type of message
  if (peer_type==TGL_PEER_USER)
  {
    if (M->flags & TGLMF_OUT) //outbound, use to_id
    {
      sender_peer=from_peer; 
      chat_peer=to_peer; 
      operation="send";
    }
    else
    {
      sender_peer=from_peer; 
      chat_peer=from_peer; 
      operation="receive";
    }
  }
  else if (peer_type==TGL_PEER_CHAT || peer_type==TGL_PEER_ENCR_CHAT || peer_type==TGL_PEER_GEO_CHAT)
  {
    is_group=1;
    sender_peer=from_peer; 
    chat_peer=to_peer; 
    operation="receive";
  }
  else if (peer_type==TGL_PEER_CHANNEL)
  {
    is_group=1;
    sender_peer=from_peer; 
    chat_peer=to_peer; 
    operation="receive";
  }
  else
  {
    sender_peer=from_peer; 
    chat_peer=to_peer; 
    operation="receive";
  }
  if (is_group && tgl_cmp_peer_id (sender_peer->id, TLS->our_id)==0)
    operation="send";

  int chat_id=tgl_get_peer_id(chat_peer->id);
  char *chat_filename=safe_malloc(1024);
  get_peer_filename(chat_filename,chat_peer);
  char *chat_folder=safe_malloc(1024);
  sprintf(chat_folder,"%s_files",chat_filename);
  char *chat_title=safe_malloc(1024);
  get_peer_title(chat_title,chat_peer);


  //date string
  char *date_string=safe_malloc(256);
  date2string(date_string,M->date);
  
  //filename
  memmove(chat_filename+5,chat_filename,strlen(chat_filename)+1);
  memcpy(chat_filename,"grab/",5);
  strcat(chat_filename,".html");
  
  if (!file_exists(chat_filename))
    initiate_html(chat_id,chat_filename,chat_title);


  //add sender to chat list
  add_to_list(chat_peer,chat_title,chat_filename);

  //start message
  xsprintf(html,"<div class='message_container'><a name='message_%lld'></a>",M->permanent_id.id);

  //sender thumbnail in group
  if (is_group && tgl_cmp_peer_id (sender_peer->id, TLS->our_id)!=0)
  {
    xsprintf(html,"<a href='https://telegram.me/%s'>\
<object class='avatar' data='_html/avatars/%d.jpg' type='image/jpg'>\
<img class='avatar' src='_html/avatars/default.jpg' />\
</object></a>",
      sender_peer->username,download_avatar(sender_peer));
  }  
  xsprintf(html,"<div class='message %s'>\n",operation);

  //sender name  
  if (is_group && tgl_cmp_peer_id (sender_peer->id, TLS->our_id)!=0)
  {
    htmlspecialchars(temp,get_peer_title (temp2,sender_peer));    
    xsprintf (html, "\t<div class='sysem'><a href='https://telegram.me/%s'>%s</a></div>\n",
      sender_peer->username,temp);
  }


  //other message types
  if (tgl_get_peer_type (M->fwd_from_id) > 0)  //forward
  {
    htmlspecialchars(temp,get_peer_title (temp2,tgl_peer_get (TLS, M->fwd_from_id)));    
    xsprintf (html, "\t<div class='sysem'>Forwarded from <a href='https://telegram.me/%s'>%s</a></div>\n",
      tgl_peer_get (TLS, M->fwd_from_id)->username,temp);
    htmlspecialchars(temp,get_peer_title (temp2,tgl_peer_get (TLS, M->fwd_from_id)));    
  }
  if (M->reply_id) //reply
  {
    xsprintf (html, "\t<div class='sysem'>Reply to <a href='#message_%d'>%d</a></div>\n",
      M->reply_id, M->reply_id);
  }
  // if (M->flags & TGLMF_MENTION) //mention, only works when someone else mentions YOU in a group (or when someone replies you)
  // {
  //   xsprintf (buf, "{mention} ");
  // }


  ////////////////////
  ///MEDIA handling///
  ////////////////////
  if (M->media.type != tgl_message_media_none) //media
  {
      struct tgl_message_media *ME=&M->media;
      if (ME->type==tgl_message_media_photo) //PHOTO
      {

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
        char *image_file=create_download_file(chat_folder,0,"jpg");
        tgl_do_load_file_location(TLS,&ME->photo->sizes[maxi].loc,download_callback,image_file);
        xsprintf(html,"<a href='../%s'><img src='../%s' class='media' /></a><div class='content'>%s",image_file,image_file,ME->caption?ME->caption:"");
      }
      else if (ME->type==tgl_message_media_document ||
        ME->type==tgl_message_media_audio ||
        ME->type==tgl_message_media_video
        )
      {
        const char *extension;
        if (ME->document->caption)
          extension=get_extension(ME->document->caption);
        if (ME->document->flags & TGLDF_IMAGE) 
          extension="jpg";
        else if (ME->document->flags & TGLDF_AUDIO)
          extension="mp3";
        else if (ME->document->flags & TGLDF_VIDEO)
          extension="avi";
        else if (ME->document->flags & TGLDF_STICKER)
          extension="png";
        
        //save in file
        char *file=create_download_file(chat_folder,0,extension);
        tgl_do_load_document(TLS,ME->document,download_callback,file);
        xsprintf(html,"<div class='content'><a href='../%s'>%s</a>",file,(ME->caption && strlen(ME->caption))?ME->caption:ME->document->caption);
      }
      else if (ME->type==tgl_message_media_document_encr)
      {
        const char *extension;
        if (ME->encr_document->caption)
          extension=get_extension(ME->encr_document->caption);
        if (ME->encr_document->flags & TGLDF_IMAGE) 
          extension="jpg";
        else if (ME->encr_document->flags & TGLDF_AUDIO)
          extension="mp3";
        else if (ME->encr_document->flags & TGLDF_VIDEO)
          extension="avi";
        else if (ME->encr_document->flags & TGLDF_STICKER)
          extension="png";
        
        //save in file
        char *file=create_download_file(chat_folder,0,extension);
        tgl_do_load_encr_document(TLS,ME->encr_document,download_callback,file);
        xsprintf(html,"<div class='content'><a href='../%s'>%s</a>",file,(ME->caption && strlen(ME->caption))?ME->caption:ME->encr_document->caption);        

      }
      else if (ME->type==tgl_message_media_geo)
      {
        xsprintf (html, "<div class='content'><a href='https://maps.google.com/?q=%.6lf,%.6lf'>Map Location</a>", 
          ME->geo.latitude, ME->geo.longitude);
      }
      else if (ME->type==tgl_message_media_contact)
      {
        xsprintf (html, "<div class='content'>Contact: %s %s (%s)",ME->first_name, ME->last_name,ME->phone);
      }
      else if (ME->type==tgl_message_media_unsupported)
      {
        xsprintf (html, "<div class='content'>[Unsupported Message]");
      }
      else if (ME->type==tgl_message_media_webpage)
      {
        xsprintf(html,"\t<div class='content'>");

        if (ME->caption)
          xsprintf(html,"<a href='%s'>%s</a>",ME->caption,ME->caption);
        assert (ME->webpage);
        if (ME->webpage->url) 
          xsprintf (html, " url:'%s'", ME->webpage->url);
        if (ME->webpage->title) 
          xsprintf (html, " title:'%s'", ME->webpage->title);
        if (ME->webpage->description) 
          xsprintf (html, " description:'%s'", ME->webpage->description);
        if (ME->webpage->author)
          xsprintf (html, " author:'%s'", ME->webpage->author);
      }
      else if (ME->type==tgl_message_media_venue)
      {
        xsprintf (html, "<div class='content'><a href='https://maps.google.com/?q=%.6lf,%.6lf'>Venue</a>", ME->venue.geo.latitude, ME->venue.geo.longitude);
        if (ME->venue.title) 
          xsprintf (html, " title:'%s'", ME->venue.title);
        if (ME->venue.address) 
          xsprintf (html, " address:'%s'", ME->venue.address);
        if (ME->venue.provider) 
          xsprintf (html, " provider:'%s'", ME->venue.provider);
        if (ME->venue.venue_id) 
          xsprintf (html, " id:'%s'", ME->venue.venue_id);
      }
      else
      {
        xsprintf(html,"\t<div class='content'>");
        xsprintf(html, "[Unknown Media]");
      }
  }  
  //message body
  if (M->message && strlen (M->message))  //the message
  {
    htmlspecialchars(temp,M->message);
    nl2br(temp2,temp);
    int is_rtl=u8_is_rtl(M->message);
    // printf("is rtl: %d\n",is_rtl);
    xsprintf (html, "\t<div class='content'%s>%s",is_rtl?" dir='rtl'":"",temp2);
  }
  // else
  //   xsprintf(html,"\t<div class='content'>");
  xsprintf(html,"<span class='date'>%s</span></div>",date_string);
  xsprintf(html,"</div><!-- msgid: %lld --></div>\n",M->permanent_id.id);

  //write to file
  FILE * f=fopen(chat_filename,"at+");
  if (f==NULL)
    perror("Error");
  fwrite(html,strlen(html),1,f);
  fclose(f);

  free(html);
  free(temp);
  free(temp2);
  free(chat_filename);
  free(chat_title);
}
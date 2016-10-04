char *nl2br(char *dest,const char * src)
{
  char *temp=str_replace(src,"\n","<br/>");
  if (dest==NULL)
    strcpy(dest,src);
  else
    strcpy(dest,temp);
  return dest;
}
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

void initialize_css(const char *filename)
{
  FILE * fp=fopen(filename,"wt");
  fprintf(fp,"\n\
    body {\n\
      background-color:#B0B8CC;\n\
    }\n\
    .message { \n\
      width:auto;\n\
      margin:3px;\n\
      padding:10px 15px 5px 20px;\n\
      border-radius:25px;\n\
      display:inline-block;\n\
      position:relative;\n\
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
    "
    );
  fclose(fp);

}
//TODO: show top/bottom button
void initialize_script(const char *filename)
{
  FILE * fp=fopen(filename,"wt");
  fprintf(fp,"\n");
  fclose(fp);
}

void initiate_html(const char *filename, const char *title)
{
  safe_mkdir("grab");
  safe_mkdir("grab/_html");
  if (!file_exists("grab/_html/style.css"))
    initialize_css("grab/_html/style.css");
  if (!file_exists("grab/_html/script.js"))
    initialize_script("grab/_html/script.js");

  char buf[1024];
  FILE * fp=fopen(filename,"wt");
  htmlspecialchars(buf,title);
  fprintf(fp,"<html>\n\t<head>\n\t\t<title>%s</title>\n\t\t<script src='_html/script.js'></script>\
\n\t\t<link rel='stylesheet' href='_html/style.css' />\n\t</head>\n\t<body><h1>%s</h1>",buf,buf);
  fclose(fp);
}
void dump_message_html(struct tgl_message *M,struct in_ev *ev)
{
  char *html=safe_malloc(1024*1024*8);
  char *temp=safe_malloc(1024*1024);
  char *temp2=safe_malloc(1024*1024);
  html[0]=0;

  tgl_peer_t *to_peer=tgl_peer_get (TLS, M->to_id);
  tgl_peer_t *from_peer=tgl_peer_get(TLS,M->from_id);
  int peer_type=tgl_get_peer_type (M->to_id);
  tgl_peer_t * target_peer=from_peer;
  char *operation=safe_malloc(1024);
  if (peer_type==TGL_PEER_USER)
  {
    if (M->flags & TGLMF_OUT) //outbound, use to_id
    {
      target_peer=to_peer; 
      operation="send";
    }
    else
      operation="receive";
  }
  else if (peer_type==TGL_PEER_CHAT || peer_type==TGL_PEER_ENCR_CHAT || peer_type==TGL_PEER_GEO_CHAT)
    operation="receive";
  else if (peer_type==TGL_PEER_CHANNEL)
    operation="receive";
  else
    operation="receive";

  char *peer_filename=safe_malloc(1024);
  get_peer_filename(peer_filename,target_peer);
  char *peer_title=safe_malloc(1024);
  get_peer_title(peer_title,target_peer);


  char *date_string=safe_malloc(256);
  //date string
  date2string(date_string,M->date);
  
  //filename
  memmove(peer_filename+5,peer_filename,strlen(peer_filename)+1);
  memcpy(peer_filename,"grab/",5);
  strcat(peer_filename,".html");
  
  if (!file_exists(peer_filename))
    initiate_html(peer_filename,peer_title);





  xsprintf(html,"<div class='message_container'><a name='message_%lld' /><div class='message %s'>\n",M->permanent_id.id,operation);
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
    //FIXME: reply_id is int, not tgl_peer_id_t
    // htmlspecialchars(temp,get_peer_title (temp2,tgl_peer_get (TLS, M->reply_id)));    
    xsprintf (html, "\t<div class='sysem'>Reply to <a href='https://telegram.me/%s'>%d</a></div>\n",
      "#", M->reply_id);
      // tgl_peer_get (TLS, M->reply_id)->username,temp);
  }
  // if (M->flags & TGLMF_MENTION) //mention, only works when someone else mentions YOU in a group (or when someone replies you)
  // {
  //   xsprintf (buf, "{mention} ");
  // }
  if (M->message && strlen (M->message))  //the message
  {
    htmlspecialchars(temp,M->message);
    nl2br(temp2,temp);
    int is_rtl=u8_is_rtl(M->message);
    // printf("is rtl: %d\n",is_rtl);

    xsprintf (html, "\t<span class='content'%s>%s</span>\n",is_rtl?" dir='rtl'":"",temp2);
  }

  /*
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
  */
  xsprintf(html,"<span class='date'>%s</span></div><!-- msgid: %lld --></div>\n",date_string,M->permanent_id.id);


  //write to file
  FILE * f=fopen(peer_filename,"at+");
  if (f==NULL)
    perror("Error");
  fwrite(html,strlen(html),1,f);
  fclose(f);

  free(html);
  free(temp);
  free(peer_filename);
  free(peer_title);
}
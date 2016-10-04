
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

void dump_message_txt(struct tgl_message *M,struct in_ev *ev)
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


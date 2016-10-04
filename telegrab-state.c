// printf ("%d %d %d %d\n",pts,qts,seq,date);
printf("You have already processed up to message %d. \
To continue, press Enter, and to reprocess enter the message number you want to start at: ",pts);
char msg[100];
int result = scanf("%99[^\n]%*c", msg);
if (result==1)
{
	int t=atoi(msg);
	if (t<0)
		pts+=t;
	else
		pts=t;
}

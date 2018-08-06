#include <stdio.h>
#include <linux/input.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h> 
#include <unistd.h>

#define DEV_PATH "/dev/input/event1"   //difference is possible

static int isFileExist(const char *path)
{
	if(path == NULL) return -1;
	if(access(path,F_OK) == 0) return 0;	
	return -1;
}

static void prtInfo()
{
	printf("###########################################################################################\n");
	printf("Usepage\n");
	printf("0 record script file\n");
	printf("1 play script file\n");
	printf("###########################################################################################\n");
}

static int getNum()
{
    char getStr[100];
    int ret = 0;

    printf("Please input number: \n");
    scanf("%100s", getStr);
    ret = atoi(getStr);
    printf("ret = [%d]\n", ret);
    return ret;
}

int main(int argc, char*argv[])
{
	int keys_fd;
	char ret[2];
	struct input_event t;
	keys_fd=open(DEV_PATH, O_RDWR);
	FILE *fp;
	if(keys_fd <= 0)
	{
		printf("open /dev/input/event1 device error!\n");
		return -1;
	}
	char data[512];
	
	struct timeval time_tmp;
	time_tmp.tv_sec = 0;
	time_tmp.tv_usec = 0;
	int num = -1;
	long long count=0;
	while(1)
	{
		prtInfo();
		num = getNum();
		printf("num:%d\n",num);
		if(num ==1)
		{	
			int exit = isFileExist("/userdata/key_event_record.log");
			if(exit == -1)
			{
				printf("not found script file, please record script file\n");
				continue;
			}
			printf("start play script file\n");
			fp = fopen("/userdata/key_event_record.log", "r");	
			struct timeval time;
			int nread = 0;
			long int sec = 0;
			long int usec = 0;
			for(;;)
			{
				nread = fscanf(fp,"%ld\t%ld\t%d\t%d\t%d",&sec,&usec, &t.type,&t.code,&t.value);
				//printf("nread:%d %ld\t%ld\t%d\t%d\t%d\n",nread,sec,usec, t.type,t.code,t.value);
				if(nread !=5)
				{
					usleep(20*1000);
					fclose(fp);
					fp = fopen("/userdata/key_event_record.log", "r");
					count++;
					printf("loop play:%lld \n",count);
					
				}
				else{
					usleep(sec*1000*1000+usec);
					gettimeofday(&t.time, NULL);				
					int ret = write(keys_fd,&t,sizeof(t)) ;
					//printf("write ret:%d\n",ret);
				}
				
				nread = 0;				
			}	

		}
		else if(num == 0) 
		{
			printf("start record script file\n");
			system("rm /userdata/key_event_record.log");
			for(;;)
			{
				long int sec = 0;
				long int usec = 0;

				if(read(keys_fd, &t, sizeof(t)) == sizeof(t))
				{
					//printf("event:%d  %d  %d  %d %d\n",t.time.tv_sec, t.time.tv_usec,t.type,t.code,t.value);
					memset(data,0x00,512);
					if((time_tmp.tv_sec == 0) && (time_tmp.tv_usec ==0))
					{
						sprintf(data,"echo \"%d\t%d\t%d\t%d\t%d\"  >> /userdata/key_event_record.log\n",0,0,t.type,t.code,t.value);
						time_tmp = t.time;
					}
					else
					{
						sec = t.time.tv_sec-time_tmp.tv_sec;
						usec = t.time.tv_usec-time_tmp.tv_usec;
						if(usec < 0)
						{
							usec = t.time.tv_usec +1000*1000 -time_tmp.tv_usec;
							sec = t.time.tv_sec -1-time_tmp.tv_sec;					
						}
						time_tmp = t.time;
						sprintf(data,"echo \"%ld\t%ld\t%d\t%d\t%d\"  >> /userdata/key_event_record.log\n",sec,usec,t.type,t.code,t.value);
					}
				
				printf("%s",data);
				system(data);
				}
			}
		}
		else
		{
			sleep(1);
		}

	}
	close(keys_fd);
		
	return 0;
}

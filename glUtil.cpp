#include "glUtil.h"
#include "string.h"

char* getFileContent(const char* filename)
{
	FILE* fp=0;
	char* vsStr=0;
	int filesize = 0;
	int allocSize = 0;
	fp = fopen(filename,"rt");
	if(fp)
	{
		fseek(fp,0,SEEK_END);
		filesize = ftell(fp);
		allocSize = filesize+4-filesize%4;
		vsStr = new char[allocSize];

		memset(vsStr,0,allocSize);
		fseek(fp,0,SEEK_SET);
		fread(vsStr,filesize,sizeof(char),fp);
	}
	else
	{
		return 0;
	}
	fclose(fp);
	return vsStr;
}


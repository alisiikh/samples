#ifndef _FIO_H
#define _FIO_H

#include <psp2/io/stat.h>

void psvMkdir(std::string dir)
{
	struct SceIoStat *dirStat = (SceIoStat *)malloc(sizeof(SceIoStat));
	if (sceIoGetstat(dir.c_str(), dirStat) < 0)
	{
		sceIoMkdir(dir.c_str(), 0777);
	}
}

#endif

#define VITASDK

#include <psp2/sysmodule.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>

#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/net/http.h>

#include <psp2/io/fcntl.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "debugScreen.h"

void netInit() {
	psvDebugScreenPrintf("Loading module SCE_SYSMODULE_NET\n");
	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	
	SceNetInitParam netInitParam;
	int size = 1*1024*1024;
	netInitParam.memory = malloc(size);
	netInitParam.size = size;
	netInitParam.flags = 0;
	sceNetInit(&netInitParam);

	sceNetCtlInit();
}

void netTerm() {
	sceNetCtlTerm();
	sceNetTerm();
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
}

void httpInit() {
	sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
	sceHttpInit(1*1024*1024);
}

void httpTerm() {
	sceHttpTerm();
	sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
}

void download(const char *url, const char *dest) {
	psvDebugScreenPrintf("\n\nDownloading %s to %s\n", url, dest);

	// Create template with user agend "PS Vita Sample App"
	int tpl = sceHttpCreateTemplate("Thumbnails downloader", 1, 1);
	// set url on the template
	int conn = sceHttpCreateConnectionWithURL(tpl, url, 0);
	// create the request with the correct method
	int request = sceHttpCreateRequestWithURL(conn, SCE_HTTP_METHOD_GET, url, 0);
	// send the actual request. Second parameter would be POST data, third would be length of it.
	int handle = sceHttpSendRequest(request, NULL, 0);
	// open destination file
	int fh = sceIoOpen(dest, SCE_O_WRONLY | SCE_O_CREAT, 0777);

	// create buffer and counter for read bytes.
	unsigned char data[16*1024];
	int read = 0;

	// read data until finished
	while ((read = sceHttpReadData(request, &data, sizeof(data))) > 0) {
		psvDebugScreenPrintf("read %d bytes\n", read);

		// writing the count of read bytes from the data buffer to the file
		int write = sceIoWrite(fh, data, read);
		psvDebugScreenPrintf("wrote %d bytes\n", write);
	}

	// close file
	sceIoClose(fh);
	psvDebugScreenPrintf("Done.\n");
}

int main(int argc, char *argv[]) {
	psvDebugScreenInit();
	psvDebugScreenPrintf("App will download a png file into ux0:data/ folder\n\n");

	netInit();
	httpInit();

	download("https://raw.githubusercontent.com/libretro/libretro-thumbnails/master/Nintendo%20-%20Nintendo%20Entertainment%20System/Named_Boxarts/Darkwing%20Duck%20(USA).png", "ux0:data/index.html");

	httpTerm();
	netTerm();

	psvDebugScreenPrintf("App closes in 5 seconds!\n");
	sceKernelDelayThread(5*1000*1000);

	sceKernelExitProcess(0);
	return 0;
}

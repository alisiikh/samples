#define VITASDK

#include <psp2/io/stat.h>
#include <psp2/sysmodule.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>

#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/net/http.h>

#include <psp2/io/fcntl.h>

#include <stdio.h>
#include <malloc.h>
#include <iostream>
#include <string.h>
#include <string>
#include <regex>
#include <vector>
#include <sstream>

#include <curl/curl.h>
#include <yaml.h>

#include "debugScreen.h"
#include "fio.h"
#include "curlutil.h"

void psvLog(std::string msg)
{
	psvDebugScreenPrintf(msg.c_str());
	psvDebugScreenPrintf("\n\n");
}

void curlDownloadFile(std::string url, std::string file)
{
	int fd = sceIoOpen(file.c_str(), SCE_O_WRONLY | SCE_O_CREAT, 0777);
	if (!fd)
	{
		return;
	}

	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if (!curl)
	{
		return;
	}

	struct stringcurl header;
	init_stringcurl(&header);

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36");
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_to_file);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fd);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_write_headers);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header);

	struct curl_slist *headerChunk = NULL;
	headerChunk = curl_slist_append(headerChunk, "Accept: */*");
	headerChunk = curl_slist_append(headerChunk, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36");
	res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerChunk);

	res = curl_easy_perform(curl);
	int respCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &respCode);

	if (res != CURLE_OK)
	{
		psvDebugScreenPrintf("Failed to download %s, status: %s", url.c_str(), curl_easy_strerror(res));
		//fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	}

	sceIoClose(fd);
	curl_easy_cleanup(curl);
}

std::vector<std::string> getThumbTypes() {
	std::vector<std::string> thumbtypes;
	thumbtypes.push_back("Named_Titles");
	thumbtypes.push_back("Named_Snaps");
	thumbtypes.push_back("Named_Boxarts");
	return thumbtypes;
}

void downloadThumbnails(std::string core, std::string title)
{
	psvDebugScreenPrintf("Downloading %s for core %s\n", title.c_str(), core.c_str());

	std::vector<std::string> thumbtypes = getThumbTypes();

	for (int i = 0; i < thumbtypes.size(); i++) {
		std::string thumbtype = thumbtypes[i];

		std::string url;
		std::string loc;

		std::stringstream ss1;
		ss1 << "https://raw.githubusercontent.com/libretro/libretro-thumbnails/master/" << core << "/" << thumbtype << "/" << title << ".png";
		url = ss1.str();

		std::stringstream ss2;
		ss2 << "ux0:data/retroarch/thumbnails/" << core << "/" << thumbtype << "/" << title << ".png";
		loc = ss2.str();

		std::regex regex("\\s"); 
		url = std::regex_replace(url, regex, "%20");

		curlDownloadFile(url, loc);
	}
}

void netInit()
{
	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);

	SceNetInitParam netInitParam;
	int size = 4 * 1024 * 1024;
	netInitParam.memory = malloc(size);
	netInitParam.size = size;
	netInitParam.flags = 0;
	sceNetInit(&netInitParam);
	sceNetCtlInit();
}

void netTerm()
{
	sceNetCtlTerm();
	sceNetTerm();
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
}

void httpInit()
{
	sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
	sceHttpInit(4 * 1024 * 1024);
}

void httpTerm()
{
	sceHttpTerm();
	sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
}

void initApp()
{
	netInit();
	httpInit();
}

void shutdownApp(int code)
{
	httpTerm();
	netTerm();

	psvDebugScreenPrintf("Shutting down in 10 seconds...");
	sceKernelDelayThread(10 * 1000 * 1000);
	sceKernelExitProcess(code);
}

void createThumbDirs(std::string core)
{
	psvDebugScreenPrintf("Creating thumbnails dirs for core %s\n\n", core.c_str());

	std::vector<std::string> thumbtypes = getThumbTypes();

	for (int i = 0; i < thumbtypes.size(); i++) {
		std::string thumbtype = thumbtypes[i];
		std::stringstream ss;

		ss << "ux0:data/retroarch/thumbnails/" << core << "/" << thumbtype;
		psvMkdir(ss.str());
		ss.clear();
	}
}

int main(int argc, char *argv[])
{
	psvDebugScreenInit();
	psvDebugScreenPrintf("App will try to download thumbnails for Retroarch roms...\n\n");

	initApp();

	struct SceIoStat *iostat = (SceIoStat *)malloc(sizeof(SceIoStat));

	if (sceIoGetstat("ux0:data/retroarch/thumbnails", iostat) < 0)
	{
		psvDebugScreenPrintf("Folder 'ux0:data/retroarch/thumbnails' doesn't exist...\n");
		shutdownApp(0);
		return 0;
	}

	if (sceIoGetstat("ux0:data/retroarch/roms", iostat) < 0)
	{
		psvDebugScreenPrintf("Folder 'ux0:data/retroarch/roms' doesn't exist!...\n");
		shutdownApp(0);
		return 0;
	}

	// TODO: traverse folder ux0:/data/retroarch/roms
	// TODO: for each $core folder and each $file in $core folder
	// TODO: download $boxart file and place it under ux0:/data/retroarch/thumbnails/$core/Named_Boxarts/$boxart

	std::string core = "Nintendo - Nintendo Entertainment System";

	createThumbDirs(core);

	downloadThumbnails(core, "Darkwing Duck (USA)");
	downloadThumbnails(core, "Battletoads (USA)");
	downloadThumbnails(core, "Battletoads-Double Dragon (USA)");

	shutdownApp(0);
	return 0;
}

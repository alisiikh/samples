#ifndef _CURLUTIL_H
#define _CURLUTIL_H

#include <stdio.h>
#include <malloc.h>
#include <curl/curl.h>

struct stringcurl
{
	char *ptr;
	size_t len;
};

void init_stringcurl(struct stringcurl *s) {
  s->len = 0;
  s->ptr = (char*)malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    return;
    //exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t curl_write_headers(void *ptr, size_t size, size_t nmemb, struct stringcurl *s) {
	size_t new_len = s->len + size * nmemb;
	s->ptr = (char *)realloc(s->ptr, new_len + 1);
	if (s->ptr == NULL)
	{
		fprintf(stderr, "realloc() failed\n");
		return 0;
	}

	memcpy(s->ptr + s->len, ptr, size * nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size * nmemb;
}

size_t curl_write_to_file(void *ptr, size_t size, size_t nmemb, void *stream) {
	size_t written = sceIoWrite(*(int *)stream, ptr, size * nmemb);
	return written;
}

#endif
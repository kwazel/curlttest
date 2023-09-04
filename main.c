#include <stdio.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include "credentials.h"

#define FILE_SIZE 100000


#ifndef CURL_PROGRESSFUNC_CONTINUE
#define CURL_PROGRESSFUNC_CONTINUE 0
#endif

struct sent_administration
{
  unsigned int bytes_send;
  bool paused;
};

size_t read_callback(char *buffer, size_t size, size_t nitems, struct sent_administration *adm);
static int xferinfo(CURL *p,
                    curl_off_t dltotal, curl_off_t dlnow,
                    curl_off_t ultotal, curl_off_t ulnow);

int main(void)
{
  struct sent_administration admin = {0, false};
  CURL *curl;
  CURLcode res;
  curl_off_t speed_upload, total_time;

  curl = curl_easy_init();

  if (curl)
  {
    /* upload to this place */
    curl_easy_setopt(curl, CURLOPT_URL,
                     DESTINATION);
    curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD);
    curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);

    /* tell it to "upload" to the URL */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    /* and give the size of the upload (optional) */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, FILE_SIZE);

    /* enable verbose for easier tracing */
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
    /* Use a read callback to read (write) data byte by byte */
    curl_easy_setopt(curl, CURLOPT_READDATA, &admin);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

    /*Use a progress callback to unpause*/
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, curl);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION,
                          xferinfo);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    }
    else
    {
      /* now extract transfer info */
      curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &speed_upload);
      curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total_time);

      fprintf(stderr, "Speed: %lu bytes/sec during %lu.%06lu seconds\n",
              (unsigned long)speed_upload,
              (unsigned long)(total_time / 1000000),
              (unsigned long)(total_time % 1000000));
    }
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  else
  {
    printf("Not able to init curl\n");
  }
  return 0;
}

size_t read_callback(char *buffer, size_t size, size_t nitems, struct sent_administration *adm)
{
  /*if (adm->bytes_send % 1000 == 0)
    printf("Bytes send = %d\n", adm->bytes_send);*/
  if (adm->bytes_send % 1 == 0 && adm->bytes_send > 0)
  {
    if (adm->paused == false)
    {
      adm->paused = true;
      return CURL_READFUNC_PAUSE;
    }
    else
    {
      adm->paused = false;
    }
  }
  if (adm->bytes_send < FILE_SIZE)
  {
    buffer[0] = 'X';
    adm->bytes_send += 1;
    return 1;
  }
  return 0;
}

static int xferinfo(CURL *p,
                    curl_off_t dltotal, curl_off_t dlnow,
                    curl_off_t ultotal, curl_off_t ulnow)
{
  // printf("Progress function called, %ld/%ld, %ld/%ld\n", dlnow, dltotal
  //        , ulnow, ultotal);
        curl_easy_pause(p, CURLPAUSE_CONT);
        
  return CURL_PROGRESSFUNC_CONTINUE;
}

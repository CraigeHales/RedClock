/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

// clockHtml.c

#include "clockMain.h"

/*
void makeHtmlFrom(char *fileName, httpd_req_t *req, int nparms, struct HtmlParm *htmlparms) {
    FILE *file = fopen(fileName, "rb");
    // test: copy out a file with no replacements
    int c;
    char buf[2] = {0};
    while ((c = fgetc(file)) != EOF)
    {
        buf[0] = (char)(c);
        bufferedhttpd_resp_sendstr_chunk(req,buf);
    }
}
*/

// I wrote this buffer code because the unbuffered version crashed. Then I saw I was sending a zero-length
// null terminated string at the point where it crashed. But it looks so much better with the buffer...
#define bufsiz 256
static char *buffer = NULL;
void bufferedhttpd_resp_sendstr_chunk(httpd_req_t *req, char const *txt) {
    if (txt) {
        if (!buffer){
            buffer = malloc(bufsiz);
            buffer[0] = 0;
            //printf("buffer allocated\n");
        }
        //if(strlen(txt)==0) printf("zero len after this ___%s___\n",buffer);
        if(strlen(buffer)+strlen(txt) > (bufsiz-1)){
            if(strlen(buffer)>0) {
                //printf("flush %d bytes\n",strlen(buffer));
                httpd_resp_sendstr_chunk(req,buffer);
                buffer[0] = 0;
            }
            if(strlen(txt) > (bufsiz-1)){
                //printf("no fit %d bytes\n",strlen(txt));
                httpd_resp_sendstr_chunk(req,txt);
            }
            else{
                //printf("appendB %d bytes\n",strlen(txt));
                strcat(buffer,txt);
            }
        }
        else{
            //printf("appendA %d bytes\n",strlen(txt));
            strcat(buffer,txt);
        }

    }
    else { // send a null buffer to flush/free
        if(buffer) {
            if(strlen(buffer)>0){
                //printf("final flush %d bytes\n",strlen(buffer));
                httpd_resp_sendstr_chunk(req,buffer);
            }
            free(buffer);
            buffer = NULL;
            //printf("buffer freed\n");
        }
        //printf("final notify\n");
        httpd_resp_sendstr_chunk(req,NULL);
    }
}

void bufferedhttpd_resp_sendjson_chunk(httpd_req_t *req, char const *txt) {
    // json escapes \ " and control chars
    int n = strlen(txt);
//    printf("n=%d\n",n);
    char c;
    for(int i = 0; i<n; i+=1){
        char out[16];
        c = txt[i];
        if(c == '\\' || c == '\"' || c < ' ') {
            snprintf(out,DIM(out),"\\u%04x",c);
        }
        else {
            out[0] = c;
            out[1] = 0;
        }
        bufferedhttpd_resp_sendstr_chunk(req, out);
    }


    
}

void bufferedhttpd_resp_send_chunk(httpd_req_t *req, char const *c, size_t n) {
    if(buffer && strlen(buffer)>0) {
        httpd_resp_sendstr_chunk(req,buffer);
        buffer[0] = 0;
    }
    httpd_resp_send_chunk(req,c,n);
}

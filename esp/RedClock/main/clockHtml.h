/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

// clockHtml.h

//typedef void (*html_cb_t)(FILE *fd, httpd_req_t *req, void *userdata);// callbacks: https://stackoverflow.com/a/147241
// https://stackoverflow.com/users/23224/russell-bryant
//struct HtmlParm {
//    char *name;
//    html_cb_t cb;
//};

//void makeHtmlFrom(char *filename, httpd_req_t *req, int nparms, struct HtmlParm *htmlparms);
void bufferedhttpd_resp_sendstr_chunk( httpd_req_t *req, char const *txt);
void bufferedhttpd_resp_send_chunk(httpd_req_t *req, char const *c, size_t n);
void bufferedhttpd_resp_sendjson_chunk(httpd_req_t *req, char const *txt);


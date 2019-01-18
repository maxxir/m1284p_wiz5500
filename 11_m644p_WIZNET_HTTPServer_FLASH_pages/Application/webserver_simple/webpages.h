/*
 * webpages.h
 *
 *  Created on: 05 дек. 2018 г.
 *      Author: maxx
 */

#ifndef WEBPAGES_H_
#define WEBPAGES_H_

#define index_page \
"<html><style>body {  max-width: 480;  margin: 0 auto;  padding: 0 5px;}h1,h3 {  text-align: center;}</style><body><span style=\"color:#0000A0\">\n"\
"<h1>W5500 Simple Web Server</h1><hr>\n"\
"<h3>AVR Mega1284p and WIZ5500</h3><hr>\n"\
"<p><form method=\"POST\">\n"\
"<strong>Uptime: <input type=\"text\" size=2 value=\"%lu\"> sec\n"\
"<p><input type=\"radio\" name=\"radio\" value=\"0\" %s>LED1 OFF\n"\
"<br><input type=\"radio\" name=\"radio\" value=\"1\" %s>LED1 ON\n"\
"<p>\n"\
"<input type=\"submit\" value=\"Update data\">\n"\
"</strong></form></span></body></html>\n"

#define page_404 \
"HTTP/1.0 404 Not Found\r\n"\
"Content-Type: text/html\r\n"\
"\r\n"\
"<!DOCTYPE HTML><html><h2>404 Not Found</h2></html>"

#endif /* WEBPAGES_H_ */

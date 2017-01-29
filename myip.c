/****************************************************************************
* Copyright © 2017 Alessandro Spallina
* email: alessandrospallina1@gmail.com
* github: https://github.com/AlessandroSpallina
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#define IPIFY "api.ipify.org"
#define HTTP_QUERY              \
        "GET / HTTP/1.1\r\n"    \
        "Host: "IPIFY "\r\n"    \
        "User-Agent: myip\r\n"  \
        "Connection: close\r\n" \
        "\r\n"

int ipify_connect (int *fd)
{
        struct addrinfo hints, *res;
        int ret;

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        ret = getaddrinfo (IPIFY, "http", &hints, &res);
        if (ret != 0) {
                //dns query fails
                return ret;
        }

        (*fd) = socket(AF_INET, SOCK_STREAM, 0);
        ret = connect((*fd), res->ai_addr, res->ai_addrlen);
        freeaddrinfo(res);

        if (ret < 0) {
                close(*fd);
                return -1;
        }

        return 0;
}

int http_request (int fd, char *address)
{
        int ret;
        char *ip;
        char response[BUFSIZ];

        ret = send (fd, HTTP_QUERY, strlen(HTTP_QUERY), 0);

        if (ret < 0) {
                close(fd);
                return -1;
        }

        ret = recv (fd, response, BUFSIZ, 0);
        if (ret < 0) {
                close(fd);
                return -2;
        }

        if ((strncmp(response, "HTTP/1.1 200 OK", 15) == 0)) {
                ip = strstr(response, "\r\n\r\n");
                if (ip) {
                        sscanf(ip, "\r\n\r\n%s", address);
                } else {
                        close(fd);
                        return -3;
                }
        } else {
                close(fd);
                return -4;
        }

        return 0;
}

int ipify_disconnect (int fd)
{
        return (close(fd));
}


// Returns 1 on error, 0 if success
int main ()
{
        int fd, ret;
        char address[BUFSIZ];

        ret = ipify_connect(&fd);

        switch (ret) {
        case 0:
                ret = http_request(fd, address);
                switch (ret) {
                case 0:
                        ret = ipify_disconnect(fd);
                        if (!ret)
                                printf("%s\n", address);
                        else
                                fprintf(stderr, "Close(fd) Fails\n");
                        break;

                case -1:
                        fprintf(stderr, "HTTP Query (send) fails\n");
                        return 1;
                case -2:
                        fprintf(stderr, "HTTP Query (recv) fails\n");
                        return 1;

                case -3:
                        fprintf(stderr, "Strstr() Fails");
                        return 1;

                case -4:
                        fprintf(stderr, "Bad Response from <%s>", IPIFY);
                        return 1;
                }
                break;

        case -1:
                fprintf(stderr, "Connection Fails\n");
                return 1;

        default:
                gai_strerror(ret);
                return 1;
        }

        return 0;
}

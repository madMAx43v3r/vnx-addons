/*
 * poll.h
 *
 *  Created on: May 19, 2021
 *      Author: jaw
 */

#ifndef __POLL_H
#define __POLL_H

#include <cstddef>


#define POLLIN 0x0001
//#define POLLPRI 0x0002
#define POLLOUT 0x0004
#define POLLERR 0x0008
//#define POLLHUP 0x0010
//#define POLLNVAL 0x0020

struct pollfd {
	int fd;
	short events;
	short revents;
};

int poll (pollfd *fds, size_t num, int timeout);


#endif

/*
 * This file is part of libmonome.
 * libmonome is copyright 2007, 2008 will light <visinin@gmail.com>
 *
 * libmonome is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "monome.h"
#include "monome_internal.h"

int monome_platform_open(monome_t *monome) {
	struct termios nt, ot;
	int fd;
	
	if( (fd = open(monome->dev, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0 ) {
		perror("libmonome: could not open monome device");
		return 1;
	}

	tcgetattr(fd, &ot);
	nt = ot;
	
	/* baud rate (9600) */
	cfsetispeed(&nt, B9600);
	cfsetospeed(&nt, B9600);

	/* parity (8N1) */
	nt.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
	nt.c_cflag |=  (CS8 | CLOCAL | CREAD);

	/* no line processing */
	nt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN);

	/* raw input */
	nt.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK |
	                INPCK | ISTRIP | IXON);

	/* raw output */
	nt.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR |
	                OFILL | OLCUC | OPOST);

	/* block until one character is read */
	nt.c_cc[VMIN]  = 1;
	nt.c_cc[VTIME] = 0;

	if( tcsetattr(fd, TCSANOW, &nt) < 0 ) {
		perror("libmonome: could not set terminal attributes");
		return 1;
	}

	tcflush(fd, TCIOFLUSH);

	monome->fd = fd;
	monome->ot = ot;

	return 0;
}

ssize_t monome_platform_write(monome_t *monome, const uint8_t *buf, ssize_t bufsize) {
	return write(monome->fd, buf, bufsize);
}

ssize_t monome_platform_read(monome_t *monome, uint8_t *buf, ssize_t count) {
	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(monome->fd, &fds);

	if( select(monome->fd + 1, &fds, NULL, NULL, NULL) < 0 )
		perror("libmonome: error in select()");

	return read(monome->fd, buf, count);
}

/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2000
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation; either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   A simple filtering streambuf emitting to a C like file descriptor.
 *   Gwenole Beauchesne 2002/05/13.
 *
 *   NOTE: this is a quick and dirty hack inspired from "The GNU C++
 *   Iostream Library", Reference Manual for libio 0.64.
 */

#ifndef STDIO_FILEBUF_H
#define STDIO_FILEBUF_H

#include <cstdio>
#include <iostream>

class stdio_filebuf: public std::streambuf
{
	FILE * file;
public:
	stdio_filebuf(FILE *fp): file(fp) { }
	int sync();
	int overflow(int ch);
	std::streamsize xsputn (char *text, std::streamsize n);
};

inline int
stdio_filebuf::sync()
{
	size_t n = pptr() - pbase();
	return (n && fwrite(pbase(), 1, n, file) != n) ? EOF : 0;
}

inline int
stdio_filebuf::overflow(int ch)
{
	size_t n = pptr() - pbase();
	if (n && sync())
		return EOF;
	if (ch != EOF) {
		if (fputc(ch, file) == EOF)
			return EOF;
	}
	pbump(-n);
	return 0;
}

inline std::streamsize
stdio_filebuf::xsputn(char * text, std::streamsize n)
{
	return sync() == EOF ? 0 : fwrite(text, 1, n, file);
}

#endif /* STDIO_FILEBUF_H */

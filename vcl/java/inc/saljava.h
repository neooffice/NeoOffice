/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU General Public License Version 2.1
 *
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SALJAVA_H
#define _SALJAVA_H

#ifdef UNX

#include <sys/time.h>
#include <unxmacxp_protos.h>

#define capacityof(a)	(sizeof(a)/sizeof(*a))

inline long Divide( long nDividend, long nDivisor )
{ return (nDividend + nDivisor/2) / nDivisor; }

inline long DPI( long pixel, long mm )
{ return Divide( pixel*254, mm*10 ); }

inline int operator >= ( const timeval &t1, const timeval &t2 )
{
	if( t1.tv_sec == t2.tv_sec )
		return t1.tv_usec >= t2.tv_usec;
	return t1.tv_sec > t2.tv_sec;
}

inline int operator > ( const timeval &t1, const timeval &t2 )
{
	if( t1.tv_sec == t2.tv_sec )
		return t1.tv_usec > t2.tv_usec;
	return t1.tv_sec > t2.tv_sec;
}

inline int operator == ( const timeval &t1, const timeval &t2 )
{
	if( t1.tv_sec == t2.tv_sec )
		return t1.tv_usec == t2.tv_usec;
	return FALSE;
}

inline timeval &operator -= ( timeval &t1, const timeval &t2 )
{
	if( t1.tv_usec < t2.tv_usec )
	{
		t1.tv_sec--;
		t1.tv_usec += 1000000;
	}
	t1.tv_sec  -= t2.tv_sec;
	t1.tv_usec -= t2.tv_usec;
	return t1;
}

inline timeval &operator += ( timeval &t1, const timeval &t2 )
{
	t1.tv_sec  += t2.tv_sec;
	t1.tv_usec += t2.tv_usec;
	if( t1.tv_usec > 1000000 )
	{
		t1.tv_sec++;
		t1.tv_usec -= 1000000;
	}
	return t1;
}

inline timeval &operator += ( timeval &t1, ULONG t2 )
{
	t1.tv_sec  += t2 / 1000;
	t1.tv_usec += t2 ? (t2 % 1000) * 1000 : 500;
	if( t1.tv_usec > 1000000 )
	{
		t1.tv_sec++;
		t1.tv_usec -= 1000000;
	}
	return t1;
}

inline timeval operator + ( const timeval &t1, const timeval &t2 )
{
	timeval t0 = t1;
	return t0 += t2;
}

inline timeval operator + ( const timeval &t1, ULONG t2 )
{
	timeval t0 = t1;
	return t0 += t2;
}

inline timeval operator - ( const timeval &t1, const timeval &t2 )
{
	timeval t0 = t1;
	return t0 -= t2;
}
#endif // UNX

#endif // _SV_SALJAVA_H

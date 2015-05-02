/*************************************************************************
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of NeoOffice.
 *
 * NeoOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * NeoOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 3 along with NeoOffice.  If not, see
 * <http://www.gnu.org/licenses/gpl-3.0.txt>
 * for a copy of the GPLv3 License.
 *
 * Modified May 2015 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/
#ifndef _GLOBNAME_HXX
#define _GLOBNAME_HXX

#include "tools/toolsdllapi.h"
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/string.hxx>
#include <tools/list.hxx>

/*************************************************************************
*************************************************************************/
struct ImpSvGlobalName
{
    BYTE        szData[ 16 ];
    USHORT      nRefCount;

                ImpSvGlobalName()
                {
                    nRefCount = 0;
                }
                ImpSvGlobalName( const ImpSvGlobalName & rObj );
                ImpSvGlobalName( int );

    BOOL    operator == ( const ImpSvGlobalName & rObj ) const;
};

#ifdef USE_JAVA
// locker die Struktur von Windows kopiert
#ifdef WNT
struct _GUID
#else
struct GUID
#endif
{
	UINT32 Data1;
	UINT16 Data2;
	UINT16 Data3;
	BYTE   Data4[8];
};
#else	// USE_JAVA
#ifdef WNT
struct _GUID;
typedef struct _GUID GUID;
#else
struct GUID;
#endif
#endif	// USE_JAVA
typedef GUID CLSID;
class SvStream;
class SvGlobalNameList;
class TOOLS_DLLPUBLIC SvGlobalName
{
friend class SvGlobalNameList;
    ImpSvGlobalName * pImp;
    void    NewImp();
public:
            SvGlobalName();
            SvGlobalName( const SvGlobalName & rObj )
            {
                pImp = rObj.pImp;
                pImp->nRefCount++;
            }
            SvGlobalName( ImpSvGlobalName * pImpP )
            {
                pImp = pImpP;
                pImp->nRefCount++;
            }
            SvGlobalName( UINT32 n1, USHORT n2, USHORT n3,
                          BYTE b8, BYTE b9, BYTE b10, BYTE b11,
                          BYTE b12, BYTE b13, BYTE b14, BYTE b15 );

            // create SvGlobalName from a platform independent representation
            SvGlobalName( const ::com::sun::star::uno::Sequence< sal_Int8 >& aSeq );

    SvGlobalName & operator = ( const SvGlobalName & rObj );
            ~SvGlobalName();

    TOOLS_DLLPUBLIC friend SvStream & operator >> ( SvStream &, SvGlobalName & );
    TOOLS_DLLPUBLIC friend SvStream & operator << ( SvStream &, const SvGlobalName & );

    BOOL            operator < ( const SvGlobalName & rObj ) const;
    SvGlobalName &  operator += ( UINT32 );
    SvGlobalName &  operator ++ () { return operator += ( 1 ); }

    BOOL    operator == ( const SvGlobalName & rObj ) const;
    BOOL    operator != ( const SvGlobalName & rObj ) const
            { return !(*this == rObj); }

    void    MakeFromMemory( void * pData );
    BOOL    MakeId( const String & rId );
    String  GetctorName() const;
    String  GetHexName() const;
    String  GetRegDbName() const
			{
				String a = '{';
				a += GetHexName();
				a += '}';
				return a;
			}

                  SvGlobalName( const CLSID & rId );
    const CLSID & GetCLSID() const { return *(CLSID *)pImp->szData; }
	const BYTE* GetBytes() const { return pImp->szData; }

    // platform independent representation of a "GlobalName"
    // maybe transported remotely
    com::sun::star::uno::Sequence < sal_Int8 > GetByteSequence() const;
};

class SvGlobalNameList
{
    List aList;
public:
                    SvGlobalNameList();
                    ~SvGlobalNameList();

    void            Append( const SvGlobalName & );
    SvGlobalName    GetObject( ULONG );
    BOOL            IsEntry( const SvGlobalName & rName );
    ULONG           Count() const { return aList.Count(); }
private:
                // nicht erlaubt
                SvGlobalNameList( const SvGlobalNameList & );
    SvGlobalNameList & operator = ( const SvGlobalNameList & );
};

#endif // _GLOBNAME_HXX


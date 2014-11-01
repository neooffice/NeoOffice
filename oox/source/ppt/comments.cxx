/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "oox/ppt/comments.hxx"
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <rtl/math.hxx>

#if SUPD == 310
#include <tools/string.hxx>
#endif	// SUPD == 310


namespace oox { namespace ppt {

void CommentAuthorList::setValues(const CommentAuthorList& list)
{
    std::vector<CommentAuthor>::const_iterator it;
    for(it = list.cmAuthorLst.begin(); it != list.cmAuthorLst.end(); ++it)
    {
        CommentAuthor temp;
        cmAuthorLst.push_back(temp);
        cmAuthorLst.back().clrIdx = it->clrIdx;
        cmAuthorLst.back().id = it->id;
        cmAuthorLst.back().initials = it->initials;
        cmAuthorLst.back().lastIdx = it->lastIdx;
        cmAuthorLst.back().name = it->name;
    }
}

//DateTime is saved as : 2013-01-10T15:53:26.000
void Comment::setDateTime (const OUString& _datetime)
{
#if SUPD == 310
    String datetime = _datetime;
    aDateTime.Year = datetime.GetToken(0,(sal_Unicode)'-').ToInt32();
    aDateTime.Month = datetime.GetToken(1,(sal_Unicode)'-').ToInt32();
    aDateTime.Day = datetime.GetToken(2,(sal_Unicode)'-').ToInt32();
    datetime = datetime.GetToken(1,(sal_Unicode)'T');
    aDateTime.Hours = datetime.GetToken(0,(sal_Unicode)':').ToInt32();
    aDateTime.Minutes = datetime.GetToken(1,(sal_Unicode)':').ToInt32();
    double seconds = datetime.GetToken(2,(sal_Unicode)':').ToDouble();
#else	// SUPD == 310
    OUString datetime = _datetime;
    aDateTime.Year = datetime.getToken(0,'-').toInt32();
    aDateTime.Month = datetime.getToken(1,'-').toInt32();
    aDateTime.Day = datetime.getToken(2,'-').toInt32();
    datetime = datetime.getToken(1,'T');
    aDateTime.Hours = datetime.getToken(0,':').toInt32();
    aDateTime.Minutes = datetime.getToken(1,':').toInt32();
    double seconds = datetime.getToken(2,':').toDouble();
#endif	// SUPD == 310
    aDateTime.Seconds = floor(seconds);
    seconds -= aDateTime.Seconds;
    aDateTime.NanoSeconds = ::rtl::math::round(seconds * 1000000000);
#if SUPD == 310
    aDateTime.HundredthSeconds = ::rtl::math::round(seconds * 100);
#endif	// SUPD == 310
    const int secondsOverflow = (aDateTime.Seconds == 60) ? 61 : 60;
    // normalise time part of aDateTime
    if (aDateTime.NanoSeconds == 1000000000)
    {
        aDateTime.NanoSeconds = 0;
#if SUPD == 310
        aDateTime.HundredthSeconds = 0;
#endif	// SUPD == 310
        ++aDateTime.Seconds;
    }
    if (aDateTime.Seconds == secondsOverflow)
    {
        aDateTime.Seconds = 0;
        ++aDateTime.Minutes;
    }
    if (aDateTime.Minutes == 60)
    {
        aDateTime.Minutes = 0;
        ++aDateTime.Hours;
    }
    // if overflow goes into date, I give up
}

OUString Comment::getAuthor ( const CommentAuthorList& list )
{
    const sal_Int32 nId = authorId.toInt32();
    std::vector<CommentAuthor>::const_iterator it;
    for(it = list.cmAuthorLst.begin(); it != list.cmAuthorLst.end(); ++it)
    {
        if(it->id.toInt32() == nId)
            return it->name;
    }
    return OUString("Anonymous");
}

const Comment& CommentList::getCommentAtIndex (int index)
{
    if(index < (int)cmLst.size() && index >= 0)
        return cmLst.at(index);
    else
        throw css::lang::IllegalArgumentException();
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

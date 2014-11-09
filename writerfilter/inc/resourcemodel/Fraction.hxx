/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_WRITERFILTER_INC_RESOURCEMODEL_FRACTION_HXX
#define INCLUDED_WRITERFILTER_INC_RESOURCEMODEL_FRACTION_HXX

#include <sal/types.h>


namespace writerfilter {
namespace resourcemodel {

class Fraction
{
public:
    explicit Fraction(sal_Int32 nNumerator, sal_Int32 nDenominator = 1);
    virtual ~Fraction();

    void init(sal_Int32 nNumerator, sal_Int32 nDenominator);
    void assign(const Fraction & rFraction);

    Fraction inverse() const;

    Fraction operator=(const Fraction & rFraction);
    Fraction operator+(const Fraction & rFraction) const;
    Fraction operator-(const Fraction & rFraction) const;
    Fraction operator*(const Fraction & rFraction) const;
    Fraction operator/(const Fraction & rFraction) const;
    operator sal_Int32() const;
    operator float() const;

private:
    sal_Int32 mnNumerator;
    sal_Int32 mnDenominator;
};
}}
#endif // INCLUDED_WRITERFILTER_INC_RESOURCEMODEL_FRACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

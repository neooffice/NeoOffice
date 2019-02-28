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
 * 
 *   Modified May 2018 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef INCLUDED_SFX2_SOURCE_INC_ALIENWARN_HXX
#define INCLUDED_SFX2_SOURCE_INC_ALIENWARN_HXX

#include <vcl/button.hxx>
#include <vcl/layout.hxx>

class SfxAlienWarningDialog : public MessageDialog
{
private:
    VclPtr<PushButton>             m_pKeepCurrentBtn;
    VclPtr<PushButton>             m_pUseDefaultFormatBtn;
    VclPtr<CheckBox>               m_pWarningOnBox;

public:
    SfxAlienWarningDialog(vcl::Window* pParent, const OUString& _rFormatName,
#ifdef USE_JAVA
                          const OUString& _rDefaultExtension, bool rDefaultIsAlien, bool bHideWarningOnBox);
#else	// USE_JAVA
                          const OUString& _rDefaultExtension, bool rDefaultIsAlien);
#endif	// USE_JAVA
    virtual ~SfxAlienWarningDialog() override;
    virtual void dispose() override;
};

#endif // INCLUDED_SFX2_SOURCE_INC_ALIENWARN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

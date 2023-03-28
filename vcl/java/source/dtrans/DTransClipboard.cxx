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

#include "DTransClipboard.hxx"
#include "DTransTransferable.hxx"

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::uno;

// ============================================================================

DTransTransferable *DTransClipboard::getContents()
{
	DTransTransferable *out = new DTransTransferable( TRANSFERABLE_TYPE_CLIPBOARD );
	if ( out )
		out->updateChangeCount();

	return out;
}

// ----------------------------------------------------------------------------

DTransTransferable *DTransClipboard::setContents( const Reference< XTransferable > &xTransferable )
{
	if ( !xTransferable.is() )
		return NULL;

	DTransTransferable *out = new DTransTransferable( TRANSFERABLE_TYPE_CLIPBOARD );
	if ( out && !out->setContents( xTransferable ) )
	{
		delete out;
		out = NULL;
	}

	return out;
}

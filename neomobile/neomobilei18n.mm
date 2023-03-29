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

#import "neomobilei18n.hxx"
#import <map>

static ::std::map< ::rtl::OUString, NSDictionary* > aLocalizationMap;
static NSDictionary *pDefaultLocaleDict = nil;
static NSDictionary *pPrimaryLocaleDict = nil;
static NSDictionary *pSecondaryLocaleDict = nil;
static NSDictionary *pTertiaryLocaleDict = nil;

using namespace com::sun::star::lang;
using namespace rtl;

/**
 * Translated strings for de locale
 */
static const sal_Char *pEntries_de[] = {
	NEOMOBILEABOUT, "Über NeoOffice Mobile",
	NEOMOBILEBACK, "Zurück",
	NEOMOBILECANCEL, "Abbrechen",
	NEOMOBILECREATEACCOUNT, "Neues Benutzerkonto erstellen",
	NEOMOBILEDOWNLOADCANCELED, "Herunterladen abgebrochen",
	NEOMOBILEDOWNLOADFAILED, "Herunterladen fehlgeschlagen",
	NEOMOBILEDOWNLOADINGFILE, "Datei wird heruntergeladen",
	NEOMOBILEERROR, "Fehler:",
	NEOMOBILEEXPORTINGFILE, "Datei wird exportiert…",
	NEOMOBILEFORGOTPASSWORD, "Passwort vergessen?",
	NEOMOBILELOADING, "Laden…",
	NEOMOBILELOGIN, "Anmelden",
	NEOMOBILELOGINTITLE, "NeoOffice Mobile Anmeldung",
	NEOMOBILEPASSWORD, "Passwort:",
	NEOMOBILESAVEPASSWORD, "Passwort speichern",
	NEOMOBILEUPLOADCONTINUE, "Möchten Sie dieses Dokument noch immer hochladen?",
	NEOMOBILEUPLOADINGFILE, "Datei wird hochgeladen…",
	NEOMOBILEUPLOADPASSWORDPROTECTED, "Dieses Dokument wird ohne jeglichen Passwortschutz hochgeladen",
	NEOMOBILEUPLOAD, "Hochladen ",
	NEOMOBILEUSERNAME, "Benutzername:",
	nil, nil
};

/**
 * Translated strings for en_US locale
 */
static const sal_Char *pEntries_en_US[] = {
	NEOMOBILEABOUT, "About NeoOffice Mobile",
	NEOMOBILEBACK, "Back",
	NEOMOBILECANCEL, "Cancel",
	NEOMOBILECREATEACCOUNT, "Create a new account",
	NEOMOBILEDOWNLOADCANCELED, "Download canceled",
	NEOMOBILEDOWNLOADFAILED, "Download failed",
	NEOMOBILEDOWNLOADINGFILE, "Downloading file",
	NEOMOBILEERROR, "Error:",
	NEOMOBILEEXPORTINGFILE, "Exporting file…",
	NEOMOBILEFORGOTPASSWORD, "Forgot password?",
	NEOMOBILELOADING, "Loading…",
	NEOMOBILELOGIN, "Login",
	NEOMOBILELOGINTITLE, "NeoOffice Mobile Login",
	NEOMOBILEMEGABYTE, "MB",
	NEOMOBILEPASSWORD, "Password:",
	NEOMOBILEPRODUCTNAME, "NeoOffice Mobile",
	NEOMOBILESAVEPASSWORD, "Save password",
	NEOMOBILEUPLOADCONTINUE, "Do you still want to upload this document?",
	NEOMOBILEUPLOADINGFILE, "Uploading file…",
	NEOMOBILEUPLOADPASSWORDPROTECTED, "This document will be uploaded without any password protection",
	NEOMOBILEUPLOAD, "Upload",
	NEOMOBILEUSERNAME, "User name:",
	nil, nil
};

/**
 * Translated strings for es locale
 */
static const sal_Char *pEntries_es[] = {
	NEOMOBILECANCEL, "Cancelar",
	NEOMOBILEDOWNLOADCANCELED, "Descarga cancelada",
	NEOMOBILEDOWNLOADFAILED, "Falló la descarga",
	NEOMOBILEDOWNLOADINGFILE, "Descargando archivo",
	NEOMOBILEERROR, "Error:",
	NEOMOBILEEXPORTINGFILE, "Exportando archivo…",
	NEOMOBILELOADING, "Cargando…",
	NEOMOBILELOGIN, "Iniciar sesión",
	NEOMOBILEPRODUCTNAME, "NeoOffice Móvil",
	NEOMOBILEUPLOADINGFILE, "Subiendo archivo…",
	nil, nil
};

/**
 * Translated strings for fr locale
 */
static const sal_Char *pEntries_fr[] = {
	NEOMOBILEABOUT, "À propos de NeoOffice Mobile",
	NEOMOBILEBACK, "Arrière",
	NEOMOBILECANCEL, "Annuler",
	NEOMOBILECREATEACCOUNT, "Créer un nouveau compte",
	NEOMOBILEDOWNLOADCANCELED, "Téléchargement annulé",
	NEOMOBILEDOWNLOADFAILED, "Echec du téléchargement",
	NEOMOBILEDOWNLOADINGFILE, "Téléchargement du fichier",
	NEOMOBILEERROR, "Erreur :",
	NEOMOBILEEXPORTINGFILE, "Exportation du fichier…",
	NEOMOBILEFORGOTPASSWORD, "Mot de passe oublié ?",
	NEOMOBILELOADING, "Chargement…",
	NEOMOBILELOGIN, "Connexion",
	NEOMOBILELOGINTITLE, "NeoOffice Mobile Connexion",
	NEOMOBILEMEGABYTE, "Mo",
	NEOMOBILEPASSWORD, "Mot de passe :",
	NEOMOBILEPRODUCTNAME, "NeoOffice Mobile",
	NEOMOBILESAVEPASSWORD, "Enregistrer mot de passe",
	NEOMOBILEUPLOADCONTINUE, "Souhaitez-vous quand-même envoyer ce document vers le serveur ?",
	NEOMOBILEUPLOADPASSWORDPROTECTED, "Ce fichier sera enregistré sur le serveur sans protection par mot de passe",
	NEOMOBILEUPLOADINGFILE, "Envoi du fichier vers le serveur…",
	NEOMOBILEUPLOAD, "Envoyer",
	NEOMOBILEUSERNAME, "Nom d'utilisateur :",
	nil, nil
};

/**
 * Translated strings for it locale
 */
static const sal_Char *pEntries_it[] = {
	NEOMOBILECANCEL, "Cancella",
	NEOMOBILEDOWNLOADCANCELED, "Trasferimento cancellato ",
	NEOMOBILEDOWNLOADFAILED, "Trasferimento fallito",
	NEOMOBILEDOWNLOADINGFILE, "Trasferimento del file",
	NEOMOBILEERROR, "Errore:",
	NEOMOBILEEXPORTINGFILE, "Esportazione del file…",
	NEOMOBILELOADING, "Caricamento…",
	NEOMOBILEPRODUCTNAME, "NeoOffice Mobile",
	NEOMOBILEUPLOADINGFILE, "Invio del file…",
	nil, nil
};

/**
 * Translated strings for nl locale
 */
static const sal_Char *pEntries_nl[] = {
	NEOMOBILECANCEL, "Annuleren",
	NEOMOBILEDOWNLOADCANCELED, "Ophalen geannuleerd",
	NEOMOBILEDOWNLOADFAILED, "Ophalen mislukt",
	NEOMOBILEDOWNLOADINGFILE, "Bestand ophalen",
	NEOMOBILEERROR, "Fout:",
	NEOMOBILEEXPORTINGFILE, "Bestand exporteren…",
	NEOMOBILELOADING, "Laden…",
	NEOMOBILEPRODUCTNAME, "NeoOffice Mobiel",
	NEOMOBILEUPLOADCONTINUE, "Wilt u dit document alsnog verzenden?",
	NEOMOBILEUPLOADINGFILE, "Bestand verzenden…",
	NEOMOBILEUPLOADPASSWORDPROTECTED, "Dit document wordt verzonden zonder bescherming met een wachtwoord",
	NEOMOBILEUPLOAD, "Verzend",
	nil, nil
};

/**
 * Translated strings for pt locale
 */
static const sal_Char *pEntries_pt[] = {
	NEOMOBILECANCEL, "Cancelar ",
	NEOMOBILEDOWNLOADCANCELED, "Transferência de arquivo cancelada",
	NEOMOBILEDOWNLOADFAILED, "Falha na transferência de arquivo",
	NEOMOBILEDOWNLOADINGFILE, "Transferindo arquivo",
	NEOMOBILEERROR, "Erro:",
	NEOMOBILEEXPORTINGFILE, "Exportando arquivo…",
	NEOMOBILELOADING, "Carregando…",
	NEOMOBILEPRODUCTNAME, "NeoOffice Móvel",
	NEOMOBILEUPLOADINGFILE, "Enviando arquivo…",
	nil, nil
};

static OUString aDelimiter = OUString::createFromAscii( "_" );

static OUString ImplGetLocaleString( Locale aLocale )
{
	OUString aLocaleString( aLocale.Language );
	if ( aLocaleString.getLength() && aLocale.Country.getLength() )
	{
		aLocaleString += aDelimiter;
		aLocaleString += aLocale.Country;
		if ( aLocale.Variant.getLength() )
		{
			aLocaleString += aDelimiter;
			aLocaleString += aLocale.Variant;
		}
	}
	return aLocaleString;
}

static void InitializeLocale( OUString aLocale, const sal_Char **pEntries )
{
	if ( !aLocale.getLength() || !pEntries )
		return;

	::std::map< OUString, NSDictionary* >::const_iterator it = aLocalizationMap.find( aLocale );
	if ( it != aLocalizationMap.end() )
		return;

	NSMutableDictionary *pDict = [NSMutableDictionary dictionaryWithCapacity:20];
	if ( !pDict )
		return;

	// Ensure that the dictionary never gets released
	[pDict retain];

	// Iterate through entries until a nil entry is found
	for ( size_t i = 0; pEntries[ i ] && pEntries[ i + 1 ]; i += 2 )
		[pDict setObject:[NSString stringWithUTF8String:pEntries[ i + 1 ]] forKey:[NSString stringWithUTF8String:pEntries[ i ]]];

	aLocalizationMap[ aLocale ] = pDict;
}

/**
 * Lookup a string and retrieve a translated string.  If no translation
 * is available, default to "en-US".
 */
NSString *NeoMobileGetLocalizedString( const sal_Char *key )
{
	if ( !key || !strlen( key ) )
		return @"";

	NSString *pKey = [NSString stringWithUTF8String:key];
	if ( !pKey )
		return @"";

	NSString *pRet = nil;

	if ( !aLocalizationMap.size() )
	{
		// Initialize dictionaries
		InitializeLocale( ImplGetLocaleString( Locale( OUString( RTL_CONSTASCII_USTRINGPARAM( "de" ) ), OUString(), OUString() ) ), pEntries_de );
		InitializeLocale( ImplGetLocaleString( Locale( OUString( RTL_CONSTASCII_USTRINGPARAM( "en" ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "US" ) ), OUString() ) ), pEntries_en_US );
		InitializeLocale( ImplGetLocaleString( Locale( OUString( RTL_CONSTASCII_USTRINGPARAM( "es" ) ), OUString(), OUString() ) ), pEntries_es );
		InitializeLocale( ImplGetLocaleString( Locale( OUString( RTL_CONSTASCII_USTRINGPARAM( "fr" ) ), OUString(), OUString() ) ), pEntries_fr );
		InitializeLocale( ImplGetLocaleString( Locale( OUString( RTL_CONSTASCII_USTRINGPARAM( "it" ) ), OUString(), OUString() ) ), pEntries_it );
		InitializeLocale( ImplGetLocaleString( Locale( OUString( RTL_CONSTASCII_USTRINGPARAM( "nl" ) ), OUString(), OUString() ) ), pEntries_nl );
		InitializeLocale( ImplGetLocaleString( Locale( OUString( RTL_CONSTASCII_USTRINGPARAM( "pt" ) ), OUString(), OUString() ) ), pEntries_pt );

		// Set locale dictionaries based on default locale
		Locale aLocale( NeoMobileGetApplicationLocale() );

		// Check if locale exists in our list of locales. Note that we ignore
		// variant at this time as no variant-specific localizations are
		// planned for this component yet.
		OUString aDefaultLocale = ImplGetLocaleString( Locale( OUString( RTL_CONSTASCII_USTRINGPARAM( "en" ) ), OUString( RTL_CONSTASCII_USTRINGPARAM( "US" ) ), OUString() ) );
		::std::map< OUString, NSDictionary* >::const_iterator it = aLocalizationMap.find( aDefaultLocale );
		if ( it != aLocalizationMap.end() )
			pDefaultLocaleDict = it->second;

		OUString aPrimaryLocale = ImplGetLocaleString( aLocale );
		it = aLocalizationMap.find( aPrimaryLocale );
		if ( it != aLocalizationMap.end() && pDefaultLocaleDict != it->second )
			pPrimaryLocaleDict = it->second;

		OUString aSecondaryLocale = ImplGetLocaleString( Locale( aLocale.Language, aLocale.Country, OUString() ) );
		it = aLocalizationMap.find( aSecondaryLocale );
		if ( it != aLocalizationMap.end() && pDefaultLocaleDict != it->second && pPrimaryLocaleDict != it->second )
			pSecondaryLocaleDict = it->second;

		OUString aTertiaryLocale = ImplGetLocaleString( Locale( aLocale.Language, OUString(), OUString() ) );
		it = aLocalizationMap.find( aTertiaryLocale );
		if ( it != aLocalizationMap.end() && pDefaultLocaleDict != it->second && pPrimaryLocaleDict != it->second && pSecondaryLocaleDict != it->second )
			pTertiaryLocaleDict = it->second;
	}

	if ( !pRet && pPrimaryLocaleDict )
		pRet = (NSString *)[pPrimaryLocaleDict objectForKey:pKey];

	if ( !pRet && pSecondaryLocaleDict )
		pRet = (NSString *)[pSecondaryLocaleDict objectForKey:pKey];

	if ( !pRet && pTertiaryLocaleDict )
		pRet = (NSString *)[pTertiaryLocaleDict objectForKey:pKey];

	if ( !pRet && pDefaultLocaleDict )
		pRet = (NSString *)[pDefaultLocaleDict objectForKey:pKey];

	if ( !pRet )
		pRet = pKey;

	return pRet;
}

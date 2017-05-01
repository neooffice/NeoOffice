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
 *		 - GNU General Public License Version 2.1
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2012 by Planamesa Inc.
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
 *************************************************************************/

#include "update_java.hxx"
#include "updatecheck.hxx"

#include <list>
#include <map>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <osl/process.h>

static std::set< OUString > aPackageNamesSet;
static std::map< OUString, OUString > aDownloadPathsMap;
static std::map< OUString, OUString > aPackagePathsMap;
static osl::Mutex aPackagesMutex;

void UpdateAddInstallerPackage(OUString aName, OUString aDownloadPath, OUString aPackagePath)
{
	if (aName.getLength() && aDownloadPath.getLength() && aPackagePath.getLength())
	{
		osl::MutexGuard aGuard(aPackagesMutex);

		aPackageNamesSet.insert(aName);
		aDownloadPathsMap[aName] = aDownloadPath;
		aPackagePathsMap[aName] = aPackagePath;
	}
}

sal_Bool UpdateHasPackagePaths()
{
	sal_Bool bRet = sal_False;

	osl::ClearableMutexGuard aGuard(aPackagesMutex);
	if (aPackageNamesSet.size())
		bRet = sal_True;
	aGuard.clear();

	return bRet;
}

void UpdateInstallNextBatchOfInstallerPackagePaths()
{
	OUString aExeURL;
	osl_getExecutableFile(&aExeURL.pData);
	sal_uInt32 nLastIndex = aExeURL.lastIndexOf('/');
	if (nLastIndex > 0)
	{
		aExeURL = aExeURL.copy(0, nLastIndex+1);
		aExeURL += "updchkruninstallers";
#ifdef WNT
		aExeURL += ".exe";
#else	// WNT
		aExeURL += ".bin";
#endif	// WNT

		osl::ClearableMutexGuard aGuard(aPackagesMutex);

#ifdef MACOSX
		OUString aMountPackageExeURL("file:///usr/bin/hdiutil");
		OUString aMountPackageAttachArg("attach");
		OUString aMountPackagePlistArg("-plist");
		rtl_uString *pMountPackageArgs[ 3 ];
		pMountPackageArgs[ 0 ] = aMountPackageAttachArg.pData;
		pMountPackageArgs[ 1 ] = aMountPackagePlistArg.pData;
		pMountPackageArgs[ 2 ] = NULL;
#endif	// MACOSX

		// Remove first batch of package paths. Note that we assume that any
		// package path with "Language_Pack" or "Patch" are not full installers
		// and that we assume that the product and version numbers lower in
		// sort order should be installed first.
		const OUString aLanguagePack("Language_Pack");
		const OUString aPatch("Patch");
		std::list< oslProcess > aMountPackageProcessList;
		std::list< OUString > aPackagePathsRunList;
		std::set< OUString >::iterator it = aPackageNamesSet.begin();
		while (it != aPackageNamesSet.end())
		{
			// Check if this is a full installer
			bool bFullInstaller = false;
			if ((*it).indexOf(aLanguagePack) < 0 && (*it).indexOf(aPatch) < 0)
			{
				if (aPackagePathsRunList.size())
					break;
				else
					bFullInstaller = true;
			}

			bool bPackagePathExists = false;
			std::map< OUString, OUString >::iterator pit = aPackagePathsMap.find(*it);
			if (pit != aPackagePathsMap.end())
			{
#ifdef MACOSX
				// Check if package directory exists
				OUString aURL;
				osl_getFileURLFromSystemPath(pit->second.pData, &aURL.pData);
				osl::Directory aDir(aURL);
				if (aDir.open() == osl::FileBase::E_None)
					bPackagePathExists = true;
				aDir.close();
#endif	// MACOSX

				aPackagePathsRunList.push_back(pit->second);
				aPackagePathsMap.erase(pit);
			}
			std::map< OUString, OUString >::iterator dit = aDownloadPathsMap.find(*it);
			if (dit != aDownloadPathsMap.end())
			{
#ifdef MACOSX
				// Run hdiutil command if the package has been unmounted
				if (!bPackagePathExists)
				{
					OUString aURL;
					osl_getFileURLFromSystemPath(dit->second.pData, &aURL.pData);
					osl::File aFile(aURL);
					if (aFile.open(osl_File_OpenFlag_Read | osl_File_OpenFlag_NoLock) == osl::FileBase::E_None)
					{
						pMountPackageArgs[ 2 ] = dit->second.pData;

						oslProcess aProcess = NULL;
						if (osl_executeProcess(aMountPackageExeURL.pData, pMountPackageArgs, 3, 0, NULL, NULL, NULL, 0, &aProcess) == osl_Process_E_None)
							aMountPackageProcessList.push_back(aProcess);
					}
				}
#endif	// MACOSX

				aDownloadPathsMap.erase(dit);
			}
			aPackageNamesSet.erase(it);
			it = aPackageNamesSet.begin();
		}

		aGuard.clear();

		bool bJoin = false;
		sal_uInt32 nPaths = aPackagePathsRunList.size();
		if (nPaths)
		{
#ifdef MACOSX
			rtl_uString *pArgs[nPaths];
#else	// MACOSX
			rtl_uString **pArgs = (rtl_uString **)rtl_allocateMemory(nPaths * sizeof(rtl_uString*));
#endif	// MACOSX
			sal_uInt32 nCurrentItem = 0;
			for (std::list< OUString >::const_iterator rit = aPackagePathsRunList.begin(); rit != aPackagePathsRunList.end() && nCurrentItem < nPaths; ++rit)
				pArgs[nCurrentItem++] = (*rit).pData;

			// Open a stdin pipe to the subprocess and don't close it and let
			// it leak to force the subprocess to not run the installers until
			// after the application has quit
			oslProcess aProcess = NULL;
			oslFileHandle aStdinHandle = NULL;
			if (osl_executeProcess_WithRedirectedIO(aExeURL.pData, pArgs, nCurrentItem, 0, NULL, NULL, NULL, 0, &aProcess, &aStdinHandle, NULL, NULL) == osl_Process_E_None)
			{
				bJoin = true;
				osl_freeProcessHandle(aProcess);
			}
#ifndef MACOSX
			rtl_freeMemory(pArgs);
#endif	// !MACOSX
		}

		TimeValue aTimeout;
		aTimeout.Seconds = 10;
		aTimeout.Nanosec = 0;
		for (std::list< oslProcess >::iterator mppit = aMountPackageProcessList.begin(); mppit != aMountPackageProcessList.end(); ++mppit)
		{
			if (bJoin && osl_joinProcessWithTimeout(*mppit, &aTimeout) == osl_Process_E_TimedOut)
				bJoin = false;
			osl_freeProcessHandle(*mppit);
		}
		aMountPackageProcessList.clear();
	}
}

void UpdateShutdownApp()
{
	rtl::Reference< UpdateCheck > aController(UpdateCheck::get());
	aController->shutdownApp();
}

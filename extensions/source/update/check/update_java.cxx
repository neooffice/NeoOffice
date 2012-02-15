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
#include <osl/mutex.hxx>
#include <osl/process.h>

static std::set< rtl::OUString > aPackageNamesSet;
static std::map< rtl::OUString, rtl::OUString > aDownloadPathsMap;
static std::map< rtl::OUString, rtl::OUString > aPackagePathsMap;
static osl::Mutex aPackagesMutex;

void UpdateAddInstallerPackage(rtl::OUString aName, rtl::OUString aDownloadPath, rtl::OUString aPackagePath)
{
	if (aName.getLength() && aDownloadPath.getLength() && aPackagePath.getLength())
	{
		osl::MutexGuard aGuard(aPackagesMutex);

		aPackageNamesSet.insert(aName);
		aDownloadPathsMap[aName] = aDownloadPath;
		aPackagePathsMap[aName] = aPackagePath;
	}
}

void UpdateInstallNextBatchOfInstallerPackagePaths()
{
	rtl::OUString aExeURL;
	osl_getExecutableFile(&aExeURL.pData);
	sal_uInt32 nLastIndex = aExeURL.lastIndexOf('/');
	if (nLastIndex > 0)
	{
		aExeURL = aExeURL.copy(0, nLastIndex+1);
		aExeURL += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("updchkruninstallers" SAL_PRGEXTENSION));

		osl::ClearableMutexGuard aGuard(aPackagesMutex);

		// Remove first batch of package paths. Note that we assume that any
		// package path with "Language_Pack" or "Patch" are not full installers
		// and that we assume that the product and version numbers lower in
		// sort order should be installed first.
		const rtl::OUString aLanguagePack(RTL_CONSTASCII_USTRINGPARAM("Language_Pack"));
		const rtl::OUString aPatch(RTL_CONSTASCII_USTRINGPARAM("Patch"));
		std::list< rtl::OUString > aPackagePathsRunList;
		std::set< rtl::OUString >::iterator it = aPackageNamesSet.begin();
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

			std::map< rtl::OUString, rtl::OUString >::iterator pit = aPackagePathsMap.find(*it);
			if (pit != aPackagePathsMap.end())
			{
				aPackagePathsRunList.push_back(pit->second);
				aPackagePathsMap.erase(pit);
			}
			std::map< rtl::OUString, rtl::OUString >::iterator dit = aDownloadPathsMap.find(*it);
			if (dit != aDownloadPathsMap.end())
				aDownloadPathsMap.erase(dit);
			aPackageNamesSet.erase(it);
			it = aPackageNamesSet.begin();
		}

		aGuard.clear();

		sal_uInt32 nPaths = aPackagePathsRunList.size();
		if (nPaths)
		{
			rtl_uString *pArgs[nPaths];
			sal_uInt32 nCurrentItem = 0;
			for (std::list< ::rtl::OUString >::const_iterator rit = aPackagePathsRunList.begin(); rit != aPackagePathsRunList.end() && nCurrentItem < nPaths; ++rit)
				pArgs[nCurrentItem++] = (*rit).pData;

			// Open a stdin pipe to the subprocess and don't close it and let
			// it leak to force the subprocess to not run the installers until
			// after the application has quit
			oslProcess aProcess;
			oslFileHandle aStdinHandle;
			osl_executeProcess_WithRedirectedIO(aExeURL.pData, pArgs, nCurrentItem, 0, NULL, NULL, NULL, 0, &aProcess, &aStdinHandle, NULL, NULL);
		}
	}
}

void UpdateShutdownApp()
{
	rtl::Reference< UpdateCheck > aController(UpdateCheck::get());
	aController->shutdownApp();
}

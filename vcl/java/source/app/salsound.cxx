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

#define _SV_SALSOUND_CXX

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALSOUND_HXX
#include <salsound.hxx>
#endif

#ifdef MACOSX

#include <premac.h>
#include <AudioToolbox/AudioToolbox.h>
#include <postmac.h>

#define AUDIOTOOLBOXLIB "/System/Library/Frameworks/AudioToolbox.framework/AudioToolbox"
#define AUDIOUNITLIB "/System/Library/Frameworks/AudioUnit.framework/AudioUnit"
#define SALSOUNDBUFSIZE 1024 * 256

struct SalSoundNativeData {
	AudioFileID			maAudioFile;
	void*				mpBuffer;
	UInt64				mnByteCount;
	UInt64				mnByteOffset;
	AudioConverterRef	maConverter;
	UInt32				mnMaxPacketSize;
	UInt64				mnPacketCount;
	UInt64				mnPacketOffset;
};

#endif	// MACOSX

using namespace rtl;

// ========================================================================

#ifdef MACOSX
OSStatus SalSoundComplexInputProc( AudioConverterRef aConverter, UInt32 *pDataPackets, AudioBufferList *pData, AudioStreamPacketDescription **pDataPacketDescription, void *pUserData)
{
	OSStatus nErr = kAudioFileUnspecifiedError;

	// Set default values
	pData->mBuffers[ 0 ].mData = NULL;
	pData->mBuffers[ 0 ].mDataByteSize = 0;

	// Feed data
	if ( pUserData )
	{
		SalSoundNativeData *pNativeData = (SalSoundNativeData *)pUserData;
		if ( pNativeData->maAudioFile )
		{
			if ( pNativeData->mnPacketOffset + *pDataPackets > pNativeData->mnPacketCount )
				*pDataPackets = pNativeData->mnPacketCount - pNativeData->mnPacketOffset;
			if ( *pDataPackets )
			{
				if ( !pNativeData->mpBuffer )
				{
					UInt64 nBytesAvailable = pNativeData->mnByteCount - pNativeData->mnByteOffset;
					pNativeData->mpBuffer = rtl_allocateMemory( SALSOUNDBUFSIZE > nBytesAvailable ? SALSOUNDBUFSIZE : nBytesAvailable );
				}
				UInt32 nBytesRead = 0;
				if ( pNativeData->mpBuffer )
				{
					nErr = AudioFileReadPackets( pNativeData->maAudioFile, false, &nBytesRead, NULL, pNativeData->mnPacketOffset, pDataPackets, pNativeData->mpBuffer );
					if ( nErr == noErr )
					{
						pNativeData->mnByteOffset += nBytesRead;
						pNativeData->mnPacketOffset += *pDataPackets;
						pData->mBuffers[ 0 ].mData = pNativeData->mpBuffer;
						pData->mBuffers[ 0 ].mDataByteSize = nBytesRead;
					}
					else
					{
						rtl_freeMemory( pNativeData->mpBuffer );
						pNativeData->mpBuffer = NULL;
					}
				}
			}
		}
	}

	return nErr;
}
#endif	// MACOSX

// ------------------------------------------------------------------------

#ifdef MACOSX
OSStatus SalSoundFileRenderProc(void *pRefCon, AudioUnitRenderActionFlags *pActionFlags, const AudioTimeStamp *pTimeStamp, UInt32 nBusNumber, UInt32 nNumFrames, AudioBufferList *pData)
{
	OSStatus nErr = kAudioFileUnspecifiedError;

	if ( pRefCon )
	{
		SalSoundNativeData *pNativeData = (SalSoundNativeData *)pRefCon;
		if ( pNativeData->maConverter )
			nErr = AudioConverterFillComplexBuffer( pNativeData->maConverter, SalSoundComplexInputProc, (void *)pNativeData, &nNumFrames, pData, NULL );
	}

	return nErr;
}
#endif	// MACOSX

// ========================================================================

ULONG SalSound::mnSoundState = SOUND_STATE_UNLOADED;

// ------------------------------------------------------------------------

SalSound::SalSound() :
	mpInst( NULL ),
	mpNativeContext( NULL ),
	mbPlaying( FALSE ),
	mpProc( NULL )
{
	mpNativeData = new SalSoundNativeData();
	memset( mpNativeData, 0, sizeof( SalSoundNativeData ) );
}

// ------------------------------------------------------------------------

SalSound::~SalSound()
{
	if ( mpNativeData )
	{
#ifdef MACOSX
		if ( mbPlaying )
			AudioOutputUnitStop( (AudioUnit)mpNativeContext );
		if ( mpNativeData->mpBuffer )
			rtl_freeMemory( mpNativeData->mpBuffer );
		if ( mpNativeData->maConverter )
			AudioConverterDispose( mpNativeData->maConverter);
		if ( mpNativeData->maAudioFile )
			AudioFileClose( mpNativeData->maAudioFile );
		if ( mpNativeContext )
		{
			AudioUnitUninitialize( (AudioUnit)mpNativeContext );
			CloseComponent( (AudioUnit)mpNativeContext );
		}
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "SalSound::~SalSound not implemented\n" );
#endif
#endif	// MACOSX
		delete mpNativeData;
	}
}

// ------------------------------------------------------------------------

BOOL SalSound::Create()
{
	BOOL bRet = FALSE;

#ifdef MACOSX
	if ( !mpNativeContext )
	{
		ComponentDescription aDesc;
		aDesc.componentType = kAudioUnitType_Output;	
		aDesc.componentSubType = kAudioUnitSubType_DefaultOutput;
		aDesc.componentManufacturer = kAudioUnitManufacturer_Apple;
		aDesc.componentFlags = 0;
		aDesc.componentFlagsMask = 0;

		Component aComponent = FindNextComponent( NULL, &aDesc );
		if ( !aComponent || OpenAComponent( aComponent, (AudioUnit *)&mpNativeContext ) != noErr )
			mpNativeContext = NULL;
		if ( mpNativeContext && AudioUnitInitialize( (AudioUnit)mpNativeContext ) != noErr )
		{
			CloseComponent( (AudioUnit)mpNativeContext );
			mpNativeContext = NULL;
		}
	}
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "SalSound::Create not implemented\n" );
#endif
#endif	// MACOSX

	if ( mpNativeContext )
	{
		mnSoundState = SOUND_STATE_VALID;
		bRet = TRUE;
	}
	else
	{
		mnSoundState = SOUND_STATE_INVALID;
	}

	return bRet;
}

// ------------------------------------------------------------------------

void SalSound::Release()
{
#ifndef MACOSX
#ifdef DEBUG
	fprintf( stderr, "SalSound::Release not implemented\n" );
#endif
#endif	// MACOSX

	mnSoundState = SOUND_STATE_UNLOADED;
}

// ------------------------------------------------------------------------

BOOL SalSound::Init( SalFrame* pFrame, const XubString& rSoundName, ULONG& rSoundLen )
{
	BOOL bRet = FALSE;

#ifdef MACOSX
	if ( mpNativeContext )
	{
		// If this is a new audio file then dispose of the old file first
		if ( rSoundName.Len() )
		{
			if ( mbPlaying )
			{
				AudioOutputUnitStop( (AudioUnit)mpNativeContext );
				mbPlaying = FALSE;
			}
			if ( mpNativeData->mpBuffer )
				rtl_freeMemory( mpNativeData->mpBuffer );
			if ( mpNativeData->maConverter )
				AudioConverterDispose( mpNativeData->maConverter);
			if ( mpNativeData->maAudioFile )
				AudioFileClose( mpNativeData->maAudioFile );
			memset( mpNativeData, 0, sizeof( SalSoundNativeData ) );
		}

		// Initialize the audio file if it has not already been done
		FSRef aPath;
		if ( mpNativeData->maAudioFile || ( FSPathMakeRef( (const UInt8 *)ByteString( rSoundName, RTL_TEXTENCODING_UTF8 ).GetBuffer(), &aPath, 0 ) == noErr && AudioFileOpen( &aPath, fsRdPerm, 0, &mpNativeData->maAudioFile ) == noErr ) )
		{
			AudioStreamBasicDescription aOutputDesc;
			UInt32 nSize;
			MacOSBoolean bWritable;

			// Set the output stream to match the audio device
			if ( AudioUnitGetPropertyInfo( (AudioUnit)mpNativeContext, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &nSize, &bWritable ) == noErr && AudioUnitGetProperty( (AudioUnit)mpNativeContext, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &aOutputDesc, &nSize ) == noErr && AudioUnitSetProperty( (AudioUnit)mpNativeContext, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &aOutputDesc, nSize ) == noErr )
			{
				// Get audio file properties
				if ( ( nSize = sizeof( mpNativeData->mnPacketCount ) ) && AudioFileGetProperty( mpNativeData->maAudioFile, kAudioFilePropertyAudioDataPacketCount, &nSize, &mpNativeData->mnPacketCount ) == noErr && ( nSize = sizeof( mpNativeData->mnByteCount ) ) && AudioFileGetProperty( mpNativeData->maAudioFile, kAudioFilePropertyAudioDataByteCount, &nSize, &mpNativeData->mnByteCount ) == noErr && ( nSize = sizeof( mpNativeData->mnMaxPacketSize ) ) && AudioFileGetProperty( mpNativeData->maAudioFile, kAudioFilePropertyMaximumPacketSize, &nSize, &mpNativeData->mnMaxPacketSize ) == noErr )
				{
					if ( !mpNativeData->maConverter )
					{
						AudioStreamBasicDescription aFileDesc;

						// Create the converter
						nSize = sizeof( AudioStreamBasicDescription );
						memset( &aFileDesc, 0, nSize );
						if ( AudioFileGetProperty( mpNativeData->maAudioFile, kAudioFilePropertyDataFormat, &nSize, &aFileDesc ) == noErr && AudioConverterNew( &aFileDesc, &aOutputDesc, &mpNativeData->maConverter ) == noErr )
						{
							// Check for magic cookies and set any decompression
							// parameters
							if ( AudioFileGetPropertyInfo( mpNativeData->maAudioFile, kAudioFilePropertyMagicCookieData, &nSize, NULL ) == noErr )
							{
								void *pMagicCookie = rtl_allocateZeroMemory( nSize );
								if ( pMagicCookie )
								{
									if ( AudioFileGetProperty( mpNativeData->maAudioFile, kAudioFilePropertyMagicCookieData, &nSize, pMagicCookie ) == noErr )
										AudioConverterSetProperty( mpNativeData->maConverter, kAudioConverterDecompressionMagicCookie, nSize, pMagicCookie );
									rtl_freeMemory( pMagicCookie );
								}
							}

							// Set audio callback function
							AURenderCallbackStruct aRenderCallback;
							memset( &aRenderCallback, 0, sizeof( AURenderCallbackStruct ) );
							aRenderCallback.inputProc = SalSoundFileRenderProc;
							aRenderCallback.inputProcRefCon = (void *)mpNativeData;
							if ( AudioUnitSetProperty( (AudioUnit)mpNativeContext, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &aRenderCallback, sizeof( AURenderCallbackStruct ) ) == noErr )
								bRet = TRUE;
						}
					}
				}
			}
		}
	}
#else 	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "SalSound::Init not implemented\n" );
#endif
#endif	// MACOSX

	return bRet;
}

// ------------------------------------------------------------------------

BOOL SalSound::Init( SalFrame* pFrame, const BYTE* pSound, ULONG nDataLen, ULONG& rSoundLen )
{
#ifdef DEBUG
	fprintf( stderr, "SalSound::Init #2 not implemented\n" );
#endif
	return FALSE;
}

// ------------------------------------------------------------------------

void SalSound::Play( ULONG nStartTime, ULONG nPlayLen, BOOL bLoop )
{
#ifdef MACOSX
	OSStatus nErr = kAudioFileUnspecifiedError;
	if ( mpNativeContext )
	{
		if ( mbPlaying && ( nErr = AudioOutputUnitStop( (AudioUnit)mpNativeContext ) ) == noErr )
			mbPlaying = FALSE;
		if ( !mbPlaying && ( nErr = AudioOutputUnitStart( (AudioUnit)mpNativeContext ) ) == noErr )
			mbPlaying = TRUE;
	}

	if ( mpProc )
	{
		if ( nErr == noErr )
			mpProc( mpInst, SOUND_NOTIFY_SUCCESS, SOUNDERR_SUCCESS );
		else
			mpProc( mpInst, SOUND_NOTIFY_ERROR, SOUNDERR_GENERAL_ERROR );
	}
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "SalSound::Play not implemented\n" );
#endif
#endif	// MACOSX
}

// ------------------------------------------------------------------------

void SalSound::Stop()
{
#ifdef MACOSX
	OSStatus nErr = kAudioFileUnspecifiedError;
	if ( mbPlaying && mpNativeContext )
	{
		if ( ( nErr = AudioOutputUnitStop( (AudioUnit)mpNativeContext ) ) == noErr )
			mbPlaying = FALSE;
	}

	if ( mpProc )
	{
		if ( nErr == noErr )
			mpProc( mpInst, SOUND_NOTIFY_SUCCESS, SOUNDERR_SUCCESS );
		else
			mpProc( mpInst, SOUND_NOTIFY_ERROR, SOUNDERR_GENERAL_ERROR );
	}
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "SalSound::Stop not implemented\n" );
#endif
#endif	// MACOSX
}

// ------------------------------------------------------------------------

void SalSound::Pause()
{
#ifdef MACOSX
	OSStatus nErr = kAudioFileUnspecifiedError;
	if ( mbPlaying && mpNativeContext )
	{
		if ( ( nErr = AudioOutputUnitStop( (AudioUnit)mpNativeContext ) ) == noErr )
			mbPlaying = FALSE;
	}

	if ( mpProc )
	{
		if ( nErr == noErr )
			mpProc( mpInst, SOUND_NOTIFY_SUCCESS, SOUNDERR_SUCCESS );
		else
			mpProc( mpInst, SOUND_NOTIFY_ERROR, SOUNDERR_GENERAL_ERROR );
	}
#else	// MACOSX
#ifdef DEBUG
	fprintf( stderr, "SalSound::Pause not implemented\n" );
#endif
#endif	// MACOSX
}

// ------------------------------------------------------------------------

void SalSound::SetNotifyProc( void* pInst, SALSOUNDPROC pProc )
{
	mpInst = pInst;
	mpProc = pProc;
}

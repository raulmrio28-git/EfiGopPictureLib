//LZSS copyright notice:
/**************************************************************
 LZSS.C -- A Data Compression Program
***************************************************************
	4/6/1989 Haruhiko Okumura
	Use, distribute, and modify this program freely.
	Please send me your improved versions.
		PC-VAN      SCIENCE
		NIFTY-Serve PAF01022
		CompuServe  74050,1022
**************************************************************/
//ZLIB copyright notice:
/* Copyright notice:

 (C) 1995-2022 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
	 claim that you wrote the original software. If you use this software
	 in a product, an acknowledgment in the product documentation would be
	 appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
	 misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu */

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include "UefiDebug.h"
#include "EfiDecompress.h"


  //===========================Defines.=============================
#define N		 4096	/* size of ring buffer */
#define F		   18	/* upper limit for match_length */
#define THRESHOLD	2   /* encode string UINT32o position and length
						   if match_length is greater than this */
#define NIL			N	/* index for root of binary search trees */

UINT32 textsize = 0;	/* text size counter */
UINT32 codesize = 0;	/* code size counter */
UINT32 printcount = 0;	/* counter for reporting progress every 1K UINT8s */
UINT8 text_buf[N + F - 1];	/* ring buffer of size N,
	with extra F-1 UINT8s to facilitate string comparison */
UINT32 match_position;
UINT32 match_length;
/* of longest match.  These are
			set by the InsertNode() procedure. */
UINT32 lson[N + 1];
UINT32 rson[N + 257];
UINT32 dad[N + 1];  /* left & right children &
		parents -- These constitute binary search trees. */

// Read nBytes bytes from Uint8 bffer and return as an unsigned 32-bit integer
UINT32 ReadNBytes(
	IN CONST	UINT8 *pBuffer,
	IN			UINT8 nBytes
)
{
	UINT32 nOut = 0;         // Output value
	UINT8 nCurrByte = 0;        // Current byte being read
	ASSERT_ENSURE(pBuffer != NULL || nBytes != 0);
	while(nCurrByte < nBytes && nCurrByte < sizeof(UINT32))
	{
		nOut |= pBuffer[nCurrByte] << (8 * nCurrByte); // Shift and OR in the next byte
		nCurrByte++;
	}

	return nOut;            // Return the output value
}

// Write an unsigned 32-bit value to UINT8 buffer using nBytes bytes
VOID WriteNBytes(
	IN OUT		UINT8 *pBuffer,
	IN			UINT32 nValue,
	IN			UINT8 nBytes
) {
	ASSERT_ENSURE(pBuffer != NULL || nBytes != 0);
	for (int i = 0; i < nBytes; i++) {        // For each byte
		pBuffer[i] = nValue & 0xFF;           // Write the least significant byte
		nValue >>= 8;                         // Shift value right by 8 bits
	}
}

// RLE decompress pInBuffer into pOutBuffer using nBpp bits per pixel
VOID RleDecompress(
	IN CONST	UINT8 *pInBuffer,
	IN			UINT32 nInSize,
	IN OUT		UINT8 *pOutBuffer,
	IN OUT		UINT32 *pnOutSize,
	IN			UINT32 nBpp)
{
	UINT32 nInPos = 0;         // Position in input buffer
	UINT32 nOutPos = 0;        // Position in output buffer
	UINT16 nCount;            // Count of repeated bytes
	UINT32 nValue;            // Value of current byte(s)
	ASSERT_ENSURE(pInBuffer != NULL || nInSize != 0);
	ASSERT_ENSURE(pOutBuffer != NULL || pnOutSize != 0 || nBpp != 0);
	while (nInPos < nInSize) { // While input position < input size

		// Check if the next value is the RLE flag (0x00)
		if (pInBuffer[nInPos] == 0x00) {  //Value is RLE flag, time to read count
			nCount = (UINT16)ReadNBytes(pInBuffer + nInPos + 1, 2); // Read the run count
			nInPos += 3;  //3 = size of flag + size of count

			nValue = ReadNBytes(pInBuffer + nInPos, (UINT8)nBpp / 8); // Read the pixel to repeat
			nInPos += nBpp / 8;

			// Write the repeated pixel nCount times
			while (nCount--)
			{
				WriteNBytes(pOutBuffer + nOutPos, nValue, (UINT8)nBpp / 8);
				nOutPos += nBpp / 8;
			}
		}
		else
		{  //Nope, we encounter a pixel
			// Move back a byte
			nInPos--;

			// Read and write the pixel(s)
			nValue = ReadNBytes(pInBuffer + nInPos, (UINT8)nBpp / 8);
			WriteNBytes(pOutBuffer + nOutPos, nValue, (UINT8)nBpp / 8);
			nInPos += nBpp / 8;
			nOutPos += nBpp / 8;
		}
	}

	*pnOutSize = nOutPos;                          // Set output size to final position
}

VOID LzssInitBuffer(
	VOID
)
{
	for (int i = 0; i < N - F; i++)
		text_buf[i] = 0xff;    // Clear the buffer with any character that will appear often
	for (int i = N - F; i < N + F - 1; i++)
		text_buf[i] = 0xff;
}

VOID LzssDecompress(
	IN	CONST	UINT8 *pInBuffer,
	IN OUT		UINT8 *pOutBuffer,
	IN			UINT32 nInSize,
	IN OUT		UINT32 *pnOutSize
)
{
	ASSERT_ENSURE(pInBuffer != NULL || nInSize != 0);
	ASSERT_ENSURE(pOutBuffer != NULL || pnOutSize != 0);
	CONST UINT8 *pCurrIn;
	UINT8 *pCurrOut;
	UINT32 i;
	UINT32 k;
	UINT32 r;
	UINT8 j;
	UINT8 c;
	UINT32 nFlags;

	LzssInitBuffer();

	pCurrIn = pInBuffer;
	pCurrOut = pOutBuffer;
	r = N - F;
	nFlags = 0;

	for (;;)
	{
		if (((nFlags >>= 1) & 256) == 0 && pCurrIn != pInBuffer + nInSize)
		{
			c = *pCurrIn++;
			nFlags = c | 0xff00;        // uses higher byte cleverly to count eight
		}

		if (nFlags & 1 && pCurrIn != pInBuffer + nInSize)
		{
			c = *pCurrIn++;
			*pCurrOut++ = c;
			text_buf[r++] = c;
			r &= (N - 1);
		}
		else
		{
			if (pCurrIn == pInBuffer + nInSize)
				break;
			i = *pCurrIn++;
			j = *pCurrIn++;
			i |= ((j & 0xf0) << 4);
			j = (j & 0x0f) + THRESHOLD;
			k = 0;
			while (k <= j)
			{
				c = text_buf[(i + k) & (N - 1)];
				*pCurrOut++ = c;
				text_buf[r++] = c;
				r &= (N - 1);
				k++;
			}//for
		}
	}
	*pnOutSize = (UINT32)(pCurrOut - pOutBuffer);

}

VOID PackBitsDecompress(
	IN CONST	UINT8 *pInBuffer,
	IN			UINT32 nInSize,
	IN OUT		UINT8 *pOutBuffer,
	IN OUT		UINT32 *pnOutSize,
	IN			UINT32 nBpp)
{
	UINT32 nInPos = 0;         // Position in input buffer
	UINT32 nOutPos = 0;        // Position in output buffer
	UINT32 nValue;
	UINT8 nCount;            // Count of repeated or non-repeated bytes
	ASSERT_ENSURE(pInBuffer != NULL || nInSize != 0);
	ASSERT_ENSURE(pOutBuffer != NULL || pnOutSize != 0 || nBpp != 0);
	while (nInPos < nInSize) { // While input position < input size
		nCount = pInBuffer[nInPos];   // Read the next byte count
		nInPos++;                               // Increment the input position by a byte

		if (nCount & 0x80) {                      // If high bit is set, we have a non-repeat count
			nCount &= 0x7F;                      // Mask out high bit
			while (nCount > 0) {                   // While count > 0
				WriteNBytes(pOutBuffer + nOutPos, ReadNBytes(pInBuffer + nInPos, (UINT8)nBpp / 8), (UINT8)nBpp / 8); // Write the byte(s)
				nInPos += nBpp / 8;                 // Increment input position by BPP/8
				nOutPos += nBpp / 8;                // Increment output position by BPP/8
				nCount--;                         // Decrement count
			}
		}
		else {                                   // Else we have a repeat count
			nValue = ReadNBytes(pInBuffer + nInPos, (UINT8)nBpp / 8); // Read the next byte(s)
			nInPos += nBpp / 8;                     // Increment input position by BPP/8
			while (nCount > 0) {                   // While count > 0
				WriteNBytes(pOutBuffer + nOutPos, nValue, (UINT8)nBpp / 8); // Write the byte(s)
				nOutPos += nBpp / 8;                // Increment output position by BPP/8
				nCount--;                         // Decrement count
			}
		}
	}

	*pnOutSize = nOutPos;                          // Set output size to final position
}

VOID PackBits16Decompress(
	IN CONST	UINT8 *pInBuffer,
	IN			UINT32 nInSize,
	IN OUT		UINT8 *pOutBuffer,
	IN OUT		UINT32 *pnOutSize,
	IN			UINT32 nBpp) 
{
	UINT32 nInPos = 0;         // Position in input buffer
	UINT32 nOutPos = 0;        // Position in output buffer
	UINT16 nCount;            // Count of repeated or non-repeated bytes
	ASSERT_ENSURE(pInBuffer != NULL || nInSize != 0);
	ASSERT_ENSURE(pOutBuffer != NULL || pnOutSize != 0 || nBpp != 0);
	while (nInPos < nInSize) { // While input position < input size
		nCount = (UINT16)ReadNBytes(pInBuffer + nInPos, 2);   // Read the next 2-byte count
		nInPos += 2;                               // Increment the input position by 2 bytes

		if (nCount & 0x8000) {                      // If high bit is set, we have a non-repeat count
			nCount &= 0x7FFF;                      // Mask out high bit
			while (nCount > 0) {                   // While count > 0
				WriteNBytes(pOutBuffer + nOutPos, ReadNBytes(pInBuffer + nInPos, (UINT8)nBpp / 8), (UINT8)nBpp / 8); // Write the byte(s)
				nInPos += nBpp / 8;                 // Increment input position by BPP/8
				nOutPos += nBpp / 8;                // Increment output position by BPP/8
				nCount--;                         // Decrement count
			}
		}
		else {                                   // Else we have a repeat count
			UINT32 nValue = ReadNBytes(pInBuffer + nInPos, (UINT8)nBpp / 8); // Read the next byte(s)
			nInPos += nBpp / 8;                     // Increment input position by BPP/8
			while (nCount > 0) {                   // While count > 0
				WriteNBytes(pOutBuffer + nOutPos, nValue, (UINT8)nBpp / 8); // Write the byte(s)
				nOutPos += nBpp / 8;                // Increment output position by BPP/8
				nCount--;                         // Decrement count
			}
		}
	}

	*pnOutSize = nOutPos;                          // Set output size to final position
}

EFI_STATUS
EFIAPI
EfiDirectDecompress
(
	IN	UINT8* pInBuffer,
	IN  UINT32  nInSize,
	OUT UINT8*  pOutBuffer,
	OUT UINT32*	pnOutSize
)
{
	VOID          *pScratch = NULL;
	UINT32        nScratchSize;
	ASSERT_ENSURE(pInBuffer != NULL || nInSize != 0);
	ASSERT_ENSURE(pnOutSize != 0);
	ASSERT_CHECK_EFISTATUS(EfiGetInfo(pInBuffer, nInSize, pnOutSize, &nScratchSize));
	ASSERT_CHECK((pScratch = AllocateZeroPool(nScratchSize)) != NULL);
	if (pOutBuffer == NULL)
		pOutBuffer = AllocateZeroPool(*pnOutSize);
	ASSERT_CHECK_EFISTATUS(EfiDecompress(pInBuffer, nInSize, pOutBuffer, *pnOutSize, pScratch, nScratchSize));
	return EFI_SUCCESS;
}

VOID OverlayDecompress(
	IN CONST	UINT8 *pInBuffer,
	IN			UINT32 nInSize,
	IN OUT		UINT8 *pOutBuffer,
	IN			UINT32 nBpp
)
{
	UINT32 nInPos = 0;         // Position in input buffer
	UINT32 nSize = 0;      // Size of pixels
	UINT32 nOffset = 0;   // Offset
	UINT32 nCurrSize = 0;   // Current size
	UINT32 nEncounters;        //Encounter count
	UINT32 nCurrEncounter = 0;            // Current encounter
	ASSERT_ENSURE(pInBuffer != NULL || nInSize != 0);
	ASSERT_ENSURE(pOutBuffer != NULL || nBpp != 0);
	nEncounters = ReadNBytes(pInBuffer, 4); //Read first 4 bytes (unsigned 32-bit value
	nInPos += 4;
	while (nInPos < nInSize && nCurrEncounter < nEncounters) { // While input position < input size
		nCurrEncounter = ReadNBytes(pInBuffer + nInPos, 4); //Read 4 bytes encounter (unsigned 32-bit value)
		nOffset = ReadNBytes(pInBuffer + nInPos + 4, 4); //Read 4 bytes offset (unsigned 32-bit value) and multiply it by nBpp/8
		nSize = ReadNBytes(pInBuffer + nInPos + 8, 4); //No multiplies needed
		nInPos += 12;
		nCurrSize = 0;
		while(nCurrSize < nSize)
		{
			WriteNBytes(pOutBuffer + (nOffset*(nBpp / 8)) + (nCurrSize*(nBpp / 8)), ReadNBytes(pInBuffer + nInPos, (UINT8)nBpp / 8), (UINT8)nBpp / 8); // Write the byte(s)
			nInPos += nBpp / 8;
			nCurrSize++;
		}
	}
}

VOID ReuseDecompress(UINT8 *pInBuffer, UINT8 *pOutBuffer, UINT32 *pnOutSize, UINT32 nBpp)
{
	CONST UINT16 *pMap;
	CONST UINT8 *pDat;
	UINT32 nMapLength;
	UINT32 nOutLength = 0;
	UINT16 wMapCommand;
	UINT32 nCount;
	UINT32 nOffset;
	UINT32 nCurrCount;
	BOOLEAN bIsRepeat = FALSE;
	ASSERT_ENSURE(pInBuffer != NULL);
	ASSERT_ENSURE(pOutBuffer != NULL || pnOutSize != 0 || nBpp != 0);
	// Get 4-byte map length and set map and data buffer pointers
	nMapLength = *((UINT32*)pInBuffer);
	pMap = (UINT16*)(pInBuffer + 4);
	pDat = pInBuffer + 4 + nMapLength * 2;

	// Decompress map buffer
	while (nMapLength--) {
		wMapCommand = *pMap++;
		bIsRepeat = wMapCommand & 0x1;   // Check if repeat (1) or non-repeat (0)

		if (bIsRepeat == TRUE) {  // If repeat
			// Next 11 bits is offset, following 4 bits is length. 
			// Copy length bytes from offset bytes back. Increment output length.
			nOffset = (wMapCommand >> 1) & 0x7ff;
			nCount = (wMapCommand >> 12) & 0xf;
			CopyMem(pOutBuffer + nOutLength, pOutBuffer + nOutLength - nOffset * (nBpp / 8), nCount * (nBpp / 8));
			nOutLength += nCount * (nBpp / 8);
		}
		else {  // If non-repeat
		 // Next 15 bits is count. Read count bytes from data buffer. Increment output length.
			nCount = (wMapCommand >> 1);
			nCurrCount = 0;
			while (nCurrCount < nCount)
			{
				pOutBuffer[nOutLength + nCurrCount * (nBpp / 8)] = pDat[nCurrCount * (nBpp / 8)];
				nCurrCount++;
			}
			nOutLength += nCount * (nBpp / 8);
			pDat += nCount * (nBpp / 8);
		}
	}

	// Return total decompressed length
	*pnOutSize = nOutLength;
}


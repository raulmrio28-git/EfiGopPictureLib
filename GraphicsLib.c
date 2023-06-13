/*
** ===========================================================================
** GraphicsLib.c
** Main code for UEFI GOP Picture library (NOT INTERFACE!).
** ---------------------------------------------------------------------------
** Date			User							Changes
** 06/07/2023	raulmrio28-git (todoroki)		Initial version
** ===========================================================================
*/

/*
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include "UefiDebug.h"
#include "GraphicsLib_Private.h"
#include "GraphicsLib.h"
#include "GraphicsLib_Decompress.h"

/*
**----------------------------------------------------------------------------
**  Definitions
**----------------------------------------------------------------------------
*/

/*
**----------------------------------------------------------------------------
**  Type Definitions
**----------------------------------------------------------------------------
*/

/*
**---------------------------------------------------------------------------
**  Global variables
**---------------------------------------------------------------------------
*/

/*
**---------------------------------------------------------------------------
**  Internal variables
**---------------------------------------------------------------------------
*/

int mnDecodedBufferSize = 0;

/*
**---------------------------------------------------------------------------
**  Function(internal use only) Declarations
**---------------------------------------------------------------------------
*/

UINT32 GraphicsLib_GetBpp(UEFI_GOP_PICTURE_BPP tBppType)
{
	if (tBppType == UEFI_GOP_PICTURE_BPP_1P || tBppType == UEFI_GOP_PICTURE_BPP_1BW)
		return 1;
	if (tBppType == UEFI_GOP_PICTURE_BPP_4)
		return 4;
	if (tBppType == UEFI_GOP_PICTURE_BPP_8P || tBppType == UEFI_GOP_PICTURE_BPP_8RGB332)
		return 8;
	if (tBppType == UEFI_GOP_PICTURE_BPP_8RGBA3328 || tBppType == UEFI_GOP_PICTURE_BPP_16RGB444 || tBppType == UEFI_GOP_PICTURE_BPP_16RGBA4444 || tBppType == UEFI_GOP_PICTURE_BPP_16RGB555 || tBppType == UEFI_GOP_PICTURE_BPP_16RGBA5551 || tBppType == UEFI_GOP_PICTURE_BPP_16RGB565)
		return 16;
	if (tBppType == UEFI_GOP_PICTURE_BPP_16RGBA5658 || tBppType == UEFI_GOP_PICTURE_BPP_18RGB666 || tBppType == UEFI_GOP_PICTURE_BPP_18RGBA6666 || tBppType == UEFI_GOP_PICTURE_BPP_24RGB888)
		return 24;
	if (tBppType == UEFI_GOP_PICTURE_BPP_24RGBA8888)
		return 32;
	else
		return 24;
}

/*
** ---------------------------------------------------------------------------
** GraphicsLib_MakeFrameOffsetBuffer()
** Create frames offset struct buffer
** ---------------------------------------------------------------------------
** Input:
**		pImage: Image source
**		nImageSize: Image source size
**		nFrameNum: Frame count
**		paFramePtrs: frame pointers
** Output: frame pointers in paFramePtrs
** Return value: EFI_LOAD_ERROR if failed
** ---------------------------------------------------------------------------
*/

EFI_STATUS
EFIAPI
GraphicsLib_MakeFrameOffsetBuffer(
	IN		UINT8*	pImage,
	IN		UINTN	nImageSize,
	IN		UINTN	nFrameNum,
	IN OUT	UINT64* paFramePtrs
)
{
	const UINT32*	paFramePtrs32;
	UINTN	nCurrFrameNum = 0;
	ASSERT_ENSURE(pImage != NULL || nImageSize != 0);
	ASSERT_CHECK((paFramePtrs = (UINT64*)AllocateZeroPool(nFrameNum)) != NULL);
	paFramePtrs32 = (UINT32*)(pImage + sizeof(UEFI_GOP_PICTURE_HEADER)); 
																		/* This is a copy from pImage.
																		 Values are stored as 32-bit addresses.
																		 From this we add the respective frame pointers with pImage's
																		 address The final pointers are stored as 64-bit values. */
	while(nCurrFrameNum < nFrameNum) {
		paFramePtrs[nCurrFrameNum] = (UINT64)(pImage + paFramePtrs32[nCurrFrameNum]);
		ASSERT_DEBUG_MSGONLY("Frame %d=%16x", nCurrFrameNum + 1, paFramePtrs[nCurrFrameNum]);
		nCurrFrameNum++;
	}
	return EFI_SUCCESS;
}

/*
** ---------------------------------------------------------------------------
** GraphicsLib_Create()
** Create graphics library struct buffer
** ---------------------------------------------------------------------------
** Input: 
**		ptGfxLibStruct: Structure to be completed
**		pImage: Image source
**		nImageSize: Image source size
** Output: <the result of this function>
** Return value: <return value>
** ---------------------------------------------------------------------------
*/

EFI_STATUS
EFIAPI
GraphicsLib_Create (
	IN OUT	UEFI_GOP_PICTURE_STRUCT*	ptGfxLibStruct,
	IN		UINT8*	pImage,
	IN		UINTN	nImageSize
)
{
	UEFI_GOP_PICTURE_HEADER		tImageHeader;
	VOID**						ptGopBlt = NULL;
	ASSERT_ENSURE(pImage != NULL || nImageSize != 0);
	ASSERT_ENSURE(ptGfxLibStruct != NULL);
	ASSERT_CHECK(*(UINT32 *)pImage == UEFI_GOP_PICTURE_HEADER_MAGIC_IMAGE || *(UINT32 *)pImage == UEFI_GOP_PICTURE_HEADER_MAGIC_ANIMATION);
	CopyMem(&tImageHeader, pImage,  sizeof(UEFI_GOP_PICTURE_HEADER));
	ASSERT_DEBUG_MSGONLY("tImageHeader:\nWidth=%d,Height=%d\nMajVer:%d,MinVer:%d\nFrames=%d", tImageHeader.nWidth, tImageHeader.nHeight, tImageHeader.tVersion.nMajor, tImageHeader.tVersion.nMinor, tImageHeader.nNumFrames);
	ptGfxLibStruct->nWidth = tImageHeader.nWidth;
	ptGfxLibStruct->nHeight = tImageHeader.nHeight;
	ptGfxLibStruct->tProperties = tImageHeader.tProperties;
	ptGfxLibStruct->nFrameNum = tImageHeader.nNumFrames;
	ptGfxLibStruct->nCurrentFrameNum = 1; /* by default we start from frame 1 */
	ptGfxLibStruct->nFrameSpeed = 0; /* by default frame speed is none */
	ptGfxLibStruct->nX = 0;
	ptGfxLibStruct->nY = 0;
	ASSERT_CHECK(ptGfxLibStruct->nWidth || ptGfxLibStruct->nHeight || ptGfxLibStruct->nFrameNum); 
	ASSERT_CHECK_EFISTATUS(GraphicsLib_MakeFrameOffsetBuffer(pImage, nImageSize, ptGfxLibStruct->nFrameNum, ptGfxLibStruct->pnFrameOffsets)); /* allocate as UINT64 ptr */
	ptGfxLibStruct->nDecFrameSize = ptGfxLibStruct->nWidth * ptGfxLibStruct->nHeight;
	ASSERT_CHECK((ptGfxLibStruct->nDecFrameSize > DivU64x32((UINTN)~0, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL))) == 0);
	mnDecodedBufferSize = ptGfxLibStruct->nDecFrameSize * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
	ASSERT_DEBUG_MSGONLY("frame size=%d", mnDecodedBufferSize);
	ASSERT_CHECK((*ptGopBlt = AllocateZeroPool(mnDecodedBufferSize)) != NULL);
	ptGfxLibStruct->ptDecFrame = *ptGopBlt;
	ASSERT_DEBUG_MSGONLY("ptGfxLibStruct:\nWidth=%d,Height=%d,X=%d,Y=%d\nFrames=%d,CurrFrame=%d\nProperties=%08x\nGopBuff=%p", ptGfxLibStruct->nWidth, ptGfxLibStruct->nHeight, ptGfxLibStruct->nX, ptGfxLibStruct->nY, ptGfxLibStruct->nFrameNum, ptGfxLibStruct->nCurrentFrameNum, ptGfxLibStruct->tProperties, ptGfxLibStruct->ptDecFrame);
	return EFI_SUCCESS;
}

VOID XorCuttImgWithPrevImg(UINT8 *pImgPrevious, const UINT8 *pImgCurr, UINT32 nSize)
{
	UINT32 nCurrPtr = 0;
	while (nCurrPtr < nSize)
	{
		pImgPrevious[nCurrPtr] ^= pImgCurr[nCurrPtr];
		nCurrPtr++;
	}
}


EFI_STATUS
EFIAPI
EgpConverter_Decompress(
	IN UINT8*	pDecodedBuffer,
	IN OUT UINT8*		pFrameBuffer,
	IN UINT32		nEncDataSize,
	IN UINT32		nNoOfPels,
	IN UINT8		nBpp,
	IN UEFI_GOP_PICTURE_COMPRESSION tCompression
)
{
	UINT8 *pImgTemp = NULL; /* see UEFI_GOP_PICTURE_COMPRESSION_RLE_AND_XOR */
	UINT32 nOutSize = 0; /* unused */
	ASSERT_ENSURE(pFrameBuffer != NULL || pDecodedBuffer != NULL || nNoOfPels != 0 || nEncDataSize != 0);
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION_RLE)
	{
		RleDecompress(pFrameBuffer, nEncDataSize, pDecodedBuffer, &nOutSize, nBpp);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION_LZSS)
	{
		LzssDecompress(pFrameBuffer, pDecodedBuffer, nEncDataSize, &nOutSize);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION_PACKBITS8)
	{
		PackBitsDecompress(pFrameBuffer, nEncDataSize, pDecodedBuffer, &nOutSize, nBpp);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION_PACKBITS16)
	{
		PackBits16Decompress(pFrameBuffer, nEncDataSize, pDecodedBuffer, &nOutSize, nBpp);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION_EFICOMPRESS)
	{
		EfiDirectDecompress(pFrameBuffer, nEncDataSize, pDecodedBuffer, &nOutSize);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION_RLE_AND_XOR)
	{
		pImgTemp = AllocateZeroPool(nNoOfPels*(nBpp / 8));
		RleDecompress(pFrameBuffer, nEncDataSize, pDecodedBuffer, &nOutSize, nBpp);
		XorCuttImgWithPrevImg(pDecodedBuffer, pImgTemp, nNoOfPels*(nBpp/8));
		FreePool(pImgTemp);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION_OVERLAY)
	{
		OverlayDecompress(pFrameBuffer, nEncDataSize, pDecodedBuffer, nBpp);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION_PREVREUSE)
	{
		ReuseDecompress(pFrameBuffer, pDecodedBuffer, &nOutSize, nBpp);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION_ZLIB)
	{
		ASSERT_NOT_IMPLEMENTED_YET();
		return EFI_OUT_OF_RESOURCES;
	}
	return EFI_SUCCESS;
}


/*
** ---------------------------------------------------------------------------
** GraphicsLib_Decode()
** Decode frame to GOP Blt
** ---------------------------------------------------------------------------
** Input:
**		ptGfxLibStruct: Structure used
** Output: <the result of this function>
** Return value: <return value>
** ---------------------------------------------------------------------------
*/
EFI_STATUS
EFIAPI
GraphicsLib_Decode(
	IN      UEFI_GOP_PICTURE_STRUCT*	ptGfxLibStruct
)
{
	UINT8 *pFrameBuffer = (UINT8*)ptGfxLibStruct->pnFrameOffsets[ptGfxLibStruct->nCurrentFrameNum];
	UINT32 nEncDataSize = 0;
	UINT8 *pDecFrame = AllocateZeroPool(ptGfxLibStruct->nDecFrameSize*GraphicsLib_GetBpp(ptGfxLibStruct->tProperties.nBpp));
	UEFI_GOP_PICTURE_IMAGE tImageInfo;
	ASSERT_ENSURE(pFrameBuffer != NULL || ptGfxLibStruct->nCurrentFrameNum <= ptGfxLibStruct->nFrameNum);
	CopyMem(&tImageInfo, pFrameBuffer, sizeof(UEFI_GOP_PICTURE_IMAGE));
	pFrameBuffer += sizeof(UEFI_GOP_PICTURE_IMAGE);
	ptGfxLibStruct->nFrameSpeed = tImageInfo.nSpeed;
	EgpConverter_Decompress(pDecFrame, pFrameBuffer, nEncDataSize, ptGfxLibStruct->nDecFrameSize, ptGfxLibStruct->tProperties.nBpp, (UEFI_GOP_PICTURE_COMPRESSION)tImageInfo.tCompression);
	return EFI_SUCCESS;
}
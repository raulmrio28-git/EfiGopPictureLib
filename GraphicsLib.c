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
	CopyMem(&tImageHeader, pImage, sizeof(UEFI_GOP_PICTURE_HEADER));
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

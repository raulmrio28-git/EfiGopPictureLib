/*
** ===========================================================================
** GraphicsLib.h
** Main header for UEFI GOP Picture library (NOT INTERFACE!).
** ---------------------------------------------------------------------------
** Date			User							Changes
** 06/07/2023	raulmrio28-git (todoroki)		Initial version
** ===========================================================================
*/

#ifndef _GRAPHICS_LIB_H_
#define _GRAPHICS_LIB_H_

/*
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/

#ifdef __cplusplus
extern "C" {
#endif

//#include <Library/UefiLib.h>
#include <Protocol/GraphicsOutput.h>
#include "GraphicsLib_Common.h"

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

#pragma pack(1)
typedef struct
{
	UINT32				nX;
	UINT32				nY;
	UINT32				nFrameNum;
	UINT32				nCurrentFrameNum;
	UINT16				nFrameSpeed;
	UINT32				nWidth;
	UINT32				nHeight;
	UEFI_GOP_PICTURE_PROPERTIES tProperties;
	UINT64*				pnFrameOffsets;
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL*	ptDecFrame;
	UINT32				nDecFrameSize;
} UEFI_GOP_PICTURE_STRUCT;
#pragma pack()

/*
**---------------------------------------------------------------------------
**  Variable Declarations
**---------------------------------------------------------------------------
*/

/*
**---------------------------------------------------------------------------
**  Function(external use only) Declarations
**---------------------------------------------------------------------------
*/

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
GraphicsLib_Create(
	IN OUT	UEFI_GOP_PICTURE_STRUCT*	ptGfxLibStruct,
	IN		UINT8*	pImage,
	IN		UINTN	nImageSize
);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _GRAPHICS_LIB_H_ */

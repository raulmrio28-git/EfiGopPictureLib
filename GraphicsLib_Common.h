/*
** ===========================================================================
** GraphicsLib_Common.h
** Common type header for UEFI GOP Picture library (both pvt. and pub. headers)
** ---------------------------------------------------------------------------
** Date			User							Changes
** 06/07/2023	raulmrio28-git (todoroki)		Initial version
** ===========================================================================
*/

#ifndef _GRAPHICS_LIB_COMMON_H_
#define _GRAPHICS_LIB_COMMON_H_

/*
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/

#ifdef __cplusplus
extern "C" {
#endif

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

typedef enum {
	UEFI_GOP_PICTURE_BPP_1P,
	UEFI_GOP_PICTURE_BPP_1BW,
	UEFI_GOP_PICTURE_BPP_4,
	UEFI_GOP_PICTURE_BPP_8P,
	UEFI_GOP_PICTURE_BPP_8RGB332,
	UEFI_GOP_PICTURE_BPP_8RGBA3328,
	UEFI_GOP_PICTURE_BPP_16RGB444,
	UEFI_GOP_PICTURE_BPP_16RGBA4444,
	UEFI_GOP_PICTURE_BPP_16RGB555,
	UEFI_GOP_PICTURE_BPP_16RGBA5551,
	UEFI_GOP_PICTURE_BPP_16RGB565,
	UEFI_GOP_PICTURE_BPP_16RGBA5658,
	UEFI_GOP_PICTURE_BPP_18RGB666,
	UEFI_GOP_PICTURE_BPP_18RGBA6666,
	UEFI_GOP_PICTURE_BPP_24RGB888,
	UEFI_GOP_PICTURE_BPP_24RGBA8888
} UEFI_GOP_PICTURE_BPP;
#pragma pack(1)
typedef struct {
	UINT8 nBpp : 4;       // 4-bit color format (enum)
	UINT8 bIsTransparent : 1;   // 1-bit is transparent boolean
	UINT8 bUseColorForTransparency : 1;     // 1-bit uses RGB color or alpha values for transparency boolean 
	UINT8 bIsAnimated : 1;     // 1-bit is animated boolean
	UINT8 bIsRotated : 1;     // 1-bit is rotated boolean
} UEFI_GOP_PICTURE_PROPERTIES;
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

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _GRAPHICS_LIB_COMMON_H_ */

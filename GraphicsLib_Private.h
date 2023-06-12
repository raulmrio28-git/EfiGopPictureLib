/*
** ===========================================================================
** GraphicsLib_Private.h
** Private type header for UEFI GOP Picture library (NOT USED IN EXPORTED FUNCS!)
** ---------------------------------------------------------------------------
** Date			User							Changes
** 06/07/2023	raulmrio28-git (todoroki)		Initial version
** ===========================================================================
*/

#ifndef _GRAPHICS_LIB_PRIVATE_H_
#define _GRAPHICS_LIB_PRIVATE_H_

/*
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/

#ifdef __cplusplus
extern "C" {
#endif
#include "GraphicsLib_Common.h"

/*
**----------------------------------------------------------------------------
**  Definitions
**----------------------------------------------------------------------------
*/

#define UEFI_GOP_PICTURE_HEADER_MAGIC_IMAGE 'IPGE'
#define UEFI_GOP_PICTURE_HEADER_MAGIC_ANIMATION 'APGE'

/*
**----------------------------------------------------------------------------
**  Type Definitions
**----------------------------------------------------------------------------
*/

typedef enum {
	UEFI_GOP_PICTURE_COMPRESSION_UNCOMPRESSED,		// No compression
	UEFI_GOP_PICTURE_COMPRESSION_RLE,				// Run - length encoding
	UEFI_GOP_PICTURE_COMPRESSION_LZSS,				// LZ55 compression (used in some formats)
	UEFI_GOP_PICTURE_COMPRESSION_PACKBITS8,			// 8-bit packbits compression
	UEFI_GOP_PICTURE_COMPRESSION_PACKBITS16,		// 16-bit packbits compression
	UEFI_GOP_PICTURE_COMPRESSION_EFICOMPRESS,		// UEFI compression
	UEFI_GOP_PICTURE_COMPRESSION_RLE_AND_XOR,		// Run-length encoding + XOR
	UEFI_GOP_PICTURE_COMPRESSION_OVERLAY,			// Image overlay
	UEFI_GOP_PICTURE_COMPRESSION_PREVREUSE,			// Reuses pixel data from the previously decoded image buffer
	UEFI_GOP_PICTURE_COMPRESSION_ZLIB				// ZLIB compression
} UEFI_GOP_PICTURE_COMPRESSION;

#pragma pack(1)
typedef struct {
	UINT32 dwMagic;     // "EGPA" for animation, "EGPI" for image
	UINT32 nWidth;         // Width of image/frame
	UINT32 nHeight;        // Height of image/frame
	struct {
		UINT8 nMajor : 4;     // 4-bit major version
		UINT8 nMinor : 4;     // 4-bit minor version
	} tVersion;
	UEFI_GOP_PICTURE_PROPERTIES tProperties;
	UINT16 nNumFrames;    // Number of frames (if animated)
} UEFI_GOP_PICTURE_HEADER;
#pragma pack()
#pragma pack(1)
typedef struct {
	UINT32 nBuffSize;		// Size of image data buffer
	UINT16 nSpeed;			   // Speed (0 is not animated)
	UINT8 tCompression;		// Compression type
	UINT8 nPaletteCount;		// Number of colors (0 = 1 color, if <= 8bpp)
								// Transparent color index (8bpp RGB332)
} UEFI_GOP_PICTURE_IMAGE;
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

#endif /* _GRAPHICS_LIB_PRIVATE_H_ */

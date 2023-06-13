VOID RleDecompress(
	IN CONST	UINT8 *pInBuffer,
	IN			UINT32 nInSize,
	IN OUT		UINT8 *pOutBuffer,
	IN OUT		UINT32 *pnOutSize,
	IN			UINT32 nBpp);

VOID LzssDecompress(
	IN	CONST	UINT8 *pInBuffer,
	IN OUT		UINT8 *pOutBuffer,
	IN			UINT32 nInSize,
	IN OUT		UINT32 *pnOutSize
);

VOID PackBitsDecompress(
	IN CONST	UINT8 *pInBuffer,
	IN			UINT32 nInSize,
	IN OUT		UINT8 *pOutBuffer,
	IN OUT		UINT32 *pnOutSize,
	IN			UINT32 nBpp);

VOID PackBits16Decompress(
	IN CONST	UINT8 *pInBuffer,
	IN			UINT32 nInSize,
	IN OUT		UINT8 *pOutBuffer,
	IN OUT		UINT32 *pnOutSize,
	IN			UINT32 nBpp);

EFI_STATUS
EFIAPI
EfiDirectDecompress
(
	IN	UINT8* pSource,
	IN  UINT32  pSrcSize,
	OUT UINT8*  pDestination,
	OUT UINT32*	pDstSize
);

VOID OverlayDecompress(IN CONST	UINT8 *pInBuffer,
	IN			UINT32 nInSize,
	IN OUT		UINT8 *pOutBuffer,
	IN			UINT32 nBpp);

VOID ReuseDecompress(UINT8 *pInBuffer, UINT8 *pOutBuffer, UINT32 *pnOutSize, UINT32 nBpp);


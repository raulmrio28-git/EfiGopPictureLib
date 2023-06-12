/*
** ===========================================================================
** File: UefiDebug.c
** Description: UEFI debug addon
** ===========================================================================
*/

/*
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>
#include <Library/Dxe/Print/Print.h>
#include <Library/SerialPortLib.h>
#include "UefiDebug.h"

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

/*
**---------------------------------------------------------------------------
**  Function(internal use only) Declarations
**---------------------------------------------------------------------------
*/

/*
** ===========================================================================
** Function: DebugPrtPrint()
** Description: Writes assert info to COM port
** Input:
**		nAssertLevel: Assert level
**		pszAssertType: nAssertLevel as ASCII string
**		pszFunction: Function as ASCII string
**		pszExpression: Expression as ASCII string
**		pszFile: Source file name
**		nLine: Source line
** Output: Assert info to COM port
** Return value: ASSERTION_NEVER_GET_HERE -> Failure, assert level -> Success
** ===========================================================================
*/
EFI_STATUS
DebugPrtPrint(
	IN INTN nAssertLevel,
	IN CHAR8 *pszAssertType,
	IN CHAR8 *pszFunction,
	IN CHAR8 *pszExpression,
	IN CHAR8 *pszFile,
	IN INTN nLine
)
{
	CHAR8   pszBuffer[DBGMSG_MAX_CHARS];
	if (pszAssertType == NULL || pszExpression == NULL || pszFile == NULL)
		return ASSERTION_NEVER_GET_HERE;
	if (nAssertLevel == ASSERTION_NEVER_GET_HERE)
		AsciiSPrint(pszBuffer, sizeof(pszBuffer), "%a:%a->Cannot continue... (file %a, line %d)\n", pszAssertType, pszFunction, pszFile, nLine);
	if (nAssertLevel == ASSERTION_NOT_IMPLEMENTED_YET)
		AsciiSPrint(pszBuffer, sizeof(pszBuffer), "%a:%a->Function not implemented! (file %a, line %d)\n", pszAssertType, pszFunction, pszFile, nLine);
	else
		AsciiSPrint(pszBuffer, sizeof(pszBuffer), "%a:%a->%a failed! (file %a, line %d)\n", pszAssertType, pszFunction, pszExpression, pszFile, nLine);
	SerialPortWrite((UINT8 *)pszBuffer, AsciiStrLen(pszBuffer));
	return nAssertLevel;
}

/*
** ===========================================================================
** Function: DebugPrtPrintMsg()
** Description: Writes assert info to COM port with custom message
** Input:
**		nAssertLevel: Assert level
**		pszAssertType: nAssertLevel as ASCII string
**		pszFunction: Function as ASCII string
**		pszExpression: Expression as ASCII string
**		pszFile: Source file name
**		nLine: Source line
**		pszMessage: String to be printed
**		etc.
** Output: Assert info to COM port
** Return value: ASSERTION_NEVER_GET_HERE -> Failure, assert level -> Success
** ===========================================================================
*/
EFI_STATUS
DebugPrtPrintMsg(
	IN INTN nAssertLevel,
	IN CHAR8 *pszAssertType,
	IN CHAR8 *pszFunction,
	IN CHAR8 *pszExpression,
	IN CHAR8 *pszFile,
	IN INTN nLine,
	IN CHAR8 *pszMessage,
	...
)
{
	CHAR8   pszBufferMsg[DBGMSG_MAX_CHARS];
	CHAR8   pszBuffer[DBGMSG_MAX_CHARS*2];
	if (pszAssertType == NULL || pszExpression == NULL || pszFile == NULL)
		return ASSERTION_NEVER_GET_HERE;
	VA_LIST pszArgList;
	VA_START(pszArgList, pszMessage);
	AsciiVSPrint(pszBufferMsg, sizeof(pszBufferMsg), pszMessage, pszArgList);
	VA_END(pszArgList);
	if (nAssertLevel == ASSERTION_NEVER_GET_HERE)
		AsciiSPrint(pszBuffer, sizeof(pszBuffer), "%a:%a->Cannot continue... (file %a, line %d)\n%a\n", pszAssertType, pszFunction, pszFile, nLine, pszBufferMsg);
	if (nAssertLevel == ASSERTION_NOT_IMPLEMENTED_YET)
		AsciiSPrint(pszBuffer, sizeof(pszBuffer), "%a:%a->Function not implemented! (file %a, line %d)\n%a\n", pszAssertType, pszFunction, pszFile, nLine, pszBufferMsg);
	else
		AsciiSPrint(pszBuffer, sizeof(pszBuffer), "%a:%a->%a (file %a, line %d)\n%a\n", pszAssertType, pszFunction, pszExpression, pszFile, nLine, pszBufferMsg);
	SerialPortWrite((UINT8 *)pszBuffer, AsciiStrLen(pszBuffer));
	return nAssertLevel;
}

/*
** ===========================================================================
** Function: DebugPrtPrintMsgOnly()
** Description: Writes assert message info to COM port expression-less
** Input:
**		nAssertLevel: Assert level
**		pszAssertType: nAssertLevel as ASCII string
**		pszFunction: Function as ASCII string
**		pszFile: Source file name
**		nLine: Source line
**		pszMessage: String to be printed
**		etc.
** Output: Assert info to COM port
** Return value: ASSERTION_NEVER_GET_HERE -> Failure, assert level -> Success
** ===========================================================================
*/
EFI_STATUS
DebugPrtPrintMsgOnly(
	IN INTN nAssertLevel,
	IN CHAR8 *pszAssertType,
	IN CHAR8 *pszFunction,
	IN CHAR8 *pszFile,
	IN INTN nLine,
	IN CHAR8 *pszMessage,
	...
)
{
	CHAR8   pszBufferMsg[DBGMSG_MAX_CHARS];
	CHAR8   pszBuffer[DBGMSG_MAX_CHARS * 2];
	if (pszAssertType == NULL || pszFile == NULL)
		return ASSERTION_NEVER_GET_HERE;
	VA_LIST pszArgList;
	VA_START(pszArgList, pszMessage);
	AsciiVSPrint(pszBufferMsg, sizeof(pszBufferMsg), pszMessage, pszArgList);
	VA_END(pszArgList);
	AsciiSPrint(pszBuffer, sizeof(pszBuffer), "%a:%a->%a (file %a, line %d)\n", pszAssertType, pszFunction, pszBufferMsg, pszFile, nLine);
	SerialPortWrite((UINT8 *)pszBuffer, AsciiStrLen(pszBuffer));
	return nAssertLevel;
}
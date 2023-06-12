/*
** ===========================================================================
** File: UefiDebug.h
** Description: UEFI debug addon
** ===========================================================================
*/

#ifndef _UEFIDEBUG_H_
#define _UEFIDEBUG_H_

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

// DebugPrtPrint() uses these
enum
{
	ASSERTION_ASSERT, //Assert 
	ASSERTION_REQUIRE, //Requie that...
	ASSERTION_ENSURE, //Ensure that...
	ASSERTION_CHECK, //Check if...
	ASSERTION_NEVER_GET_HERE, //Action cannot complete
	ASSERTION_NOT_IMPLEMENTED_YET //Not worked on!
};
#define DBGMSG_MAX_CHARS 256

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
);

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
);

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
);

/*
**----------------------------------------------------------------------------
**  Definitions (macros for assertion)
**----------------------------------------------------------------------------
*/

#define ASSERT_DEBUG(type, expression) DebugPrtPrint(type, #type, __func__, #expression, __FILE__, __LINE__)
// leaving it here unless we can figure out how to parse this...
#define ASSERT_DEBUG_MSGONLY(msg, ...) DebugPrtPrintMsgOnly(ASSERTION_ASSERT, "ASSERTION_MSG", __func__, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define ASSERT_DEBUG_MSG(type, expression, msg, ...) DebugPrtPrintMsg(__func__, #expression, __FILE__, __LINE__, msg, ##__VA_ARGS__)

#define ASSERT_ASSERT(expr)    \
	if (!(expr)) {ASSERT_DEBUG(ASSERTION_ASSERT, expr); return EFI_LOAD_ERROR; }
#define ASSERT_REQUIRE(expr)    \
	if (!(expr)) {ASSERT_DEBUG(ASSERTION_ASSERT, expr); return EFI_LOAD_ERROR; }
#define ASSERT_ENSURE(expr)    \
	if (!(expr)) {ASSERT_DEBUG(ASSERTION_ASSERT, expr); return EFI_LOAD_ERROR; }
#define ASSERT_CHECK(expr)        \
	if (!(expr)) {ASSERT_DEBUG(ASSERTION_ASSERT, expr); return EFI_LOAD_ERROR; }
#define ASSERT_ASSERT_EFISTATUS(expr)    \
	if ((expr) != EFI_SUCCESS) {ASSERT_DEBUG(ASSERTION_ASSERT, expr); return EFI_LOAD_ERROR; }
#define ASSERT_REQUIRE_EFISTATUS(expr)    \
	if ((expr) != EFI_SUCCESS) {ASSERT_DEBUG(ASSERTION_ASSERT, expr); return EFI_LOAD_ERROR; }
#define ASSERT_ENSURE_EFISTATUS(expr)    \
	if ((expr) != EFI_SUCCESS) {ASSERT_DEBUG(ASSERTION_ASSERT, expr); return EFI_LOAD_ERROR; }
#define ASSERT_CHECK_EFISTATUS(expr)        \
	if ((expr) != EFI_SUCCESS) {ASSERT_DEBUG(ASSERTION_ASSERT, expr); return EFI_LOAD_ERROR; }
#define ASSERT_NEVER_GET_HERE()    ASSERT_DEBUG(ASSERTION_NEVER_GET_HERE, NULL)
#define ASSERT_NOT_IMPLEMENTED_YET()    ASSERT_DEBUG(ASSERTION_NOT_IMPLEMENTED_YET, NULL)

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _UEFIDEBUG_H_ */

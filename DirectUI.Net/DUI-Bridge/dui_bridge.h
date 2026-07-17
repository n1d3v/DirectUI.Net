#pragma once

#ifdef DUI_BRIDGE_EXPORTS
#define DUI_BRIDGE_API __declspec(dllexport)
#else
#define DUI_BRIDGE_API __declspec(dllimport)
#endif

#include <Windows.h>

extern "C" {

typedef void (CALLBACK* PFNDUI_CLICK)(LPCWSTR pszElementId, void* pContext);
typedef void (CALLBACK* PFNDUI_PROPCHANGE)(LPCWSTR pszElementId, LPCWSTR pszPropName, void* pContext);
typedef void (CALLBACK* PFNDUI_SELECTION)(LPCWSTR pszSelectorId, LPCWSTR pszSelectedId, int dSelectedIndex, void* pContext);
typedef void (CALLBACK* PFNDUI_INPUT)(LPCWSTR pszElementId, int dDevice, int dCode, int dFlags, void* pContext);
typedef void (CALLBACK* PFNDUI_PARSEERROR)(LPCWSTR pszError, LPCWSTR pszToken, int dLine, void* pContext);

DUI_BRIDGE_API BOOL WINAPI DuiBridge_Init(HINSTANCE hInst);
DUI_BRIDGE_API void WINAPI DuiBridge_Shutdown();

DUI_BRIDGE_API HWND WINAPI DuiBridge_CreateWindow(
    HWND hwndParent, LPCWSTR pszTitle,
    int x, int y, int cx, int cy, DWORD dwStyle);
DUI_BRIDGE_API void WINAPI DuiBridge_ShowWindow(HWND hwnd, int nCmdShow);
DUI_BRIDGE_API void WINAPI DuiBridge_DestroyWindow(HWND hwnd);

DUI_BRIDGE_API BOOL WINAPI DuiBridge_LoadXml(HWND hwnd, LPCWSTR pszXml, LPCWSTR pszResId);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_LoadXmlFile(HWND hwnd, LPCWSTR pszFilePath, LPCWSTR pszResId);

DUI_BRIDGE_API BOOL WINAPI DuiBridge_MountFragment(HWND hwnd, LPCWSTR pszParentId, LPCWSTR pszXml, LPCWSTR pszResId);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_MountFragmentFile(HWND hwnd, LPCWSTR pszParentId, LPCWSTR pszFilePath, LPCWSTR pszResId);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_RemoveElement(HWND hwnd, LPCWSTR pszId);

DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetText(HWND hwnd, LPCWSTR pszId, LPCWSTR pszText);
DUI_BRIDGE_API LPWSTR WINAPI DuiBridge_GetText(HWND hwnd, LPCWSTR pszId);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetEnabled(HWND hwnd, LPCWSTR pszId, BOOL bEnabled);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_GetEnabled(HWND hwnd, LPCWSTR pszId);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetVisible(HWND hwnd, LPCWSTR pszId, BOOL bVisible);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_GetVisible(HWND hwnd, LPCWSTR pszId);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetAlpha(HWND hwnd, LPCWSTR pszId, int alpha);
DUI_BRIDGE_API int WINAPI DuiBridge_GetAlpha(HWND hwnd, LPCWSTR pszId);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetClass(HWND hwnd, LPCWSTR pszId, LPCWSTR pszClass);
DUI_BRIDGE_API LPWSTR WINAPI DuiBridge_GetClass(HWND hwnd, LPCWSTR pszId);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetWidth(HWND hwnd, LPCWSTR pszId, int width);
DUI_BRIDGE_API int WINAPI DuiBridge_GetWidth(HWND hwnd, LPCWSTR pszId);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetHeight(HWND hwnd, LPCWSTR pszId, int height);
DUI_BRIDGE_API int WINAPI DuiBridge_GetHeight(HWND hwnd, LPCWSTR pszId);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetBackground(HWND hwnd, LPCWSTR pszId, COLORREF color);
DUI_BRIDGE_API COLORREF WINAPI DuiBridge_GetBackground(HWND hwnd, LPCWSTR pszId);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetForeground(HWND hwnd, LPCWSTR pszId, COLORREF color);
DUI_BRIDGE_API COLORREF WINAPI DuiBridge_GetForeground(HWND hwnd, LPCWSTR pszId);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetLayoutPos(HWND hwnd, LPCWSTR pszId, int layoutPos);
DUI_BRIDGE_API int WINAPI DuiBridge_GetLayoutPos(HWND hwnd, LPCWSTR pszId);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_ElementExists(HWND hwnd, LPCWSTR pszId);

DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetStringProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName, LPCWSTR pszValue);
DUI_BRIDGE_API LPWSTR WINAPI DuiBridge_GetStringProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetIntProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName, int value);
DUI_BRIDGE_API int WINAPI DuiBridge_GetIntProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetBoolProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName, BOOL value);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_GetBoolProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetColorProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName, COLORREF color);
DUI_BRIDGE_API COLORREF WINAPI DuiBridge_GetColorProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName);

DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetSelection(HWND hwnd, LPCWSTR pszSelectorId, int dIndex);
DUI_BRIDGE_API int WINAPI DuiBridge_GetSelection(HWND hwnd, LPCWSTR pszSelectorId);

DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetSheet(HWND hwnd, LPCWSTR pszId, LPCWSTR pszSheetResId);

DUI_BRIDGE_API BOOL WINAPI DuiBridge_StartDefer(HWND hwnd);
DUI_BRIDGE_API BOOL WINAPI DuiBridge_EndDefer(HWND hwnd);

DUI_BRIDGE_API void WINAPI DuiBridge_SetClickCallback(HWND hwnd, PFNDUI_CLICK pfn, void* pCtx);
DUI_BRIDGE_API void WINAPI DuiBridge_SetPropertyCallback(HWND hwnd, PFNDUI_PROPCHANGE pfn, void* pCtx);
DUI_BRIDGE_API void WINAPI DuiBridge_SetSelectionCallback(HWND hwnd, PFNDUI_SELECTION pfn, void* pCtx);
DUI_BRIDGE_API void WINAPI DuiBridge_SetInputCallback(HWND hwnd, PFNDUI_INPUT pfn, void* pCtx);
DUI_BRIDGE_API void WINAPI DuiBridge_SetParseErrorCallback(PFNDUI_PARSEERROR pfn, void* pCtx);

DUI_BRIDGE_API void WINAPI DuiBridge_RunMessagePump();
DUI_BRIDGE_API void WINAPI DuiBridge_StopMessagePump();

DUI_BRIDGE_API BOOL WINAPI DuiBridge_SetContentBitmap(HWND hwnd, LPCWSTR pszId, HBITMAP hBitmap);

DUI_BRIDGE_API void WINAPI DuiBridge_FreeString(LPWSTR pszStr);

}
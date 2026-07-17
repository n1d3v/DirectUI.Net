#include "dui_bridge.h"
#include "func_helpers.h"
#include <map>
#include <string>
#include <vector>

std::map<HWND, WindowCtx*> g_windows;
HINSTANCE g_hInst = nullptr;

PFNDUI_PARSEERROR g_pfnParseError = nullptr;
void* g_pParseErrorCtx = nullptr;

BOOL WINAPI DuiBridge_Init(HINSTANCE hInst)
{
    g_hInst = hInst ? hInst : GetModuleHandleW(nullptr);

    HRESULT hr = InitProcessPriv(DUI_VERSION, g_hInst, true, true, false);
    if (FAILED(hr)) return FALSE;

    hr = InitThread(2);
    if (FAILED(hr))
    {
        UnInitProcessPriv(g_hInst);
        return FALSE;
    }

    RegisterStandardControls();
    RegisterCommonControls();
    RegisterExtendedControls();

    return TRUE;
}

void WINAPI DuiBridge_Shutdown()
{
    for (auto& [hwnd, wndCtx] : g_windows)
        func_helpers::DestroyWindowCtx(wndCtx);

    g_windows.clear();
    UnInitThread();
    UnInitProcessPriv(g_hInst);
}

HWND WINAPI DuiBridge_CreateWindow(HWND hwndParent, LPCWSTR pszTitle, int x, int y, int cx, int cy, DWORD dwStyle)
{
    WindowCtx* wndCtx = new WindowCtx();
    HRESULT hr = NativeHWNDHost::Create(
        pszTitle, hwndParent, nullptr,
        x, y, cx, cy, 0,
        dwStyle ? dwStyle : WS_OVERLAPPEDWINDOW,
        0, &wndCtx->pHost);

    if (FAILED(hr)) { delete wndCtx; return nullptr; }

    HWND hwnd = wndCtx->pHost->GetHWND();
    g_windows[hwnd] = wndCtx;

    return hwnd;
}

BOOL WINAPI DuiBridge_LoadXml(HWND hwnd, LPCWSTR pszXml, LPCWSTR pszResId)
{
    WindowCtx* wndCtx = func_helpers::FindCtx(hwnd);
    if (!wndCtx || !wndCtx->pHost || !pszXml) return FALSE;

    DUIXmlParser* duiParser = func_helpers::CreateParserFromXml(pszXml);
    if (!duiParser) return FALSE;

    DWORD dwDefer = 0;
    if (FAILED(HWNDElement::Create(wndCtx->pHost->GetHWND(), true, 0, nullptr, &dwDefer, (Element**)&wndCtx->pHwndElement)))
    {
        duiParser->Destroy();
        return FALSE;
    }
    if (FAILED(duiParser->CreateElement(pszResId ? pszResId : L"main", wndCtx->pHwndElement, nullptr, nullptr, &wndCtx->pRoot)) || !wndCtx->pRoot)
    {
        wndCtx->pHwndElement->EndDefer(dwDefer);
        duiParser->Destroy();
        return FALSE;
    }

    wndCtx->pRoot->SetVisible(true);
    wndCtx->pRoot->EndDefer(dwDefer);
    wndCtx->pRoot->AddListener(wndCtx);
    wndCtx->parsers.push_back(duiParser);
    wndCtx->pHost->Host(wndCtx->pRoot);
    wndCtx->pHost->ShowWindow(SW_SHOW);

    return TRUE;
}

BOOL WINAPI DuiBridge_LoadXmlFile(HWND hwnd, LPCWSTR pszFilePath, LPCWSTR pszResId)
{
    std::wstring duiXml = func_helpers::ReadFileAsWide(pszFilePath);
    if (duiXml.empty()) return FALSE;

    return DuiBridge_LoadXml(hwnd, duiXml.c_str(), pszResId);
}

BOOL WINAPI DuiBridge_MountFragment(HWND hwnd, LPCWSTR pszParentId, LPCWSTR pszXml, LPCWSTR pszResId)
{
    WindowCtx* wndCtx = func_helpers::FindCtx(hwnd);
    if (!wndCtx || !wndCtx->pRoot || !pszXml) return FALSE;

    Element* pParent = func_helpers::FindElement(wndCtx, pszParentId);
    if (!pParent) return FALSE;

    DUIXmlParser* duiParser = func_helpers::CreateParserFromXml(pszXml);
    if (!duiParser) return FALSE;

    DWORD dwDefer = 0;
    Element* peFragment = nullptr;

    HRESULT hr = duiParser->CreateElement(pszResId ? pszResId : L"main", nullptr, nullptr, &dwDefer, &peFragment);

    if (FAILED(hr) || !peFragment)
    {
        duiParser->Destroy();
        return FALSE;
    }

    if (dwDefer) peFragment->EndDefer(dwDefer);

    hr = pParent->Add(peFragment);
    if (FAILED(hr))
    {
        peFragment->Destroy(false);
        duiParser->Destroy();
        return FALSE;
    }

    wndCtx->parsers.push_back(duiParser);
    return TRUE;
}

BOOL WINAPI DuiBridge_MountFragmentFile(HWND hwnd, LPCWSTR pszParentId, LPCWSTR pszFilePath, LPCWSTR pszResId)
{
    std::wstring duiXml = func_helpers::ReadFileAsWide(pszFilePath);
    if (duiXml.empty()) return FALSE;

    return DuiBridge_MountFragment(hwnd, pszParentId, duiXml.c_str(), pszResId);
}

BOOL WINAPI DuiBridge_RemoveElement(HWND hwnd, LPCWSTR pszId) { return func_helpers::SetElementValue(hwnd, pszId, &Element::Destroy, true); }

BOOL WINAPI DuiBridge_SetText(HWND hwnd, LPCWSTR pszId, LPCWSTR pszText) { return func_helpers::SetElementValue(hwnd, pszId, &Element::SetContentString, pszText); }
LPWSTR WINAPI DuiBridge_GetText(HWND hwnd, LPCWSTR pszId) { return func_helpers::GetElementString(hwnd, pszId, &Element::GetContentString); }

BOOL WINAPI DuiBridge_SetEnabled(HWND hwnd, LPCWSTR pszId, BOOL bEnabled) { return func_helpers::SetElementValue(hwnd, pszId, &Element::SetEnabled, bEnabled != 0); }
BOOL WINAPI DuiBridge_GetEnabled(HWND hwnd, LPCWSTR pszId) { return func_helpers::GetElementValue(hwnd, pszId, &Element::GetEnabled, false); }

BOOL WINAPI DuiBridge_SetVisible(HWND hwnd, LPCWSTR pszId, BOOL bVisible) { return func_helpers::SetElementValue(hwnd, pszId, &Element::SetVisible, bVisible != 0); }
BOOL WINAPI DuiBridge_GetVisible(HWND hwnd, LPCWSTR pszId) { return func_helpers::GetElementValue(hwnd, pszId, &Element::GetVisible, false); }

BOOL WINAPI DuiBridge_SetAlpha(HWND hwnd, LPCWSTR pszId, int alpha) { return func_helpers::SetElementValue(hwnd, pszId, &Element::SetAlpha, alpha); }
int WINAPI DuiBridge_GetAlpha(HWND hwnd, LPCWSTR pszId) { return func_helpers::GetElementValue(hwnd, pszId, &Element::GetAlpha, 255); }

BOOL WINAPI DuiBridge_SetClass(HWND hwnd, LPCWSTR pszId, LPCWSTR pszClass) { return func_helpers::SetElementValue(hwnd, pszId, &Element::SetClass, pszClass); }
LPWSTR WINAPI DuiBridge_GetClass(HWND hwnd, LPCWSTR pszId) { return func_helpers::GetElementString(hwnd, pszId, &Element::GetClass); }

BOOL WINAPI DuiBridge_SetWidth(HWND hwnd, LPCWSTR pszId, int width) { return func_helpers::SetElementValue(hwnd, pszId, &Element::SetWidth, width); }
int WINAPI DuiBridge_GetWidth(HWND hwnd, LPCWSTR pszId) { return func_helpers::GetElementValue(hwnd, pszId, &Element::GetWidth, 0); }

BOOL WINAPI DuiBridge_SetHeight(HWND hwnd, LPCWSTR pszId, int height) { return func_helpers::SetElementValue(hwnd, pszId, &Element::SetHeight, height); }
int WINAPI DuiBridge_GetHeight(HWND hwnd, LPCWSTR pszId) { return func_helpers::GetElementValue(hwnd, pszId, &Element::GetHeight, 0); }

BOOL WINAPI DuiBridge_SetBackground(HWND hwnd, LPCWSTR pszId, COLORREF color) { return func_helpers::SetElementValue(hwnd, pszId, static_cast<SetColorMethod>(&Element::SetBackgroundColor), color); }
COLORREF WINAPI DuiBridge_GetBackground(HWND hwnd, LPCWSTR pszId) { return func_helpers::GetElementColor(hwnd, pszId, &Element::GetBackgroundColor); }

BOOL WINAPI DuiBridge_SetForeground(HWND hwnd, LPCWSTR pszId, COLORREF color) { return func_helpers::SetElementValue(hwnd, pszId, static_cast<SetColorMethod>(&Element::SetForegroundColor), color); }
COLORREF WINAPI DuiBridge_GetForeground(HWND hwnd, LPCWSTR pszId) { return func_helpers::GetElementColor(hwnd, pszId, &Element::GetForegroundColor); }

BOOL WINAPI DuiBridge_SetLayoutPos(HWND hwnd, LPCWSTR pszId, int layoutPos) { return func_helpers::SetElementValue(hwnd, pszId, &Element::SetLayoutPos, layoutPos); }
int WINAPI DuiBridge_GetLayoutPos(HWND hwnd, LPCWSTR pszId) { return func_helpers::GetElementValue(hwnd, pszId, &Element::GetLayoutPos, -1); }

BOOL WINAPI DuiBridge_ElementExists(HWND hwnd, LPCWSTR pszId) { return func_helpers::FindElement(hwnd, pszId) != nullptr; }

BOOL WINAPI DuiBridge_SetStringProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName, LPCWSTR pszValue) { return func_helpers::SetPropertyValue(hwnd, pszId, pszPropName, Value::CreateString(pszValue ? pszValue : L"", g_hInst)); }

LPWSTR WINAPI DuiBridge_GetStringProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName)
{
    Value* propertyValue = func_helpers::GetPropertyValue(hwnd, pszId, pszPropName);
    if (!propertyValue) return nullptr;

    LPWSTR stringResult = func_helpers::DupString(propertyValue->GetString());
    propertyValue->Release();

    return stringResult;
}

BOOL WINAPI DuiBridge_SetIntProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName, int value) { return func_helpers::SetPropertyValue(hwnd, pszId, pszPropName, Value::CreateInt(value)); }

int WINAPI DuiBridge_GetIntProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName)
{
    Value* propertyValue = func_helpers::GetPropertyValue(hwnd, pszId, pszPropName);
    if (!propertyValue) return 0;

    int intResult = propertyValue->GetInt();
    propertyValue->Release();

    return intResult;
}

BOOL WINAPI DuiBridge_SetBoolProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName, BOOL value) { return func_helpers::SetPropertyValue(hwnd, pszId, pszPropName, Value::CreateBool(value != 0)); }

BOOL WINAPI DuiBridge_GetBoolProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName)
{
    Value* propertyValue = func_helpers::GetPropertyValue(hwnd, pszId, pszPropName);
    if (!propertyValue) return FALSE;

    BOOL boolResult = propertyValue->GetBool() ? TRUE : FALSE;
    propertyValue->Release();

    return boolResult;
}

BOOL WINAPI DuiBridge_SetColorProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName, COLORREF color) { return func_helpers::SetPropertyValue(hwnd, pszId, pszPropName, Value::CreateColor(color)); }

COLORREF WINAPI DuiBridge_GetColorProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName)
{
    Value* propertyValue = func_helpers::GetPropertyValue(hwnd, pszId, pszPropName);
    if (!propertyValue) return 0;

    COLORREF cr = func_helpers::FillToColorRef(propertyValue->GetFill());
    propertyValue->Release();

    return cr;
}

BOOL WINAPI DuiBridge_SetSelection(HWND hwnd, LPCWSTR pszSelectorId, int dIndex)
{
    Element* pEl = func_helpers::FindElement(hwnd, pszSelectorId);
    if (!pEl) return FALSE;

    Selector* pSel = static_cast<Selector*>(pEl);
    Value* childrenValue = nullptr;
    DynamicArray<Element*>* childElements = pSel->GetChildren(&childrenValue);

    BOOL selectionResult = FALSE;
    if (childElements && dIndex >= 0 && (UINT)dIndex < childElements->GetSize())
        selectionResult = SUCCEEDED(pSel->SetSelection(childElements->GetItem((UINT)dIndex)));

    if (childrenValue) childrenValue->Release();
    return selectionResult;
}

int WINAPI DuiBridge_GetSelection(HWND hwnd, LPCWSTR pszSelectorId)
{
    Element* pEl = func_helpers::FindElement(hwnd, pszSelectorId);
    if (!pEl) return -1;

    Selector* pSel = static_cast<Selector*>(pEl);
    Element* peSelected = pSel->GetSelection();
    if (!peSelected) return -1;

    Value* childrenValue = nullptr;
    DynamicArray<Element*>* childElements = pSel->GetChildren(&childrenValue);

    int indexResult = func_helpers::IndexOfChild(childElements, peSelected);
    if (childrenValue) childrenValue->Release();

    return indexResult;
}

BOOL WINAPI DuiBridge_SetSheet(HWND hwnd, LPCWSTR pszId, LPCWSTR pszSheetResId)
{
    WindowCtx* wndCtx = func_helpers::FindCtx(hwnd);
    if (!wndCtx || wndCtx->parsers.empty() || !pszSheetResId) return FALSE;

    Element* pEl = func_helpers::FindElement(wndCtx, pszId);
    if (!pEl) return FALSE;

    Value* sheetValue = nullptr;
    HRESULT hr = E_FAIL;

    for (DUIXmlParser* duiParser : wndCtx->parsers)
    {
        if (duiParser && SUCCEEDED(duiParser->GetSheet(pszSheetResId, &sheetValue)) && sheetValue)
        {
            hr = pEl->SetSheet(sheetValue->GetStyleSheet());
            sheetValue->Release();

            break;
        }
    }
    return SUCCEEDED(hr);
}

BOOL WINAPI DuiBridge_StartDefer(HWND hwnd)
{
    WindowCtx* wndCtx = func_helpers::FindCtx(hwnd);
    if (!wndCtx || !wndCtx->pRoot || wndCtx->deferring) return FALSE;

    wndCtx->pRoot->StartDefer(&wndCtx->dwDeferCookie);
    wndCtx->deferring = true;

    return TRUE;
}

BOOL WINAPI DuiBridge_EndDefer(HWND hwnd)
{
    WindowCtx* wndCtx = func_helpers::FindCtx(hwnd);
    if (!wndCtx || !wndCtx->pRoot || !wndCtx->deferring) return FALSE;

    wndCtx->pRoot->EndDefer(wndCtx->dwDeferCookie);
    wndCtx->deferring = false;

    return TRUE;
}

void WINAPI DuiBridge_SetClickCallback(HWND hwnd, PFNDUI_CLICK pfn, void* pCtx) { func_helpers::SetCallback(hwnd, &WindowCtx::pfnClick, &WindowCtx::pClickCtx, pfn, pCtx); }
void WINAPI DuiBridge_SetPropertyCallback(HWND hwnd, PFNDUI_PROPCHANGE pfn, void* pCtx) { func_helpers::SetCallback(hwnd, &WindowCtx::pfnProp, &WindowCtx::pPropCtx, pfn, pCtx); }
void WINAPI DuiBridge_SetSelectionCallback(HWND hwnd, PFNDUI_SELECTION pfn, void* pCtx) { func_helpers::SetCallback(hwnd, &WindowCtx::pfnSel, &WindowCtx::pSelCtx, pfn, pCtx); }
void WINAPI DuiBridge_SetInputCallback(HWND hwnd, PFNDUI_INPUT pfn, void* pCtx) { func_helpers::SetCallback(hwnd, &WindowCtx::pfnInput, &WindowCtx::pInputCtx, pfn, pCtx); }

void WINAPI DuiBridge_SetParseErrorCallback(PFNDUI_PARSEERROR pfn, void* pCtx)
{
    g_pfnParseError = pfn;
    g_pParseErrorCtx = pCtx;
}

void WINAPI DuiBridge_ShowWindow(HWND hwnd, int nCmdShow)
{
    WindowCtx* wndCtx = func_helpers::FindCtx(hwnd);
    if (wndCtx && wndCtx->pHost) wndCtx->pHost->ShowWindow(nCmdShow);
}

void WINAPI DuiBridge_DestroyWindow(HWND hwnd)
{
    WindowCtx* wndCtx = func_helpers::FindCtx(hwnd);
    if (!wndCtx) return;

    func_helpers::DestroyWindowCtx(wndCtx);
    g_windows.erase(hwnd);
}

void WINAPI DuiBridge_RunMessagePump() { StartMessagePump(); }
void WINAPI DuiBridge_StopMessagePump() { StopMessagePump(); }

BOOL WINAPI DuiBridge_SetContentBitmap(HWND hwnd, LPCWSTR pszId, HBITMAP hBitmap)
{
    Element* pEl = func_helpers::FindElement(hwnd, pszId);
    if (!pEl) return FALSE;

    if (!hBitmap)
        return SUCCEEDED(pEl->SetValue(Element::ContentProp(), 1, Value::GetNull()));

    Value* graphicValue = Value::CreateGraphic(hBitmap, 2, 0xFF, false, false, false);
    if (!graphicValue) return FALSE;

    HRESULT hr = pEl->SetValue(Element::ContentProp(), 1, graphicValue);
    graphicValue->Release();

    return SUCCEEDED(hr);
}

void WINAPI DuiBridge_FreeString(LPWSTR pszStr) { if (pszStr) CoTaskMemFree(pszStr); }
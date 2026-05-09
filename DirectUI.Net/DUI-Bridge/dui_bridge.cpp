#include "dui_bridge.h"
#include "DirectUI/DirectUI.h"
#include <map>
#include <string>
#include <vector>
#include <fstream>

using namespace DirectUI;

struct WindowCtx : IElementListener
{
    NativeHWNDHost* pHost = nullptr;
    HWNDElement* pHwndElement = nullptr;

    Element* pRoot = nullptr;
    std::vector<DUIXmlParser*> parsers;

    PFNDUI_CLICK pfnClick = nullptr;
    void* pClickCtx = nullptr;

    PFNDUI_PROPCHANGE pfnProp = nullptr;
    void* pPropCtx = nullptr;

    PFNDUI_SELECTION pfnSel = nullptr;
    void* pSelCtx = nullptr;

    PFNDUI_INPUT pfnInput = nullptr;
    void* pInputCtx = nullptr;

    DWORD dwDeferCookie = 0;
    bool deferring = false;

    void OnListenerAttach(Element*) override {}
    void OnListenerDetach(Element*) override {}
    bool OnListenedPropertyChanging(Element*, const PropertyInfo*, int, Value*, Value*) override { return true; }

    void OnListenedPropertyChanged(Element* peFrom,
        const PropertyInfo* ppi, int, Value*, Value*) override
    {
        if (!pfnProp || !ppi || !ppi->pszName) return;

        WCHAR szId[128] = {};
        if (ATOM id = peFrom->GetID())
            GetAtomNameW(id, szId, 128);

        pfnProp(szId, ppi->pszName, pPropCtx);
    }

    void OnListenedInput(Element* peFrom, InputEvent* pInput) override
    {
        if (!pfnInput || !pInput) return;

        WCHAR szId[128] = {};
        if (ATOM id = peFrom->GetID())
            GetAtomNameW(id, szId, 128);

        pfnInput(szId, (int)pInput->nDevice, (int)pInput->nCode, (int)pInput->uModifiers, pInputCtx);
    }

    void OnListenedEvent(Element* peFrom, Event* pEvent) override
    {
        if (!pEvent || !pEvent->peTarget) return;

        WCHAR szId[128] = {};
        if (ATOM id = pEvent->peTarget->GetID())
            GetAtomNameW(id, szId, 128);

        if (pfnClick && pEvent->uidType == Button::Click())
        {
            pfnClick(szId, pClickCtx);
            return;
        }

        if (pfnSel && pEvent->uidType == Selector::SelectionChange())
        {
            Selector* pSel = static_cast<Selector*>(pEvent->peTarget);
            Element* peSelected = pSel->GetSelection();

            WCHAR szSelected[128] = {};
            int dIndex = -1;
            if (peSelected)
            {
                if (ATOM idSel = peSelected->GetID())
                    GetAtomNameW(idSel, szSelected, 128);

                Value* pvChildren = nullptr;
                if (DynamicArray<Element*>* pda = pSel->GetChildren(&pvChildren))
                {
                    UINT n = pda->GetSize();
                    for (UINT i = 0; i < n; ++i)
                    {
                        if (pda->GetItem(i) == peSelected) { dIndex = (int)i; break; }
                    }
                }
                if (pvChildren) pvChildren->Release();
            }

            pfnSel(szId, szSelected, dIndex, pSelCtx);
            return;
        }
    }
};

static std::map<HWND, WindowCtx*> g_windows;
static HINSTANCE g_hInst = nullptr;

struct EditSubclassData {
    WNDPROC pfnOrig = nullptr;
    HFONT hFontNormal = nullptr;
    HFONT hFontItalic = nullptr;
};

static std::map<HWND, EditSubclassData> g_editSubclass;

static LRESULT CALLBACK EditSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto it = g_editSubclass.find(hWnd);
    if (it == g_editSubclass.end())
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);

    EditSubclassData& data = it->second;

    switch (uMsg)
    {
    case WM_CHAR:
    {
        if (data.hFontNormal && wParam != VK_BACK && GetWindowTextLengthW(hWnd) == 0)
            SendMessageW(hWnd, WM_SETFONT, (WPARAM)data.hFontNormal, TRUE);
        LRESULT r = CallWindowProcW(data.pfnOrig, hWnd, uMsg, wParam, lParam);
        if (data.hFontItalic && GetWindowTextLengthW(hWnd) == 0)
            SendMessageW(hWnd, WM_SETFONT, (WPARAM)data.hFontItalic, TRUE);
        return r;
    }

    case WM_KILLFOCUS:
        if (data.hFontItalic && GetWindowTextLengthW(hWnd) == 0)
            SendMessageW(hWnd, WM_SETFONT, (WPARAM)data.hFontItalic, TRUE);
        break;

    case WM_NCDESTROY:
    {
        LRESULT r = CallWindowProcW(data.pfnOrig, hWnd, uMsg, wParam, lParam);
        if (data.hFontItalic) DeleteObject(data.hFontItalic);
        g_editSubclass.erase(it);
        return r;
    }
    }

    return CallWindowProcW(data.pfnOrig, hWnd, uMsg, wParam, lParam);
}

static PFNDUI_PARSEERROR g_pfnParseError = nullptr;
static void* g_pParseErrorCtx = nullptr;

static void CALLBACK InternalParseErrorCB(LPCWSTR err, LPCWSTR tok, int line, void*)
{
    if (g_pfnParseError)
        g_pfnParseError(err, tok, line, g_pParseErrorCtx);
    if (err) OutputDebugStringW(err);
}

static WindowCtx* FindCtx(HWND hwnd)
{
    auto it = g_windows.find(hwnd);
    if (it == g_windows.end()) return nullptr;

    return it->second;
}

static Element* FindElement(WindowCtx* ctx, LPCWSTR pszId)
{
    if (!ctx || !ctx->pRoot || !pszId) return nullptr;
    return ctx->pRoot->FindDescendent(StrToID(pszId));
}

static const PropertyInfo* FindPropertyByName(Element* pe, LPCWSTR pszName)
{
    if (!pe || !pszName) return nullptr;

    for (IClassInfo* pci = pe->GetClassInfoW(); pci; pci = pci->GetBaseClass())
    {
        UINT cPI = pci->GetPICount();
        for (UINT i = 0; i < cPI; ++i)
        {
            const PropertyInfo* ppi = pci->GetByClassIndex(i);
            if (ppi && ppi->pszName && wcscmp(ppi->pszName, pszName) == 0)
                return ppi;
        }
    }
    return nullptr;
}

static std::wstring ReadFileAsWide(LPCWSTR pszFilePath)
{
    std::ifstream file(pszFilePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {};

    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buf((size_t)size);
    file.read(buf.data(), size);
    file.close();

    const char* pData = buf.data();
    int cbData = (int)buf.size();

    if (cbData >= 3 && (unsigned char)pData[0] == 0xEF
        && (unsigned char)pData[1] == 0xBB && (unsigned char)pData[2] == 0xBF)
    {
        pData += 3;
        cbData -= 3;
    }

    if (cbData == 0) return {};

    int cch = MultiByteToWideChar(CP_UTF8, 0, pData, cbData, nullptr, 0);
    if (cch <= 0) return {};

    std::wstring result(cch, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, pData, cbData, &result[0], cch);
    return result;
}

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
    for (auto& [hwnd, ctx] : g_windows)
    {
        if (ctx->pRoot)       ctx->pRoot->RemoveListener(ctx);
        if (ctx->pHwndElement) ctx->pHwndElement->Destroy(false);
        if (ctx->pHost)        ctx->pHost->Destroy();
        for (DUIXmlParser* p : ctx->parsers)
            if (p) p->Destroy();
        delete ctx;
    }
    g_windows.clear();
    UnInitThread();
    UnInitProcessPriv(g_hInst);
}

HWND WINAPI DuiBridge_CreateWindow(
    HWND hwndParent, LPCWSTR pszTitle,
    int x, int y, int cx, int cy, DWORD dwStyle)
{
    WindowCtx* ctx = new WindowCtx();
    HRESULT hr = NativeHWNDHost::Create(
        pszTitle, hwndParent, nullptr,
        x, y, cx, cy, 0,
        dwStyle ? dwStyle : WS_OVERLAPPEDWINDOW,
        0, &ctx->pHost);

    if (FAILED(hr)) { delete ctx; return nullptr; }

    HWND hwnd = ctx->pHost->GetHWND();
    g_windows[hwnd] = ctx;

    return hwnd;
}

BOOL WINAPI DuiBridge_LoadXml(HWND hwnd, LPCWSTR pszXml, LPCWSTR pszResId)
{
    WindowCtx* ctx = FindCtx(hwnd);
    if (!ctx || !ctx->pHost || !pszXml) return FALSE;

    DUIXmlParser* pParser = nullptr;
    if (FAILED(DUIXmlParser::Create(&pParser, nullptr, nullptr, InternalParseErrorCB, nullptr)))
        return FALSE;

    if (FAILED(pParser->SetXML(pszXml, g_hInst, g_hInst)))
    {
        pParser->Destroy();
        return FALSE;
    }

    DWORD dwDefer = 0;
    if (FAILED(HWNDElement::Create(ctx->pHost->GetHWND(), true, 0, nullptr, &dwDefer, (Element**)&ctx->pHwndElement)))
    {
        pParser->Destroy();
        return FALSE;
    }

    if (FAILED(pParser->CreateElement(pszResId ? pszResId : L"main", ctx->pHwndElement, nullptr, nullptr, &ctx->pRoot)) || !ctx->pRoot)
    {
        ctx->pHwndElement->EndDefer(dwDefer);
        pParser->Destroy();
        return FALSE;
    }

    ctx->pRoot->SetVisible(true);
    ctx->pRoot->EndDefer(dwDefer);
    ctx->pRoot->AddListener(ctx);
    ctx->parsers.push_back(pParser);
    ctx->pHost->Host(ctx->pRoot);
    ctx->pHost->ShowWindow(SW_SHOW);
    return TRUE;
}

BOOL WINAPI DuiBridge_LoadXmlFile(HWND hwnd, LPCWSTR pszFilePath, LPCWSTR pszResId)
{
    std::wstring xml = ReadFileAsWide(pszFilePath);
    if (xml.empty()) { return FALSE; }

    return DuiBridge_LoadXml(hwnd, xml.c_str(), pszResId);
}

BOOL WINAPI DuiBridge_MountFragment(HWND hwnd, LPCWSTR pszParentId, LPCWSTR pszXml, LPCWSTR pszResId)
{
    WindowCtx* ctx = FindCtx(hwnd);
    if (!ctx || !ctx->pRoot || !pszXml) return FALSE;

    Element* pParent = FindElement(ctx, pszParentId);
    if (!pParent) return FALSE;

    DUIXmlParser* pParser = nullptr;
    HRESULT hr = DUIXmlParser::Create(&pParser, nullptr, nullptr, InternalParseErrorCB, nullptr);
    if (FAILED(hr)) return FALSE;

    hr = pParser->SetXML(pszXml, g_hInst, g_hInst);
    if (FAILED(hr)) { pParser->Destroy(); return FALSE; }

    DWORD dwDefer = 0;
    Element* peFragment = nullptr;
    hr = pParser->CreateElement(
        pszResId ? pszResId : L"main",
        nullptr, nullptr, &dwDefer, &peFragment);

    if (FAILED(hr) || !peFragment)
    {
        pParser->Destroy();
        return FALSE;
    }

    if (dwDefer) peFragment->EndDefer(dwDefer);

    hr = pParent->Add(peFragment);
    if (FAILED(hr))
    {
        peFragment->Destroy(false);
        pParser->Destroy();
        return FALSE;
    }

    ctx->parsers.push_back(pParser);
    return TRUE;
}

BOOL WINAPI DuiBridge_MountFragmentFile(HWND hwnd, LPCWSTR pszParentId, LPCWSTR pszFilePath, LPCWSTR pszResId)
{
    std::wstring xml = ReadFileAsWide(pszFilePath);
    if (xml.empty()) return FALSE;

    return DuiBridge_MountFragment(hwnd, pszParentId, xml.c_str(), pszResId);
}

BOOL WINAPI DuiBridge_RemoveElement(HWND hwnd, LPCWSTR pszId)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return FALSE;

    return SUCCEEDED(pEl->Destroy(true));
}

BOOL WINAPI DuiBridge_SetText(HWND hwnd, LPCWSTR pszId, LPCWSTR pszText)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return FALSE;

    return SUCCEEDED(pEl->SetContentString(pszText));
}

LPWSTR WINAPI DuiBridge_GetText(HWND hwnd, LPCWSTR pszId)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return nullptr;

    Value* pv = nullptr;
    const WCHAR* pszText = pEl->GetContentString(&pv);

    LPWSTR result = nullptr;
    if (pszText)
    {
        size_t len = wcslen(pszText) + 1;
        result = (LPWSTR)CoTaskMemAlloc(len * sizeof(WCHAR));
        if (result) wcscpy_s(result, len, pszText);
    }

    if (pv) pv->Release();
    return result;
}

BOOL WINAPI DuiBridge_SetEnabled(HWND hwnd, LPCWSTR pszId, BOOL bEnabled)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl && SUCCEEDED(pEl->SetEnabled(bEnabled != 0));
}

BOOL WINAPI DuiBridge_GetEnabled(HWND hwnd, LPCWSTR pszId)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl && pEl->GetEnabled();
}

BOOL WINAPI DuiBridge_SetVisible(HWND hwnd, LPCWSTR pszId, BOOL bVisible)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl && SUCCEEDED(pEl->SetVisible(bVisible != 0));
}

BOOL WINAPI DuiBridge_GetVisible(HWND hwnd, LPCWSTR pszId)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl && pEl->GetVisible();
}

BOOL WINAPI DuiBridge_SetAlpha(HWND hwnd, LPCWSTR pszId, int alpha)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl && SUCCEEDED(pEl->SetAlpha(alpha));
}

int WINAPI DuiBridge_GetAlpha(HWND hwnd, LPCWSTR pszId)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl ? pEl->GetAlpha() : 255;
}

BOOL WINAPI DuiBridge_SetClass(HWND hwnd, LPCWSTR pszId, LPCWSTR pszClass)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl && SUCCEEDED(pEl->SetClass(pszClass));
}

LPWSTR WINAPI DuiBridge_GetClass(HWND hwnd, LPCWSTR pszId)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return nullptr;

    Value* pv = nullptr;
    const WCHAR* psz = pEl->GetClass(&pv);

    LPWSTR result = nullptr;
    if (psz)
    {
        size_t len = wcslen(psz) + 1;
        result = (LPWSTR)CoTaskMemAlloc(len * sizeof(WCHAR));
        if (result) wcscpy_s(result, len, psz);
    }
    if (pv) pv->Release();
    return result;
}

BOOL WINAPI DuiBridge_SetWidth(HWND hwnd, LPCWSTR pszId, int width)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl && SUCCEEDED(pEl->SetWidth(width));
}

int WINAPI DuiBridge_GetWidth(HWND hwnd, LPCWSTR pszId)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl ? pEl->GetWidth() : 0;
}

BOOL WINAPI DuiBridge_SetHeight(HWND hwnd, LPCWSTR pszId, int height)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl && SUCCEEDED(pEl->SetHeight(height));
}

int WINAPI DuiBridge_GetHeight(HWND hwnd, LPCWSTR pszId)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl ? pEl->GetHeight() : 0;
}

BOOL WINAPI DuiBridge_SetBackground(HWND hwnd, LPCWSTR pszId, COLORREF color)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl && SUCCEEDED(pEl->SetBackgroundColor(color));
}

COLORREF WINAPI DuiBridge_GetBackground(HWND hwnd, LPCWSTR pszId)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return 0;
    Value* pv = nullptr;
    const Fill* pFill = pEl->GetBackgroundColor(&pv);
    COLORREF cr = (pFill && pFill->dType == 0) ? pFill->ref.cr : 0;
    if (pv) pv->Release();
    return cr;
}

BOOL WINAPI DuiBridge_SetForeground(HWND hwnd, LPCWSTR pszId, COLORREF color)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl && SUCCEEDED(pEl->SetForegroundColor(color));
}

COLORREF WINAPI DuiBridge_GetForeground(HWND hwnd, LPCWSTR pszId)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return 0;
    Value* pv = nullptr;
    const Fill* pFill = pEl->GetForegroundColor(&pv);
    COLORREF cr = (pFill && pFill->dType == 0) ? pFill->ref.cr : 0;
    if (pv) pv->Release();
    return cr;
}

BOOL WINAPI DuiBridge_SetLayoutPos(HWND hwnd, LPCWSTR pszId, int layoutPos)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl && SUCCEEDED(pEl->SetLayoutPos(layoutPos));
}

int WINAPI DuiBridge_GetLayoutPos(HWND hwnd, LPCWSTR pszId)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    return pEl ? pEl->GetLayoutPos() : -1;
}

BOOL WINAPI DuiBridge_ElementExists(HWND hwnd, LPCWSTR pszId) { return FindElement(FindCtx(hwnd), pszId) != nullptr; }

BOOL WINAPI DuiBridge_SetStringProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName, LPCWSTR pszValue)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return FALSE;

    const PropertyInfo* ppi = FindPropertyByName(pEl, pszPropName);
    if (!ppi) return FALSE;

    Value* pv = Value::CreateString(pszValue ? pszValue : L"", g_hInst);
    if (!pv) return FALSE;

    HRESULT hr = pEl->SetValue(ppi, 0, pv);
    pv->Release();
    return SUCCEEDED(hr);
}

LPWSTR WINAPI DuiBridge_GetStringProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return nullptr;

    const PropertyInfo* ppi = FindPropertyByName(pEl, pszPropName);
    if (!ppi) return nullptr;

    Value* pv = pEl->GetValue(ppi, 0, nullptr);
    if (!pv) return nullptr;

    LPWSTR result = nullptr;
    const WCHAR* psz = pv->GetString();
    if (psz)
    {
        size_t len = wcslen(psz) + 1;
        result = (LPWSTR)CoTaskMemAlloc(len * sizeof(WCHAR));
        if (result) wcscpy_s(result, len, psz);
    }
    pv->Release();
    return result;
}

BOOL WINAPI DuiBridge_SetIntProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName, int value)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return FALSE;

    const PropertyInfo* ppi = FindPropertyByName(pEl, pszPropName);
    if (!ppi) return FALSE;

    Value* pv = Value::CreateInt(value);
    if (!pv) return FALSE;

    HRESULT hr = pEl->SetValue(ppi, 0, pv);
    pv->Release();
    return SUCCEEDED(hr);
}

int WINAPI DuiBridge_GetIntProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return 0;

    const PropertyInfo* ppi = FindPropertyByName(pEl, pszPropName);
    if (!ppi) return 0;

    Value* pv = pEl->GetValue(ppi, 0, nullptr);
    if (!pv) return 0;

    int result = pv->GetInt();
    pv->Release();
    return result;
}

BOOL WINAPI DuiBridge_SetBoolProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName, BOOL value)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return FALSE;

    const PropertyInfo* ppi = FindPropertyByName(pEl, pszPropName);
    if (!ppi) return FALSE;

    Value* pv = Value::CreateBool(value != 0);
    if (!pv) return FALSE;

    HRESULT hr = pEl->SetValue(ppi, 0, pv);
    pv->Release();
    return SUCCEEDED(hr);
}

BOOL WINAPI DuiBridge_GetBoolProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return FALSE;

    const PropertyInfo* ppi = FindPropertyByName(pEl, pszPropName);
    if (!ppi) return FALSE;

    Value* pv = pEl->GetValue(ppi, 0, nullptr);
    if (!pv) return FALSE;

    BOOL result = pv->GetBool() ? TRUE : FALSE;
    pv->Release();
    return result;
}

BOOL WINAPI DuiBridge_SetColorProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName, COLORREF color)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return FALSE;

    const PropertyInfo* ppi = FindPropertyByName(pEl, pszPropName);
    if (!ppi) return FALSE;

    Value* pv = Value::CreateColor(color);
    if (!pv) return FALSE;

    HRESULT hr = pEl->SetValue(ppi, 0, pv);
    pv->Release();
    return SUCCEEDED(hr);
}

COLORREF WINAPI DuiBridge_GetColorProperty(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return 0;

    const PropertyInfo* ppi = FindPropertyByName(pEl, pszPropName);
    if (!ppi) return 0;

    Value* pv = pEl->GetValue(ppi, 0, nullptr);
    if (!pv) return 0;

    const Fill* pFill = pv->GetFill();
    COLORREF cr = (pFill && pFill->dType == 0) ? pFill->ref.cr : 0;
    pv->Release();
    return cr;
}

BOOL WINAPI DuiBridge_SetSelection(HWND hwnd, LPCWSTR pszSelectorId, int dIndex)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszSelectorId);
    if (!pEl) return FALSE;

    Selector* pSel = static_cast<Selector*>(pEl);

    Value* pvChildren = nullptr;
    DynamicArray<Element*>* pda = pSel->GetChildren(&pvChildren);
    if (!pda) { if (pvChildren) pvChildren->Release(); return FALSE; }

    BOOL result = FALSE;
    if (dIndex >= 0 && (UINT)dIndex < pda->GetSize())
    {
        result = SUCCEEDED(pSel->SetSelection(pda->GetItem((UINT)dIndex)));
    }
    if (pvChildren) pvChildren->Release();
    return result;
}

int WINAPI DuiBridge_GetSelection(HWND hwnd, LPCWSTR pszSelectorId)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszSelectorId);
    if (!pEl) return -1;

    Selector* pSel = static_cast<Selector*>(pEl);
    Element* peSelected = pSel->GetSelection();
    if (!peSelected) return -1;

    Value* pvChildren = nullptr;
    DynamicArray<Element*>* pda = pSel->GetChildren(&pvChildren);
    int result = -1;
    if (pda)
    {
        UINT n = pda->GetSize();
        for (UINT i = 0; i < n; ++i)
            if (pda->GetItem(i) == peSelected) { result = (int)i; break; }
    }
    if (pvChildren) pvChildren->Release();
    return result;
}

BOOL WINAPI DuiBridge_SetSheet(HWND hwnd, LPCWSTR pszId, LPCWSTR pszSheetResId)
{
    WindowCtx* ctx = FindCtx(hwnd);
    if (!ctx || ctx->parsers.empty() || !pszSheetResId) return FALSE;

    Element* pEl = FindElement(ctx, pszId);
    if (!pEl) return FALSE;

    Value* pvSheet = nullptr;
    HRESULT hr = E_FAIL;
    for (DUIXmlParser* p : ctx->parsers)
    {
        if (p && SUCCEEDED(p->GetSheet(pszSheetResId, &pvSheet)) && pvSheet)
        {
            hr = pEl->SetSheet(pvSheet->GetStyleSheet());
            pvSheet->Release();
            break;
        }
    }
    return SUCCEEDED(hr);
}

BOOL WINAPI DuiBridge_StartDefer(HWND hwnd)
{
    WindowCtx* ctx = FindCtx(hwnd);
    if (!ctx || !ctx->pRoot || ctx->deferring) return FALSE;

    ctx->pRoot->StartDefer(&ctx->dwDeferCookie);
    ctx->deferring = true;
    return TRUE;
}

BOOL WINAPI DuiBridge_EndDefer(HWND hwnd)
{
    WindowCtx* ctx = FindCtx(hwnd);
    if (!ctx || !ctx->pRoot || !ctx->deferring) return FALSE;

    ctx->pRoot->EndDefer(ctx->dwDeferCookie);
    ctx->deferring = false;
    return TRUE;
}

void WINAPI DuiBridge_SetClickCallback(HWND hwnd, PFNDUI_CLICK pfn, void* pCtx)
{
    WindowCtx* ctx = FindCtx(hwnd);
    if (ctx) { ctx->pfnClick = pfn; ctx->pClickCtx = pCtx; }
}

void WINAPI DuiBridge_SetPropertyCallback(HWND hwnd, PFNDUI_PROPCHANGE pfn, void* pCtx)
{
    WindowCtx* ctx = FindCtx(hwnd);
    if (ctx) { ctx->pfnProp = pfn; ctx->pPropCtx = pCtx; }
}

void WINAPI DuiBridge_SetSelectionCallback(HWND hwnd, PFNDUI_SELECTION pfn, void* pCtx)
{
    WindowCtx* ctx = FindCtx(hwnd);
    if (ctx) { ctx->pfnSel = pfn; ctx->pSelCtx = pCtx; }
}

void WINAPI DuiBridge_SetInputCallback(HWND hwnd, PFNDUI_INPUT pfn, void* pCtx)
{
    WindowCtx* ctx = FindCtx(hwnd);
    if (ctx) { ctx->pfnInput = pfn; ctx->pInputCtx = pCtx; }
}

void WINAPI DuiBridge_SetParseErrorCallback(PFNDUI_PARSEERROR pfn, void* pCtx)
{
    g_pfnParseError = pfn;
    g_pParseErrorCtx = pCtx;
}

void WINAPI DuiBridge_ShowWindow(HWND hwnd, int nCmdShow)
{
    WindowCtx* ctx = FindCtx(hwnd);
    if (ctx && ctx->pHost) ctx->pHost->ShowWindow(nCmdShow);
}

void WINAPI DuiBridge_DestroyWindow(HWND hwnd)
{
    WindowCtx* ctx = FindCtx(hwnd);
    if (!ctx) return;

    if (ctx->pRoot) ctx->pRoot->RemoveListener(ctx);
    if (ctx->pHwndElement) ctx->pHwndElement->Destroy(false);
    if (ctx->pHost) ctx->pHost->Destroy();
    for (DUIXmlParser* p : ctx->parsers)
        if (p) p->Destroy();
    delete ctx;

    g_windows.erase(hwnd);
}

void WINAPI DuiBridge_RunMessagePump() { StartMessagePump(); }
void WINAPI DuiBridge_StopMessagePump() { StopMessagePump(); }

BOOL WINAPI DuiBridge_SetCueBannerItalic(HWND hwnd, LPCWSTR pszId)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return FALSE;

    HWNDElement* pHwndEl = static_cast<HWNDElement*>(pEl);
    HWND hEdit = pHwndEl->GetHWND();
    if (!hEdit) return FALSE;

    if (g_editSubclass.count(hEdit)) return TRUE;

    HFONT hFontNormal = (HFONT)SendMessageW(hEdit, WM_GETFONT, 0, 0);
    if (!hFontNormal) hFontNormal = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    LOGFONTW lf = {};
    GetObjectW(hFontNormal, sizeof(lf), &lf);
    lf.lfItalic = TRUE;
    HFONT hFontItalic = CreateFontIndirectW(&lf);
    if (!hFontItalic) return FALSE;

    EditSubclassData data;
    data.hFontNormal = hFontNormal;
    data.hFontItalic = hFontItalic;
    data.pfnOrig = (WNDPROC)SetWindowLongPtrW(hEdit, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
    g_editSubclass[hEdit] = data;

    SendMessageW(hEdit, WM_SETFONT, (WPARAM)hFontItalic, TRUE);
    return TRUE;
}

BOOL WINAPI DuiBridge_SetContentBitmap(HWND hwnd, LPCWSTR pszId, HBITMAP hBitmap)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return FALSE;

    if (!hBitmap)
        return SUCCEEDED(pEl->SetValue(Element::ContentProp(), 1, Value::GetNull()));

    Value* pv = Value::CreateGraphic(hBitmap, 2, 0xFF, false, false, false);
    if (!pv) return FALSE;

    HRESULT hr = pEl->SetValue(Element::ContentProp(), 1, pv);
    pv->Release();
    return SUCCEEDED(hr);
}

BOOL WINAPI DuiBridge_SetCueBanner(HWND hwnd, LPCWSTR pszId, LPCWSTR pszText)
{
    Element* pEl = FindElement(FindCtx(hwnd), pszId);
    if (!pEl) return FALSE;

    HWNDElement* pHwndEl = static_cast<HWNDElement*>(pEl);
    HWND hEdit = pHwndEl->GetHWND();
    if (!hEdit) return FALSE;

    SendMessageW(hEdit, EM_SETCUEBANNER, TRUE, (LPARAM)pszText);
    return TRUE;
}

void WINAPI DuiBridge_FreeString(LPWSTR pszStr) { if (pszStr) CoTaskMemFree(pszStr); }
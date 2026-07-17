#pragma once
#include "dui_bridge.h"
#include "DirectUI/DirectUI.h"
#include <string>
#include <vector>

using namespace DirectUI;

struct WindowCtx;

using SetColorMethod = HRESULT (Element::*)(COLORREF);

class func_helpers
{
public:
    static void GetElementIdName(Element* pe, WCHAR(&szBuf)[128]);
    static int IndexOfChild(DynamicArray<Element*>* childElements, Element* peChild);

    static WindowCtx* FindCtx(HWND hwnd);

    template <typename TFn>
    static void SetCallback(HWND hwnd, TFn WindowCtx::* pfnMember, void* WindowCtx::* pCtxMember, TFn pfn, void* pCtx);

    static Element* FindElement(WindowCtx* wndCtx, LPCWSTR pszId);
    static Element* FindElement(HWND hwnd, LPCWSTR pszId);

    static const PropertyInfo* FindPropertyByName(Element* pe, LPCWSTR pszName);

    static Value* GetPropertyValue(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName);
    static BOOL SetPropertyValue(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName, Value* propertyValue);

    static LPWSTR DupString(const WCHAR* psz);
    static COLORREF FillToColorRef(const Fill* pFill);

    template <typename Method, typename T>
    static BOOL SetElementValue(HWND hwnd, LPCWSTR pszId, Method setter, T value);

    template <typename Method, typename T>
    static T GetElementValue(HWND hwnd, LPCWSTR pszId, Method getter, T defaultValue);

    template <typename Method>
    static COLORREF GetElementColor(HWND hwnd, LPCWSTR pszId, Method getter);

    template <typename Method>
    static LPWSTR GetElementString(HWND hwnd, LPCWSTR pszId, Method getter);

    static std::wstring ReadFileAsWide(LPCWSTR pszFilePath);
    static DUIXmlParser* CreateParserFromXml(LPCWSTR pszXml);
    static void DestroyWindowCtx(WindowCtx* wndCtx);

    static void CALLBACK InternalParseErrorCB(LPCWSTR err, LPCWSTR tok, int line, void*);
};

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

        WCHAR szId[128];
        func_helpers::GetElementIdName(peFrom, szId);
        pfnProp(szId, ppi->pszName, pPropCtx);
    }

    void OnListenedInput(Element* peFrom, InputEvent* pInput) override
    {
        if (!pfnInput || !pInput) return;

        WCHAR szId[128];
        func_helpers::GetElementIdName(peFrom, szId);
        pfnInput(szId, (int)pInput->nDevice, (int)pInput->nCode, (int)pInput->uModifiers, pInputCtx);
    }

    void OnListenedEvent(Element* peFrom, Event* pEvent) override
    {
        if (!pEvent || !pEvent->peTarget) return;

        WCHAR szId[128];
        func_helpers::GetElementIdName(pEvent->peTarget, szId);

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
                func_helpers::GetElementIdName(peSelected, szSelected);

                Value* childrenValue = nullptr;
                DynamicArray<Element*>* childElements = pSel->GetChildren(&childrenValue);
                dIndex = func_helpers::IndexOfChild(childElements, peSelected);
                if (childrenValue) childrenValue->Release();
            }

            pfnSel(szId, szSelected, dIndex, pSelCtx);
            return;
        }
    }
};

// Template members must stay in the header: their definitions need to be visible
// wherever they're instantiated (dui_bridge.cpp), not just in func_helpers.cpp.

template <typename TFn>
void func_helpers::SetCallback(HWND hwnd, TFn WindowCtx::* pfnMember, void* WindowCtx::* pCtxMember, TFn pfn, void* pCtx)
{
    WindowCtx* wndCtx = FindCtx(hwnd);
    if (wndCtx) { wndCtx->*pfnMember = pfn; wndCtx->*pCtxMember = pCtx; }
}

template <typename Method, typename T>
BOOL func_helpers::SetElementValue(HWND hwnd, LPCWSTR pszId, Method setter, T value)
{
    Element* pEl = FindElement(hwnd, pszId);
    return pEl && SUCCEEDED((pEl->*setter)(value));
}

template <typename Method, typename T>
T func_helpers::GetElementValue(HWND hwnd, LPCWSTR pszId, Method getter, T defaultValue)
{
    Element* pEl = FindElement(hwnd, pszId);
    return pEl ? (pEl->*getter)() : defaultValue;
}

template <typename Method>
COLORREF func_helpers::GetElementColor(HWND hwnd, LPCWSTR pszId, Method getter)
{
    Element* pEl = FindElement(hwnd, pszId);
    if (!pEl) return 0;

    Value* colorValue = nullptr;
    COLORREF cr = FillToColorRef((pEl->*getter)(&colorValue));
    if (colorValue) colorValue->Release();
    return cr;
}

template <typename Method>
LPWSTR func_helpers::GetElementString(HWND hwnd, LPCWSTR pszId, Method getter)
{
    Element* pEl = FindElement(hwnd, pszId);
    if (!pEl) return nullptr;

    Value* stringValue = nullptr;
    LPWSTR result = DupString((pEl->*getter)(&stringValue));
    if (stringValue) stringValue->Release();
    return result;
}

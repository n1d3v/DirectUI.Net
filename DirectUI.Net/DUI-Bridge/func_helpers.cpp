#include "func_helpers.h"
#include <map>
#include <fstream>

extern std::map<HWND, WindowCtx*> g_windows;
extern HINSTANCE g_hInst;
extern PFNDUI_PARSEERROR g_pfnParseError;
extern void* g_pParseErrorCtx;

void func_helpers::GetElementIdName(Element* pe, WCHAR(&szBuf)[128])
{
    szBuf[0] = L'\0';
    if (pe)
    {
        if (ATOM id = pe->GetID())
            GetAtomNameW(id, szBuf, 128);
    }
}

int func_helpers::IndexOfChild(DynamicArray<Element*>* childElements, Element* peChild)
{
    if (!childElements || !peChild) return -1;

    UINT n = childElements->GetSize();
    for (UINT i = 0; i < n; ++i)
        if (childElements->GetItem(i) == peChild) return (int)i;

    return -1;
}

WindowCtx* func_helpers::FindCtx(HWND hwnd)
{
    auto it = g_windows.find(hwnd);
    return it != g_windows.end() ? it->second : nullptr;
}

Element* func_helpers::FindElement(WindowCtx* wndCtx, LPCWSTR pszId)
{
    if (!wndCtx || !wndCtx->pRoot || !pszId) return nullptr;
    return wndCtx->pRoot->FindDescendent(StrToID(pszId));
}

Element* func_helpers::FindElement(HWND hwnd, LPCWSTR pszId) { return FindElement(FindCtx(hwnd), pszId); }

const PropertyInfo* func_helpers::FindPropertyByName(Element* pe, LPCWSTR pszName)
{
    if (!pe || !pszName) return nullptr;

    for (IClassInfo* pClassInfo = pe->GetClassInfoW(); pClassInfo; pClassInfo = pClassInfo->GetBaseClass())
    {
        UINT propertyCount = pClassInfo->GetPICount();
        for (UINT i = 0; i < propertyCount; ++i)
        {
            const PropertyInfo* ppi = pClassInfo->GetByClassIndex(i);
            if (ppi && ppi->pszName && wcscmp(ppi->pszName, pszName) == 0)
                return ppi;
        }
    }

    return nullptr;
}

Value* func_helpers::GetPropertyValue(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName)
{
    Element* pEl = FindElement(hwnd, pszId);
    const PropertyInfo* ppi = pEl ? FindPropertyByName(pEl, pszPropName) : nullptr;

    return ppi ? pEl->GetValue(ppi, 0, nullptr) : nullptr;
}

BOOL func_helpers::SetPropertyValue(HWND hwnd, LPCWSTR pszId, LPCWSTR pszPropName, Value* propertyValue)
{
    if (!propertyValue) return FALSE;

    Element* pEl = FindElement(hwnd, pszId);
    const PropertyInfo* ppi = pEl ? FindPropertyByName(pEl, pszPropName) : nullptr;

    HRESULT hr = ppi ? pEl->SetValue(ppi, 0, propertyValue) : E_FAIL;
    propertyValue->Release();

    return SUCCEEDED(hr);
}

LPWSTR func_helpers::DupString(const WCHAR* psz)
{
    if (!psz) return nullptr;

    size_t len = wcslen(psz) + 1;
    LPWSTR stringResult = (LPWSTR)CoTaskMemAlloc(len * sizeof(WCHAR));

    if (stringResult) wcscpy_s(stringResult, len, psz);
    return stringResult;
}

COLORREF func_helpers::FillToColorRef(const Fill* pFill) { return (pFill && pFill->dType == 0) ? pFill->ref.cr : 0; }

std::wstring func_helpers::ReadFileAsWide(LPCWSTR pszFilePath)
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

    if (cbData >= 3 && (unsigned char)pData[0] == 0xEF && (unsigned char)pData[1] == 0xBB && (unsigned char)pData[2] == 0xBF)
    {
        pData += 3;
        cbData -= 3;
    }

    if (cbData == 0) return {};

    int cch = MultiByteToWideChar(CP_UTF8, 0, pData, cbData, nullptr, 0);
    if (cch <= 0) return {};

    std::wstring wideResult(cch, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, pData, cbData, &wideResult[0], cch);

    return wideResult;
}

DUIXmlParser* func_helpers::CreateParserFromXml(LPCWSTR pszXml)
{
    DUIXmlParser* duiParser = nullptr;

    if (FAILED(DUIXmlParser::Create(&duiParser, nullptr, nullptr, InternalParseErrorCB, nullptr)))
        return nullptr;
    if (FAILED(duiParser->SetXML(pszXml, g_hInst, g_hInst)))
    {
        duiParser->Destroy();
        return nullptr;
    }

    return duiParser;
}

void func_helpers::DestroyWindowCtx(WindowCtx* wndCtx)
{
    if (wndCtx->pRoot) wndCtx->pRoot->RemoveListener(wndCtx);
    if (wndCtx->pHwndElement) wndCtx->pHwndElement->Destroy(false);
    if (wndCtx->pHost) wndCtx->pHost->Destroy();

    for (DUIXmlParser* duiParser : wndCtx->parsers)
        if (duiParser) duiParser->Destroy();

    delete wndCtx;
}

void CALLBACK func_helpers::InternalParseErrorCB(LPCWSTR err, LPCWSTR tok, int line, void*)
{
    if (g_pfnParseError)
        g_pfnParseError(err, tok, line, g_pParseErrorCtx);

    if (err) OutputDebugStringW(err);
}
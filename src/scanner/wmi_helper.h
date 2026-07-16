#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <comdef.h>
#include <wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

#include <string>
#include <stdexcept>

namespace GameCore::Scanner {

inline void Trim(std::string& s)
{
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) { s.clear(); return; }
    s = s.substr(first, s.find_last_not_of(" \t\r\n") - first + 1);
}

inline std::string WmiQueryFirst(const wchar_t* wql, const wchar_t* property)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool comInit = SUCCEEDED(hr);

    hr = CoInitializeSecurity(
        nullptr, -1, nullptr, nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr, EOAC_NONE, nullptr);

    if (FAILED(hr) && hr != RPC_E_TOO_LATE) {
        if (comInit) CoUninitialize();
        throw std::runtime_error("CoInitializeSecurity failed");
    }

    IWbemLocator* locator = nullptr;
    hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, reinterpret_cast<void**>(&locator));
    if (FAILED(hr)) {
        if (comInit) CoUninitialize();
        throw std::runtime_error("Failed to create IWbemLocator");
    }

    IWbemServices* services = nullptr;
    hr = locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"),
        nullptr, nullptr, nullptr, 0, nullptr, nullptr, &services);
    locator->Release();
    if (FAILED(hr)) {
        if (comInit) CoUninitialize();
        throw std::runtime_error("WMI ConnectServer failed");
    }

    CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
        nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr, EOAC_NONE);

    IEnumWbemClassObject* enumerator = nullptr;
    hr = services->ExecQuery(
        _bstr_t(L"WQL"), _bstr_t(wql),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr, &enumerator);
    services->Release();
    if (FAILED(hr)) {
        if (comInit) CoUninitialize();
        throw std::runtime_error("WMI ExecQuery failed");
    }

    IWbemClassObject* obj = nullptr;
    ULONG returned = 0;
    enumerator->Next(WBEM_INFINITE, 1, &obj, &returned);
    enumerator->Release();

    std::string result;
    if (obj && returned > 0) {
        VARIANT var{};
        VariantInit(&var);
        if (SUCCEEDED(obj->Get(property, 0, &var, nullptr, nullptr))
            && var.vt == VT_BSTR)
        {
            int len = WideCharToMultiByte(CP_UTF8, 0, var.bstrVal, -1,
                                          nullptr, 0, nullptr, nullptr);
            result.resize(len > 0 ? len - 1 : 0);
            if (len > 0)
                WideCharToMultiByte(CP_UTF8, 0, var.bstrVal, -1,
                                    result.data(), len, nullptr, nullptr);
        }
        VariantClear(&var);
        obj->Release();
    }

    if (comInit) CoUninitialize();
    return result;
}

} // namespace GameCore::Scanner
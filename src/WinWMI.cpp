/* Copyright (C) 2017 BARBOTIN Nicolas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define M_WMI_DECLARE
#include "mgpcl/WinWMI.h"
#include "mgpcl/Mutex.h"

#ifdef MGPCL_WIN

static bool g_initialized = false;
static m::Mutex g_mutex;
static m::String g_error;
static IWbemServices *g_svc;

bool m::wmi::acquire()
{
    g_mutex.lock();
    if(g_initialized)
        return true;

    HRESULT res = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if(FAILED(res)) {
        g_error = "CoInitializeEx failed";
        return false;
    }

    res = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
    if(FAILED(res)) {
        CoUninitialize();
        g_error = "CoInitializeSecurity failed";
        return false;
    }

    IWbemLocator *locator = nullptr;
    res = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<void**>(&locator));
    if(FAILED(res)) {
        CoUninitialize();
        g_error = "CoCreateInstance failed";
        return false;
    }

    res = locator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &g_svc);
    if(FAILED(res)) {
        locator->Release();
        CoUninitialize();
        g_error = "IWbemLocator::ConnectServer failed";
        return false;
    }

    res = CoSetProxyBlanket(g_svc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    if(FAILED(res)) {
        g_svc->Release();
        locator->Release();
        CoUninitialize();
        g_error = "CoSetProxyBlanket failed";
        return false;
    }

    g_initialized = true;
    return true;
}

m::WMIResult *m::wmi::query(const char *q)
{
    IEnumWbemClassObject *enumerator = nullptr;
    if(FAILED(g_svc->ExecQuery(bstr_t("WQL"), bstr_t(q), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &enumerator)) || enumerator == nullptr) {
        g_error = "IWbemServices::ExecQuery failed";
        return nullptr;
    }

    WMIResult *ret = new WMIResult(enumerator);
    enumerator->Release();
    return ret;
}

void m::wmi::release()
{
    g_mutex.unlock();
}

const m::String &m::wmi::lastError()
{
    return g_error;
}

m::WMIResult::WMIResult(IEnumWbemClassObject *ienum) : m_refs(1)
{
    m_enumerator = ienum;
    m_entry = nullptr;
    m_enumerator->AddRef();
}

bool m::WMIResult::next()
{
    if(m_entry != nullptr) {
        m_entry->Release();
        m_entry = nullptr;
    }

    ULONG cnt = 1;
    if(FAILED(m_enumerator->Next(WBEM_INFINITE, 1, &m_entry, &cnt))) {
        g_error = "IEnumWbemClassObject::Next failed";
        return false;
    }

    if(!g_error.isEmpty())
        g_error.cleanup();

    return cnt > 0 && m_entry != nullptr;
}

m::String m::WMIResult::getString(LPCWSTR key)
{
    if(m_entry == nullptr)
        return String();

    VARIANT v;
    m_entry->Get(key, 0, &v, nullptr, nullptr);

    const size_t sz = wcslen(v.bstrVal) * 2 + 2;
    char *tmp = new char[sz];
    size_t cvt = 0;
    wcstombs_s(&cvt, tmp, sz, v.bstrVal, _TRUNCATE);

    String ret(tmp, static_cast<int>(cvt - 1));
    delete[] tmp;

    VariantClear(&v);
    return ret;
}

uint32_t m::WMIResult::getUInt32(LPCWSTR key)
{
    if(m_entry == nullptr)
        return 0xFFFFFFFF;

    VARIANT v;
    m_entry->Get(key, 0, &v, nullptr, nullptr);

    uint32_t ret = v.uintVal;
    VariantClear(&v);
    return ret;
}

void m::WMIResult::addRef()
{
    m_refs.increment();
}

void m::WMIResult::releaseRef()
{
    if(m_refs.decrement()) {
        if(m_entry != nullptr)
            m_entry->Release();

        m_enumerator->Release();
        delete this;
    }
}

#endif

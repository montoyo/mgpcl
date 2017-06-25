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

#include "mgpcl/SSLContext.h"

#ifndef MGPCL_NO_SSL
#include <openssl/ssl.h>
#include <openssl/x509.h>

#define m_ctx (*reinterpret_cast<SSL_CTX**>(&m_ctx_))

static const SSL_METHOD *g_sslMethod(m::SSLContextMethod method)
{
	switch(method) {
	case m::kSCM_v23Method:
		return SSLv23_method();

	case m::kSCM_v23Client:
		return SSLv23_client_method();

	case m::kSCM_v23Server:
		return SSLv23_server_method();

	default:
		return nullptr;
	}
}

m::SSLContext::SSLContext()
{
	m_ctx = nullptr;
	m_refs = nullptr;
}

m::SSLContext::SSLContext(SSLContextMethod method)
{
	const SSL_METHOD *m = g_sslMethod(method);

	if(m == nullptr) {
		m_ctx = nullptr;
		m_refs = nullptr;
	} else {
		m_ctx = SSL_CTX_new(m);
		m_refs = new Atomic(1);
	}
}

m::SSLContext::SSLContext(const SSLContext &src)
{
	if(src.m_refs == nullptr) {
		m_ctx = nullptr;
		m_refs = nullptr;
	} else {
		m_ctx_ = src.m_ctx_;
		m_refs = src.m_refs;
		m_refs->increment();
	}
}

m::SSLContext::SSLContext(SSLContext &&src)
{
	m_ctx_ = src.m_ctx_;
	m_refs = src.m_refs;

	src.m_refs = nullptr;
	src.m_ctx_ = nullptr; //Just in case
}

m::SSLContext::~SSLContext()
{
	if(m_refs != nullptr && m_refs->decrement()) {
		delete m_refs;
		SSL_CTX_free(m_ctx);
	}
}

bool m::SSLContext::initialize(SSLContextMethod method)
{
	if(m_refs != nullptr && m_refs->decrement()) {
		delete m_refs;
		SSL_CTX_free(m_ctx);
	}

	const SSL_METHOD *m = g_sslMethod(method);
	if(m == nullptr) {
		m_ctx = nullptr;
		m_refs = nullptr;
		return false;
	}

	m_ctx = SSL_CTX_new(m);
	m_refs = new Atomic(1);
	return true;
}

m::SSLContext &m::SSLContext::operator = (const SSLContext &src)
{
	if(m_ctx_ == src.m_ctx_)
		return *this;

	if(m_refs != nullptr && m_refs->decrement()) {
		delete m_refs;
		SSL_CTX_free(m_ctx);
	}

	if(src.m_refs == nullptr) {
		m_ctx = nullptr;
		m_refs = nullptr;
	} else {
		m_ctx_ = src.m_ctx_;
		m_refs = src.m_refs;
		m_refs->increment();
	}

	return *this;
}

m::SSLContext &m::SSLContext::operator = (SSLContext &&src)
{
	if(m_refs != nullptr && m_refs->decrement()) {
		delete m_refs;
		SSL_CTX_free(m_ctx);
	}

	m_ctx_ = src.m_ctx_;
	m_refs = src.m_refs;

	src.m_refs = nullptr;
	src.m_ctx_ = nullptr; //Just in case
	return *this;
}

bool m::SSLContext::loadVerifyPath(const String &path)
{
	if(m_refs == nullptr)
		return false;

	return SSL_CTX_load_verify_locations(m_ctx, nullptr, path.raw()) != 0;
}

bool m::SSLContext::loadVerifyFile(const String &file)
{
	if(m_refs == nullptr)
		return false;

	return SSL_CTX_load_verify_locations(m_ctx, file.raw(), nullptr) != 0;
}

bool m::SSLContext::setVerifyFlags(int flags)
{
	if(m_refs == nullptr)
		return false;

	int mode = 0;
	if(flags & kSVF_VerifyPeer)
		mode |= SSL_VERIFY_PEER;
	
	if(flags & kSVF_FailIfNoPeerCert)
		mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;

	if(flags & kSVF_VerifyClientOnce)
		mode |= SSL_VERIFY_CLIENT_ONCE;

	SSL_CTX_set_verify(m_ctx, mode, nullptr);
	return true;
}

bool m::SSLContext::setVerifyDepth(int depth)
{
	if(m_refs == nullptr)
		return false;

	SSL_CTX_set_verify_depth(m_ctx, depth);
	return true;
}

#ifdef MGPCL_WIN
#include <Wincrypt.h>
#include <iostream>

bool m::SSLContext::loadOSVerify()
{
	if(m_ctx == nullptr)
		return false;

	HANDLE store = CertOpenSystemStore(0, "ROOT");
	if(store == nullptr)
		return false;

	PCCERT_CONTEXT cert = nullptr;
	X509_STORE *store2 = SSL_CTX_get_cert_store(m_ctx);
	bool create = (store2 == nullptr);

	if(create)
		store2 = X509_STORE_new();

	while((cert = CertEnumCertificatesInStore(store, cert)) != nullptr) {
		const unsigned char *p = cert->pbCertEncoded;
		X509 *cert2 = d2i_X509(nullptr, &p, cert->cbCertEncoded);

		if(cert2 == nullptr)
			return false;

		X509_STORE_add_cert(store2, cert2);
	}

	if(create)
		SSL_CTX_set_cert_store(m_ctx, store2);

	CertCloseStore(store, 0);
	return true;
}
#else
bool m::SSLContext::loadOSVerify()
{
	if(m_refs == nullptr)
		return false;

	return SSL_CTX_load_verify_locations(m_ctx, nullptr, "/etc/ssl/certs") != 0;
}
#endif
#endif

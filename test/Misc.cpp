#include "TestAPI.h"
#include <fstream>
#include <mgpcl/FFT.h>
#include <mgpcl/Time.h>
#include <mgpcl/Mem.h>
#include <mgpcl/Util.h>
#include <mgpcl/String.h>
#include <mgpcl/RSA.h>
#include <mgpcl/SHA.h>
#include <mgpcl/AES.h>
#include <mgpcl/Random.h>
#include <mgpcl/ByteBuf.h>
#include <mgpcl/BufferIOStream.h>
#include <mgpcl/StringIOStream.h>
#include <mgpcl/SimpleConfig.h>
#include <mgpcl/File.h>

Declare Test("misc"), Priority(14.0);

#define M_FFT_EPSILON 0.01f

static bool checkValues(float *a, float *b)
{
    std::ifstream in("fft.csv");
    testAssert(!!in, "couldn't open input frequencies");

    int cnt;
    float ta, tb;
    char comma;

    for(cnt = 0; cnt < 2048 && !in.eof(); cnt++) {
        in >> ta >> comma >> tb;
        testAssert(comma == ';' || comma == ',', "invalid csv");

        if(a[cnt] < ta - M_FFT_EPSILON || a[cnt] > ta + M_FFT_EPSILON)
            return false;

        if(b[cnt] < tb - M_FFT_EPSILON || b[cnt] > tb + M_FFT_EPSILON)
            return false;
    }

    in.close();
    testAssert(cnt >= 2048, "didn't read enough frequencies");
    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;
    std::ifstream in("samples.csv");
    testAssert(!!in, "couldn't open input samples");

    float *samples = m::Mem::alignedNew<float>(2048, 16);
    int cnt;

    for(cnt = 0; cnt < 2048 && !in.eof(); cnt++)
        in >> samples[cnt];

    in.close();
    testAssert(cnt >= 2048, "didn't read enough samples!");

    float *a = m::Mem::alignedNew<float>(2048, 16);
    float *b = m::Mem::alignedNew<float>(2048, 16);

    double start = m::time::getTimeMs();
    m::fft::applySSE(samples, a, b, 2048);
    std::cout << "[i]\tfft::applySSE() took " << m::time::getTimeMs() - start << std::endl;
    testAssert(checkValues(a, b), "got wrong FFT computations (SSE)");

    //Clear it, just to make sure...
    m::Mem::zero(a, sizeof(float) * 2048);
    m::Mem::zero(b, sizeof(float) * 2048);

    start = m::time::getTimeMs();
    m::fft::apply(samples, a, b, 2048);
    std::cout << "[i]\tfft::apply() took " << m::time::getTimeMs() - start << std::endl;
    testAssert(checkValues(a, b), "got wrong FFT computations");

    m::Mem::alignedDelete<float>(samples);
    m::Mem::alignedDelete<float>(a);
    m::Mem::alignedDelete<float>(b);
    return true;
}

class AutoDelBuf
{
public:
    AutoDelBuf(uint32_t sz)
    {
        data = new uint8_t[sz];
    }

    ~AutoDelBuf()
    {
        delete[] data;
    }

    uint8_t *ptr() const
    {
        return data;
    }

    const char *str() const
    {
        return reinterpret_cast<const char*>(data);
    }

private:
    uint8_t *data;
};

TEST
{
    volatile StackIntegrityChecker sic;
    const char i1[] = "How could one man have slipped through your force's fingers time and time again? How is it possible? This is not some agent provocateur or highly trained assassin we are discussing. Gordon Freeman is a theoretical physicist who had hardly earned the distinction of his Ph.D. at the time of the Black Mesa Incident. I have good reasons to believe that in the intervening years, he was in a state that precluded further development of covert skills. The man you have consistently failed to slow, let alone capture, is by all standards simply that - an ordinary man. How can you have failed to apprehend him?";
    const char o1[] = "SG93IGNvdWxkIG9uZSBtYW4gaGF2ZSBzbGlwcGVkIHRocm91Z2ggeW91ciBmb3JjZSdzIGZpbmdlcnMgdGltZSBhbmQgdGltZSBhZ2Fpbj8gSG93IGlzIGl0IHBvc3NpYmxlPyBUaGlzIGlzIG5vdCBzb21lIGFnZW50IHByb3ZvY2F0ZXVyIG9yIGhpZ2hseSB0cmFpbmVkIGFzc2Fzc2luIHdlIGFyZSBkaXNjdXNzaW5nLiBHb3Jkb24gRnJlZW1hbiBpcyBhIHRoZW9yZXRpY2FsIHBoeXNpY2lzdCB3aG8gaGFkIGhhcmRseSBlYXJuZWQgdGhlIGRpc3RpbmN0aW9uIG9mIGhpcyBQaC5ELiBhdCB0aGUgdGltZSBvZiB0aGUgQmxhY2sgTWVzYSBJbmNpZGVudC4gSSBoYXZlIGdvb2QgcmVhc29ucyB0byBiZWxpZXZlIHRoYXQgaW4gdGhlIGludGVydmVuaW5nIHllYXJzLCBoZSB3YXMgaW4gYSBzdGF0ZSB0aGF0IHByZWNsdWRlZCBmdXJ0aGVyIGRldmVsb3BtZW50IG9mIGNvdmVydCBza2lsbHMuIFRoZSBtYW4geW91IGhhdmUgY29uc2lzdGVudGx5IGZhaWxlZCB0byBzbG93LCBsZXQgYWxvbmUgY2FwdHVyZSwgaXMgYnkgYWxsIHN0YW5kYXJkcyBzaW1wbHkgdGhhdCAtIGFuIG9yZGluYXJ5IG1hbi4gSG93IGNhbiB5b3UgaGF2ZSBmYWlsZWQgdG8gYXBwcmVoZW5kIGhpbT8=";
    const char i2[] = "Old gabe newell had a farm, ei ei oh";
    const char o2[] = "T2xkIGdhYmUgbmV3ZWxsIGhhZCBhIGZhcm0sIGVpIGVpIG9o";
    const char i3[] = "Prepare for unforseen consequences";
    const char o3[] = "UHJlcGFyZSBmb3IgdW5mb3JzZWVuIGNvbnNlcXVlbmNlcw==";
    const char *input[] = { i1, i2, i3 };
    const char *output[] = { o1, o2, o3 };
    uint32_t crcs[] = { 0x4B19B121, 0x671A2A16, 0x1B6E2B1A };

    m::String result;

    for(int i = 0; i < 3; i++) {
        std::cout << "[i]\tTesting string " << i + 1 << std::endl;

        result.cleanup();
        m::base64Encode(reinterpret_cast<const uint8_t*>(input[i]), static_cast<uint32_t>(strlen(input[i])), result);
        testAssert(result == output[i], "invalid base64 encoding");

        uint32_t bufLen;
        testAssert(m::base64Decode(output[i], nullptr, bufLen), "couldn't estimate base64 size");

        AutoDelBuf buf(bufLen + 1);
        testAssert(m::base64Decode(output[i], buf.ptr(), bufLen), "couldn't decode base64");
        buf.ptr()[bufLen] = 0;
        testAssert(strcmp(buf.str(), input[i]) == 0, "invalid base64 decoding");

        uint32_t crc = m::crc32(reinterpret_cast<const uint8_t*>(input[i]), static_cast<uint32_t>(strlen(input[i])));
        testAssert(crc == crcs[i], "invalid crc32");
    }

    return true;
}

#ifndef MGPCL_NO_SSL

static bool g_privKeyCmp(const m::RSAPrivateKey &a, const m::RSAPrivateKey &b)
{
    //It's expanded for debug reasons
    bool tp = (a.p() == b.p());
    bool tq = (a.q() == b.q());
    bool te = (a.e() == b.e());
    bool tn = (a.n() == b.n());
    bool td = (a.d() == b.d());

    return tp && tq && te && tn && td;
}

TEST
{
    volatile StackIntegrityChecker sic;

    //Public encrypt/private decrypt test...
    m::RSA rsa;
    double t = m::time::getTimeMs();
    testAssert(rsa.generateKeys(2048), "couldn't generate keys");
    t = m::time::getTimeMs() - t;
    std::cout << "[i]\tGenerated RSA keys in " << t << " ms" << std::endl;

    const char *test = "why isn't the coding finished?";
    uint8_t *cipher = new uint8_t[rsa.size()];
    testAssert(rsa.publicEncrypt(reinterpret_cast<const uint8_t*>(test), static_cast<uint32_t>(strlen(test)), cipher, m::kRSAP_PKCS1OAEP), "couldn't encrypt data 1");

    char *plain = new char[rsa.size()];
    int sz = rsa.privateDecrypt(cipher, rsa.size(), reinterpret_cast<uint8_t*>(plain), m::kRSAP_PKCS1OAEP);
    testAssert(sz >= 0, "couldn't decypt data 1");
    testAssert(sz == strlen(test) && m::Mem::cmp(test, plain, sz) == 0, "chipertext does not match plaintext 1");

    //Private encrypt/public decrypt test...
    testAssert(rsa.privateEncrypt(reinterpret_cast<const uint8_t*>(test), static_cast<uint32_t>(strlen(test)), cipher, m::kRSAP_PKCS1), "couldn't encrypt data 2");
    sz = rsa.publicDecrypt(cipher, rsa.size(), reinterpret_cast<uint8_t*>(plain), m::kRSAP_PKCS1);
    testAssert(sz >= 0, "couldn't decypt data 2");
    testAssert(sz == strlen(test) && m::Mem::cmp(test, plain, sz) == 0, "chipertext does not match plaintext 2");

    delete[] cipher;
    delete[] plain;

    //Private key exportation
    {
        m::RSAPrivateKey priv(rsa.privateKey());
        m::RSAPrivateKey fromPQE(m::RSAPrivateKey::fromPQE(priv.p(), priv.q(), priv.e()));

        testAssert(g_privKeyCmp(priv, fromPQE), "could not recover from PQE");
    }

    //Signing test
    uint32_t mdLen = m::SHA::digestSize(m::kSHAV_Sha1);
    uint8_t *md = new uint8_t[mdLen];
    testAssert(m::SHA::quick(m::kSHAV_Sha1, reinterpret_cast<const uint8_t*>(test), static_cast<uint32_t>(strlen(test)), md, mdLen), "could not hash input string");

    uint32_t sigLen = rsa.size();
    uint8_t *signature = new uint8_t[sigLen];
    testAssert(rsa.sign(m::kRSASA_SHA1, md, mdLen, signature, &sigLen), "rsa signing failed");
    testAssert(rsa.verify(m::kRSASA_SHA1, signature, sigLen, md, mdLen), "rsa verify failed");

    delete[] signature;
    delete[] md;
    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;

    //Generate keys
    m::Random<> prng;
    const uint32_t keyLen = m::AES::keySize(m::kAESV_256CBC);
    const uint32_t ivLen = m::AES::ivSize(m::kAESV_256CBC);
    uint8_t *key = new uint8_t[keyLen + ivLen];
    uint8_t *iv = key + keyLen;
    prng.nextBytes(key, keyLen + ivLen);

    //Encrypt
    const char *test = "bush hid the facts";
    const uint8_t *utest = reinterpret_cast<const uint8_t*>(test);
    const uint32_t testLen = static_cast<uint32_t>(strlen(test));

    m::AES aes(m::kAESM_Encrypt, m::kAESV_256CBC, key, iv);
    testAssert(aes.isValid(), "couldn't init encryption");

    uint32_t totalEnc;
    uint8_t *enc = new uint8_t[testLen + aes.blockSize() - 1 + aes.blockSize()];

    uint32_t tmpSz = testLen + aes.blockSize() - 1;
    testAssert(aes.update(utest, testLen, enc, &tmpSz), "could not encrypt data");
    totalEnc = tmpSz;

    tmpSz = aes.blockSize();
    testAssert(aes.final(enc + totalEnc, &tmpSz), "could not finalize encryption");
    totalEnc += tmpSz;

    //Try a reset
    aes.reset(key, iv);
    testAssert(aes.isValid(), "m::AES::reset() failed!");

    tmpSz = testLen + aes.blockSize() - 1;
    testAssert(aes.update(utest, testLen, enc, &tmpSz), "could not re-encrypt data");
    totalEnc = tmpSz;

    tmpSz = aes.blockSize();
    testAssert(aes.final(enc + totalEnc, &tmpSz), "could not finalize re-encryption");
    totalEnc += tmpSz;

    //Decrypt
    testAssert(aes.init(m::kAESM_Decrypt, m::kAESV_256CBC, key, iv), "could not init decryption");

    uint32_t totalDec = 0;
    uint8_t *dec = new uint8_t[totalEnc + 2 * aes.blockSize()];

    tmpSz = totalEnc + aes.blockSize();
    testAssert(aes.update(enc, totalEnc, dec, &tmpSz), "could not decrypt data");
    totalDec += tmpSz;

    tmpSz = aes.blockSize();
    testAssert(aes.final(enc + totalDec, &tmpSz), "could not finalize decryption");
    totalDec += tmpSz;

    //Check & free
    testAssert(testLen == totalDec && memcmp(utest, dec, testLen) == 0, "decrypted data does not match input");
    delete[] dec;
    delete[] enc;

    //Try with streams
    m::ByteBuf encBB;

    {
        //Encrypt
        m::SSharedPtr<m::OutputStream> bbos(encBB.outputStream<m::RefCounter>());
        m::SSharedPtr<m::OutputStream> ecos(new m::AESOutputStream(m::kAESM_Encrypt, m::kAESV_256CBC, key, iv, bbos));
        m::SSharedPtr<m::InputStream> strr(new m::BufferInputStream(reinterpret_cast<const uint8_t*>(test), testLen));
        testAssert(m::IO::transfer(ecos.ptr(), strr.ptr(), 256), "could not encrypt data (streams)");

        ecos.staticCast<m::AESOutputStream>()->finalAndFlush(key, iv); //Don't forget to close the AESOutputStream in order to write the final block (if any).
    }

    {
        //Decrypt
        m::SSharedPtr<m::InputStream> bbis(encBB.inputStream<m::RefCounter>());
        m::SSharedPtr<m::InputStream> dcis(new m::AESInputStream(m::kAESM_Decrypt, m::kAESV_256CBC, key, iv, bbis));
        m::SSharedPtr<m::StringOStream> strw(new m::StringOStream);
        testAssert(m::IO::transfer(strw.ptr(), dcis.ptr(), 256), "could not decrypt data (streams)");

        testAssert(strw->data() != test, "decrypted data does not match input (streams)");
    }

    delete[] key;
    return true;
}

#endif

TEST
{
    volatile StackIntegrityChecker sic;

    {
        m::File file("test.ini");
        if(file.exists())
            testAssert(file.deleteFileHarder(), "couldn't delete test.ini");
    }

    {
        m::SimpleConfig cfg("test.ini");
        cfg["test"]["prop1"].setBoolOO(true);
        cfg["another"]["again"].setValue("this is a test ");
        cfg["test"]["prop2"].setDoubleValue(72.1236);
        testAssert(cfg.save(), "couldn't save config 1");
    }

    {
        m::SimpleConfig cfg("test.ini");
        testAssert(cfg.load() == m::kCLE_None, "could not load config 1");
        testAssert(cfg["another"]["again"].value() == "this is a test", "another.again should be \"this is a test\"");
        testAssert(cfg["test"]["prop1"].asBool(), "test.prop1 should be true");
        std::cout << '\t' << cfg["test"]["prop2"].asDouble() << std::endl;
        //testAssert(cfg["test"]["prop2"].asDouble() == 72.1236, "test.prop2 should be 72.1236"); //FUCKIN SUX
    }

    {
        m::File file("insert.ini");
        testAssert(file.exists(), "missing test file insert.ini");
    }

    {
        m::SimpleConfig cfg("insert.ini");
        testAssert(cfg.load() == m::kCLE_None, "could not load config 2");
        testAssert(cfg["first"]["prop1"].asInt() == 1, "first.prop1 should be 1");
        testAssert(cfg["first"]["prop2"].asInt() == 2, "first.prop1 should be 2");
        testAssert(cfg["first"]["prop3"].asInt() == 3, "first.prop1 should be 3");

        testAssert(cfg["second"]["prop1"].asInt() == 4, "second.prop1 should be 4");
        testAssert(cfg["second"]["prop2"].asInt() == 5, "second.prop2 should be 5");
        testAssert(cfg["second"]["prop3"].asInt() == 6, "second.prop3 should be 6");

        testAssert(cfg["third"]["prop1"].asInt() == 7, "third.prop1 should be 7");
        testAssert(cfg["third"]["prop2"].asInt() == 8, "third.prop2 should be 8");
        testAssert(cfg["third"]["prop3"].asInt() == 9, "third.prop3 should be 9");

        //Change something and save somewhere else
        cfg["first"]["prop2"].setValue("does this work?");
        cfg["test"]["wow"].setBoolTF(true);
        cfg["second"]["hello"].setIntValue(42);
        cfg.setFileName("insert_result.ini");
        testAssert(cfg.save(), "couldn't save config 2");
    }

    return true;
}

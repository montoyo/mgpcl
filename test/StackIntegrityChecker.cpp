#include "StackIntegrityChecker.h"

//If GCC sees this shift he'll just remove everything...
//So better turn optimization off, hopefully this won't make the class useless!
#pragma optimize("", off)

StackIntegrityChecker::StackIntegrityChecker()
{
	m_checksums[0] = 0x11223344;
	m_checksums[1] = 0x89ABCDEF;
	m_checksums[2] = 0xC0DEC470;
	m_checksums[3] = 0x66BADA55;
}

StackIntegrityChecker::~StackIntegrityChecker()
{
	if(m_checksums[0] != 0x11223344 || m_checksums[1] != 0x89ABCDEF || m_checksums[2] != 0xC0DEC470 || m_checksums[3] != 0x66BADA55) {
		std::cerr << "[!] STACK CORRUPTION DETECTED!!" << std::endl;
		std::abort();
	}
}

void StackIntegrityChecker::setChecksum(int i, int val)
{
	m_checksums[i] = val;
}

int StackIntegrityChecker::checksum(int i) const
{
	return m_checksums[i];
}

#pragma optimize("", on)

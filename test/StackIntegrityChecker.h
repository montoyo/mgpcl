#pragma once
#include <iostream>
#include <cstdlib>

//Not sure this works as I want...
class StackIntegrityChecker
{
public:
	StackIntegrityChecker();
	~StackIntegrityChecker();

	void setChecksum(int i, int val);
	int checksum(int i) const;

private:
	volatile int m_checksums[4];
};

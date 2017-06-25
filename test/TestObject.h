#pragma once
#include <iostream>

class TestObject
{
private:
	static int m_instances;
	static bool m_printDbg;

public:
	TestObject()
	{
		if(m_printDbg)
			std::cout << "\t[+] Create TestObject #" << m_instances << std::endl;

		m_instances++;
		m_val = 0xDEADBEEF;
	}

	TestObject(int val)
	{
		if(m_printDbg)
			std::cout << "\t[+] Create TestObject #" << m_instances << std::endl;

		m_instances++;
		m_val = val;
	}

	TestObject(const TestObject &obj)
	{
		if(m_printDbg)
			std::cout << "\t[+] Copy TestObject #" << m_instances << std::endl;

		m_instances++;
		m_val = obj.m_val;
	}

	TestObject(TestObject &&obj)
	{
		if(m_printDbg)
			std::cout << "\t[+] Move TestObject #" << m_instances << std::endl;

		m_instances++;
		m_val = obj.m_val;
		obj.m_val = ~m_val;
	}

	~TestObject()
	{
		if(m_printDbg || m_instances <= 0)
			std::cout << "\t[-] Deleted TestObject #" << --m_instances << std::endl;
		else
			m_instances--;
	}

	TestObject &operator = (const TestObject &src)
	{
		m_val = src.m_val;
		return *this;
	}

	TestObject &operator = (TestObject &&src)
	{
		m_val = src.m_val;
		src.m_val = ~m_val;
		return *this;
	}

	int value() const
	{
		return m_val;
	}

	void setValue(int val)
	{
		m_val = val;
	}

	bool valueMatches(int val) const
	{
		return m_val == val;
	}

	bool valueMatches() const
	{
		return m_val == 0xDEADBEEF;
	}

	static int instances()
	{
		return m_instances;
	}

	static void setPrintDebug()
	{
		m_printDbg = true;
	}

private:
	int m_val;
};
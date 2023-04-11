#pragma once
#include "CoreCommon.h"
#include <fstream>

class DelayWriteManager
{
public:
	DelayWriteManager()
		: m_lock()
		, m_toAddData()
		, m_writeFile(m_filePath.data())
	{
	}

	~DelayWriteManager()
	{
		m_writeFile.close();
	}

public:
	mutex m_lock;
	vector<string> m_toAddData;

	string m_filePath = "test.txt";
	ofstream m_writeFile;
	
	void addData(string data)
	{
		lock_guard<mutex> lock(m_lock);
		m_toAddData.push_back(data);
	}

	void WriteFile()
	{
		lock_guard<mutex> lock(m_lock);
		for (string& str : m_toAddData)
		{
			if (m_writeFile.is_open())
			{
				m_writeFile << str << "\n";
			}
		}
		m_toAddData.clear();
	}

};

extern DelayWriteManager gDelayWriteMgr;
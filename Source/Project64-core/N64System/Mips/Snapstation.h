#pragma once

#include <Project64-core/Logging.h>

class Snapstation :
	public CLogging
{
public:
	Snapstation();
	void ReadFrom(uint16_t address, uint8_t * data);
    void WriteTo(int32_t Control, uint16_t address, uint8_t * data);

private:
	Snapstation(const Snapstation&);
	Snapstation& operator=(const Snapstation&);

	void ResetState();

	static void printBuffer(uint8_t *data);

	bool m_resetFlag;
	unsigned int m_displayHoldTime;
	uint8_t m_c000_state = 0;
};

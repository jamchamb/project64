#include "stdafx.h"
#include "Snapstation.h"

#include <Project64-core/N64System/SystemGlobals.h>
#include <Project64-core/N64System/Mips/SystemEvents.h>
#include <Project64-core/N64System/N64System.h>
#include <Project64-core/Plugins/Plugin.h>
#include <Project64-core/Plugins/ControllerPlugin.h>
#include <Project64-core/Logging.h>

Snapstation::Snapstation() {
	ResetState();
}

void Snapstation::ReadFrom(uint16_t address, uint8_t * data)
{
	bool showData = true;

	LogMessage("Snapstation::ReadFrom 0x%04x", address);

	if (address == 0x8000) {
		// Device ID 0x85
		memset(data, 0x85, 32);
	}
	else if (address == 0xC000) {
		// Display states: echo back or return 8 to busy loop
		if (m_displayHoldTime > 0) {
			memset(data, 0, 32);
			data[31] = 0x08;
			m_displayHoldTime -= 1;
		}
		else if (m_resetFlag) {
			memset(data, 0, 32);
			data[31] = 0x08;

			m_resetFlag = false;
			g_BaseSystem->ExternalEvent(SysEvent_ResetCPU_Soft);
		}
		else {
			memset(data, 0, 32);
			data[31] = m_c000_state;

			if (m_c000_state == 4) {
				// Finished boot display, reset the state for next use
				ResetState();
			}
		}
	}
	else {
		LogMessage("Unhandled address or state");
		showData = false;
	}

	if (showData) {
		printBuffer(data);
		LogMessage("");
	}
}

void Snapstation::WriteTo(int32_t Control, uint16_t address, uint8_t * data)
{
	LogMessage("Snapstation::WriteTo 0x%04x", address);
	printBuffer(data);
	LogMessage("");

	// 8000 will send FEFEFEFE and then 85858585

	if (address == 0xC000) {
		m_c000_state = data[31];

		if (data[31] < 4) {
			m_resetFlag = false;
		}

		// 0x01: starting bootup print display
		// 0x02: displaying an image (16 times)
		// 0x04: done
		if (data[31] == 0x02) {
			m_displayHoldTime = 30;
		}

		// 0xCC, 0x33: starting to save photos selected for pint
		// 0x5A: saved & ready to reboot into print
		else if (data[31] == 0x5a) {
			m_resetFlag = true;
		}
	}
}

void Snapstation::ResetState()
{
	m_displayHoldTime = 0;
	m_resetFlag = false;
	m_c000_state = 0;
}

void Snapstation::printBuffer(uint8_t * data)
{
	// 2*4 spaces + (2+1) * 32 byte & space + 1 null = 105
	char hexbuf[105];
	char *hexbufPtr = hexbuf;

	for (size_t i = 0; i < 32; i++) {
		// Leading indent
		if (i == 0 || i == 16) {
			snprintf(hexbufPtr, 5, "    ");
			hexbufPtr += 4;
		}

		snprintf(hexbufPtr, 4, "%02x ", data[i]);
		hexbufPtr += 3;
	}

	hexbuf[51] = '\n';
	hexbuf[sizeof(hexbuf)-1] = '\0';

	LogMessage(hexbuf);
}

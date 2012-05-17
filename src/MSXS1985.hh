// $Id$

/*
 * This class implements the
 *   backup RAM
 *   bitmap function
 * of the S1985 MSX-engine
 *
 *  TODO explanation
 */

#ifndef S1985_HH
#define S1985_HH

#include "MSXDevice.hh"
#include "MSXSwitchedDevice.hh"
#include <memory>

namespace openmsx {

class Ram;

class MSXS1985 : public MSXDevice, public MSXSwitchedDevice
{
public:
	MSXS1985(MSXMotherBoard& motherBoard, const DeviceConfig& config);
	virtual ~MSXS1985();

	// MSXDevice
	virtual void powerUp(EmuTime::param time);
	virtual void reset(EmuTime::param time);

	// MSXSwitchedDevice
	virtual byte readSwitchedIO(word port, EmuTime::param time);
	virtual byte peekSwitchedIO(word port, EmuTime::param time) const;
	virtual void writeSwitchedIO(word port, byte value, EmuTime::param time);

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	const std::auto_ptr<Ram> ram;
	nibble address;
	byte color1;
	byte color2;
	byte pattern;
};

} // namespace openmsx

#endif

// $Id$

#ifndef ROMHOLYQURAN_HH
#define ROMHOLYQURAN_HH

#include "RomBlocks.hh"

namespace openmsx {

class RomHolyQuran : public Rom8kBBlocks
{
public:
	RomHolyQuran(MSXMotherBoard& motherBoard, const DeviceConfig& config,
	             std::auto_ptr<Rom> rom);

	virtual void reset(EmuTime::param time);
	virtual void writeMem(word address, byte value, EmuTime::param time);
	virtual byte* getWriteCacheLine(word address) const;
};

} // namespace openmsx

#endif

// $Id$

// Korean 80-in-1 cartridge
//
// Information obtained by studying MESS sources:
//   0x4000 : 0x4000-0x5FFF
//   0x4001 : 0x6000-0x7FFF
//   0x4002 : 0x8000-0x9FFF
//   0x4003 : 0xA000-0xBFFF

#include "RomKorean80in1.hh"
#include "CacheLine.hh"
#include "Rom.hh"

namespace openmsx {

RomKorean80in1::RomKorean80in1(
		MSXMotherBoard& motherBoard, const XMLElement& config,
		const EmuTime& time, std::auto_ptr<Rom> rom)
	: Rom8kBBlocks(motherBoard, config, rom)
{
	reset(time);
}

void RomKorean80in1::reset(const EmuTime& /*time*/)
{
	setBank(0, unmappedRead);
	setBank(1, unmappedRead);
	for (int i = 2; i < 6; i++) {
		setRom(i, i - 2);
	}
	setBank(6, unmappedRead);
	setBank(7, unmappedRead);
}

void RomKorean80in1::writeMem(word address, byte value, const EmuTime& /*time*/)
{
	if ((0x4000 <= address) && (address < 0x4004)) {
		setRom(2 + (address - 0x4000), value);
	}
}

byte* RomKorean80in1::getWriteCacheLine(word address) const
{
	if (address == (0x4000 & CacheLine::HIGH)) {
		return NULL;
	} else {
		return unmappedWrite;
	}
}

} // namespace openmsx

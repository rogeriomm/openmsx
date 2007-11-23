// $Id$

#include "MSXYamahaSFG.hh"
#include "YM2151.hh"
#include "YM2148.hh"
#include "Rom.hh"

namespace openmsx {

MSXYamahaSFG::MSXYamahaSFG(MSXMotherBoard& motherBoard, const XMLElement& config,
                   const EmuTime& time)
	: MSXDevice(motherBoard, config)
	, rom(new Rom(motherBoard, getName() + " ROM", "rom", config))
	, ym2151(new YM2151(motherBoard, getName(),
	                    "Yamaha SFG-01/05", config, time))
	, ym2148(new YM2148())
{
	reset(time);
}

MSXYamahaSFG::~MSXYamahaSFG()
{
}

void MSXYamahaSFG::reset(const EmuTime& time)
{
	ym2151->reset(time);
	ym2148->reset();
	registerLatch = 0; // TODO check
	irqVector = 255; // TODO check
}

void MSXYamahaSFG::writeMem(word address, byte value, const EmuTime& time)
{
	if (address < 0x3FF0 || address >= 0x3FF8) {
		return;
	}

	switch (address & 0x3FFF) {
	case 0x3FF0:
		writeRegisterPort(value, time);
		break;
	case 0x3FF1:
		writeDataPort(value, time);
		break;
	case 0x3FF2:
		// TODO: keyboardLatch = value;
		//std::cerr << "TODO: keyboardLatch = " << (int)value << std::endl;
		break;
	case 0x3FF3:
		ym2148->setVector(value);
		break;
	case 0x3FF4:
		// IRQ vector for YM2151 (+ default vector ???)
		irqVector = value;
		break;
	case 0x3FF5:
		ym2148->writeData(value);
		break;
	case 0x3FF6:
		ym2148->writeCommand(value);
		break;
	}
}

byte MSXYamahaSFG::readIRQVector()
{
	return irqVector;
}

void MSXYamahaSFG::writeRegisterPort(byte value, const EmuTime& /*time*/)
{
	registerLatch = value;
}

void MSXYamahaSFG::writeDataPort(byte value, const EmuTime& time)
{
	//PRT_DEBUG("YM2151: reg "<<(int)registerLatch<<" val "<<(int)value);
	ym2151->writeReg(registerLatch, value, time);
}

byte MSXYamahaSFG::readMem(word address, const EmuTime& /*time*/)
{
	if (address < 0x3FF0 || address >= 0x3FF8) {
		// size can also be 16kB for SFG-01 or 32kB for SFG-05
		return (*rom)[address & (rom->getSize() - 1)];
	}

	switch (address & 0x3FFF) {
	case 0x3FF0:
		return ym2151->readStatus();
	case 0x3FF1:
		return ym2151->readStatus();
	case 0x3FF2:
		// TODO: return getKbdStatus();
		break;
	case 0x3FF5:
		// For now disabled, as it breaks working functionality (YRM-104)
		// return ym2148->readData();
		break;
	case 0x3FF6:
		// For now disabled, as it breaks working functionality (YRM-104)
		//return ym2148->readStatus();
		break;
	}
	return 0xFF;
}

} // namespace openmsx

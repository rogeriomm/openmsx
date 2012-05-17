// $Id$

#include "NationalFDC.hh"
#include "CacheLine.hh"
#include "DriveMultiplexer.hh"
#include "WD2793.hh"
#include "serialize.hh"

namespace openmsx {

NationalFDC::NationalFDC(MSXMotherBoard& motherBoard, const DeviceConfig& config)
	: WD2793BasedFDC(motherBoard, config)
{
}

byte NationalFDC::readMem(word address, EmuTime::param time)
{
	byte value;
	switch (address & 0x3FC7) {
	case 0x3F80:
		value = controller->getStatusReg(time);
		break;
	case 0x3F81:
		value = controller->getTrackReg(time);
		break;
	case 0x3F82:
		value = controller->getSectorReg(time);
		break;
	case 0x3F83:
		value = controller->getDataReg(time);
		break;
	case 0x3F84:
	case 0x3F85:
	case 0x3F86:
	case 0x3F87:
		value = 0x7F;
		if (controller->getIRQ(time))  value |=  0x80;
		if (controller->getDTRQ(time)) value &= ~0x40;
		break;
	default:
		value = NationalFDC::peekMem(address, time);
		break;
	}
	return value;
}

byte NationalFDC::peekMem(word address, EmuTime::param time) const
{
	byte value;
	// According to atarulum:
	//  7FBC        is mirrored in 7FBC - 7FBF
	//  7FB8 - 7FBF is mirrored in 7F80 - 7FBF
	switch (address & 0x3FC7) {
	case 0x3F80:
		value = controller->peekStatusReg(time);
		break;
	case 0x3F81:
		value = controller->peekTrackReg(time);
		break;
	case 0x3F82:
		value = controller->peekSectorReg(time);
		break;
	case 0x3F83:
		value = controller->peekDataReg(time);
		break;
	case 0x3F84:
	case 0x3F85:
	case 0x3F86:
	case 0x3F87:
		// Drive control IRQ and DRQ lines are not connected to Z80 interrupt request
		// bit 7: intrq
		// bit 6: !dtrq
		// other bits read 1
		value = 0x7F;
		if (controller->peekIRQ(time))  value |=  0x80;
		if (controller->peekDTRQ(time)) value &= ~0x40;
		break;
	default:
		if (address < 0x8000) {
			// ROM only visible in 0x0000-0x7FFF
			value = MSXFDC::peekMem(address, time);
		} else {
			value = 255;
		}
		break;
	}
	//PRT_DEBUG("NationalFDC read 0x" << hex << (int)address << " 0x" << (int)value << dec);
	return value;
}

const byte* NationalFDC::getReadCacheLine(word start) const
{
	if ((start & 0x3FC0 & CacheLine::HIGH) == (0x3F80 & CacheLine::HIGH)) {
		// FDC at 0x7FB8-0x7FBC (also mirrored)
		return NULL;
	} else if (start < 0x8000) {
		// ROM at 0x0000-0x7FFF
		return MSXFDC::getReadCacheLine(start);
	} else {
		return unmappedRead;
	}
}

void NationalFDC::writeMem(word address, byte value, EmuTime::param time)
{
	//PRT_DEBUG("NationalFDC write 0x" << hex << (int)address << " 0x" << (int)value << dec);
	switch (address & 0x3FC7) {
	case 0x3F80:
		controller->setCommandReg(value, time);
		break;
	case 0x3F81:
		controller->setTrackReg(value, time);
		break;
	case 0x3F82:
		controller->setSectorReg(value, time);
		break;
	case 0x3F83:
		controller->setDataReg(value, time);
		break;
	case 0x3F84:
	case 0x3F85:
	case 0x3F86:
	case 0x3F87:
		// bit 0 -> select drive 0
		// bit 1 -> select drive 1
		// bit 2 -> side select
		// bit 3 -> motor on
		DriveMultiplexer::DriveNum drive;
		switch (value & 3) {
			case 1:
				drive = DriveMultiplexer::DRIVE_A;
				break;
			case 2:
				drive = DriveMultiplexer::DRIVE_B;
				break;
			default:
				drive = DriveMultiplexer::NO_DRIVE;
		}
		multiplexer->selectDrive(drive, time);
		multiplexer->setSide((value & 0x04) != 0);
		multiplexer->setMotor((value & 0x08) != 0, time);
		break;
	}
}

byte* NationalFDC::getWriteCacheLine(word address) const
{
	if ((address & 0x3FC0) == (0x3F80 & CacheLine::HIGH)) {
		// FDC at 0x7FB8-0x7FBC (also mirrored)
		return NULL;
	} else {
		return unmappedWrite;
	}
}


template<typename Archive>
void NationalFDC::serialize(Archive& ar, unsigned /*version*/)
{
	ar.template serializeBase<WD2793BasedFDC>(*this);
}
INSTANTIATE_SERIALIZE_METHODS(NationalFDC);
REGISTER_MSXDEVICE(NationalFDC, "NationalFDC");

} // namespace openmsx

// $Id$

/*
 * Password Cartridge
 *
 * Access: write 0x00 to I/O port 0x7e
 *         provide 0xaa, <char1>, <char2>, continuous 0xff sequence reading I/O port 0x7e
 *         write any non-zero to I/O port 0x7e
 *	   provide 0xff at all time reading I/O port 0x7e
 */

#include "PasswordCart.hh"
#include "XMLElement.hh"
#include <algorithm>

namespace openmsx {

PasswordCart::PasswordCart(MSXMotherBoard& motherBoard,
                           const XMLElement& config, const EmuTime& time)
	: MSXDevice(motherBoard, config)
{
	password = config.getChildDataAsInt("password", 0);
	reset(time);
}

void PasswordCart::reset(const EmuTime& /*time*/)
{
	pointer = 3;
}

void PasswordCart::writeIO(word /*port*/, byte value, const EmuTime& /*time*/)
{
	pointer = (value == 0) ? 0 : 3;
}

byte PasswordCart::readIO(word port, const EmuTime& time)
{
	byte result = peekIO(port, time);
	pointer = std::min(3, pointer + 1);
	return result;
}

byte PasswordCart::peekIO(word /*port*/, const EmuTime& /*time*/) const
{
	switch (pointer) {
	case 0:
		return 0xaa;
	case 1:
		return password >> 8;
	case 2:
		return password & 0xff;
	default:
		return 0xff;
	}
}

} // namespace

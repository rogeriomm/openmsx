// $Id$

#include "Inputs.hh"
#include "MSXPPI.hh"
#include "Leds.hh"
#include <assert.h>


// MSXDevice

MSXPPI::MSXPPI()
{
	PRT_DEBUG("Creating an MSXPPI object");
	keyboardGhosting = true;
	i8255 = new I8255(*this);
}

MSXPPI::~MSXPPI()
{
	PRT_DEBUG("Destroying an MSXPPI object");
}

MSXPPI* MSXPPI::instance(void)
{
	if (oneInstance == NULL ) {
		oneInstance = new MSXPPI();
	}
	return oneInstance;
}
MSXPPI *MSXPPI::oneInstance = NULL;


void MSXPPI::init()
{
	MSXDevice::init();
	// Register I/O ports A8..AB for reading
	MSXMotherBoard::instance()->register_IO_In(0xA8,this);
	MSXMotherBoard::instance()->register_IO_In(0xA9,this);
	MSXMotherBoard::instance()->register_IO_In(0xAA,this);
	MSXMotherBoard::instance()->register_IO_In(0xAB,this);
	// Register I/O ports A8..AB for writing
	MSXMotherBoard::instance()->register_IO_Out(0xA8,this);
	MSXMotherBoard::instance()->register_IO_Out(0xA9,this); 
	MSXMotherBoard::instance()->register_IO_Out(0xAA,this);
	MSXMotherBoard::instance()->register_IO_Out(0xAB,this);
}

void MSXPPI::reset()
{
	MSXDevice::reset();
	i8255->reset();
}

byte MSXPPI::readIO(byte port, Emutime &time)
{
	//Note Emutime argument is ignored, I think that's ok
	switch (port) {
	case 0xA8:
		return i8255->readPortA();
	case 0xA9:
		return i8255->readPortB();
	case 0xAA:
		return i8255->readPortC();
	case 0xAB:
		return i8255->readControlPort();
	default:
		assert (false); // code should never be reached
		return 255;	// avoid warning
	}
}

void MSXPPI::writeIO(byte port, byte value, Emutime &time)
{
	//Note Emutime argument is ignored, I think that's ok
	switch (port) {
	case 0xA8:
		i8255->writePortA(value);
		break;
	case 0xA9:
		//BOGUS read-only register in case of MSX
		i8255->writePortB(value);
		break;
	case 0xAA:
		//TODO use these bits
		//4    CASON  Cassette motor relay        (0=On, 1=Off)
		//5    CASW   Cassette audio out          (Pulse)
		//6    CAPS   CAPS-LOCK lamp              (0=On, 1=Off)
		//7    SOUND  Keyboard klick bit          (Pulse)
		i8255->writePortC(value);
		break;
	case 0xAB:
		//Theoretically these registers can be used as input or output 
		//ports. However, in the MSX, PPI Port A and C are always used as output, 
		//and PPI Port B as input. The BIOS initializes it like that by writing 
		//82h to Port ABh on startup. Afterwards it makes no sense to change this 
		//setting, and thus we could simply discard any write :-)
		//but wouter wouldn't let us ;-)
		i8255->writeControlPort(value);
		break;
	default:
		assert (false); // code should never be reached
	}
}


// I8255Interface

byte MSXPPI::readA() {
	// port A is normally an output on MSX, reading from an output port
	// is handled internally in the 8255
	return 255;	//TODO check this
}
void MSXPPI::writeA(byte value) {
	MSXMotherBoard::instance()->set_A8_Register(value);
}

byte MSXPPI::readB() {
	const byte* src = Inputs::instance()->getKeys(); //reading the keyboard events into the matrix
	byte* dst = MSXKeyMatrix;
	for (int i=0; i<Inputs::NR_KEYROWS; i++) {
		*dst++ = *src++;
	}
	if (keyboardGhosting) {
		keyGhosting();
	}
	return MSXKeyMatrix[selectedRow];
}
void MSXPPI::writeB(byte value) {
	// probably nothing happens on a real MSX
}

nibble MSXPPI::readC1() {
	return 15;	// TODO check this
}
nibble MSXPPI::readC0() {
	return 15;	// TODO check this
}
void MSXPPI::writeC1(nibble value) {
	//TODO use these bits
	//  4    CASON  Cassette motor relay        (0=On, 1=Off)
	//  5    CASW   Cassette audio out          (Pulse)
	//  7    SOUND  Keyboard klick bit          (Pulse)
	Leds::LEDCommand caps = (value&4) ? Leds::CAPS_OFF : Leds::CAPS_ON;
	Leds::instance()->setLed(caps);
}
void MSXPPI::writeC0(nibble value) {
	selectedRow = value;
}


void MSXPPI::keyGhosting(void)
{
// This routine enables keyghosting as seen on a real MSX
//
// If on a real MSX in the keyboardmatrix the
// real buttons are pressed as in the left matrix
//           then the matrix to the
// 10111111  right will be read by   10110101
// 11110101  because of the simple   10110101
// 10111101  electrical connections  10110101
//           that are established  by
// the closed switches 
	bool changed_something = false;
	do {
		for (int i=0; i<=Inputs::NR_KEYROWS; i++) {
			for (int j=0; j<=Inputs::NR_KEYROWS; j++) {
				if ((MSXKeyMatrix[i]|MSXKeyMatrix[j]) != 255) {
					int rowanded=MSXKeyMatrix[i]&MSXKeyMatrix[j];
					if (rowanded != MSXKeyMatrix[i]) {
						MSXKeyMatrix[i]=rowanded;
						changed_something = true;
					}
				}
			}
		}
	} while (changed_something);
}


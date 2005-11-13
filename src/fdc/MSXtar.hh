// $Id$

// This code implements the functionality of my older msxtar program
// that could manipulate files and directories on dsk and ide-hd images
// Integrating it is seen as temporary bypassing of the need for a
// DirAsDisk2 that supports subdirs, partitions etc. since this class will
// of those functionalities although not on a dynamic base

#ifndef MSXTAR_HH
#define MSXTAR_HH

#include "openmsx.hh"
#include <string>
#include <vector>

namespace openmsx {

class SectorAccessibleDisk;

class MSXtar
{
public:
	explicit MSXtar(SectorAccessibleDisk& disk);
	~MSXtar();

	void format();
	void format(unsigned partitionsectorsize);
	bool hasPartitionTable();
	bool hasPartition(int partition);
	bool usePartition(int partition);

	void createDiskFile(std::vector<unsigned> sizes);
	void chdir(const std::string& newRootDir);
	void mkdir(const std::string& newRootDir);
	std::string dir();

	// temporary way to test import MSXtar functionality
	std::string addFile(const std::string& Filename);
	std::string addDir(const std::string& rootDirName);
	void getDir(const std::string& rootDirName);

private:
	struct MSXBootSector {
		byte jumpcode[3];	// 0xE5 to bootprogram
		byte name[8];
		byte bpsector[2];	// bytes per sector (always 512)
		byte spcluster[1];	// sectors per cluster (always 2)
		byte reservedsectors[2];// amount of non-data sectors (ex bootsector)
		byte nrfats[1];		// number of fats
		byte direntries[2];	// max number of files in root directory
		byte nrsectors[2];	// number of sectors on this disk
		byte descriptor[1];	// media descriptor
		byte sectorsfat[2];	// sectors per FAT
		byte sectorstrack[2];	// sectors per track
		byte nrsides[2];	// number of sides
		byte hiddensectors[2];	// not used
		byte bootprogram[512-30];// actual bootprogram
	};

	struct MSXDirEntry {
		byte filename[8];
		byte ext[3];
		byte attrib;
		byte reserved[10];	// unused
		byte time[2];
		byte date[2];
		byte startcluster[2];
		byte size[4];
	};

	//Modified struct taken over from Linux' fdisk.h
	struct Partition {
		byte boot_ind;         // 0x80 - active
		byte head;             // starting head
		byte sector;           // starting sector
		byte cyl;              // starting cylinder
		byte sys_ind;          // What partition type
		byte end_head;         // end head
		byte end_sector;       // end sector
		byte end_cyl;          // end cylinder
		byte start4[4];        // starting sector counting from 0
		byte size4[4];         // nr of sectors in partition
	};

	struct DirEntry {
		int sector;
		int index;
	};

	int partitionNbSectors;
	int nbSectors;
	int maxCluster;
	int sectorsPerCluster;
	int sectorsPerFat;
	int nbFats;
	int nbSides;
	int nbSectorsPerCluster;
	int nbRootDirSectors;
	int rootDirStart; // first sector from the root directory
	int rootDirEnd;   // last sector from the root directory
	int MSXchrootSector;
	int partitionOffset;
	const byte* defaultBootBlock;
	SectorAccessibleDisk& disk;
	std::vector<byte> fatBuffer;

	int clusterToSector(int cluster);
	void setBootSector(byte* buf, unsigned nbsectors);
	word sectorToCluster(int sector);
	void parseBootSector(const byte* buf);
	void parseBootSectorFAT(const byte* buf);
	word readFAT(word clnr) const;
	void writeFAT(word clnr, word val);
	word findFirstFreeCluster(void);
	int findUsableIndexInSector(int sector);
	int getNextSector(int sector);
	int appendClusterToSubdir(int sector);
	DirEntry addEntryToDir(int sector);
	std::string makeSimpleMSXFileName(const std::string& fullfilename);
	int addMSXSubdir(const std::string& msxName, int t, int d, int sector);
	void alterFileInDSK(MSXDirEntry& msxdirentry, const std::string& hostName);
	int addSubdirtoDSK(const std::string& hostName,
	                   const std::string& msxName, int sector);
	DirEntry findEntryInDir(const std::string& name, int sector,
	                        byte* sectorbuf);
	std::string addFiletoDSK(const std::string& hostName,
	                         const std::string& msxName, int sector);
	std::string recurseDirFill(const std::string& dirName, int sector);
	std::string condensName(MSXDirEntry& direntry);
	void changeTime(std::string resultFile, MSXDirEntry& direntry);
	void fileExtract(std::string resultFile, MSXDirEntry& direntry);
	void recurseDirExtract(const std::string& dirName, int sector);
	void chroot(const std::string& newRootDir, bool createDir);

	bool isPartitionTableSector(byte* buf);
};

} // namespace openmsx

#endif

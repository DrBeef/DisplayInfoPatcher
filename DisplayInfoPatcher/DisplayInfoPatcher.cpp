// DisplayInfoPatcher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// Include the OculusVR SDK
#include "OVR.h"
#include "OVR_SensorImpl.h"
#include "OVR_Alg.h"

#include <iostream>
#include <fstream>
#include <string>


using namespace OVR;

typedef std::basic_ofstream<unsigned char, std::char_traits<unsigned char> > uofstream;
typedef std::basic_ifstream<unsigned char, std::char_traits<unsigned char> > uifstream;

System *g_ovrSystem = NULL;

HMDDevice* Initialization(DeviceManager** ppManager)
{
	g_ovrSystem = new System;

	DeviceManager* pManager = DeviceManager::Create();
	if (!pManager)
	{
		//error
		std::cout << "Error: Could not create DeviceManager" << std::endl;
		return NULL;
	}

	HMDDevice* pHMD = (pManager)->EnumerateDevices<HMDDevice>().CreateDevice();
	if (pHMD)
	{
		HMDInfo info;
		pHMD->GetDeviceInfo(&info);
		char* displayName = info.DisplayDeviceName;

		//Return all initialised
		*ppManager = pManager;
		return pHMD;
	}
	else
	{
		//error
		std::cout << "Error: Could not locate Oculus Rift" << std::endl;
		return NULL;
	}
}


bool OutputDisplayInfoDetails(std::string diName, SensorDisplayInfoImpl &displayInfo)
{
	if (displayInfo.HResolution == 1280 &&
		displayInfo.VResolution == 800 &&
		Alg::DecodeUInt32(displayInfo.Buffer+8) == 149760 &&
		Alg::DecodeUInt32(displayInfo.Buffer+12) == 93600 &&
		Alg::DecodeUInt32(displayInfo.Buffer+16) == 46800 )
	{
		std::cout << diName << " DisplayInfo Configuration: DK1" << std::endl;
	}
	else if (displayInfo.HResolution == 1920 &&
		displayInfo.VResolution == 1080 &&
		Alg::DecodeUInt32(displayInfo.Buffer+8) == 129600 &&
		Alg::DecodeUInt32(displayInfo.Buffer+12) == 72900 &&
		Alg::DecodeUInt32(displayInfo.Buffer+16) == 36450 )
	{
		std::cout << diName << " DisplayInfo Configuration: RiftUp!" << std::endl;
	}
	else if (displayInfo.HResolution == 1920 &&
		displayInfo.VResolution == 1080)
	{
		std::cout << diName << " DisplayInfo Configuration: Other Full HD" << std::endl;
	}
	else
	{
		std::cout << diName << " DisplayInfo Configuration: **Cannot be identified**" << std::endl;
 		std::cout << std::endl;
		return false;
	}

	std::cout << std::endl;
	return true;
}


bool ValidateInputFile(UByte *Buffer)
{
	//Validate first byte
	if (Buffer[0] != 9)
	{
		std::cerr << "Error: first byte of input file must be 0x09" << std::endl;
		std::cerr << "Aborting..." << std::endl;
		return false;
	}

	return true;
}


int main(int argc, char* argv[])
{
	std::string writeFilename;
	std::string backupFilename;

    std::cout << "  Tracker DK DisplayInfo Backup/Writing tool" << std::endl; 
    std::cout << "  ==========================================" << std::endl; 
    std::cout << std::endl; 
    std::cout << "  NOTE: I accept no responsibility for any damage you may do to your HMD using this tool" << std::endl; 
    std::cout << "  USE WITH CAUTION, YOU DO SO ENTIRELY AT YOUR OWN RISK" << std::endl; 
    std::cout << std::endl; 
    std::cout << "  *** TRIPLE CHECK THE CONTENTS OF YOUR BIN FILE BEFORE APPLYING ***" << std::endl; 
    std::cout << std::endl; 
    std::cout << std::endl; 

    if (argc <= 2)
	{
        std::cout << "Usage:" << std::endl; 
        std::cout << "  DisplayInfoPatcher [-backup <backupfilename>] [-write <writefilename>]" << std::endl; 
        std::cout << std::endl; 
        std::cout << "Parameters:" << std::endl; 
        std::cout << "  -backup  :  Creates a backup of the current HMD DisplayInfo buffer to a binary file with name provided" << std::endl; 
        std::cout << "  -write  :  Will write the supplied file to the HMD DisplayInfo buffer" << std::endl; 
        std::cout << std::endl;
        std::cout << "Press RETURN to exit" << std::endl; 
        std::cin.get();
        exit(0);
    } 
	else 
	{
		int pos = std::string(argv[0]).find_last_of('\\');
		std::string path = std::string(argv[0]).substr(0, pos+1);

        for (int i = 1; i < argc; i++)
		{
            if (i + 1 != argc)
			{
                if (std::string(argv[i]) == "-write")
				{
					writeFilename = argv[i + 1];
					if (writeFilename.find_last_of('\\') == std::string::npos)
						writeFilename = path + writeFilename;
                } 
				else if (std::string(argv[i]) == "-backup") 
				{
 					backupFilename = argv[i + 1];
					if (backupFilename.find_last_of('\\') == std::string::npos)
						backupFilename = path + backupFilename;
               } 
				else 
				{
                    std::cerr << "Not enough or invalid arguments, please try again." << std::endl;
                    exit(0);
				}
			}
        }
	}

	DeviceManager* pManager = NULL;
	HMDDevice* pHMD = Initialization(&pManager);

	if (pHMD)
	{
		if (!pHMD->IsConnected())
		{
			std::cerr << "Error: Oculus Rift is not connected" << std::endl;
			return 0;
		}

		//get the sensor
		SensorDevice *pSensorDevice = pHMD->GetSensor();

		if (pSensorDevice)
		{
			SensorDisplayInfoImpl originalDisplayInfo;
			if (pSensorDevice->GetFeatureReport(originalDisplayInfo.Buffer, originalDisplayInfo.PacketSize))
			{
				originalDisplayInfo.Unpack();
				OutputDisplayInfoDetails("Current Tracker", originalDisplayInfo);
			}
			else
			{
				std::cerr << "Error: Could not retrieve current DisplayInfo from the rift" << std::endl;
				pHMD->Disconnect(pSensorDevice);
				pHMD->Release();
				pManager->Release();
				return 0;
			}

			bool abort = false;

			if (backupFilename.length())
			{
 				std::cout << "Backing up DisplayInfo to file: " << backupFilename << std::endl;
 				std::cout << std::endl;
				uofstream outputFile;
				outputFile.open(backupFilename, std::ios_base::binary);
				if(outputFile.fail())
				{
					std::cerr << "Error: Opening file: " + backupFilename << std::endl;
					std::cerr << "Aborting..." << std::endl;
					abort = true;
				}
				else
				{
					outputFile.write(originalDisplayInfo.Buffer, originalDisplayInfo.PacketSize);
					if(outputFile.fail())
					{
						std::cerr << "Error: Writing to file: " + backupFilename << std::endl;
						std::cerr << "Aborting..." << std::endl;
						abort = true;
					}
					outputFile.close();
				}
			}

			//Don't try to write if we couldn't backup
			if (writeFilename.length() && !abort)
			{
				std::cout << "Writing DisplayInfo from file: " << writeFilename << std::endl;
				std::cout << std::endl;
				uifstream inputFile;
				inputFile.open(writeFilename, std::ios_base::binary);
				if(inputFile.fail())
				{
					std::cerr << "Error: Opening file: " + writeFilename << std::endl;
					std::cerr << "Aborting..." << std::endl;
				}
				else
				{
					//Read the potentially new display info from the file
					SensorDisplayInfoImpl newDisplayInfo;
					inputFile.read(newDisplayInfo.Buffer, newDisplayInfo.PacketSize);
					if(inputFile.fail())
					{
						std::cerr << "Error: Reading from file: " + writeFilename << std::endl;
						std::cerr << "Aborting..." << std::endl;
					}
					else
					{
						//We should now be at the end of the file - check by trying to read something else
						UByte end;
						inputFile >> end;
						if (inputFile.eof())
						{
							//Validation of input file - This will exit the app if it finds anything wrong
							if (!ValidateInputFile(newDisplayInfo.Buffer))
							{
								//We'll already have shown an appropriate error message
								pHMD->Disconnect(pSensorDevice);
								pHMD->Release();
								pManager->Release();
								exit(0);
							}

							//Unpack what we just read from the file
							newDisplayInfo.Unpack();
							if (!OutputDisplayInfoDetails("Input file", newDisplayInfo))
							{
								//Custom HMDs DisplayInfo will trigger this
 								std::cout << "Do you wish to flash this file anyway? <Y/N>: ";
								char c;
								std::cin >> c;
								if (c != 'y' && c != 'Y')
								{
									abort = true;
									std::cout << "Aborting - The new DisplayInfo must be identifiable" << std::endl;
								}

							}

							//Last chance to back out
 							std::cout << "ABOUT TO WRITE NEW DISPLAYINFO" << std::endl << "DO YOU WISH TO CONTINUE? <Y/N>: ";
							char c;
							std::cin >> c;
							if (c != 'y' && c != 'Y')
							{
								abort = true;
								std::cout << "Aborting" << std::endl;
							}

							if (!abort)
							{
								//Now we write the new feature report to the rift
								if (pSensorDevice->SetFeatureReport(newDisplayInfo.Buffer, newDisplayInfo.PacketSize))
								{
									//And to confirm, let's read the updated report from the rift
									SensorDisplayInfoImpl trackerDisplayInfo;
									if (pSensorDevice->GetFeatureReport(trackerDisplayInfo.Buffer, trackerDisplayInfo.PacketSize))
									{
										trackerDisplayInfo.Unpack();
										OutputDisplayInfoDetails("Read back from Tracker", trackerDisplayInfo);
 										std::cout << "WRITE DISPLAYINFO COMPLETE" << std::endl;
									}
									else
									{
										//Error condition, can't identify the new display info type
										std::cerr << "Error: Failed to read updated DisplayInfo FeatureReport from RIFT" << std::endl;
										std::cerr << "Aborting..." << std::endl;
									}						
								}
								else
								{
									//Error condition, can't identify the new display info type
									std::cerr << "Error: Failed to write DisplayInfo FeatureReport to RIFT" << std::endl;
									std::cerr << "Aborting..." << std::endl;
								}
							}
						}
						else
						{
							//Error condition, input file is too big!!
							std::cerr << "Error: input file for write is too big!!" << std::endl;
							std::cerr << "Aborting..." << std::endl;
						}
					}
				}
			}

			//Clean up time
			pHMD->Disconnect(pSensorDevice);
		}
		else
		{
			std::cerr << "Error: Could not get handle to Sensor" << std::endl;
			std::cerr << "Aborting..." << std::endl;
		}

		pHMD->Release();
		pManager->Release();
	}

	std::cout << std::endl;
	std::cout << "Press RETURN to exit" << std::endl; 
	std::cin.get();

	return 0;
}


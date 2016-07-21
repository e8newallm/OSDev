unsigned int ReadFromPCI(char Bus, char Device, char Function, char Offset)
{
	Output32(PCI_CONFIG_ADDRESS, (int)(((int)Bus << 16) | ((int)Device << 11) | ((int)Function << 8) | ((int)Offset & 0xfc) | ((int)0x80000000)));
	return Input32(PCI_CONFIG_DATA);
}

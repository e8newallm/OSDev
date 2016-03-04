struct RSDPDescriptor {
 char Signature[8];
 char Checksum;
 char OEMID[6];
 char Revision;
 int RsdtAddress;
} __attribute__ ((packed));

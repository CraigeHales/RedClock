/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

struct TZHeader {
    int n; // number of pointers that follow:
    int stringTableIndexOffset;
    int stringTableDataOffset;
    int RLETableIndexOffset;
    int RLETableDataOffset;
    int buildDateOffset;
    int nrows; // lat
    int ncols; // lon
    int bot; //-90 lat coverage for nrows
    int top; //90 lat coverage for nrows
    int left; //-180 lon cover for ncols
    int right; //180 lon cover for ncols
    char sig[4]; //"END."
};
struct TZStringTableIndex {
    int offset[1];
};
struct TZStringTableData {
    char data[8]; // would be 1, except 8 is useful for clockserver Dummy version for isExecutable
};
struct TZRLETableIndex {
    int offset[1];
};
struct TZRLETableData {
    char data[2]; // would be 1, except 2 is useful for clockserver Dummy version for isExecutable
};
struct TZBuildDate {
    char text[6]; // would be 1, except 6 is useful for clockserver Dummy version for isExecutable
};

struct SMALLTIMEZONE { // for the dummy executable loaded in clockserver/clockmain
    struct TZHeader h;
    struct TZStringTableIndex sti;
    struct TZStringTableData std;
    struct TZRLETableIndex rti;
    struct TZRLETableData rtd;
    struct TZBuildDate bd;
    int executableLength; // executable follows, immediately
};

extern const struct SMALLTIMEZONE *gTimeZoneData;

extern int timezoneInit(void);

extern char *timezoneGet(double lat, double lon);

extern double lerp(double x, double x0, double y0, double x1, double y1);

extern void DumpHex(const void* data, size_t offset, size_t size);



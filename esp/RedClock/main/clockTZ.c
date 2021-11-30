/* 
    RedClock Copyright © 2021 Craige Hales
    All rights reserved.

    This source code is licensed under the BSD-style license found in the
    LICENSE file in the root directory of this source tree. 

    thanks to many people that posted bits of information!

*/

#include "clockMain.h"


    static struct TZHeader *h;
    static struct TZStringTableIndex *stringtableindex;
    static struct TZStringTableData *stringtabledata;
    static struct TZRLETableIndex  *rletableindex;
    static struct TZRLETableData *rletabledata;
    static struct TZBuildDate *builddate;

const struct SMALLTIMEZONE *gTimeZoneData;

int timezoneInit(void)
{
    esp_err_t error;

    const esp_partition_t *part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "tzmap");
    if(part==NULL){
        printf("tzmap partition not found\n");
    }
    
    spi_flash_mmap_handle_t handle; // for the unmmap, which will never happen
    error = esp_partition_mmap(part, 0, part->size, SPI_FLASH_MMAP_DATA, (const void**)&gTimeZoneData, &handle);
    if(error!=ESP_OK){
        printf("error esp_partition_mmap %d\n",error);
    }
    printf("gTimeZoneData at %d on core %d\n",(int)(gTimeZoneData), xPortGetCoreID());

    h = (struct TZHeader *)gTimeZoneData;
    sClockState.timezoneBuildDate = "bad"; // assume bad
    if(h->n > 0) {
        stringtableindex = (struct TZStringTableIndex *)((char*)gTimeZoneData + h->stringTableIndexOffset);
        stringtabledata = (struct TZStringTableData *)((char*)gTimeZoneData + h->stringTableDataOffset);
        rletableindex = (struct TZRLETableIndex *)((char*)gTimeZoneData + h->RLETableIndexOffset);
        rletabledata = (struct TZRLETableData *)((char*)gTimeZoneData + h->RLETableDataOffset);
        builddate = (struct TZBuildDate *)((char*)gTimeZoneData + h->buildDateOffset);


        printf("gpsheader n=%d stringTableIndexOffset=%d stringTableDataOffset=%d RLETableIndexOffset=%d RLETableDataOffset=%d buildDateOffset=%d nrows=%d ncols=%d bot=%d top=%d left=%d right=%d sig=%s\n",
                    h->n,h->stringTableIndexOffset,h->stringTableDataOffset,h->RLETableIndexOffset,
                    h->RLETableDataOffset,h->buildDateOffset,h->nrows,h->ncols,h->bot,h->top,h->left,h->right,h->sig);
        if(h->nrows != 16000){ // updating RedClock.bin case...152(below) does not exist
        
            // this is unused, left over from before proper OTA code was written. I thought
            // I could use the timezone map area as a staging area for the bin file. Instead,
            // I made the spiffs much smaller, removed demo html files from spiffs, shrank the
            // time zone partititon to just big enough, and made two bin partitions so OTA could work.
            
            //printf("sti[0] off=%d\n",stringtableindex->offset[0]);
            printf("string[0]=%s\n", &(stringtabledata->data[stringtableindex->offset[0]]) ); // RedClock.bin uses nr=1 and Etc/UTC
        }
        else { // normal case...this is just a sanity check...displays "string[152]=EST5EDT,M3.2.0,M11.1.0"
            //printf("sti[0] off=%d\n",stringtableindex->offset[152]);
            //printf("stringtabledata=%ld (before)\n",(long)stringtabledata);
            //printf("rletableindex=%ld (after)\n",(long)rletableindex);
            //printf("(long)(&(stringtabledata->data))=%ld (between)\n",(long)(&(stringtabledata->data)));
            //printf("stringtableindex->offset[152]=%ld\n",(long)(stringtableindex->offset[152]));
            //printf("&(stringtabledata->data[stringtableindex->offset[152]])=%ld\n",(long)(&(stringtabledata->data[stringtableindex->offset[152]])));
            //DumpHex(0, (size_t)&(stringtabledata->data[stringtableindex->offset[152]]), 100);
            if (builddate->text[0]!=0xFF) { // uninitialized is FFFFFFFFFFFF. Might have been interrupted, don't use it!
                printf("string[152]=%s\n", &(stringtabledata->data[stringtableindex->offset[152]]) ); // America/New_York
                printf("build=%s\n",builddate->text );
                sClockState.timezoneBuildDate = builddate->text;
            }
        }
    }
    if(strcmp(sClockState.timezoneBuildDate, "bad")==0) {
        printf("*******BAD TZ TABLE******\n");
    }
    return 0;
}

double lerp(double x, double x0, double y0, double x1, double y1) {
    double fraction = (x - x0) / (x1 - x0);
    double range = y1 - y0;
    return y0 + fraction * range;
}

char *timezoneGet(double lat, double lon) {
//    static char buffer[200];
//    sprintf(buffer,"fixme lat=%f lon=%f",lat,lon);
//    return buffer;
    if (strcmp(sClockState.timezoneBuildDate, "bad") != 0) {
        int row = lerp( lat, h->bot, h->nrows-1, h->top, 0 );
        unsigned char *r = (unsigned char *)(rletabledata->data) + rletableindex->offset[row];
        int col = lerp( lon, h->left, 1, h->right, h->ncols );
        int c = 0; // count the rle pixels
        short code = 0;
        while(c < col) { // until past the desired column
            // get the 1 or 2 compressed code for this run in the rle data
            code = *r++;
            if ( code > 127 ) {
                code = -(code * 256 + *r++);
            }
            // get the 1 or 2 compressed length of this run in the rle data
            short length = *r++;
            if ( length > 127 ) {
                length = -(length * 256 + *r++);
            }
            c += length; // how many pixels have we advanced through
        }
        //printf("timezone code slot for(%f,%f) %d\n",lat,lon,code);
        return stringtabledata->data + stringtableindex->offset[code];
    }
    return "UTC0";
// if the code is 0, above returns an empty string \0 for water.
// these could be baked into the rle data, but it would be 16K*12 (about) transitions(~3) bigger (a lot bigger, 500K maybe, might not have that much!)
// lon 0 is centered in GMT0. 360 degrees/24 is 15 degrees, so +/- 7.5 degrees
// later...
/*
"Etc/GMT","GMT0"
"Etc/GMT-0","GMT0"
"Etc/GMT-1","<+01>-1"
"Etc/GMT-2","<+02>-2"
"Etc/GMT-3","<+03>-3"
"Etc/GMT-4","<+04>-4"
"Etc/GMT-5","<+05>-5"
"Etc/GMT-6","<+06>-6"
"Etc/GMT-7","<+07>-7"
"Etc/GMT-8","<+08>-8"
"Etc/GMT-9","<+09>-9"
"Etc/GMT-10","<+10>-10"
"Etc/GMT-11","<+11>-11"
"Etc/GMT-12","<+12>-12"
"Etc/GMT-13","<+13>-13"
"Etc/GMT-14","<+14>-14"
"Etc/GMT0","GMT0"
"Etc/GMT+0","GMT0"
"Etc/GMT+1","<-01>1"
"Etc/GMT+2","<-02>2"
"Etc/GMT+3","<-03>3"
"Etc/GMT+4","<-04>4"
"Etc/GMT+5","<-05>5"
"Etc/GMT+6","<-06>6"
"Etc/GMT+7","<-07>7"
"Etc/GMT+8","<-08>8"
"Etc/GMT+9","<-09>9"
"Etc/GMT+10","<-10>10"
"Etc/GMT+11","<-11>11"
"Etc/GMT+12","<-12>12"
*/
  //  }
}

//https://gist.github.com/ccbrown/9722406
void DumpHex(const void* data, size_t offset, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
	
	    if(i%16==0) printf("%08X  ",offset+i); // wch add zero-based offsets
	
		printf("%02X ", ((unsigned char*)data)[offset+i]);
		if (((unsigned char*)data)[offset+i] >= ' ' && ((unsigned char*)data)[offset+i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[offset+i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <inttypes.h>

#include <chm_lib.h>
#include "chm.h"
#include "encint.h"

// #define PMGL_CHUNK_LEN (4096 - _CHMC_PMGL_LEN)
#define BUF_LEN 4096
#define dump_offset() do { \
		unsigned long long off = (unsigned long long)lseek(fd, 0, SEEK_CUR); \
		if (off != last_off) { \
			printf("file offset: %llu (0x%llx)\n", off, off); \
			last_off = off; \
		} \
	} while (0)

#define RESET_TABLE "::DataSpace/Storage/MSCompressed/Transform/{7FC28940-9D31-11D0-9B27-00A0C91E9C7C}/InstanceData/ResetTable"
#define CONTROL_DATA "::DataSpace/Storage/MSCompressed/ControlData"
#define SECT1_SPANINFO "::DataSpace/Storage/MSCompressed/SpanInfo"
#define SECT1_XFORM "::DataSpace/Storage/MSCompressed/Transform/List"
#define SECT1_CONTENT "::DataSpace/Storage/MSCompressed/Content"
#define SYSTEM "/#SYSTEM"
#define URLSTR "/#URLSTR"
#define URLTBL "/#URLTBL"
#define WINDOWS "/#WINDOWS"
#define STRINGS "/#STRINGS"
#define IDXHDR "/#IDXHDR"
#define TOPICS "/#TOPICS"
#define TOCIDX "/#TOCIDX"

struct idxhdr_st {
	char signature[4];
	Int32 unknown4;
	Int32 unknown8;
	Int32 num_of_topic;
	Int32 unknown10;
	Int32 offImageList;
	Int32 unknown18;
	Int32 imageTypeFolder;
	Int32 background;
	Int32 foreground;
	Int32 offFont;
	Int32 winStyle;
	Int32 exWinStyle;
	Int32 unknown34;
	Int32 offFrameName;
	Int32 offWinName;
	Int32 numOfInfo;
	Int32 unknown44;
	Int32 NumOfMergeFiles;
	Int32 unknown4C;
	Int32 MergeFilesOffsets[1004];
} *idxhdr;

#if 1 /* wins T */
typedef int BOOL;
typedef const char *LPCTSTR;
typedef unsigned long DWORD;
typedef long LONG;
typedef void *HWND;

typedef struct _RECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *PRECT;

typedef struct tagHH_WINTYPE
{
     int           cbStruct;
     BOOL          fUniCodeStrings;
     LPCTSTR       pszType;
     DWORD         fsValidMembers;
     DWORD         fsWinProperties;
     LPCTSTR       pszCaption;
     DWORD         dwStyles;
     DWORD         dwExStyles;
     RECT          rcWindowPos;
     int           nShowState;
     HWND          hwndHelp;
     HWND          hwndCaller;
     HWND          hwndToolBar;
     HWND          hwndNavigation;
     HWND          hwndHTML;
     int           iNavWidth;
     RECT          rcHTML;
     LPCTSTR       pszToc;
     LPCTSTR       pszIndex;
     LPCTSTR       pszFile;
     LPCTSTR       pszHome;
     DWORD         fsToolBarFlags;
     BOOL          fNotExpanded;
     int           curNavType;
     int           idNotify;
     LPCTSTR       pszJump1;
     LPCTSTR       pszJump2;
     LPCTSTR       pszUrlJump1;
     LPCTSTR       pszUrlJump2;
} HH_WINTYPE;
#endif

static struct chmFile *chm_file = NULL;


static struct chmcItsfHeader Itsf;
static struct chmcSect0 Sect0;
static struct chmcItspHeader Itsp;
static char buf[BUF_LEN];

static const UChar DIR_UUID[] = { 0x10,0xfd,0x01,0x7c,0xaa,0x7b,0xd0,0x11,0x9e,0x0c,0x00,0xa0,0xc9,0x22,0xe6,0xec };
static const UChar STREAM_UUID[] = { 0x11,0xfd,0x01,0x7c,0xaa,0x7b,0xd0,0x11,0x9e,0x0c,0x00,0xa0,0xc9,0x22,0xe6,0xec };
static const UChar SYS_UUID[] = { 0x6a,0x92,0x02,0x5d,0x2e,0x21,0xd0,0x11,0x9d,0xf9,0x00,0xa0,0xc9,0x22,0xe6,0xec };

static unsigned long long reset_table_off = 0;
static unsigned long long reset_table_len = 0;
static void *reset_table_buf = NULL;

static unsigned long long section_0_off = 0;
// static unsigned long long section_1_off = 0;

static unsigned long long ctrl_data_off = 0;
static unsigned long long ctrl_data_len = 0;
static void *ctrl_data_buf = NULL;


static unsigned long long system_off = 0;
static unsigned long long system_len = 0;
static void *system_buf = NULL;


static unsigned long long urlstr_off = 0;
static unsigned long long urlstr_len = 0;
static void *urlstr_buf = NULL;


static unsigned long long last_off = -1;
static unsigned long long sect1_spaninfo_off = 0;


static unsigned long long sect1_xform_off = 0;
static unsigned long long sect1_xform_len = 0;
static void *sect1_xform_buf = NULL;

static void dump_idxhdr(struct idxhdr_st *idxhdr)
{
	int i;

	printf("\tIDXHDR magic = '%.*s'\n", 4, idxhdr->signature);
	printf("\tIDXHDR unknown 4 = %d\n", idxhdr->unknown4);
	printf("\tIDXHDR unknown 8 = %d\n", idxhdr->unknown8);
	printf("\tIDXHDR num of topic = %d\n", idxhdr->num_of_topic);
	printf("\tIDXHDR unknown 10 = %d\n", idxhdr->unknown10);
	printf("\tIDXHDR image list offset = %d\n", idxhdr->offImageList);
	printf("\tIDXHDR unknown 18 = %d\n", idxhdr->unknown18);
	printf("\tIDXHDR image type folder = %d\n", idxhdr->imageTypeFolder);
	printf("\tIDXHDR background = %d\n", idxhdr->background);
	printf("\tIDXHDR foreground = %d\n", idxhdr->foreground);
	printf("\tIDXHDR font offset = %d\n", idxhdr->offFont);
	printf("\tIDXHDR WindowStyle = %d\n", idxhdr->winStyle);
	printf("\tIDXHDR ExWindowStyle = %d\n", idxhdr->exWinStyle);
	printf("\tIDXHDR unknown 34 = %d\n", idxhdr->unknown34);
	printf("\tIDXHDR FrameName offset = %d\n", idxhdr->offFrameName);
	printf("\tIDXHDR WindowName offset = %d\n", idxhdr->offWinName);
	printf("\tIDXHDR number of info = %d\n", idxhdr->numOfInfo);
	printf("\tIDXHDR unknown 44 = %d\n", idxhdr->unknown44);
	printf("\tIDXHDR num of merge files = %d\n", idxhdr->NumOfMergeFiles);
	printf("\tIDXHDR unknown 4C = %d\n", idxhdr->unknown4C);

	for (i=0; i<idxhdr->NumOfMergeFiles && i < 1004 ; i++)
		printf("\tIDXHDR merge file[%d] = %d\n", idxhdr->MergeFilesOffsets[i]);
}

static void hexdump (UChar *p, unsigned long long off, unsigned long len)
{
	unsigned long done, tocmp, i, lastcmp, out;
	int size;
	UChar row[16];

	done = 0;
	lastcmp = 0;
	while (done < len) {
		out = 0;

		// trim new row to dump
		if (done + 16 < len)
			size = 16;
		else
			size = len - done;

		// check if we have some previous data already
		if (done > 0) {
			// trim compare old with new
			if (done > 16)
				tocmp = 16;
			else
				tocmp = done;

			if(!memcmp(&p[done],row,tocmp)) {
				if (!lastcmp) {
					printf("*\n");
					lastcmp = 1;
				}
				done += tocmp;
				continue;
			} else
				lastcmp = 0;

		}

		// get new data into row
		memcpy(row,&p[done],size);

		printf("0x%08x: ", off + done);
		out += 12;
		for(i=0; i<size; i++) {
			printf("%02x ",row[i]);
			out += 3; 
		}
// 		printf(" (%d) ",out);
		while(out < 60) {
			putchar(' ');
			out++;
		}
		for(i=0; i<size; i++) {
			printf("%c",isprint(row[i]) ? row[i] : '.');
		}
		putchar('\n');
		done += size;
	}
}

int main (int argc, char *argv[])
{
	int fd, i, block;
	size_t rx;
	struct stat fd_stat;
	struct chmUnitInfo ui;

	if (argc != 2) {
		fprintf(stderr,"usage: chmchk [file]\n");
		exit(1);
	}

	chm_file = chm_open (argv[1]);
	if (!chm_file) {
		fprintf(stderr,"error: can't chm_open '%s'\n", argv[1]);
		exit(1);
	}

	fd = open(argv[1],O_RDONLY);
	if (fd < 0) {
		fprintf(stderr,"error: can't open '%s': %s\n", argv[1], strerror(errno));
		exit(1);
	}

	i = fstat(fd, &fd_stat);
	if (i < 0) {
		fprintf(stderr,"error: can't stat '%s': %s\n", argv[1], strerror(errno));
		exit(1);
	}
	// NOTE PRId64 suggested to print off_t but use wrong size on my linux
	// NOTE use %zd suggested for size_t, but read return ssize_t (great...)
	printf("file size: %zd\n", fd_stat.st_size);
	dump_offset();
	rx = read(fd, &Itsf, _CHMC_ITSF_V3_LEN);
	if (rx != _CHMC_ITSF_V3_LEN) {
		fprintf(stderr,"error: reading ITSF, expected %lld len, got %zd\n", _CHMC_ITSF_V3_LEN, rx);
		exit(1);
	}

	// check ITSF

	if (memcmp("ITSF",&Itsf.signature,4)) {
		fprintf(stderr,"error: expected ITSF magic, got '%.*s'\n", 4, &Itsf.signature);
		exit(1);
	}
	printf("ITSF version = %d\n", Itsf.version);
	if (Itsf.version != 3) {
		fflush(stdout);
		fprintf(stderr,"warning: chmc should create version 3\n");
	}
	printf("ITSF header len = %d\n", Itsf.header_len);
	if (Itsf.header_len != _CHMC_ITSF_V3_LEN) {
		fflush(stdout);
		fprintf(stderr,"warning: chmc should create header len %lu\n",_CHMC_ITSF_V3_LEN);
	}
	printf("ITSF unknown 000c = 0x%08x\n", Itsf.unknown_000c);
	printf("ITSF last modified = %lu\n", Itsf.last_modified);
	printf("ITSF lang ID = 0%08x\n", Itsf.lang_id);
	printf("ITSF dir UUID = ");
	for(i=0; i<16; i++)
		printf("%02x", Itsf.dir_uuid[i]);
	printf("\n");
	if (memcmp(DIR_UUID,&Itsf.dir_uuid,16)) {
		fflush(stdout);
		fprintf(stderr,"warning: expected dir UUID ");
		for(i=0; i<16; i++)
			fprintf(stderr,"%02x", DIR_UUID[i]);
		fprintf(stderr,"\n");
	}
	printf("ITSF stream UUID = ");
	for(i=0; i<16; i++)
		printf("%02x", Itsf.stream_uuid[i]);
	printf("\n");
	if (memcmp(STREAM_UUID,&Itsf.stream_uuid,16)) {
		fflush(stdout);
		fprintf(stderr,"warning: expected stream UUID ");
			for(i=0; i<16; i++)
				fprintf(stderr,"%02x", STREAM_UUID[i]);
		fprintf(stderr,"\n");
	}
	printf("ITSF sect0 offset = %llu\n", Itsf.sect0_offset);
	if (Itsf.sect0_offset != _CHMC_ITSF_V3_LEN) {
		fflush(stdout);
		fprintf(stderr,"warning: usually expected %lu\n",_CHMC_ITSF_V3_LEN);
	}

	printf("ITSF sect0 len = %llu\n", Itsf.sect0_len);
	printf("ITSF dir offset = %llu\n", Itsf.dir_offset);
	printf("ITSF dir len = %llu\n", Itsf.dir_len);
	// FIXME (Not present before V3)
	printf("ITSF data offset = %llu\n", Itsf.data_offset);
	// FIXME if it is version 3 read from data_offset, else < 3 current seek is fine
	section_0_off = Itsf.data_offset;


	// check SECT 0

	if (lseek(fd, Itsf.sect0_offset, SEEK_SET) < 0) {
		fprintf(stderr,"error: can't lseek %llu: %s\n", (unsigned long long)Itsf.sect0_offset, strerror(errno));
		exit(1);
	}
	dump_offset();
	rx = read(fd, &Sect0, _CHMC_SECT0_LEN);
	if (rx != _CHMC_SECT0_LEN) {
		fprintf(stderr,"error: reading sect 0, expected %lu len, got %llu\n", _CHMC_SECT0_LEN, (unsigned long long)rx);
		exit(1);
	}
	printf("SECT0 unknown 0000 = 0x%08x\n", Sect0.unknown_0000);
	printf("SECT0 unknown 0004 = 0x%08x\n", Sect0.unknown_0004);
	printf("SECT0 file length = %llu\n", (unsigned long long)Sect0.file_len);
	if (Sect0.file_len != fd_stat.st_size) {
		fflush(stdout);
		fprintf(stderr,"warning: sect 0 file length mismatch real file length\n");
	}
	printf("SECT0 unknown 0010 = 0x%08x\n", Sect0.unknown_0010);
	printf("SECT0 unknown 0014 = 0x%08x\n", Sect0.unknown_0014);

	// check ITSP

	if (lseek(fd, Itsf.dir_offset, SEEK_SET) < 0) {
		fprintf(stderr,"error: can't lseek %llu: %s\n", (unsigned long long)Itsf.dir_offset, strerror(errno));
		exit(1);
	}
	dump_offset();
	rx = read(fd, &Itsp, _CHMC_ITSP_V1_LEN);
	if (rx != _CHMC_ITSP_V1_LEN) {
		fprintf(stderr,"error: reading ITSP, expected %lu len, got %llu\n", _CHMC_ITSP_V1_LEN, (unsigned long long)rx);
		exit(1);
	}
	if (memcmp("ITSP",&Itsp.signature,4)) {
		fprintf(stderr,"error: expected ITSP magic, got '%.*s'\n", 4, &Itsp.signature);
		exit(1);
	}
	printf("ITSP version = %d\n", Itsp.version);
	if (Itsp.version != 1) {
		fflush(stdout);
		fprintf(stderr,"warning: chmc should create version 3\n");
	}
	printf("ITSP version = %d\n", Itsp.version);
	printf("ITSP header len = %d\n", Itsp.header_len);
	if (Itsp.header_len != _CHMC_ITSP_V1_LEN) {
		fflush(stdout);
		fprintf(stderr,"warning: expected %d\n", _CHMC_ITSP_V1_LEN);
	}
	printf("ITSP unknown 000c = 0x%08x\n", Itsp.unknown_000c);
	printf("ITSP block len = %lu\n", Itsp.block_len);
	printf("ITSP block index interleave level = %d\n", Itsp.blockidx_intvl);
	if (Itsp.blockidx_intvl != CHM_IDX_INTVL) {
		fflush(stdout);
		fprintf(stderr,"warning: chmc should create interleave %d\n", CHM_IDX_INTVL);
	}
	printf("ITSP index depth = %d\n", Itsp.index_depth);
	printf("ITSP index root = %d\n", Itsp.index_root);
	printf("ITSP index head = %d\n", Itsp.index_head);
	printf("ITSP index last = %d\n", Itsp.index_last);
	printf("ITSP unknown 0028 = 0x%08x\n", Itsp.unknown_0028);
	printf("ITSP number of blocks = %lu\n", Itsp.num_blocks);
	printf("ITSP language ID = 0x%08x\n", Itsp.lang_id);
	printf("ITSP system UUID = ");
	for(i=0; i<16; i++)
		printf("%02x", Itsp.system_uuid[i]);
	printf("\n");
	if (memcmp(SYS_UUID,&Itsp.system_uuid,16)) {
		fflush(stdout);
		fprintf(stderr,"warning: expected system UUID ");
		for(i=0; i<16; i++)
			fprintf(stderr,"%02x", SYS_UUID[i]);
		fprintf(stderr,"\n");
	}
	printf("ITSP header len 2 = %lu\n", Itsp.header_len2);
	if (Itsp.header_len2 != _CHMC_ITSP_V1_LEN) {
		fflush(stdout);
		fprintf(stderr,"warning: expected %d\n", _CHMC_ITSP_V1_LEN);
	}
	printf("ITSP unknown 0048 = ");
	for(i=0; i<12; i++)
		printf("%02x", Itsp.unknown_0048[i]);
	printf("\n");

	// check PMGL/PMGI

	for (block = 0; block < Itsp.num_blocks; block++) {
		dump_offset();
		rx = read(fd, buf, BUF_LEN);
		if (rx != BUF_LEN) {
			fflush(stdout);
			fprintf(stderr,"error: reading expected %lu len, got %llu\n", BUF_LEN, (unsigned long long)rx);
			exit(1);
		}
		if (!memcmp("PMGL",buf,4)) {
			struct chmcPmglHeader *Pmgl = (struct chmcPmglHeader *)buf;
			UInt32 namelen, section, offset, length;
			int vlen, len, todo;
			UChar *p, *name;
			short *qr, quickref_entries;
			int entry = 0;

			printf("PMGL[%d] free space = %lu\n", block, Pmgl->free_space);
			printf("PMGL[%d] unknown 0008 = 0x%08x\n", block, Pmgl->unknown_0008);
			printf("PMGL[%d] previous block = %d\n", block, Pmgl->block_prev);
			printf("PMGL[%d] next block = %d\n", block, Pmgl->block_prev);

			p = buf + sizeof(struct chmcPmglHeader);
			todo = BUF_LEN - Pmgl->free_space - sizeof(struct chmcPmglHeader);
			while (todo) {
				vlen = chmc_decint(p, &namelen);
				p += vlen;
				todo -= vlen;
				// FIXME name are utf-8
				name = p;
				p += namelen;
				todo -= namelen;
				len = chmc_decint(p, &section);
				p += len;
				todo -= len;
				len = chmc_decint(p, &offset);
				p += len;
				todo -= len;
				len = chmc_decint(p, &length);
				p += len;
				todo -= len;
				printf("PMGL[%d][%d] len size %d, name len %lu, '%.*s' section %lu, offset %lu, length %lu\n",
					block, entry++, vlen, namelen, namelen, name, section, offset, length);
				if (!strncmp(RESET_TABLE, name,sizeof(RESET_TABLE)-1)) {
					reset_table_off = section_0_off + offset;
					reset_table_len = length;
				} else if (!strncmp(CONTROL_DATA, name,sizeof(CONTROL_DATA)-1)) {
					ctrl_data_off = section_0_off + offset;
					ctrl_data_len = length;
				} else if (!strncmp(SECT1_SPANINFO, name,sizeof(SECT1_SPANINFO)-1)) {
					sect1_spaninfo_off = section_0_off + offset;
					if (length != 8) {
						fflush(stdout);
						fprintf(stderr,"warning: expected length 8 for MSCompressed/SpanInfo\n");
					}
				} else if (!strncmp(SECT1_XFORM, name,sizeof(SECT1_XFORM)-1)) {
					sect1_xform_off = section_0_off + offset;
					sect1_xform_len = length;
				} else if (!strncmp(SYSTEM, name, sizeof(SYSTEM)-1)) {
					system_off = section_0_off + offset;
					system_len = length;
				}
			}
			if (Pmgl->free_space >= 2) {
				qr = (short *)(buf + BUF_LEN - 2);
				entry = 0;
				printf("PMGL[%d] quick ref entries = %d\n", block, *qr);
				todo = *qr;
				if (entry < todo)
					printf("PMGL[%d] quick ref = ", block);
				while (entry < todo)
					printf("[%d]%d%c",entry++, *(--qr), (entry < todo - 1) ? ' ' : '\n');
			}
			dump_offset();
		} else if (!memcmp("PMGI",buf,4)) {
			struct chmcPmgiHeader *Pmgi = (struct chmcPmgiHeader *)buf;
			UInt32 namelen, chunk;
			int vlen, len, todo;
			UChar *p, *name;
			short *qr, quickref_entries;
			int entry = 0;

			printf("PMGI[%d] free space = %lu\n", block, Pmgi->free_space);

			p = buf + sizeof(struct chmcPmgiHeader);
			todo = BUF_LEN - Pmgi->free_space - sizeof(struct chmcPmglHeader);
			while (todo) {
				vlen = chmc_decint(p, &namelen);
				if (namelen == 0)
					break;
				p += vlen;
				todo -= vlen;
				// FIXME name are utf-8
				name = p;
				p += namelen;
				todo -= namelen;
				len = chmc_decint(p, &chunk);
				p += len;
				todo -= len;
				printf("PMGI[%d][%d] len size %d, name len %lu, '%.*s' chunk %lu\n",
					block, entry++, vlen, namelen, namelen, name, chunk);
			}
			if (Pmgi->free_space >= 2) {
				qr = (short *)(buf + BUF_LEN - 2);
				entry = 0;
				printf("PMGI[%d] quick ref entries = %d\n", block, *qr);
				todo = *qr;
				if (entry < todo)
					printf("PMGI[%d] quick ref = ", block);
				while (entry < todo)
					printf("[%d]%d%c",entry++, *(--qr), (entry < todo - 1) ? ' ' : '\n');
			}
			dump_offset();
		} else {
			fflush(stdout);
			fprintf(stderr,"error: unknown magic '%.*s'\n", 4, buf);
			exit(1);
		}
	}
	// dump offset after all PMGL/PMGI
	dump_offset();

	// check Section/Namelist

	{
	unsigned long file_len, entries;
	int j, entry_len;
	short *wbuf = (short *)buf;

	if (lseek(fd, Itsf.data_offset, SEEK_SET) < 0) {
		fprintf(stderr,"error: can't lseek %llu: %s\n", (unsigned long long)Itsf.data_offset, strerror(errno));
		exit(1);
	}
	dump_offset();
	rx = read(fd, wbuf, 2);
	if (rx != 2) {
		fprintf(stderr,"error: reading namelist file length %llu\n", (unsigned long long)rx);
		exit(1);
	}
	file_len = *wbuf;
	printf("NAMELIST file length = %lu\n", file_len);
	rx = read(fd, &wbuf[1], file_len * 2);
	if (rx != file_len * 2) {
		fprintf(stderr,"error: reading namelist %llu\n", (unsigned long long)rx);
		exit(1);
	}
	entries = *(++wbuf);
	printf("NAMELIST number of entries = %lu\n", entries);
	for (i=0; i<entries; i++) {
		entry_len = *(++wbuf);
		printf("NAMELIST[%d] name len = %d, '", i, entry_len);
		for (j=0; j<entry_len; j++) {
			putchar(*(++wbuf));
		}
		printf("'\n");
		++wbuf;
	}
	dump_offset();
	}

	// check control data

	if ((ctrl_data_off) && (ctrl_data_len)) {
		struct chmcLzxcControlData *ctrlData;

		if (lseek(fd, ctrl_data_off, SEEK_SET) < 0) {
			fprintf(stderr,"error: can't lseek %llu: %s\n", ctrl_data_off, strerror(errno));
			exit(1);
		}
		dump_offset();
		ctrl_data_buf = malloc(ctrl_data_len);
		if (!ctrl_data_buf) {
			fflush(stdout);
			fprintf(stderr,"error: malloc failed\n");
			exit(1);
		}
		rx = read(fd, ctrl_data_buf, ctrl_data_len);
		if (rx != ctrl_data_len) {
			fprintf(stderr,"error: reading control data %llu\n", (unsigned long long)rx);
			exit(1);
		}

		ctrlData = (struct chmcLzxcControlData *)ctrl_data_buf;
		printf("CONTROLDATA size = %lu\n", ctrlData->size);
		printf("CONTROLDATA version = %lu\n", ctrlData->version);
		if (ctrlData->version != 2) {
			fflush(stdout);
			fprintf(stderr,"warning: expected 2\n");
		}
		printf("CONTROLDATA signature = '%.*s'\n", 4, &ctrlData->signature);
		if (memcmp("LZXC", ctrlData->signature, 4)) {
			fflush(stdout);
			fprintf(stderr,"warning: expected LZXC\n");
			exit(1);
		}
		printf("CONTROLDATA reset interval = %lu\n", ctrlData->resetInterval);
		printf("CONTROLDATA window size = %lu\n", ctrlData->windowSize);
		printf("CONTROLDATA window per reset = %lu\n", ctrlData->windowsPerReset);
		printf("CONTROLDATA unknown 18 = %lu\n", ctrlData->unknown_18);
	} else {
		fflush(stdout);
		fprintf(stderr,"error: control data not fount or 0 length\n");
		exit(1);
	}

	// check MSCompressed/SpanInfo

	if ((sect1_spaninfo_off)) {
		UInt64 len;

		if (lseek(fd, sect1_spaninfo_off, SEEK_SET) < 0) {
			fprintf(stderr,"error: can't lseek %llu: %s\n", sect1_spaninfo_off, strerror(errno));
			exit(1);
		}
		dump_offset();
		rx = read(fd, &len, 8);
		if (rx != 8) {
			fprintf(stderr,"error: reading reset table %llu\n", (unsigned long long)rx);
			exit(1);
		}
		printf("SPANINFO[1] length %llu\n", len);
	}

	// check MSCompressed/Transform/List
	if ((sect1_xform_off) && (sect1_xform_len)) {
		short *p;

		if (lseek(fd, sect1_xform_off, SEEK_SET) < 0) {
			fprintf(stderr,"error: can't lseek %llu: %s\n", sect1_xform_off, strerror(errno));
			exit(1);
		}
		dump_offset();
		sect1_xform_buf = malloc(sect1_xform_len);
		if (!sect1_xform_buf) {
			fflush(stdout);
			fprintf(stderr,"error: malloc failed\n");
			exit(1);
		}
		rx = read(fd, sect1_xform_buf, sect1_xform_len);
		if (rx != sect1_xform_len) {
            fflush(stdout);
			fprintf(stderr,"error: reading MSCompressed/Transform/List %llu\n", (unsigned long long)rx);
			exit(1);
		}
		printf("XFORMLIST[1] length %llu\n", sect1_xform_len);
		printf("XFORMLIST[1] ");
		p = (short *)sect1_xform_buf;
		for(i=0; i<sect1_xform_len; i += 2) {
			printf("0x%02x%c", *p, (i<sect1_xform_len - 2) ? ' ' : '\n');
			p++;
		}
	}

	// check reset table

	if ((reset_table_off) && (reset_table_len)) {
		struct chmcLzxcResetTable *rstTbl;
		UInt64 *entry;

		if (lseek(fd, reset_table_off, SEEK_SET) < 0) {
			fprintf(stderr,"error: can't lseek %llu: %s\n", reset_table_off, strerror(errno));
			exit(1);
		}
		dump_offset();
		reset_table_buf = malloc(reset_table_len);
		if (!reset_table_buf) {
			fflush(stdout);
			fprintf(stderr,"error: malloc failed\n");
			exit(1);
		}
		rx = read(fd, reset_table_buf, reset_table_len);
		if (rx != reset_table_len) {
			fprintf(stderr,"error: reading reset table %llu\n", (unsigned long long)rx);
			exit(1);
		}
		rstTbl = (struct chmcLzxcResetTable *)reset_table_buf;
		printf("RESETTABLE version = %lu\n", rstTbl->version);
		if (rstTbl->version != 2) {
			fflush(stdout);
			fprintf(stderr,"warning: expected 2\n");
		}
		printf("RESETTABLE entries count = %lu\n", rstTbl->block_count);
		printf("RESETTABLE entry size = %lu\n", rstTbl->entry_size);
		if (rstTbl->entry_size != 8) {
			fflush(stdout);
			fprintf(stderr,"warning: expected 8\n");
		}
		printf("RESETTABLE table offset = %lu\n", rstTbl->table_offset);
		printf("RESETTABLE uncompressed length = %llu\n", rstTbl->uncompressed_len);
		printf("RESETTABLE compressed length = %llu\n", rstTbl->compressed_len);
		printf("RESETTABLE block length = 0x%llx\n", rstTbl->block_len);
		entry = (UInt64 *)(reset_table_buf + rstTbl->table_offset);
		printf("RESETTABLE entries = ");
		for(i=0; i<rstTbl->block_count; i++)
			printf("[%d]%lld(0x%llx)%c", i, entry[i], entry[i], (i<rstTbl->block_count-1) ? ' ' : '\n');
		dump_offset();
	} else {
		fflush(stdout);
		fprintf(stderr,"error: reset table not fount or 0 length\n");
		exit(1);
	}

	// check /#SYSTEM

	if ((system_off) && (system_len)) {
		void *p;
		Int32 system_version;
		int how2dump, eof, done;

		struct st_system_entry {
			Int16 code;
			Int16 len;
			UChar data[65535];
		} *system_entry;
		struct st_system_entry4 {
			Int32 LCID;
			Int32 DBCS;
			Int32 full_text;
			Int32 KLinks;
			Int32 ALinks;
			UInt64 timestamp;
			Int32 unknown1;
			Int32 unknown2;
		} *system_entry4;
		struct st_system_entry8 {
			Int32 unknown0;
			Int32 offset0;
			Int32 unknown1;
			Int32 offset1;
		} *system_entry8;

		if (lseek(fd, system_off, SEEK_SET) < 0) {
			fprintf(stderr,"error: can't lseek %llu: %s\n", system_off, strerror(errno));
			exit(1);
		}
		dump_offset();
		system_buf = malloc(system_len);
		if (!system_buf) {
			fflush(stdout);
			fprintf(stderr,"error: malloc failed\n");
			exit(1);
		}
		rx = read(fd, system_buf, system_len);
		if (rx != system_len) {
            fflush(stdout);
			fprintf(stderr,"error: reading MSCompressed/Transform/List %llu\n", (unsigned long long)rx);
			exit(1);
		}
		system_version = *(Int32 *)system_buf;
		printf("SYSTEM version = %d, compatibility set to ", system_version);
		if (system_version == 2)
			printf("1.0\n");
		else if (system_version == 3)
			printf(">= 1.1\n");
		else
			printf("unknown!");
		system_entry = (struct st_system_entry *)(((void *) system_buf) + 4);
		i = 0;
		eof = 0;
        done = 0;
		while (!eof && (done < system_len)) {
			how2dump = 0;
			printf("SYSTEM entry[%d] code = %d, ", i, system_entry->code);
			switch(system_entry->code) {
			case 0:
				printf("Value of Contents file in the [OPTIONS] section of the HHP file. NT\n");
				break;
			case 1:
				printf("Value of Index file in the [OPTIONS] section of the HHP file. NT\n");
				break;
			case 2:
				printf("Value of Default topic in the [OPTIONS] section of the HHP file. NT\n");
				how2dump = 1;
				break;
			case 3:
				printf("Value of Title in the [OPTIONS] section of the HHP file. NT\n");
				how2dump = 1;
				break;
			case 4:
				if (system_entry->len == 28) {
					printf("HHA Version 4.72.7294 and earlier\n");
					how2dump = -1;
				} else
				if (system_entry->len == 36) {
					printf("HHA Version 4.72.8086 and later\n");
					how2dump = -1;
				} else
					printf("HHA Version unknown\n");
				system_entry4 = (struct st_system_entry4 *)system_entry->data;
				if (system_entry->len >= 28) {
					printf("SYSTEM entry[%d] len %d, data = \n",i,system_entry->len);
					printf("\tLCID = 0x%x\n",system_entry4->LCID);
					printf("\tDBCS = %d\n",system_entry4->DBCS);
					printf("\tfull text = %d\n",system_entry4->full_text);
					printf("\tKLinks = %d\n",system_entry4->KLinks);
					printf("\tALinks = %d\n",system_entry4->ALinks);
					printf("\ttimestamp = %lld\n",system_entry4->timestamp);
				}
				if (system_entry->len == 36) {
					printf("\tunknown 1 = 0x%d\n",system_entry4->unknown1);
					printf("\tunknown 2 = 0x%d\n",system_entry4->unknown2);
				}
				break;
			case 5:
				how2dump = 1;
				printf("Value of Default Window in the [OPTIONS] section of the HHP file. NT\n");
				break;
			case 6:
				printf("Value of Compiled file in the [OPTIONS] section of the HHP file (lowercase)\n");
				how2dump = 1;
				break;
			case 7:
				printf("DWORD present in files with Binary Index turned on.\n"
					"\tThe entry in the #URLTBL file that points to the sitemap index had the same first DWORD\n");
				break;
			case 8:
				how2dump = -1;
				system_entry8 = (struct st_system_entry8 *)system_entry->data;
				printf("SYSTEM entry[%d] len %d, data = \n",i,system_entry->len);
				printf("\tunknown 0 = %d\n",system_entry8->unknown0);
				printf("\toffset 0 = %d\n",system_entry8->offset0);
				printf("\tunknown 1 = %d\n",system_entry8->unknown1);
				printf("\toffset 1 = %d\n",system_entry8->offset1);
				break;
			case 9:
				printf("compiler version - shown in version dialog as 'Compiled with %%s'\n");
				how2dump = 1;
				break;
			case 10:
				printf("time_t timestamp. Derived from GetLocalTime. Not sure of the conversion yet.\n");
				break;
			case 11:
				printf("DWORD present in files with Binary TOC turned on.\n"
					"\tThe entry in the #URLTBL file that points to the sitemap contents has the same first DWORD\n");
				break;
			case 12:
				printf("DWORD. Number of information types\n");
				break;
			case 13:
				how2dump = -1;
				printf("The #IDXHDR file contains exactly the same bytes\n");
				printf("SYSTEM entry[%d] len %d, data =\n",i,system_entry->len);
				dump_idxhdr((struct idxhdr_st *)system_entry->data);
				break;
			case 14:
				printf("Rare. The ones I saw were from MS Word 2000. guess it is an MSOffice extension (or maybe not) that overrides the names & window types of the navigation tabs\n");
				break;
			case 15:
				printf("DWORD. Information type checksum. Unknown algorithm & data source\n");
				eof = 1;
				break;
			case 16:
				printf("Value of Default Font in [OPTIONS] section of the HHP file. NT\n");
				break;
			default:
				printf("unknown\n");
			}
// 			printf("SYSTEM entry[%d] len = %d\n", i, system_entry->len);
			//printf("SYSTEM entry[%d] data = %.*s\n", i, system_entry->len, system_entry->data);
			if (how2dump == 0) {
				printf("SYSTEM entry[%d] len %d, data = ",i,system_entry->len);
				putchar('\n');
				hexdump (system_entry->data,  system_off +
					((void *)system_entry->data) - ((void *)system_buf), system_entry->len);
			} else
			if (how2dump == 1) {
				printf("SYSTEM entry[%d] len %d, data = '%s'\n",i,system_entry->len,system_entry->data);
			} else
			if (how2dump == -1)
				/* ignore */;
			system_entry = (struct st_system_entry *)(((void *)system_entry) + system_entry->len + 4);
			i++;
			done += system_entry->len + 4;
		}
		hexdump (system_buf, system_off, system_len);
	} else {
		fflush(stdout);
		fprintf(stderr,"warning: /#SYSTEM not fount or 0 length\n");
	}

	if (CHM_RESOLVE_SUCCESS == chm_resolve_object(chm_file, URLSTR, &ui)) {
		LONGINT64 gotLen;
		UChar *p;
		UInt32 URL, FrameName, len, done, i, chunk_done, blocks, block;

		urlstr_buf = malloc(ui.length);
		if (!urlstr_buf) {
			fflush(stdout);
			fprintf(stderr,"error: malloc failed\n");
			exit(1);
		}
		gotLen = chm_retrieve_object(chm_file, &ui, urlstr_buf, 0, ui.length);
		if (gotLen != ui.length) {
			fflush(stdout);
			fprintf(stderr,"error: gotLen != ui.length\n");
			exit(1);
		}

		blocks = ui.length / 4096;
		if (ui.length % 4096)
			blocks += 1;
		printf ("/#URLSTR start = %llu, length = %llu, blocks = %lu\n", ui.start, ui.length, blocks);

		p = urlstr_buf;
		printf("URLSTR byte 0 = 0x%02x\n", p[0]);
		p += 1;
		done = 1;
		chunk_done = 1;
		i = 0;
		block = 0;
		while (done + 8 + 2 < ui.length) {
			memcpy (&URL, p, 4);
			memcpy (&FrameName, p + 4, 4);
			len = strlen(p + 8);
			if (!len)  {
				if (block + 1 < blocks) {
					p += 4096 - chunk_done;
					done += 4096 - chunk_done;
					chunk_done = 0;
					block++;
					continue;
				} else
					break;
			}
			printf("URLSTR %d URL, FrameName = %lu, %lu, len = %d, value = %.*s\n",
				i, URL, FrameName, len, len, p + 8);
			p += 8 + len + 1;
			done += 8 + len + 1;
			chunk_done += 8 + len + 1;
			if (chunk_done >= 4096) {
				chunk_done -= 4096;
				block++;
			}
			i++;
		}
// 		hexdump (urlstr_buf, ui.start, ui.length);
		free(urlstr_buf);
	}

	if (CHM_RESOLVE_SUCCESS == chm_resolve_object(chm_file, WINDOWS, &ui)) {
		LONGINT64 gotLen;
		UChar *p;
		UInt32 entries, size, i, val;
		UChar *buf;

		buf = malloc(ui.length);
		if (!buf) {
			fflush(stdout);
			fprintf(stderr,"error: malloc failed\n");
			exit(1);
		}
		gotLen = chm_retrieve_object(chm_file, &ui, buf, 0, ui.length);
		if (gotLen != ui.length) {
			fflush(stdout);
			fprintf(stderr,"error: gotLen != ui.length\n");
			exit(1);
		}
		// FIXME gotLen != ui.length
		printf ("/#WINDOWS start = %llu, length = %llu\n", ui.start, ui.length);

		memcpy(&entries, buf, 4);
		memcpy(&size, buf + 4, 4);

		printf("WINDOWS entries = %d, size = %d\n", entries, size);
		p = buf + 8;
		i = 0;
		while (i < entries) {
			HH_WINTYPE *win = p;

			memcpy (&val, p + 0x14, 4);
			printf ("WINDOWS %d title = %lu\n", i, val);
// 			uint32_t off_title = 
// 				*(uint32_t *)(raw + offset + 0x14);

			memcpy (&val, p + 0x68, 4);
			printf ("WINDOWS %d home = %lu\n", i, val);
// 			uint32_t off_home = 
// 				*(uint32_t *)(raw + offset + 0x68);

			memcpy (&val, p + 0x60, 4);
			printf ("WINDOWS %d hhc = %lu\n", i, val);
// 			uint32_t off_hhc = 
// 				*(uint32_t *)(raw + offset + 0x60);
			
			memcpy (&val, p + 0x64, 4);
			printf ("WINDOWS %d hhk = %lu\n", i, val);
// 			uint32_t off_hhk = 
// 				*(uint32_t *)(raw + offset + 0x64);


			{

			printf ("sizeof(HH_WINTYPE) = %d\n", sizeof(HH_WINTYPE));
			printf("HH_WINTYPE cbStruct = %d\n", win->cbStruct);
			printf("HH_WINTYPE fUniCodeStrings = %d\n", win->fUniCodeStrings);
			printf("HH_WINTYPE pszType = 0x%08lx\n", win->pszType);
			printf("HH_WINTYPE fsValidMembers = 0x%08lx\n", win->fsValidMembers);
			printf("HH_WINTYPE fsWinProperties = 0x%08lx\n", win->fsWinProperties);
			printf("HH_WINTYPE pszCaption = 0x%08lx\n", win->pszCaption);
			printf("HH_WINTYPE dwStyles = 0x%08lx\n", win->dwStyles);
			printf("HH_WINTYPE dwExStyles = 0x%08lx\n", win->dwExStyles);
			printf("HH_WINTYPE rcWindowPos = { %d, %d, %d, %d }\n", 
				win->rcWindowPos.left, win->rcWindowPos.top,
				win->rcWindowPos.right, win->rcWindowPos.bottom);
			printf("HH_WINTYPE nShowState = %d\n", win->nShowState);
			printf("HH_WINTYPE hwndHelp = %p\n", win->hwndHelp);
			printf("HH_WINTYPE hwndCaller = %p\n", win->hwndCaller);
			printf("HH_WINTYPE hwndToolBar = %p\n", win->hwndToolBar);
			printf("HH_WINTYPE hwndNavigation = %p\n", win->hwndNavigation);
			printf("HH_WINTYPE hwndHTML = %p\n", win->hwndHTML);
			printf("HH_WINTYPE iNavWidth = %d\n", win->iNavWidth);
			printf("HH_WINTYPE rcHTML = { %d, %d, %d, %d }\n", 
				win->rcHTML.left, win->rcHTML.top,
				win->rcHTML.right, win->rcHTML.bottom);
			printf("HH_WINTYPE pszToc = 0x%08lx\n", win->pszToc);
			printf("HH_WINTYPE pszIndex = 0x%08lx\n", win->pszIndex);
			printf("HH_WINTYPE pszFile = 0x%08lx\n", win->pszFile);
			printf("HH_WINTYPE pszHome = 0x%08lx\n", win->pszHome);
			printf("HH_WINTYPE fsToolBarFlags = 0x%08lx\n", win->fsToolBarFlags);
			printf("HH_WINTYPE fNotExpanded = %d\n", win->fNotExpanded);
			printf("HH_WINTYPE curNavType = %d\n", win->curNavType);
			printf("HH_WINTYPE idNotify = %d\n", win->idNotify);
			printf("HH_WINTYPE pszJump1 = 0x%08lx\n", win->pszJump1);
			printf("HH_WINTYPE pszJump2 = 0x%08lx\n", win->pszJump2);
			printf("HH_WINTYPE pszUrlJump1 = 0x%08lx\n", win->pszUrlJump1);
			printf("HH_WINTYPE pszUrlJump2 = 0x%08lx\n", win->pszUrlJump2);
			}

			hexdump (p, ui.start + p - buf, ui.length);
			i++;
		}
		free(buf);
	}

	if (CHM_RESOLVE_SUCCESS == chm_resolve_object(chm_file, IDXHDR, &ui)) {
		LONGINT64 gotLen;
		UChar *buf;

		buf = malloc(ui.length);
		if (!buf) {
			fflush(stdout);
			fprintf(stderr,"error: malloc failed\n");
			exit(1);
		}
		gotLen = chm_retrieve_object(chm_file, &ui, buf, 0, ui.length);
		if (gotLen != ui.length) {
			fflush(stdout);
			fprintf(stderr,"error: gotLen != ui.length\n");
			exit(1);
		}
		// FIXME gotLen != ui.length
		printf ("/#IDXHDR start = %llu, length = %llu\n", ui.start, ui.length);

		hexdump (buf, ui.start, ui.length);
		free(buf);
	}

	if (CHM_RESOLVE_SUCCESS == chm_resolve_object(chm_file, STRINGS, &ui)) {
		LONGINT64 gotLen;
		UChar *buf, *p;
		UInt32 i, done, chunk_done, len;

		buf = malloc(ui.length);
		if (!buf) {
			fflush(stdout);
			fprintf(stderr,"error: malloc failed\n");
			exit(1);
		}
		gotLen = chm_retrieve_object(chm_file, &ui, buf, 0, ui.length);
		if (gotLen != ui.length) {
			fflush(stdout);
			fprintf(stderr,"error: gotLen != ui.length\n");
			exit(1);
		}
		// FIXME gotLen != ui.length
		printf ("/#STRINGS start = %llu, length = %llu\n", ui.start, ui.length);

		i = 0;
		done = 0;
		p = buf;
		while (done < ui.length) {
			len = strlen(p);
			if (len)
				printf("STRINGS %d offset = %lu, len = %d %.*s\n", i, p - buf, len, len, p);
			else {
				if (i > 0)
					break;
				printf("STRINGS %d offset = %lu, len = 0\n", i, p - buf);
			}
			p += len + 1;
			done += len + 1;
			i++;
		}
// 		hexdump (buf, ui.start, ui.length);
		free(buf);
	}

	if (CHM_RESOLVE_SUCCESS == chm_resolve_object(chm_file, TOPICS, &ui)) {
		LONGINT64 gotLen;
		UChar *buf;
		UInt32 done, i;
		struct chmcTopicEntry {
			UInt32 off_tocidx;
			UInt32 off_strings;
			UInt32 off_urltbl;
			UInt16 in_content;
			UInt16 unknown;
		};
		struct chmcTopicEntry *entry;

		buf = malloc(ui.length);
		if (!buf) {
			fflush(stdout);
			fprintf(stderr,"error: malloc failed\n");
			exit(1);
		}
		gotLen = chm_retrieve_object(chm_file, &ui, buf, 0, ui.length);
		if (gotLen != ui.length) {
			fflush(stdout);
			fprintf(stderr,"error: gotLen != ui.length\n");
			exit(1);
		}
		// FIXME gotLen != ui.length
		printf ("/#TOPICS start = %llu, length = %llu\n", ui.start, ui.length);

		done = 0;
		entry = buf;
		i = 0;
		while (done < ui.length) {
			printf( "TOPICS %d off_tocidx = %d\n", i, entry->off_tocidx );
			printf( "TOPICS %d off_strings = %d\n", i, entry->off_strings );
			printf( "TOPICS %d off_urltbl = %d\n", i, entry->off_urltbl );
			printf( "TOPICS %d in_content = %d\n", i, entry->in_content );
			printf( "TOPICS %d unknown = %d\n", i, entry->unknown );
			done += sizeof(struct chmcTopicEntry);
			entry++;
			i++;
		}

		hexdump (buf, ui.start, ui.length);

		free(buf);
	}


	if (CHM_RESOLVE_SUCCESS == chm_resolve_object(chm_file, URLTBL, &ui)) {
		LONGINT64 gotLen;
		UChar *buf;
		UInt32 done, i;
		struct chmcUrlTblEntry {
			UInt32 unknown;
			UInt32 topics_index;
			UInt32 off_urlstr;
		};
		struct chmcUrlTbl {
			struct chmcUrlTblEntry entry[341];
			UInt32 blocklen;
		};
		struct chmcUrlTbl *urlTbl;

		buf = malloc(ui.length);
		if (!buf) {
			fflush(stdout);
			fprintf(stderr,"error: malloc failed\n");
			exit(1);
		}
		gotLen = chm_retrieve_object(chm_file, &ui, buf, 0, ui.length);
		if (gotLen != ui.length) {
			fflush(stdout);
			fprintf(stderr,"error: gotLen != ui.length\n");
			exit(1);
		}
		// FIXME gotLen != ui.length
		printf ("/#URLTBL start = %llu, length = %llu\n", ui.start, ui.length);

		i = 0;
		urlTbl = buf;
		done = 0;
		for (i=0; i<341; i++) {
			printf("URLTBL %d unknown 0x%08lx\n", i, urlTbl->entry[i].unknown);
			printf("URLTBL %d topics_index %lu\n", i, urlTbl->entry[i].topics_index);
			printf("URLTBL %d off_urlstr %lu\n", i, urlTbl->entry[i].off_urlstr);
			done += sizeof(struct chmcUrlTblEntry);
			if (done >= ui.length)
				break;
		}
// 		printf("URLTBL blocklen %lu\n", urlTbl->blocklen);

		hexdump (buf, ui.start, ui.length);

		free(buf);
	}

	if (CHM_RESOLVE_SUCCESS == chm_resolve_object(chm_file, TOCIDX, &ui)) {
		LONGINT64 gotLen;
		UChar *buf;
		UInt32 done, i;

		buf = malloc(ui.length);
		if (!buf) {
			fflush(stdout);
			fprintf(stderr,"error: malloc failed\n");
			exit(1);
		}
		gotLen = chm_retrieve_object(chm_file, &ui, buf, 0, ui.length);
		if (gotLen != ui.length) {
			fflush(stdout);
			fprintf(stderr,"error: gotLen != ui.length\n");
			exit(1);
		}
		// FIXME gotLen != ui.length
		printf ("/#TOCIDX start = %llu, length = %llu\n", ui.start, ui.length);

		hexdump (buf, ui.start, ui.length);

		free(buf);
	}

	if (system_buf)
		free(system_buf);

	if (sect1_xform_buf)
		free(sect1_xform_buf);

	if (ctrl_data_buf)
		free(ctrl_data_buf);

	if (reset_table_buf)
		free(reset_table_buf);

	close (fd);

	chm_close (chm_file);

	exit(0);
}


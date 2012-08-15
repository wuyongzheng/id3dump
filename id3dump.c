#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* from utf-util.c. don't bother to have a header file */
int decode_utf8 (const unsigned char *instr, unsigned int *punival);
int encode_utf8 (char *outstr, unsigned int unival);
int decode_utf16ne (const unsigned short *instr, unsigned int *punival);

struct id3v1_header {
	char magic   [3];
	char title   [30];
	char artist  [30];
	char album   [30];
	char year    [4];
	char comment [30]; // last byte may be track number
	unsigned char genre;
};

struct id3v2_header {
	char magic [3];
	unsigned char major_version;
	unsigned char minor_version;
	unsigned char flags;
	char size [4];
};

struct id3v2_ext_header {
	char size [4];
	char flagno;
	char flags;
};

struct id3v2_frame_header {
	char id [4];
	char size [4];
	unsigned char flags [2];
};

static const char *const id3v1_genres[] = {
	"Blues", "Classic Rock", "Country", "Dance",
	"Disco", "Funk", "Grunge", "Hip-Hop",
	"Jazz", "Metal", "New Age", "Oldies",
	"Other", "Pop", "R&B", "Rap",
	"Reggae", "Rock", "Techno", "Industrial",
	"Alternative", "Ska", "Death Metal", "Pranks",
	"Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop",
	"Vocal", "Jazz+Funk", "Fusion", "Trance",
	"Classical", "Instrumental", "Acid", "House",
	"Game", "Sound Clip", "Gospel", "Noise",
	"AlternRock", "Bass", "Soul", "Punk",
	"Space", "Meditative", "Instrumental Pop", "Instrumental Rock",
	"Ethnic", "Gothic", "Darkwave", "Techno-Industrial",
	"Electronic", "Pop-Folk", "Eurodance", "Dream",
	"Southern Rock", "Comedy", "Cult", "Gangsta",
	"Top 40", "Christian Rap", "Pop/Funk", "Jungle",
	"Native American", "Cabaret", "New Wave", "Psychadelic",
	"Rave", "Showtunes", "Trailer", "Lo-Fi",
	"Tribal", "Acid Punk", "Acid Jazz", "Polka",
	"Retro", "Musical", "Rock & Roll", "Hard Rock",
	"Folk", "Folk-Rock", "National Folk", "Swing",
	"Fast Fusion", "Bebob", "Latin", "Revival",
	"Celtic", "Bluegrass", "Avantgarde", "Gothic Rock",
	"Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock",
	"Big Band", "Chorus", "Easy Listening", "Acoustic",
	"Humour", "Speech", "Chanson", "Opera",
	"Chamber Music", "Sonata", "Symphony", "Booty Bass",
	"Primus", "Porn Groove", "Satire", "Slow Jam",
	"Club", "Tango", "Samba", "Folklore",
	"Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle",
	"Duet", "Punk Rock", "Drum Solo", "A capella",
	"Euro-House", "Dance Hall"};

unsigned int get_synchunsafe (const char *synchsafe)
{
	return (unsigned char)synchsafe[0] << 21 |
		(unsigned char)synchsafe[1] << 14 |
		(unsigned char)synchsafe[2] << 7 |
		(unsigned char)synchsafe[3];
}

void dump_v1 (struct id3v1_header *header)
{
	char str[32];

	str[30] = '\0';

	if (header->title[0] != '\0') {
		strncpy(str, header->title, 30);
		printf("Title:   \"%s\"\n", str);
	}

	if (header->artist[0] != '\0') {
		strncpy(str, header->artist, 30);
		printf("Artist:  \"%s\"\n", str);
	}

	if (header->album[0] != '\0') {
		strncpy(str, header->album, 30);
		printf("Album:   \"%s\"\n", str);
	}

	if (header->year[0] != '\0') {
		strncpy(str, header->year, 4);
		str[4] = '\0';
		printf("Year:    %s\n", str);
	}

	if (header->comment[0] != '\0') {
		strncpy(str, header->comment, 30);
		printf("Comment: \"%s\"\n", str);
	}

	if (header->comment[28] == '\0') {
		printf("Track:   %d\n", (unsigned char)header->comment[29]);
	}

	if (header->genre < sizeof(id3v1_genres) / sizeof(id3v1_genres[0])) {
		printf("Genre:   %d -> %s\n", header->genre, id3v1_genres[header->genre]);
	}
}

void print_utf16ne_as_utf8 (const unsigned short *utf16, int length)
{
	while (length > 0) {
		unsigned int unival;
		char buff[16];
		int adv = decode_utf16ne(utf16, &unival);
		if (adv == 0) {
			utf16 ++;
			length --;
			continue;
		}
		if (unival == 0)
			return;
		fwrite(buff, encode_utf8(buff, unival), 1, stdout);
		utf16 += adv;
		length -= adv;
	}
}

void dump_v2_frame (const struct id3v2_frame_header *frame)
{
	int i, size = get_synchunsafe(frame->size);

	putchar(frame->id[0]);
	putchar(frame->id[1]);
	putchar(frame->id[2]);
	putchar(frame->id[3]);
	putchar(' ');
	printf("%4u ", size);
	for (i = 0; i < 8; i ++)
		printf("%c", frame->flags[0] & (1 << (7-i)) ? '1' : '0');
	printf(" ");
	for (i = 0; i < 8; i ++)
		printf("%c", frame->flags[1] & (1 << (7-i)) ? '1' : '0');
	printf(" ");
	for (i = 0; i < size && i < 32; i ++)
		printf("%02x", ((unsigned char *)(frame + 1))[i]);
	printf("\n");

	if (*(unsigned int *)frame->id == 0x32544954 || *(unsigned int *)frame->id == 0x31455054) {
		if (((unsigned char *)(frame + 1))[0] == 0) {
			printf("ATP-TXT2\t%d\t", *(unsigned int *)frame->id == 0x32544954);
			fwrite(((unsigned char *)(frame + 1)) + 1, size - 1, 1, stdout);
			printf("\n");
		} else if (((unsigned char *)(frame + 1))[0] == 1 &&
				((unsigned char *)(frame + 1))[1] == 0xff &&
				((unsigned char *)(frame + 1))[2] == 0xfe) {
			printf("ATP-TXT2\t%d\t", *(unsigned int *)frame->id == 0x32544954);
			print_utf16ne_as_utf8((const unsigned short *)(((unsigned char *)(frame + 1)) + 3), (size - 3) / 2);
			printf("\n");
		} else {
			printf("ATP-TXT2\t%d\t??\n", *(unsigned int *)frame->id == 0x32544954);
		}
	}
}

void dump_v2 (const struct id3v2_header *header, const char *data, unsigned data_size)
{
	printf("Version: %d.%d\n", header->major_version, header->minor_version);

	printf("Flags: {");
	if (header->flags & 128)
		printf(" Unsynchronisation");
	if (header->flags & 64)
		printf(" Extended-header");
	if (header->flags & 32)
		printf(" Experimental-indicator");
	if (header->flags & 16)
		printf(" Footer-present");
	printf("}\n");

	printf("Size: 10 + %d + %d + %d = %d\n",
			header->flags & 64 ? get_synchunsafe(((const struct id3v2_ext_header *)data)->size) : 0,
			header->flags & 64 ? get_synchunsafe(header->size) - get_synchunsafe(((const struct id3v2_ext_header *)data)->size) : get_synchunsafe(header->size),
			header->flags & 16 ? 10 : 0,
			10 + get_synchunsafe(header->size) + (header->flags & 16 ? 10 : 0));

	if (header->flags & 64) {
		const struct id3v2_ext_header *ext = (const struct id3v2_ext_header *)data;
		if (get_synchunsafe(ext->size) + 10 > data_size) {
			printf("error: extended header larger than id3v2 data size\n");
			return;
		}
		// dump ext
		data += get_synchunsafe(ext->size);
		data_size -= get_synchunsafe(ext->size);
	}

	while (1) {
		const struct id3v2_frame_header *frame = (const struct id3v2_frame_header *)data;
		if (frame->id[0] == 0) {
			printf("+PAD %3d\n", data_size);
			break;
		}
		if (data_size < 10 + get_synchunsafe(frame->size)) {
			printf("warning: last frame overflows id3v2 data size.\n");
			break;
		}
		dump_v2_frame(frame);
		data += 10 + get_synchunsafe(frame->size);
		data_size -= 10 + get_synchunsafe(frame->size);
		if (data_size == 0)
			break;
	}
}

void dump (const char *filename)
{
	FILE *fp;
	unsigned int file_size;
	struct id3v1_header header1;
	struct id3v2_header header2;
	unsigned int data_size;
	char *data;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		perror(filename);
		goto out;
	}

	if (fseek(fp, 0, SEEK_END) == -1) {
		perror(filename);
		goto out;
	}

	file_size = ftell(fp);
	if (file_size < 10) {
		fprintf(stderr, "%s: empty file or size too small to hold an id3 tag\n", filename);
		goto out;
	}

	printf("%s:\n", filename);

	/* id3v1 in the end */
	if (file_size >= 128) {
		fseek(fp, -128, SEEK_END);
		if (fread(&header1, 128, 1, fp) != 1) {
			perror(filename);
			goto out;
		}
		if (memcmp(header1.magic, "TAG", 3) == 0) {
			printf("ID3v1 tag found at %u-%u (%u):\n", file_size - 128, file_size, 128);
			dump_v1(&header1);
		}
	}

	/* id3v2 in the front */
	fseek(fp, 0, SEEK_SET);
	if (fread(&header2, 10, 1, fp) != 1) {
		perror(filename);
		goto out;
	}

	if (memcmp(header2.magic, "ID3", 3) == 0) {
		data_size = get_synchunsafe(header2.size);
		if (data_size < file_size) {
			data = (char *)malloc(data_size);
			if (data == NULL) {
				fprintf(stderr, "Can't allocate %d bytes memory\n", data_size);
				goto out;
			}
			if (fread(data, data_size, 1, fp) != 1) {
				perror(filename);
				goto out;
			}
			printf("ID3v2 tag found at 0-%u (%u):\n",
					data_size + (header2.flags & 16 ? 20 : 10),
					data_size + (header2.flags & 16 ? 20 : 10));
			dump_v2(&header2, data, data_size);
		}
	}

	/* id3v2 in the end */
	fseek(fp, -10, SEEK_END);
	if (fread(&header2, 10, 1, fp) != 1) {
		perror(filename);
		goto out;
	}

	/* try to find id3v2 immidiately before id3v1 */
	if (file_size > 20 + 128 && memcmp(header2.magic, "3DI", 3) != 0) {
		fseek(fp, -128 - 10, SEEK_END);
		if (fread(&header2, 10, 1, fp) != 1) {
			perror(filename);
			goto out;
		}
	}

	if (memcmp(header2.magic, "3DI", 3) == 0) {
		data_size = get_synchunsafe(header2.size);
		if (data_size < file_size) {
			data = (char *)malloc(data_size);
			if (data == NULL) {
				fprintf(stderr, "Can't allocate %d memory\n", data_size);
				goto out;
			}
			fseek(fp, -data_size - 10, SEEK_CUR);
			if (fread(data, data_size, 1, fp) != 1) {
				perror(filename);
				goto out;
			}
			printf("ID3v2 tag found at %u-%u (%u):\n",
					(unsigned int)ftell(fp) - data_size - 10,
					(unsigned int)ftell(fp) + 10,
					data_size + 20);
			dump_v2(&header2, data, data_size);
		}
	}

out:
	if (fp != NULL)
		fclose(fp);
}

int main (int argc, char *argv[])
{
	int i;

	if (argc <= 1) {
		printf("Usage: id3dump file ...\n"
				"Example: id3dump abc.mp3 def.mp3 ghi.mp3\n");
		return 0;
	}

	for (i = 1; i < argc; i ++)
		dump(argv[i]);

	return 0;
}

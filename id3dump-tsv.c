#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
	char flags;
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
		printf("%s", str);
	}
	printf("\t");

	if (header->artist[0] != '\0') {
		strncpy(str, header->artist, 30);
		printf("%s", str);
	}
	printf("\t");

	if (header->album[0] != '\0') {
		strncpy(str, header->album, 30);
		printf("%s", str);
	}
	printf("\t");

	if (header->year[0] != '\0') {
		strncpy(str, header->year, 4);
		str[4] = '\0';
		printf("%s", str);
	}
	printf("\t");

	if (header->comment[0] != '\0') {
		strncpy(str, header->comment, 30);
		printf("%s", str);
	}
	printf("\t");

	if (header->comment[28] == '\0') {
		printf("%d", (unsigned char)header->comment[29]);
	}
	printf("\t");

	if (header->genre < sizeof(id3v1_genres) / sizeof(id3v1_genres[0])) {
		printf("%d", header->genre);
	}
	printf("\t");
}

void dump_v2_frame (const struct id3v2_frame_header *frame)
{
	putchar(frame->id[0]);
	putchar(frame->id[1]);
	putchar(frame->id[2]);
	putchar(frame->id[3]);
	putchar(' ');
	printf("%3u ", get_synchunsafe(frame->size));
	// todo
	printf("\n");
}

void dump_v2 (const struct id3v2_header *header, const char *data, unsigned data_size)
{
	printf("id3v2 Version: %d.%d", header->major_version, header->minor_version);
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

	printf("%s\t", filename);

	/* id3v1 in the end */
	if (file_size >= 128) {
		fseek(fp, -128, SEEK_END);
		if (fread(&header1, 128, 1, fp) != 1) {
			perror(filename);
			goto out;
		}
		if (memcmp(header1.magic, "TAG", 3) == 0) {
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
			dump_v2(&header2, data, data_size);
		}
	}

	printf("\n");
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

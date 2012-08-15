
int decode_utf8 (const unsigned char *instr, unsigned int *punival)
{
	const unsigned char *uinstr = instr;
	if (uinstr[0] < 0x80) {
		*punival = uinstr[0];
		return 1;
	}
	if (uinstr[0] < 0xc0)
		return 0;
	if (uinstr[0] < 0xe0) {
		if ((uinstr[1] & 0xc0) != 0x80)
			return 0;
		*punival = ((uinstr[0] & 0x1f) << 6) | (uinstr[1] & 0x3f);
		return 2;
	}
	if (uinstr[0] < 0xf0) {
		if ((uinstr[1] & 0xc0) != 0x80 || (uinstr[2] & 0xc0) != 0x80)
			return 0;
		*punival = ((uinstr[0] & 0x0f) << 12) |
			((uinstr[1] & 0x3f) << 6) |
			(uinstr[2] & 0x3f);
		return 3;
	}
	if (uinstr[0] < 0xf8) {
		if ((uinstr[1] & 0xc0) != 0x80 || (uinstr[2] & 0xc0) != 0x80 ||
				(uinstr[3] & 0xc0) != 0x80)
			return 0;
		*punival = ((uinstr[0] & 0x07) << 18) |
			((uinstr[1] & 0x3f) << 12) |
			((uinstr[2] & 0x3f) << 6) |
			(uinstr[3] & 0x3f);
		return 4;
	}
	return 0;
}

/* given a unicode character "unival", generate the UTF-8
 * string in "outstr", return the length of it.
 * The output string is NOT null terminated. */
int encode_utf8 (char *outstr, unsigned int unival)
{
	if (unival < 0x80) {
		outstr[0] = unival;
		return 1;
	}
	if (unival < 0x800) {
		outstr[0] = 0xc0 | (unival >> 6);
		outstr[1] = 0x80 | (unival & 0x3f);
		return 2;
	}
	if (unival < 0x10000) {
		outstr[0] = 0xe0 | (unival >> 12);
		outstr[1] = 0x80 | ((unival >> 6) & 0x3f);
		outstr[2] = 0x80 | (unival & 0x3f);
		return 3;
	}
	if (unival < 0x110000) {
		outstr[0] = 0xf0 | (unival >> 18);
		outstr[1] = 0x80 | ((unival >> 12) & 0x3f);
		outstr[2] = 0x80 | ((unival >> 6) & 0x3f);
		outstr[3] = 0x80 | (unival & 0x3f);
		return 4;
	}
	return 0;
}

/* utf-16 native endian */
int decode_utf16ne (const unsigned short *instr, unsigned int *punival)
{
	if (instr[0] < 0xD800 || instr[0] > 0xDFFF) {
		*punival = instr[0];
		return 1;
	} else if (instr[0] < 0xDC00) {
		*punival = (((instr[0] - 0xD800)) << 10) + (instr[1] - 0xDC00);
		return 2;
	} else {
		return 0;
	}
}

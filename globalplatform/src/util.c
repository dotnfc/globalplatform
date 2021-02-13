/*  Copyright (c) 2009, Karsten Ohme
 *  This file is part of GlobalPlatform.
 *
 *  GlobalPlatform is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GlobalPlatform is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with GlobalPlatform.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "util.h"
#include <string.h>

/**
 * \param b [in] The Le BYTE.
 * \return Value of b.
 */
DWORD convert_byte(BYTE b) {
	return (b == 0) ? 256 : b;
}

/**
 * \param buf [in] The buffer.
 * \param offset [in] The offset in the buffer.
 * \return the unsigned int value.
 */
DWORD get_int(PBYTE buf, DWORD offset) {
	return ((buf[offset] & 0xFF) << 24) | ((buf[offset+1] & 0xFF) << 16) | ((buf[offset+2] & 0xFF) << 8) | (buf[offset+3] & 0xFF);
}

/**
 * \param buf [in] The buffer.
 * \param offset [in] The offset in the buffer.
 * \param numLength The length of the number.
 * \return the unsigned int value.
 */
DWORD get_number(PBYTE buf, DWORD offset, BYTE numLength) {
	if (numLength == 1) {
		return buf[offset];
	}
	if (numLength == 2) {
		return get_short(buf, offset);
	}
	if (numLength == 3) {
		return buf[offset] << 16 | get_short(buf, offset+1);
	}
	if (numLength == 4) {
		return get_int(buf, offset);
	}
	return 0;
}

/**
 * \param buf [in] The buffer.
 * \param offset [in] The offset in the buffer.
 * \return the unsigned short int value.
 */
DWORD get_short(PBYTE buf, DWORD offset) {
	return ((buf[offset] & 0xFF) << 8) | (buf[offset+1] & 0xFF);
}

/**
 * The tag must be coded on at most two octet and the length must be < 65535.
 * \param buffer [in] The buffer.
 * \param length [in] The length of the buffer.
 * \param *tlv [out] the returned TLV struct.
 * \return -1 in case of error, consumed length otherwise.
 */
LONG read_TLV(PBYTE buffer, DWORD length, TLV *tlv) {
	LONG result;
	DWORD offset = 0;
	DWORD numLengthOctets;
	BYTE l1, l2;
	BYTE tag1, tag2;
	int i;
	tag1 = buffer[offset++];
	// if bit 5 to 1 = 11111 not a single octet
	if ((tag1 & 31) == 31) {
		// we support only tags with two octets, so bit 8 must be not set to
		// mark this octet as last octet
		if (length < offset+1) {
			result = -1;
			goto end;
		}
		tag2 = buffer[offset++];
		if ((tag2 & 0x80) != 0) {
			result = -1;
			goto end;
		}
		tlv->tag = ((tag1 << 8) | tag2);
	} else {
		tlv->tag = tag1;
	}

	if (length < offset+1) {
		result = -1;
		goto end;
	}
	l1 = buffer[offset++];
	if ((l1 & 0x80) != 0) {
		if (length < offset+1) {
			result = -1;
			goto end;
		}
		l2 = buffer[offset++];
		numLengthOctets = l1 & 127;
		// we support only 65535
		if (numLengthOctets > 2) {
			result = -1;
			goto end;
		}
		tlv->length = l2 << 8 * (numLengthOctets - 1);
		for (i = numLengthOctets - 1; i > 0; i--) {
			tlv->length |= (buffer[offset++] << 8) * (i - 1);
		}
	} else {
		tlv->length = l1;
	}

	memmove(tlv->value, buffer+offset, tlv->length);
	offset += tlv->length;
	result = offset;
	tlv->tlvLength = result;
end:
	return result;
}

LONG parse_apdu_case(PBYTE apduCommand, DWORD apduCommandLength, PBYTE caseAPDU, PDWORD lc, PDWORD le) {
	*le = 0;
	*lc = 0;
	// Determine which type of Exchange between the reader
	if (apduCommandLength == 4) {
		// Case 1 short
		*caseAPDU = 1;
	} else if (apduCommandLength == 5) {
		// Case 2 short
		*caseAPDU = 2;
		*le = apduCommand[4];
	} else {
		*lc = apduCommand[4];
		if (*lc + 5 == apduCommandLength) {
			// Case 3 short
			*caseAPDU = 3;
		} else if (*lc + 5 + 1 == apduCommandLength) {
			// Case 4 short
			*caseAPDU = 4;
			*le = apduCommand[apduCommandLength - 1];
		} else {
			if (apduCommandLength >= 7) {
				if (apduCommand[4] != 0) {
					return -1;
				}
				if (apduCommandLength == 7) {
					// Case 2 extended
					*caseAPDU = 0x2e;
					*le = get_short(apduCommand, 5);
				}
				else {
					*lc = get_short(apduCommand, 5);
					if (*lc + 7 == apduCommandLength) {
						// Case 3 extended
						*caseAPDU = 0x3e;
					}
					else if (*lc + 9 == apduCommandLength) {
						// Case 4 extended
						*caseAPDU = 0x4e;
						*le = get_short(apduCommand, apduCommandLength - 2);
					}
					else {
						return -1;
					}
				}
			} else {
				return -1;
			}
		}
	}
	return 0;
}

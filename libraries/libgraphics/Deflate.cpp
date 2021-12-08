/*
	This file is part of duckOS.

	duckOS is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	duckOS is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include "Deflate.h"
#include <stdlib.h>

unsigned int read_bits(DEFLATE* def, size_t num_bits) {
	unsigned int ret = 0;
	for(size_t i = 0; i < num_bits; i++) {
		if(def->bit_pos == 8) { //We've read all 8 bits from the current byte, read a new one
			def->bit_buf = def->read(def->arg);
			def->bit_pos = 0;
		}
		ret |= (def->bit_buf & 0x1u) << i;
		def->bit_buf >>= 1;
		def->bit_pos++;
	}
	return ret;
}

void create_huffman(const uint8_t lengths[], uint32_t size, huffman* huff) {
	//Zero out code length counts
	for(size_t i = 0; i < 16; i++)
		huff->counts[i] = 0;

	//Count the number of symbols with each code length
	for(size_t i = 0; i < size; i++)
		huff->counts[lengths[i]]++;

	huff->counts[0] = 0;

	//Figure out the starting indexes into the final symbol array for each code length
	uint32_t count = 0;
	uint16_t indexes[16];
	for(uint16_t i = 0; i < 16; i++) {
		indexes[i] = count;
		count += huff->counts[i];
	}

	//Create the final symbol array which we can index into using codes
	for (uint16_t i = 0; i < size; i++) {
		if (lengths[i])
			huff->symbols[indexes[lengths[i]]++] = i;
	}
}

uint16_t huffman_decode(DEFLATE* def, huffman* huff) {
	int count = 0;
	int cur = 0;
	for(int i = 1; cur >= 0; i++) {
		cur = read_bits(def, 1) | (cur << 1);
		count += huff->counts[i];
		cur -= huff->counts[i];
	}
	return huff->symbols[count + cur];
}

void def_write(DEFLATE* def, uint8_t byte) {
	if(def->frame_pointer == 0x8000)
		def->frame_pointer = 0;
	def->reading_frame[def->frame_pointer++] = byte;
	def->write(byte, def->arg);
}

void inflate(DEFLATE* def, huffman* len_huff, huffman* dist_huff) {
	//The lengths corresponding to symbols > 256
	static const uint16_t lengths[] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 };
	//The number of extra bits to read and add to the lengths corresponding to symbols > 256
	static const uint16_t lengths_extrabits[] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };
	//The distances for decoded symbols after the symbols > 256
	static const uint16_t distances[] = { 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 };
	//The number of extra bits to read and add to the distances above
	static const uint16_t distances_extrabits[] = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };

	while(1) {
		uint16_t sym = huffman_decode(def, len_huff);
		if(sym < 256) {
			def_write(def, sym);
		} else {
			if(sym == 256)
				break;
			uint16_t length = read_bits(def, lengths_extrabits[sym - 257]) + lengths[sym - 257];
			uint16_t distance_index = huffman_decode(def, dist_huff);
			uint16_t distance = distances[distance_index] + read_bits(def, distances_extrabits[distance_index]);
			//If sym > 256, write length bytes to the output from distance bytes behind the current position
			for(uint16_t i = 0; i < length; i++) {
				uint8_t byte = def->reading_frame[(def->frame_pointer - distance) % 0x8000];
				def_write(def, byte);
			}
		}
	}
}

void inflate_dynamic(DEFLATE* def) {
	//The code lengths for the dynamic huffman alphabet
	static const uint8_t codelen_alphabet[] = {
			16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
	};

	uint16_t hlit = read_bits(def, 5) + 257; //# of Literal/Length codes
	uint8_t hdist = read_bits(def, 5) + 1; //# of Distance codes
	uint8_t hclen = read_bits(def, 4) + 4; //# of Code Length codes

	//Get the code lengths for each entry in the code length alphabet
	uint8_t alphabet_codelens[19] = {0};
	for(uint32_t i = 0; i < hclen; i++)
		alphabet_codelens[codelen_alphabet[i]] = read_bits(def, 3);

	//Build the huffman code table for the literal/length alphabet
	huffman codes_huff;
	create_huffman(alphabet_codelens, 19, &codes_huff);

	//Get the lengths for the literal/length alphabet
	uint16_t i = 0;
	uint8_t lengths[320];
	while(i < hdist + hlit) {
		uint16_t sym = huffman_decode(def, &codes_huff);
		if(sym < 16) { //0-15: Represent code lengths of 0 - 15
			lengths[i++] = sym;
		} else {
			uint8_t num_repeats = 0;
			uint8_t to_repeat = 0;
			switch(sym) {
				case 16: //16: Copy the previous code length 3 - 6 times.
					to_repeat = lengths[i - 1];
					num_repeats = read_bits(def, 2) + 3;
					break;
				case 17: //17: Repeat a code length of 0 for 3 - 10 times.
					num_repeats = read_bits(def, 3) + 3;
					break;
				case 18: //18: Repeat a code length of 0 for 11 - 138 times
					num_repeats = read_bits(def, 7) + 11;
					break;
				default:
					//Shouldn't happen...
					break;
			}
			for(uint8_t j = 0; j < num_repeats; j++)
				lengths[i++] = to_repeat;
		}
	}

	//Build the length/dist tables
	huffman length_huff;
	create_huffman(lengths, hlit, &length_huff);
	huffman dist_huff;
	create_huffman(lengths + hlit, hdist, &dist_huff);

	inflate(def, &length_huff, &dist_huff);
}

int inflate_uncompressed(DEFLATE* def) {
	def->bit_buf = 0;
	def->bit_pos = 8;

	uint16_t len_a = def->read(def->arg);
	uint16_t len_b = def->read(def->arg);
	uint16_t len = len_a | (len_b << 8);

	uint16_t lencomp_a = def->read(def->arg);
	uint16_t lencomp_b = def->read(def->arg);
	uint16_t lencomp = lencomp_a | (lencomp_b << 8);

	//Make sure lencomp is actually the ones complement of length
	if((lencomp & 0xFFFF) != (~len & 0xFFFF))
		return -1;

	for(uint16_t i = 0; i < len; i++)
		def_write(def, def->read(def->arg));

	return 0;
}

huffman fixed_len_huff;
huffman fixed_dist_huff;
int made_fixed = 0;

int decompress(DEFLATE* def) {
	def->bit_pos = 8;
	def->frame_pointer = 0;
	def->bit_buf = 0;

	uint8_t bfinal = 0;

	//Make the huffman code tables for fixed code inflation if they don't exist already
	if(!made_fixed) {
		made_fixed = 1;
		uint8_t len_lengths[288];
		for(uint16_t i = 0; i < 288; i++) {
			if(i < 144 || i >= 280)
				len_lengths[i] = 8;
			else if(i < 256)
				len_lengths[i] = 9;
			else if(i < 280)
				len_lengths[i] = 7;
		}
		create_huffman(len_lengths, 288, &fixed_len_huff);

		uint8_t dist_lengths[30];
		for(uint8_t i = 0; i < 30; i++)
			dist_lengths[i] = 5;
		create_huffman(dist_lengths, 30, &fixed_dist_huff);
	}

	while(!bfinal) {
		bfinal = read_bits(def, 1);
		uint8_t btype = read_bits(def, 2);
		switch(btype) {
			case 0b00:
				if(inflate_uncompressed(def) < 0)
					return -1;
				break;
			case 0b01:
				inflate(def, &fixed_len_huff, &fixed_dist_huff);
				break;
			case 0b10:
				inflate_dynamic(def);
				break;
			default:
				fprintf(stderr, "deflate: Invalid btype 0b11\n");
				return -1;
		}
	}

	return 0;
}
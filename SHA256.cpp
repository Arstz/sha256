#include <iostream>
#include "SHA256.h"
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <random>
#include <fstream>
#include <vector>

constexpr std::array<uint32_t, 64> SHA256::K;

SHA256::SHA256() : m_blocklen(0), m_bitlen(0) {
	m_state[0] = 0x6a09e667;
	m_state[1] = 0xbb67ae85;
	m_state[2] = 0x3c6ef372;
	m_state[3] = 0xa54ff53a;
	m_state[4] = 0x510e527f;
	m_state[5] = 0x9b05688c;
	m_state[6] = 0x1f83d9ab;
	m_state[7] = 0x5be0cd19;
}

void SHA256::update(const uint8_t* data, size_t length) {
	for (size_t i = 0; i < length; i++) {
		m_data[m_blocklen++] = data[i];
		if (m_blocklen == 64) {
			transform();

			// End of the block
			m_bitlen += 512;
			m_blocklen = 0;
		}
	}
}

void SHA256::update(const std::string& data) {
	update(reinterpret_cast<const uint8_t*> (data.c_str()), data.size());
}

std::array<uint8_t, 32> SHA256::digest() {
	std::array<uint8_t, 32> hash;

	pad();
	revert(hash);

	return hash;
}

uint32_t SHA256::rotr(uint32_t x, uint32_t n) {
	return (x >> n) | (x << (32 - n));
}

uint32_t SHA256::choose(uint32_t e, uint32_t f, uint32_t g) {
	return (e & f) ^ (~e & g);
}

uint32_t SHA256::majority(uint32_t a, uint32_t b, uint32_t c) {
	return (a & (b | c)) | (b & c);
}

uint32_t SHA256::sig0(uint32_t x) {
	return SHA256::rotr(x, 7) ^ SHA256::rotr(x, 18) ^ (x >> 3);
}

uint32_t SHA256::sig1(uint32_t x) {
	return SHA256::rotr(x, 17) ^ SHA256::rotr(x, 19) ^ (x >> 10);
}

void SHA256::transform() {
	uint32_t maj, xorA, ch, xorE, sum, newA, newE, m[64];
	uint32_t state[8];

	for (uint8_t i = 0, j = 0; i < 16; i++, j += 4) { // Split data in 32 bit blocks for the 16 first words
		m[i] = (m_data[j] << 24) | (m_data[j + 1] << 16) | (m_data[j + 2] << 8) | (m_data[j + 3]);
	}

	for (uint8_t k = 16; k < 64; k++) { // Remaining 48 blocks
		m[k] = SHA256::sig1(m[k - 2]) + m[k - 7] + SHA256::sig0(m[k - 15]) + m[k - 16];
	}

	for (uint8_t i = 0; i < 8; i++) {
		state[i] = m_state[i];
	}

	for (uint8_t i = 0; i < 64; i++) {
		maj = SHA256::majority(state[0], state[1], state[2]);
		xorA = SHA256::rotr(state[0], 2) ^ SHA256::rotr(state[0], 13) ^ SHA256::rotr(state[0], 22);

		ch = choose(state[4], state[5], state[6]);

		xorE = SHA256::rotr(state[4], 6) ^ SHA256::rotr(state[4], 11) ^ SHA256::rotr(state[4], 25);

		sum = m[i] + K[i] + state[7] + ch + xorE;
		newA = xorA + maj + sum;
		newE = state[3] + sum;

		state[7] = state[6];
		state[6] = state[5];
		state[5] = state[4];
		state[4] = newE;
		state[3] = state[2];
		state[2] = state[1];
		state[1] = state[0];
		state[0] = newA;
	}

	for (uint8_t i = 0; i < 8; i++) {
		m_state[i] += state[i];
	}
}

void SHA256::pad() {

	uint64_t i = m_blocklen;
	uint8_t end = m_blocklen < 56 ? 56 : 64;

	m_data[i++] = 0x80; // Append a bit 1
	while (i < end) {
		m_data[i++] = 0x00; // Pad with zeros
	}

	if (m_blocklen >= 56) {
		transform();
		memset(m_data, 0, 56);
	}

	// Append to the padding the total message's length in bits and transform.
	m_bitlen += m_blocklen * 8;
	m_data[63] = m_bitlen;
	m_data[62] = m_bitlen >> 8;
	m_data[61] = m_bitlen >> 16;
	m_data[60] = m_bitlen >> 24;
	m_data[59] = m_bitlen >> 32;
	m_data[58] = m_bitlen >> 40;
	m_data[57] = m_bitlen >> 48;
	m_data[56] = m_bitlen >> 56;
	transform();
}

void SHA256::revert(std::array<uint8_t, 32>& hash) {
	// SHA uses big endian byte ordering
	// Revert all bytes
	for (uint8_t i = 0; i < 4; i++) {
		for (uint8_t j = 0; j < 8; j++) {
			hash[i + (j * 4)] = (m_state[j] >> (24 - i * 8)) & 0x000000ff;
		}
	}
}

bool starts_with_32_zero_bits(const std::array<uint8_t, 32>& hash) {
	return hash[0] == 0 && hash[1] == 0 && hash[2] == 0 && hash[3] == 0;
}

// int main() {
// 	SHA256 sha256;
//
// 	const std::string original_message = "give my friend 2 bitcoins for a pizza";
// 	std::random_device rd;
// 	std::mt19937 rng(rd());
// 	std::uniform_int_distribution<unsigned int> dist(0, 255);
//
// 	std::array<uint8_t, 20> prefix{};
// 	std::string full_message;
// 	uint64_t attempts = 0;
//
// 	while (true) {
// 		// Generate random 20-byte prefix
// 		for (auto& byte : prefix) {
// 			byte = static_cast<uint8_t>(dist(rng));
// 		}
//
// 		// Create full message as std::string
// 		full_message.clear();
// 		full_message.reserve(20 + original_message.size());
//
// 		for (auto byte : prefix) {
// 			full_message += static_cast<char>(byte);
// 		}
// 		full_message += original_message;
//
// 		// Hash the message
// 		sha256.update(full_message);
// 		std::array<uint8_t, 32> hash = sha256.digest();
//
// 		if (starts_with_32_zero_bits(hash)) {
// 			std::cout << "Found matching prefix after " << attempts << " attempts:\n";
// 			for (auto byte : prefix) {
// 				std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
// 			}
// 			std::cout << std::dec << "\n";
// 			break;
// 		}
//
// 		++attempts;
// 		if (attempts % 1000000 == 0) {
// 			std::cout << "Attempts: " << attempts << "\n";
// 		}
// 	}
//
// 	return 0;
// }
int main() {
	std::ifstream file("kse.der", std::ios::binary);
	if (!file) {
		std::cerr << "Error while opening the file" << std::endl;
		return 1;
	}
	std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)),
								std::istreambuf_iterator<char>());
	SHA256 sha256;
	sha256.update(buffer.data(), buffer.size());
	std::array<uint8_t, 32> digest = sha256.digest();
	std::ostringstream oss;
	for (size_t i = 0; i < digest.size(); i++) {
		if (i > 0)
			oss << ":";
		oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
			<< static_cast<int>(digest[i]);
	}
	std::cout << oss.str() << std::endl;
	return 0;
}

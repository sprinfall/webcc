#include "webcc/base64.h"

// Modified from Boost.Beast (boost\beast\core\detail\base64.hpp)
// See the comments below for the original authors and copyrights.

//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

/*
   Portions from http://www.adp-gmbh.ch/cpp/common/base64.html
   Copyright notice:

   base64.cpp and base64.h

   Copyright (C) 2004-2008 Rene Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   Rene Nyffenegger rene.nyffenegger@adp-gmbh.ch
*/

namespace webcc {

namespace base64 {

// The Base 64 Alphabet.
static const char kAlphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// kInverse['z'] -> 51 which is the index of 'z' in kAlphabet.
static const char kInverse[] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  //   0-15
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  //  16-31
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,  //  32-47
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,  //  48-63
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,  //  64-79
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,  //  80-95
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,  //  96-111
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,  // 112-127
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 128-143
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 144-159
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 160-175
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 176-191
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 192-207
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 208-223
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 224-239
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1   // 240-255
};

// Return max chars needed to encode a base64 string.
inline std::size_t EncodedSize(std::size_t n) {
  return 4 * ((n + 2) / 3);
}

// Return max bytes needed to decode a base64 string.
inline std::size_t DecodedSize(std::size_t n) {
  // return 3 * n / 4;
  return n / 4 * 3;  // requires n&3==0, smaller
}

std::size_t Encode(const void* src, std::size_t len, void* dst) {
  char* out = static_cast<char*>(dst);
  const char* in = static_cast<const char*>(src);

  for (auto n = len / 3; n--;) {
    *out++ = kAlphabet[ (in[0] & 0xfc) >> 2];
    *out++ = kAlphabet[((in[0] & 0x03) << 4) + ((in[1] & 0xf0) >> 4)];
    *out++ = kAlphabet[((in[1] & 0x0f) << 2) + ((in[2] & 0xc0) >> 6)];
    *out++ = kAlphabet[  in[2] & 0x3f];
    in += 3;
  }

  switch (len % 3) {
    case 2:
      *out++ = kAlphabet[ (in[0] & 0xfc) >> 2];
      *out++ = kAlphabet[((in[0] & 0x03) << 4) + ((in[1] & 0xf0) >> 4)];
      *out++ = kAlphabet[ (in[1] & 0x0f) << 2];
      *out++ = '=';
      break;

    case 1:
      *out++ = kAlphabet[ (in[0] & 0xfc) >> 2];
      *out++ = kAlphabet[((in[0] & 0x03) << 4)];
      *out++ = '=';
      *out++ = '=';
      break;

    case 0:
      break;
  }

  return out - static_cast<char*>(dst);
}

using SizePair = std::pair<std::size_t, std::size_t>;

SizePair Decode(const char* src, std::size_t len, void* dst) {
  auto in = reinterpret_cast<const unsigned char*>(src);
  unsigned char* out = static_cast<unsigned char*>(dst);

  char c4[4];
  unsigned char c3[3];
  int i = 0;

  while (len-- && *in != '=') {
    char v = kInverse[*in];
    if(v == -1) { break; }
    ++in;
    c4[i] = v;

    if(++i == 4) {
      c3[0] =  (c4[0]        << 2) + ((c4[1] & 0x30) >> 4);
      c3[1] = ((c4[1] & 0xf) << 4) + ((c4[2] & 0x3c) >> 2);
      c3[2] = ((c4[2] & 0x3) << 6) +   c4[3];

      for (i = 0; i < 3; i++) {
        *out++ = c3[i];
      }

      i = 0;
    }
  }

  if (i > 0) {
    c3[0] = ( c4[0]        << 2) + ((c4[1] & 0x30) >> 4);
    c3[1] = ((c4[1] & 0xf) << 4) + ((c4[2] & 0x3c) >> 2);
    c3[2] = ((c4[2] & 0x3) << 6) +   c4[3];

    for (int j = 0; j < i - 1; j++) {
      *out++ = c3[j];
    }
  }

  return {out - static_cast<unsigned char*>(dst),
          in - reinterpret_cast<const unsigned char*>(src)};
}

}  // namespace base64

std::string Base64Encode(const std::uint8_t* data, std::size_t length) {
  std::string dst;
  dst.resize(base64::EncodedSize(length));
  dst.resize(base64::Encode(data, length, &dst[0]));
  return dst;
}

std::string Base64Encode(const std::string& input) {
  return Base64Encode(reinterpret_cast<const std::uint8_t*>(input.data()),
                      input.size());
}

std::string Base64Decode(const std::string& input) {
  std::string dst;
  dst.resize(base64::DecodedSize(input.size()));
  auto result = base64::Decode(input.data(), input.size(), &dst[0]);
  dst.resize(result.first);
  return dst;
}

}  // namespace webcc

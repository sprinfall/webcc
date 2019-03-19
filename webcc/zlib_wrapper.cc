#include "webcc/zlib_wrapper.h"

#include <utility>  // std::move

#include "zlib.h"

#include "webcc/logger.h"

namespace webcc {

// Modified from:
//   http://windrealm.org/tutorials/decompress-gzip-stream.php

bool Decompress(const std::string& input, std::string& output) {
  output.clear();

  if (input.empty()) {
    return true;
  }

  // Initialize the output buffer with the same size as the input.
  std::string buf;
  buf.resize(input.size());

  z_stream strm;
  strm.next_in = (Bytef*)input.c_str();
  strm.avail_in = (uInt)input.size();
  strm.total_out = 0;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;

  // About the windowBits paramter:
  //   (https://stackoverflow.com/a/1838702)
  //   (http://www.zlib.net/manual.html)
  // windowBits can also be greater than 15 for optional gzip decoding. Add 32
  // to windowBits to enable zlib and gzip decoding with automatic header
  // detection, or add 16 to decode only the gzip format (the zlib format will
  // return a Z_DATA_ERROR). If a gzip stream is being decoded, strm->adler is
  // a crc32 instead of an adler32.
  if (inflateInit2(&strm, (32 + MAX_WBITS)) != Z_OK) {
    return false;
  }

  while (true) {
    // Enlarge the output buffer if it's too small.
    if (strm.total_out >= buf.size()) {
      buf.resize(buf.size() + input.size() / 2);
    }

    strm.next_out = (Bytef*)(buf.c_str() + strm.total_out);
    strm.avail_out = (uInt)buf.size() - strm.total_out;

    // Inflate another chunk.
    //int err = inflate(&strm, Z_SYNC_FLUSH);
    int err = inflate(&strm, Z_FULL_FLUSH);

    if (err == Z_STREAM_END) {
      break;
    } else if (err != Z_OK) {
      inflateEnd(&strm);
      if (strm.msg != nullptr) {
        LOG_ERRO("zlib inflate error: %s", strm.msg);
      }
      return false;
    }
  }

  if (inflateEnd(&strm) != Z_OK) {
    return false;
  }

  // Remove the unused buffer.
  buf.erase(strm.total_out);

  // Move the buffer to the output.
  output = std::move(buf);

  return true;
}

}  // namespace webcc

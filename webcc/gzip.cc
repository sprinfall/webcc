#include "webcc/gzip.h"

#include <cassert>
#include <utility>  // std::move

#include "zlib.h"

#include "webcc/logger.h"

namespace webcc {
namespace gzip {

bool Compress(const std::string& input, std::string* output) {
  output->clear();

  if (input.empty()) {
    return true;
  }

  z_stream stream;
  stream.next_in = (Bytef*)input.data();
  stream.avail_in = (uInt)input.size();
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;

  int ret = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                         MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY);
  if (ret != Z_OK) {
    return false;
  }

  std::string buf;
  buf.resize(input.size() / 2);  // TODO

  // Run deflate() on input until output buffer is not full.
  do {
    stream.avail_out = (uInt)buf.size();
    stream.next_out = (Bytef*)buf.data();

    int err = deflate(&stream, Z_FINISH);

    if (err != Z_OK && err != Z_STREAM_END) {
      deflateEnd(&stream);
      if (stream.msg != nullptr) {
        LOG_ERRO("zlib deflate error: %s", stream.msg);
      }
      return false;
    }

    std::size_t size = buf.size() - stream.avail_out;
    output->append(buf.data(), size);

  } while (stream.avail_out == 0);

  if (deflateEnd(&stream) != Z_OK) {
    return false;
  }

  return true;
}

// Modified from:
//   http://windrealm.org/tutorials/decompress-gzip-stream.php
bool Decompress(const std::string& input, std::string* output) {
  output->clear();

  if (input.empty()) {
    return true;
  }

  // Initialize the output buffer with the same size as the input.
  std::string buf;
  buf.resize(input.size());

  z_stream stream;
  stream.next_in = (Bytef*)input.data();
  stream.avail_in = (uInt)input.size();
  stream.total_out = 0;
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;

  // About the windowBits paramter:
  //   (https://stackoverflow.com/a/1838702)
  //   (http://www.zlib.net/manual.html)
  // windowBits can also be greater than 15 for optional gzip decoding. Add 32
  // to windowBits to enable zlib and gzip decoding with automatic header
  // detection, or add 16 to decode only the gzip format (the zlib format will
  // return a Z_DATA_ERROR).
  if (inflateInit2(&stream, MAX_WBITS + 32) != Z_OK) {
    return false;
  }

  while (true) {
    // Enlarge the output buffer if it's too small.
    if (stream.total_out >= buf.size()) {
      buf.resize(buf.size() + input.size() / 2);
    }

    stream.next_out = (Bytef*)(buf.data() + stream.total_out);
    stream.avail_out = (uInt)buf.size() - stream.total_out;

    // Inflate another chunk.
    int err = inflate(&stream, Z_SYNC_FLUSH);

    if (err == Z_STREAM_END) {
      break;
    }

    if (err != Z_OK) {
      inflateEnd(&stream);
      if (stream.msg != nullptr) {
        LOG_ERRO("zlib inflate error: %s", stream.msg);
      }
      return false;
    }
  }

  if (inflateEnd(&stream) != Z_OK) {
    return false;
  }

  // Remove the unused part then move to the output
  buf.erase(stream.total_out);
  *output = std::move(buf);

  return true;
}

}  // namespace gzip
}  // namespace webcc

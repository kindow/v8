// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/ostreams.h"

#if V8_OS_WIN
#define snprintf sprintf_s
#endif

namespace v8 {
namespace internal {

OFStreamBase::OFStreamBase(FILE* f) : f_(f) {}


OFStreamBase::~OFStreamBase() {}


int OFStreamBase::sync() {
  std::fflush(f_);
  return 0;
}


OFStreamBase::int_type OFStreamBase::overflow(int_type c) {
  return (c != EOF) ? std::fputc(c, f_) : c;
}


std::streamsize OFStreamBase::xsputn(const char* s, std::streamsize n) {
  return static_cast<std::streamsize>(
      std::fwrite(s, 1, static_cast<size_t>(n), f_));
}


OFStream::OFStream(FILE* f) : std::ostream(nullptr), buf_(f) {
  DCHECK_NOT_NULL(f);
  rdbuf(&buf_);
}


OFStream::~OFStream() {}


namespace {

// Locale-independent predicates.
bool IsPrint(uint16_t c) { return 0x20 <= c && c <= 0x7e; }
bool IsSpace(uint16_t c) { return (0x9 <= c && c <= 0xd) || c == 0x20; }
bool IsOK(uint16_t c) { return (IsPrint(c) || IsSpace(c)) && c != '\\'; }


std::ostream& PrintUC16(std::ostream& os, uint16_t c, bool (*pred)(uint16_t)) {
  char buf[10];
  const char* format = pred(c) ? "%c" : (c <= 0xff) ? "\\x%02x" : "\\u%04x";
  snprintf(buf, sizeof(buf), format, c);
  return os << buf;
}

}  // namespace


std::ostream& operator<<(std::ostream& os, const AsReversiblyEscapedUC16& c) {
  return PrintUC16(os, c.value, IsOK);
}


std::ostream& operator<<(std::ostream& os, const AsEscapedUC16ForJSON& c) {
  if (c.value == '\n') return os << "\\n";
  if (c.value == '\r') return os << "\\r";
  if (c.value == '\"') return os << "\\\"";
  if (c.value == '\'') return os << "\\\'";
  return PrintUC16(os, c.value, IsOK);
}


std::ostream& operator<<(std::ostream& os, const AsUC16& c) {
  return PrintUC16(os, c.value, IsPrint);
}

}  // namespace internal
}  // namespace v8

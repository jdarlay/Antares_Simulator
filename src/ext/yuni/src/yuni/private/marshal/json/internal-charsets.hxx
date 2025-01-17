/*
** This file is part of libyuni, a cross-platform C++ framework (http://libyuni.org).
**
** This Source Code Form is subject to the terms of the Mozilla Public License
** v.2.0. If a copy of the MPL was not distributed with this file, You can
** obtain one at http://mozilla.org/MPL/2.0/.
**
** github: https://github.com/libyuni/libyuni/
** gitlab: https://gitlab.com/libyuni/libyuni/ (mirror)
*/
#ifndef __CPP_PARSER_CHARSETS_HXX__
#define __CPP_PARSER_CHARSETS_HXX__

// Generated by grammar-converter

#include <yuni/yuni.h>
#include <yuni/core/string/utf8char.h>

namespace Grammar
{
namespace Private
{
namespace JSON
{
namespace Charset
{
enum
{
    count = 28
};

struct Entry
{
    //! All ascii characters for the current table
    const char* const ascii;
    //! characters count
    const uint asciiSize;
    //! Allow UTF8 characterst
    const bool utf8chars;
};

/*!
** \brief All charset tables
*/
static const Entry table[count] = {
  {// CHARSET INDEX 0
   /* ascii */ "\t\n\013\014\r ",
   /* size  */ 6,
   /* utf8  */ false},
  {// CHARSET INDEX 1
   /* ascii */ ",",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 2
   /* ascii */ ":",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 3
   /* ascii */ "[",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 4
   /* ascii */ "]",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 5
   /* ascii */ "{",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 6
   /* ascii */ "}",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 7
   /* ascii */ "f",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 8
   /* ascii */ "n",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 9
   /* ascii */ "-",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 10
   /* ascii */ "0",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 11
   /* ascii */ "123456789",
   /* size  */ 9,
   /* utf8  */ false},
  {// CHARSET INDEX 12
   /* ascii */ "\"",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 13
   /* ascii */ "t",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 14
   /* ascii */ "a",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 15
   /* ascii */ "l",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 16
   /* ascii */ "s",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 17
   /* ascii */ "e",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 18
   /* ascii */ "u",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 19
   /* ascii */ ".",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 20
   /* ascii */ "Ee",
   /* size  */ 2,
   /* utf8  */ false},
  {// CHARSET INDEX 21
   /* ascii */ "0123456789",
   /* size  */ 10,
   /* utf8  */ false},
  {// CHARSET INDEX 22
   /* ascii */ "+-",
   /* size  */ 2,
   /* utf8  */ false},
  {// CHARSET INDEX 23
   /* ascii */ "\032\033\034\035\036\037 "
               "!#$%&'()*+,-./"
               "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~"
               "\0177\00\01\02\03\04\05\06\07\010\t\n\013\014\r\016\017\020\021\022\023\024\025\026"
               "\027\030\031\032\033\034\035\036\037 "
               "!\"#$%&'()*+,-./"
               "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
               "\0177",
   /* size  */ 228,
   /* utf8  */ true},
  {// CHARSET INDEX 24
   /* ascii */ "\\",
   /* size  */ 1,
   /* utf8  */ false},
  {// CHARSET INDEX 25
   /* ascii */ "\"/\\bfnrt",
   /* size  */ 8,
   /* utf8  */ false},
  {// CHARSET INDEX 26
   /* ascii */ "0123456789ABCDEFabcdef",
   /* size  */ 22,
   /* utf8  */ false},
  {// CHARSET INDEX 27
   /* ascii */ "r",
   /* size  */ 1,
   /* utf8  */ false},
};

} // namespace Charset
} // namespace JSON
} // namespace Private
} // namespace Grammar

#endif // __CPP_PARSER_JSON_CHARSETS_HXX__

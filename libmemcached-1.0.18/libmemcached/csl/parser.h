/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_CONFIG_LIBMEMCACHED_CSL_PARSER_H_INCLUDED
# define YY_CONFIG_LIBMEMCACHED_CSL_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int config_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    COMMENT = 258,
    END = 259,
    CSL_ERROR = 260,
    RESET = 261,
    PARSER_DEBUG = 262,
    INCLUDE = 263,
    CONFIGURE_FILE = 264,
    EMPTY_LINE = 265,
    SERVER = 266,
    CSL_SOCKET = 267,
    SERVERS = 268,
    SERVERS_OPTION = 269,
    UNKNOWN_OPTION = 270,
    UNKNOWN = 271,
    BINARY_PROTOCOL = 272,
    BUFFER_REQUESTS = 273,
    CONNECT_TIMEOUT = 274,
    DISTRIBUTION = 275,
    HASH = 276,
    HASH_WITH_NAMESPACE = 277,
    IO_BYTES_WATERMARK = 278,
    IO_KEY_PREFETCH = 279,
    IO_MSG_WATERMARK = 280,
    KETAMA_HASH = 281,
    KETAMA_WEIGHTED = 282,
    NOREPLY = 283,
    NUMBER_OF_REPLICAS = 284,
    NUMBER_OF_K = 285,
    NUMBER_OF_M = 286,
    POLL_TIMEOUT = 287,
    RANDOMIZE_REPLICA_READ = 288,
    RCV_TIMEOUT = 289,
    REMOVE_FAILED_SERVERS = 290,
    RETRY_TIMEOUT = 291,
    SND_TIMEOUT = 292,
    SOCKET_RECV_SIZE = 293,
    SOCKET_SEND_SIZE = 294,
    SORT_HOSTS = 295,
    SUPPORT_CAS = 296,
    USER_DATA = 297,
    USE_UDP = 298,
    VERIFY_KEY = 299,
    _TCP_KEEPALIVE = 300,
    _TCP_KEEPIDLE = 301,
    _TCP_NODELAY = 302,
    FETCH_VERSION = 303,
    NAMESPACE = 304,
    POOL_MIN = 305,
    POOL_MAX = 306,
    MD5 = 307,
    CRC = 308,
    FNV1_64 = 309,
    FNV1A_64 = 310,
    FNV1_32 = 311,
    FNV1A_32 = 312,
    HSIEH = 313,
    MURMUR = 314,
    JENKINS = 315,
    CONSISTENT = 316,
    MODULA = 317,
    RANDOM = 318,
    CSL_TRUE = 319,
    CSL_FALSE = 320,
    CSL_FLOAT = 321,
    NUMBER = 322,
    PORT = 323,
    WEIGHT_START = 324,
    IPADDRESS = 325,
    HOSTNAME = 326,
    STRING = 327,
    QUOTED_STRING = 328,
    FILE_PATH = 329
  };
#endif

/* Value type.  */



int config_parse (class Context *context, yyscan_t *scanner);

#endif /* !YY_CONFIG_LIBMEMCACHED_CSL_PARSER_H_INCLUDED  */

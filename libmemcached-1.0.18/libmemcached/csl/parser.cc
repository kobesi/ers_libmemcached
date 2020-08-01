/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         config_parse
#define yylex           config_lex
#define yyerror         config_error
#define yydebug         config_debug
#define yynerrs         config_nerrs


/* Copy the first part of user declarations.  */
#line 37 "libmemcached/csl/parser.yy" /* yacc.c:339  */


#include <libmemcached/csl/common.h>

class Context;

#line 60 "libmemcached/csl/parser.yy" /* yacc.c:339  */


#include <libmemcached/options.hpp>

#include <libmemcached/csl/context.h>
#include <libmemcached/csl/symbol.h>
#include <libmemcached/csl/scanner.h>

#ifndef __INTEL_COMPILER
# pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#ifndef __INTEL_COMPILER
# ifndef __clang__
#  pragma GCC diagnostic ignored "-Wlogical-op"
#  pragma GCC diagnostic ignored "-Wunsafe-loop-optimizations"
# endif
#endif

int conf_lex(YYSTYPE* lvalp, void* scanner);

#define select_yychar(__context) yychar == UNKNOWN ? ( (__context)->previous_token == END ? UNKNOWN : (__context)->previous_token ) : yychar   

#define stryytname(__yytokentype) ((__yytokentype) <  YYNTOKENS ) ? yytname[(__yytokentype)] : ""

#define parser_abort(__context, __error_message) do { (__context)->abort((__error_message), yytokentype(select_yychar(__context)), stryytname(YYTRANSLATE(select_yychar(__context)))); YYABORT; } while (0) 

// This is bison calling error.
inline void __config_error(Context *context, yyscan_t *scanner, const char *error, int last_token, const char *last_token_str)
{
  if (not context->end())
  {
    context->error(error, yytokentype(last_token), last_token_str);
  }
  else
  {
    context->error(error, yytokentype(last_token), last_token_str);
  }
}

#define config_error(__context, __scanner, __error_msg) do { __config_error((__context), (__scanner), (__error_msg), select_yychar(__context), stryytname(YYTRANSLATE(select_yychar(__context)))); } while (0)



#line 124 "libmemcached/csl/parser.cc" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "parser.h".  */
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

/* Copy the second part of user declarations.  */

#line 244 "libmemcached/csl/parser.cc" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  74
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   77

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  78
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  12
/* YYNRULES -- Number of rules.  */
#define YYNRULES  70
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  88

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   329

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,    77,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    66,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    67,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    68,    69,    70,    71,    72,    73,    74,    75,    76
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   205,   205,   206,   210,   212,   214,   216,   221,   226,
     230,   234,   245,   255,   265,   274,   278,   282,   286,   290,
     302,   306,   319,   332,   339,   346,   355,   361,   365,   369,
     373,   377,   381,   385,   389,   393,   397,   401,   405,   409,
     413,   420,   424,   428,   432,   436,   440,   444,   448,   452,
     456,   460,   464,   471,   472,   477,   478,   483,   487,   491,
     495,   499,   503,   507,   511,   515,   522,   526,   533,   537,
     541
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "COMMENT", "END", "CSL_ERROR", "RESET",
  "PARSER_DEBUG", "INCLUDE", "CONFIGURE_FILE", "EMPTY_LINE", "SERVER",
  "CSL_SOCKET", "SERVERS", "SERVERS_OPTION", "UNKNOWN_OPTION", "UNKNOWN",
  "BINARY_PROTOCOL", "BUFFER_REQUESTS", "CONNECT_TIMEOUT", "DISTRIBUTION",
  "HASH", "HASH_WITH_NAMESPACE", "IO_BYTES_WATERMARK", "IO_KEY_PREFETCH",
  "IO_MSG_WATERMARK", "KETAMA_HASH", "KETAMA_WEIGHTED", "NOREPLY",
  "NUMBER_OF_REPLICAS", "NUMBER_OF_K", "NUMBER_OF_M", "POLL_TIMEOUT",
  "RANDOMIZE_REPLICA_READ", "RCV_TIMEOUT", "REMOVE_FAILED_SERVERS",
  "RETRY_TIMEOUT", "SND_TIMEOUT", "SOCKET_RECV_SIZE", "SOCKET_SEND_SIZE",
  "SORT_HOSTS", "SUPPORT_CAS", "USER_DATA", "USE_UDP", "VERIFY_KEY",
  "_TCP_KEEPALIVE", "_TCP_KEEPIDLE", "_TCP_NODELAY", "FETCH_VERSION",
  "NAMESPACE", "POOL_MIN", "POOL_MAX", "MD5", "CRC", "FNV1_64", "FNV1A_64",
  "FNV1_32", "FNV1A_32", "HSIEH", "MURMUR", "JENKINS", "CONSISTENT",
  "MODULA", "RANDOM", "CSL_TRUE", "CSL_FALSE", "','", "'='", "CSL_FLOAT",
  "NUMBER", "PORT", "WEIGHT_START", "IPADDRESS", "HOSTNAME", "STRING",
  "QUOTED_STRING", "FILE_PATH", "' '", "$accept", "begin", "statement",
  "expression", "behaviors", "behavior_number", "behavior_boolean",
  "optional_port", "optional_weight", "hash", "string", "distribution", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,    44,    61,   321,   322,
     323,   324,   325,   326,   327,   328,   329,    32
};
# endif

#define YYPACT_NINF -64

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-64)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      -2,   -64,   -64,   -64,   -64,   -64,   -63,   -24,   -64,   -20,
     -24,   -64,   -64,   -64,   -50,    13,   -64,   -64,   -64,   -64,
     -64,   -64,   -64,   -64,   -64,   -64,   -64,   -64,   -64,   -64,
     -64,   -64,   -64,   -64,   -64,   -64,   -64,   -64,   -64,   -64,
     -64,   -24,   -44,   -15,     0,   -64,   -64,   -64,   -11,   -64,
     -24,   -64,   -64,   -64,   -10,   -10,   -12,   -64,   -64,   -64,
      -5,   -64,   -64,   -64,   -64,   -64,   -64,   -64,   -64,   -64,
     -64,   -64,   -64,   -64,   -64,    -2,   -64,   -64,   -64,   -12,
     -12,   -64,   -64,    13,   -64,   -64,   -64,   -64
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     5,     7,     8,     9,    10,     0,     0,     6,     0,
       0,    41,    42,    28,     0,     0,    43,    30,    31,    29,
      44,    32,    33,    34,    35,    45,    36,    27,    37,    38,
      39,    40,    46,    47,    26,    51,    52,    49,    50,    48,
      20,     0,     0,     0,     0,     2,     4,    18,     0,    25,
       0,    66,    67,    15,    53,    53,    55,    68,    69,    70,
      21,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      23,    19,    16,    17,     1,     0,    24,    11,    54,    55,
      55,    56,    14,     0,     3,    13,    12,    22
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -64,   -64,   -13,   -64,   -64,   -64,   -64,     8,   -23,    -9,
      14,   -64
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    44,    45,    46,    47,    48,    49,    79,    82,    70,
      53,    60
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      74,     1,     2,     3,     4,     5,     6,     7,     8,     9,
      10,    57,    58,    59,    50,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    56,    72,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      51,    52,    54,    55,    73,    71,    85,    86,    76,    81,
      78,    83,    84,    80,    77,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    87,     0,     0,    75
};

static const yytype_int8 yycheck[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    61,    62,    63,    77,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    10,    69,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      74,    75,    72,    73,    69,    41,    79,    80,    69,    71,
      70,    66,    75,    55,    50,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    83,    -1,    -1,    77
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    79,    80,    81,    82,    83,    84,
      77,    74,    75,    88,    72,    73,    88,    61,    62,    63,
      89,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      87,    88,    69,    69,     0,    77,    69,    88,    70,    85,
      85,    71,    86,    66,    80,    86,    86,    87
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    78,    79,    79,    80,    80,    80,    80,    80,    80,
      80,    80,    81,    81,    81,    81,    81,    81,    81,    82,
      82,    82,    82,    82,    82,    82,    82,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    85,    85,    86,    86,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    88,    88,    89,    89,
      89
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     3,     4,     4,     3,     2,     2,     2,     1,     2,
       1,     2,     4,     2,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     1,     0,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (context, scanner, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, context, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, class Context *context, yyscan_t *scanner)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (context);
  YYUSE (scanner);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, class Context *context, yyscan_t *scanner)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, context, scanner);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, class Context *context, yyscan_t *scanner)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              , context, scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, context, scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, class Context *context, yyscan_t *scanner)
{
  YYUSE (yyvaluep);
  YYUSE (context);
  YYUSE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (class Context *context, yyscan_t *scanner)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:
#line 211 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    { }
#line 1408 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 5:
#line 213 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    { }
#line 1414 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 6:
#line 215 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    { }
#line 1420 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 7:
#line 217 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            context->set_end();
            YYACCEPT;
          }
#line 1429 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 8:
#line 222 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            context->rc= MEMCACHED_PARSE_USER_ERROR;
            parser_abort(context, "ERROR called directly");
          }
#line 1438 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 9:
#line 227 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            memcached_reset(context->memc);
          }
#line 1446 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 10:
#line 231 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            yydebug= 1;
          }
#line 1454 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 11:
#line 235 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            if ((context->rc= memcached_parse_configure_file(*context->memc, (yyvsp[0].string).c_str, (yyvsp[0].string).size)) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "Failed to parse configuration file");
            }
          }
#line 1465 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 12:
#line 246 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            if (memcached_failed(context->rc= memcached_server_add_with_weight(context->memc, (yyvsp[-2].server).c_str, (yyvsp[-1].number), uint32_t((yyvsp[0].number)))))
            {
              char buffer[1024];
              snprintf(buffer, sizeof(buffer), "Failed to add server: %s:%u", (yyvsp[-2].server).c_str, uint32_t((yyvsp[-1].number)));
              parser_abort(context, buffer);
            }
            context->unset_server();
          }
#line 1479 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 13:
#line 256 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            if (memcached_failed(context->rc= memcached_server_add_with_weight(context->memc, (yyvsp[-2].server).c_str, (yyvsp[-1].number), uint32_t((yyvsp[0].number)))))
            {
              char buffer[1024];
              snprintf(buffer, sizeof(buffer), "Failed to add server: %s:%u", (yyvsp[-2].server).c_str, uint32_t((yyvsp[-1].number)));
              parser_abort(context, buffer);
            }
            context->unset_server();
          }
#line 1493 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 14:
#line 266 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            if (memcached_failed(context->rc= memcached_server_add_unix_socket_with_weight(context->memc, (yyvsp[-1].string).c_str, uint32_t((yyvsp[0].number)))))
            {
              char buffer[1024];
              snprintf(buffer, sizeof(buffer), "Failed to add socket: %s", (yyvsp[-1].string).c_str);
              parser_abort(context, buffer);
            }
          }
#line 1506 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 15:
#line 275 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            memcached_set_configuration_file(context->memc, (yyvsp[0].string).c_str, (yyvsp[0].string).size);
          }
#line 1514 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 16:
#line 279 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            context->memc->configure.initial_pool_size= uint32_t((yyvsp[0].number));
          }
#line 1522 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 17:
#line 283 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            context->memc->configure.max_pool_size= uint32_t((yyvsp[0].number));
          }
#line 1530 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 19:
#line 291 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            if (memcached_callback_get(context->memc, MEMCACHED_CALLBACK_PREFIX_KEY, NULL))
            {
              parser_abort(context, "--NAMESPACE can only be called once");
            }

            if ((context->rc= memcached_set_namespace(*context->memc, (yyvsp[0].string).c_str, (yyvsp[0].string).size)) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, memcached_last_error_message(context->memc));
            }
          }
#line 1546 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 20:
#line 303 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            memcached_flag(*context->memc, MEMCACHED_FLAG_IS_FETCHING_VERSION, true);
          }
#line 1554 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 21:
#line 307 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            // Check to see if DISTRIBUTION has already been set
            if ((context->rc= memcached_behavior_set(context->memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, (yyvsp[0].distribution))) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "--DISTRIBUTION can only be called once");
            }

            if ((context->rc= memcached_behavior_set(context->memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, (yyvsp[0].distribution))) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, memcached_last_error_message(context->memc));;
            }
          }
#line 1571 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 22:
#line 320 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            // Check to see if DISTRIBUTION has already been set
            if ((context->rc= memcached_behavior_set(context->memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, (yyvsp[-2].distribution))) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "--DISTRIBUTION can only be called once");
            }

            if ((context->rc= memcached_behavior_set_distribution_hash(context->memc, (yyvsp[0].hash))) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "Unable to set the hash for the DISTRIBUTION requested");
            }
          }
#line 1588 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 23:
#line 333 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            if (context->set_hash((yyvsp[0].hash)) == false)
            {
              parser_abort(context, "--HASH can only be set once");
            }
          }
#line 1599 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 24:
#line 340 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            if ((context->rc= memcached_behavior_set(context->memc, (yyvsp[-1].behavior), (yyvsp[0].number))) != MEMCACHED_SUCCESS)
            {
              parser_abort(context, "Unable to set behavior");
            }
          }
#line 1610 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 25:
#line 347 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            if ((context->rc= memcached_behavior_set(context->memc, (yyvsp[0].behavior), true)) != MEMCACHED_SUCCESS)
            {
              char buffer[1024];
              snprintf(buffer, sizeof(buffer), "Could not set: %s", libmemcached_string_behavior((yyvsp[0].behavior)));
              parser_abort(context, buffer);
            }
          }
#line 1623 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 26:
#line 356 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
          }
#line 1630 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 27:
#line 362 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS;
          }
#line 1638 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 28:
#line 366 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT;
          }
#line 1646 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 29:
#line 370 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_IO_MSG_WATERMARK;
          }
#line 1654 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 30:
#line 374 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_IO_BYTES_WATERMARK;
          }
#line 1662 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 31:
#line 378 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_IO_KEY_PREFETCH;
          }
#line 1670 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 32:
#line 382 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS;
          }
#line 1678 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 33:
#line 386 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_NUMBER_OF_K;
          }
#line 1686 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 34:
#line 390 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_NUMBER_OF_M;
          }
#line 1694 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 35:
#line 394 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_POLL_TIMEOUT;
          }
#line 1702 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 36:
#line 398 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_RCV_TIMEOUT;
          }
#line 1710 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 37:
#line 402 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_RETRY_TIMEOUT;
          }
#line 1718 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 38:
#line 406 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_SND_TIMEOUT;
          }
#line 1726 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 39:
#line 410 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_SOCKET_RECV_SIZE;
          }
#line 1734 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 40:
#line 414 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_SOCKET_SEND_SIZE;
          }
#line 1742 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 41:
#line 421 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_BINARY_PROTOCOL;
          }
#line 1750 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 42:
#line 425 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_BUFFER_REQUESTS;
          }
#line 1758 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 43:
#line 429 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_HASH_WITH_PREFIX_KEY;
          }
#line 1766 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 44:
#line 433 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_NOREPLY;
          }
#line 1774 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 45:
#line 437 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_RANDOMIZE_REPLICA_READ;
          }
#line 1782 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 46:
#line 441 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_SORT_HOSTS;
          }
#line 1790 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 47:
#line 445 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_SUPPORT_CAS;
          }
#line 1798 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 48:
#line 449 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_TCP_NODELAY;
          }
#line 1806 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 49:
#line 453 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_TCP_KEEPALIVE;
          }
#line 1814 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 50:
#line 457 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_TCP_KEEPIDLE;
          }
#line 1822 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 51:
#line 461 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_USE_UDP;
          }
#line 1830 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 52:
#line 465 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.behavior)= MEMCACHED_BEHAVIOR_VERIFY_KEY;
          }
#line 1838 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 53:
#line 471 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    { (yyval.number)= MEMCACHED_DEFAULT_PORT;}
#line 1844 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 54:
#line 473 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    { }
#line 1850 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 55:
#line 477 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    { (yyval.number)= 1; }
#line 1856 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 56:
#line 479 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    { }
#line 1862 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 57:
#line 484 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.hash)= MEMCACHED_HASH_MD5;
          }
#line 1870 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 58:
#line 488 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.hash)= MEMCACHED_HASH_CRC;
          }
#line 1878 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 59:
#line 492 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.hash)= MEMCACHED_HASH_FNV1_64;
          }
#line 1886 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 60:
#line 496 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.hash)= MEMCACHED_HASH_FNV1A_64;
          }
#line 1894 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 61:
#line 500 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.hash)= MEMCACHED_HASH_FNV1_32;
          }
#line 1902 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 62:
#line 504 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.hash)= MEMCACHED_HASH_FNV1A_32;
          }
#line 1910 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 63:
#line 508 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.hash)= MEMCACHED_HASH_HSIEH;
          }
#line 1918 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 64:
#line 512 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.hash)= MEMCACHED_HASH_MURMUR;
          }
#line 1926 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 65:
#line 516 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.hash)= MEMCACHED_HASH_JENKINS;
          }
#line 1934 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 66:
#line 523 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.string)= (yyvsp[0].string);
          }
#line 1942 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 67:
#line 527 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.string)= (yyvsp[0].string);
          }
#line 1950 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 68:
#line 534 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.distribution)= MEMCACHED_DISTRIBUTION_CONSISTENT;
          }
#line 1958 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 69:
#line 538 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.distribution)= MEMCACHED_DISTRIBUTION_MODULA;
          }
#line 1966 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;

  case 70:
#line 542 "libmemcached/csl/parser.yy" /* yacc.c:1646  */
    {
            (yyval.distribution)= MEMCACHED_DISTRIBUTION_RANDOM;
          }
#line 1974 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
    break;


#line 1978 "libmemcached/csl/parser.cc" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (context, scanner, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (context, scanner, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, context, scanner);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, context, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (context, scanner, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, context, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, context, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 547 "libmemcached/csl/parser.yy" /* yacc.c:1906  */
 

void Context::start() 
{
  config_parse(this, (void **)scanner);
}


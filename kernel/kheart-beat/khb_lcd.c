/*****************************************************************************
** COPYRIGHT(C) : Samsung Electronics Co.Ltd, 2002-2006 ALL RIGHTS RESERVED
** AUTHOR               : KyoungHOON Kim (khoonk)
******************************************************************************/

#include <linux/module.h>
#include "khb_main.h"

/*****************************************************************************
** global variables
*****************************************************************************/

const unsigned short font_eng_8x16[] =
{
/* Character 32 (0x20):	SPACE	*/
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
/* Character 33 (0x21): 	[!]		*/
0x0000,0x0000,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x0000,0x1800,0x1800,0x0000,0x0000,0x0000,
/* Character 34 (0x22):	["]		*/
0x0000,0x3600,0x3600,0x3600,0x3600,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
/* Character 35 (0x23):	[#]		*/
0x0000,0x0000,0x3600,0x3600,0x3600,0x7f00,0x3600,0x3600,0x3600,0x7f00,0x3600,0x3600,0x3600,0x0000,0x0000,0x0000,
/* Character 36 (0x24):	[$]		*/
0x0000,0x0800,0x1c00,0x3e00,0x6b00,0x6b00,0x3800,0x1c00,0x0e00,0x6b00,0x6b00,0x3e00,0x1c00,0x0800,0x0000,0x0000,
/* Character 37 (0x25):	[%]		*/
0x0000,0x0000,0x6600,0xd600,0xd600,0xdc00,0x6c00,0x1800,0x3600,0x3b00,0x6b00,0x6b00,0x6600,0x0000,0x0000,0x0000,
/* Character 38 (0x26):	[&]		*/
0x0000,0x0000,0x3800,0x6c00,0x6c00,0x6c00,0x3800,0x7800,0xcd00,0xc700,0xc600,0xcf00,0x7900,0x0000,0x0000,0x0000,
/* Character 39 (0x27):	[']		*/
0x0000,0x1800,0x1800,0x1800,0x1800,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
/* Character 40 (0x28):	[(]		*/
0x0600,0x0c00,0x1800,0x1800,0x3000,0x3000,0x3000,0x3000,0x3000,0x3000,0x3000,0x1800,0x1800,0x0c00,0x0600,0x0000,
/* Character 41 (0x29):	[)]		*/
0x3000,0x1800,0x0c00,0x0c00,0x0600,0x0600,0x0600,0x0600,0x0600,0x0600,0x0600,0x0c00,0x0c00,0x1800,0x3000,0x0000,
/* Character 42 (0x2a):	[*]		*/
0x0000,0x0000,0x0000,0x0000,0x0800,0x6b00,0x3e00,0x1c00,0x3e00,0x6b00,0x0800,0x0000,0x0000,0x0000,0x0000,0x0000,
/* Character 43 (0x2b):	[+]		*/
0x0000,0x0000,0x0000,0x0000,0x1800,0x1800,0x1800,0x7e00,0x1800,0x1800,0x1800,0x0000,0x0000,0x0000,0x0000,0x0000,
/* Character 44 (0x2c):		[,]		*/
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x6000,0x6000,0x2000,0x4000,0x0000,
/* Character 45 (0x2d):	[-]		*/
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x7e00,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
/* Character 46 (0x2e):	[.]		*/
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x6000,0x6000,0x0000,0x0000,0x0000,
/* Character 47 (0x2f):		[/]		*/
0x0000,0x0c00,0x0c00,0x0c00,0x1800,0x1800,0x1800,0x1800,0x3000,0x3000,0x3000,0x3000,0x6000,0x6000,0x6000,0x0000,
/* Character 48 (0x30):	[0]		*/
0x0000,0x0000,0x3e00,0x6300,0x6300,0x6300,0x6700,0x6b00,0x7300,0x6300,0x6300,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 49 (0x31):	[1]		*/
0x0000,0x0000,0x1800,0x1800,0x7800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x0000,0x0000,0x0000,
/* Character 50 (0x32):	[2]		*/
0x0000,0x0000,0x3e00,0x6300,0x6300,0x0300,0x0600,0x0c00,0x1800,0x3000,0x6000,0x6000,0x7f00,0x0000,0x0000,0x0000,
/* Character 51 (0x33):	[3]		*/
0x0000,0x0000,0x3e00,0x6300,0x6300,0x0300,0x0300,0x1e00,0x0300,0x0300,0x6300,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 52 (0x34):	[4]		*/
0x0000,0x0000,0x0600,0x0e00,0x0e00,0x1e00,0x1e00,0x3600,0x3600,0x6600,0x7f00,0x0600,0x0600,0x0000,0x0000,0x0000,
/* Character 53 (0x35):	[5]		*/
0x0000,0x0000,0x7f00,0x6000,0x6000,0x6000,0x7e00,0x6300,0x0300,0x0300,0x6300,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 54 (0x36):	[6]		*/
0x0000,0x0000,0x3e00,0x6300,0x6300,0x6000,0x7e00,0x6300,0x6300,0x6300,0x6300,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 55 (0x37):	[7]		*/
0x0000,0x0000,0x7f00,0x0300,0x0600,0x0600,0x0c00,0x0c00,0x0c00,0x1800,0x1800,0x1800,0x1800,0x0000,0x0000,0x0000,
/* Character 56 (0x38):	[8]		*/
0x0000,0x0000,0x3e00,0x6300,0x6300,0x6300,0x6300,0x3e00,0x6300,0x6300,0x6300,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 57 (0x39):	[9]		*/
0x0000,0x0000,0x3e00,0x6300,0x6300,0x6300,0x6300,0x6300,0x3f00,0x0300,0x6300,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 58 (0x3a):	[:]		*/
0x0000,0x0000,0x0000,0x1800,0x1800,0x0000,0x0000,0x0000,0x0000,0x1800,0x1800,0x0000,0x0000,0x0000,0x0000,0x0000,
/* Character 59 (0x3b):	[;]		*/
0x0000,0x0000,0x0000,0x1800,0x1800,0x0000,0x0000,0x0000,0x0000,0x1800,0x1800,0x0800,0x1000,0x0000,0x0000,0x0000,
/* Character 60 (0x3c):		[<]		*/
0x0000,0x0000,0x0000,0x0600,0x0c00,0x1800,0x3000,0x6000,0x3000,0x1800,0x0c00,0x0600,0x0000,0x0000,0x0000,0x0000,
/* Character 61 (0x3d):	[=]		*/
0x0000,0x0000,0x0000,0x0000,0x0000,0x7e00,0x0000,0x0000,0x0000,0x7e00,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
/* Character 62 (0x3e):	[>]		*/
0x0000,0x0000,0x0000,0x6000,0x3000,0x1800,0x0c00,0x0600,0x0c00,0x1800,0x3000,0x6000,0x0000,0x0000,0x0000,0x0000,
/* Character 63 (0x3f):		[?]		*/
0x0000,0x0000,0x7c00,0xc600,0xc600,0x0600,0x0c00,0x1800,0x3000,0x3000,0x3000,0x0000,0x3000,0x3000,0x0000,0x0000,
/* Character 64 (0x40):	[@]		*/
0x0000,0x0000,0x3c00,0x6600,0xc300,0xdb00,0xeb00,0xeb00,0xeb00,0xd600,0xc000,0x6600,0x3c00,0x0000,0x0000,0x0000,
/* Character 65 (0x41):	[A]		*/
0x0000,0x0000,0x1c00,0x3600,0x6300,0x6300,0x6300,0x6300,0x6300,0x7f00,0x6300,0x6300,0x6300,0x0000,0x0000,0x0000,
/* Character 66 (0x42):	[B]		*/
0x0000,0x0000,0x7e00,0x6300,0x6300,0x6300,0x6300,0x7e00,0x6300,0x6300,0x6300,0x6300,0x7e00,0x0000,0x0000,0x0000,
/* Character 67 (0x43):	[C]		*/
0x0000,0x0000,0x3e00,0x6300,0x6300,0x6000,0x6000,0x6000,0x6000,0x6000,0x6300,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 68 (0x44):	[D]		*/
0x0000,0x0000,0x7e00,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x7e00,0x0000,0x0000,0x0000,
/* Character 69 (0x45):	[E]		*/
0x0000,0x0000,0x7e00,0x6000,0x6000,0x6000,0x6000,0x7e00,0x6000,0x6000,0x6000,0x6000,0x7e00,0x0000,0x0000,0x0000,
/* Character 70 (0x46):	[F]		*/
0x0000,0x0000,0x7e00,0x6000,0x6000,0x6000,0x6000,0x7e00,0x6000,0x6000,0x6000,0x6000,0x6000,0x0000,0x0000,0x0000,
/* Character 71 (0x47):	[G]		*/
0x0000,0x0000,0x3e00,0x6300,0x6300,0x6000,0x6000,0x6f00,0x6300,0x6300,0x6300,0x6300,0x3f00,0x0000,0x0000,0x0000,
/* Character 72 (0x48):	[H]		*/
0x0000,0x0000,0x6300,0x6300,0x6300,0x6300,0x6300,0x7f00,0x6300,0x6300,0x6300,0x6300,0x6300,0x0000,0x0000,0x0000,
/* Character 73 (0x49):	[I]		*/
0x0000,0x0000,0x3c00,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x3c00,0x0000,0x0000,0x0000,
/* Character 74 (0x4a):	[J]		*/
0x0000,0x0000,0x0f00,0x0300,0x0300,0x0300,0x0300,0x0300,0x0300,0x0300,0x6300,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 75 (0x4b):	[k]		*/
0x0000,0x0000,0x6300,0x6300,0x6600,0x6c00,0x7800,0x7000,0x7800,0x6c00,0x6600,0x6300,0x6300,0x0000,0x0000,0x0000,
/* Character 76 (0x4c):		[L]		*/
0x0000,0x0000,0x6000,0x6000,0x6000,0x6000,0x6000,0x6000,0x6000,0x6000,0x6000,0x6000,0x7f00,0x0000,0x0000,0x0000,
/* Character 77 (0x4d):	[M]		*/
0x0000,0x0000,0x6300,0x6300,0x6300,0x7700,0x7700,0x7700,0x7f00,0x7f00,0x6b00,0x6b00,0x6b00,0x0000,0x0000,0x0000,
/* Character 78 (0x4e):	[N]		*/
0x0000,0x0000,0x6300,0x6300,0x7300,0x7300,0x7b00,0x6b00,0x6f00,0x6700,0x6700,0x6300,0x6300,0x0000,0x0000,0x0000,
/* Character 79 (0x4f):		[O]		*/
0x0000,0x0000,0x3e00,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 80 (0x50):	[P]		*/
0x0000,0x0000,0x7e00,0x6300,0x6300,0x6300,0x6300,0x6300,0x7e00,0x6000,0x6000,0x6000,0x6000,0x0000,0x0000,0x0000,
/* Character 81 (0x51):	[Q]		*/
0x0000,0x0000,0x3e00,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6f00,0x6600,0x3f00,0x0000,0x0000,0x0000,
/* Character 82 (0x52):	[R]		*/
0x0000,0x0000,0x7e00,0x6300,0x6300,0x6300,0x6300,0x6300,0x7e00,0x6300,0x6300,0x6300,0x6300,0x0000,0x0000,0x0000,
/* Character 83 (0x53):	[S]		*/
0x0000,0x0000,0x3e00,0x6300,0x6300,0x6000,0x3000,0x1c00,0x0600,0x0300,0x6300,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 84 (0x54):	[T]		*/
0x0000,0x0000,0xff00,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x0000,0x0000,0x0000,
/* Character 85 (0x55):	[U]		*/
0x0000,0x0000,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 86 (0x56):	[V]		*/
0x0000,0x0000,0xc300,0xc300,0xc300,0xc300,0x6600,0x6600,0x6600,0x3c00,0x3c00,0x1800,0x1800,0x0000,0x0000,0x0000,
/* Character 87 (0x57):	[W]		*/
0x0000,0x0000,0x6b00,0x6b00,0x6b00,0x6b00,0x6b00,0x6b00,0x6b00,0x3600,0x3600,0x3600,0x3600,0x0000,0x0000,0x0000,
/* Character 88 (0x58):	[X]		*/
0x0000,0x0000,0x6300,0x6300,0x6300,0x3600,0x1c00,0x1c00,0x3600,0x6300,0x6300,0x6300,0x6300,0x0000,0x0000,0x0000,
/* Character 89 (0x59):	[Y]		*/
0x0000,0x0000,0xc300,0xc300,0x6600,0x6600,0x3c00,0x3c00,0x1800,0x1800,0x1800,0x1800,0x1800,0x0000,0x0000,0x0000,
/* Character 90 (0x5a):	[Z]		*/
0x0000,0x0000,0x7f00,0x0380,0x0600,0x0600,0x0c00,0x1800,0x1800,0x3000,0x6000,0x6000,0x7f80,0x0000,0x0000,0x0000,
/* Character 91 (0x5b):	[[]		*/
0x0e00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0e00,0x0000,
/* Character 92 (0x5c):		[WON]	*/
0x0000,0x0000,0x6b00,0x6b00,0x6b00,0x7f00,0x6b00,0x6b00,0x7f00,0x6b00,0x3600,0x3600,0x3600,0x0000,0x0000,0x0000,
/* Character 93 (0x5d):	[]]		*/
0x3800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x3800,0x0000,
/* Character 94 (0x5e):	[^]		*/
0x1800,0x1800,0x3c00,0x3c00,0x6600,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
/* Character 95 (0x5f):		[_]		*/
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x7f00,0x0000,
/* Character 96 (0x60):	[`]		*/
0x0200,0x0400,0x0600,0x0600,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
/* Character 97 (0x61):	[a]		*/
0x0000,0x0000,0x0000,0x0000,0x0000,0x3e00,0x6300,0x0300,0x3f00,0x6300,0x6300,0x6700,0x3b00,0x0000,0x0000,0x0000,
/* Character 98 (0x62):	[b]		*/
0x0000,0x0000,0x6000,0x6000,0x6000,0x6e00,0x7300,0x6300,0x6300,0x6300,0x6300,0x7300,0x6e00,0x0000,0x0000,0x0000,
/* Character 99 (0x63):	[c]		*/
0x0000,0x0000,0x0000,0x0000,0x0000,0x3e00,0x6300,0x6300,0x6000,0x6000,0x6300,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 100 (0x64):	[d]		*/
 0x0000,0x0000,0x0300,0x0300,0x0300,0x3b00,0x6700,0x6300,0x6300,0x6300,0x6300,0x6700,0x3b00,0x0000,0x0000,0x0000,
/* Character 101 (0x65):	[e]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0x3e00,0x6300,0x6300,0x6300,0x7f00,0x6000,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 102 (0x66):	[f]		*/
 0x0000,0x0000,0x0e00,0x1800,0x1800,0x1800,0x7f00,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x0000,0x0000,0x0000,
/* Character 103 (0x67):	[g]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0x3b00,0x6700,0x6300,0x6300,0x6300,0x6700,0x3b00,0x0380,0x6380,0x3e00,0x0000,
/* Character 104 (0x68):	[h]		*/
 0x0000,0x0000,0x6000,0x6000,0x6000,0x6e00,0x7300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x0000,0x0000,0x0000,
/* Character 105 (0x69):	[i]		*/
 0x0000,0x0000,0x0c00,0x0c00,0x0000,0x1c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0000,0x0000,0x0000,
/* Character 106 (0x6a):	[j]		*/
 0x0000,0x0000,0x0600,0x0600,0x0000,0x0e00,0x0600,0x0600,0x0600,0x0600,0x0600,0x0600,0x0600,0x6600,0x3c00,0x0000,
/* Character 107 (0x6b):	[k]		*/
 0x0000,0x0000,0x6000,0x6000,0x6000,0x6300,0x6600,0x6c00,0x7800,0x7800,0x6c00,0x6600,0x6300,0x0000,0x0000,0x0000,
/* Character 108 (0x6c):	[l]		*/
 0x0000,0x0000,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1c00,0x0000,0x0000,0x0000,
/* Character 109 (0x6d):	[m]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0x7e00,0x6b00,0x6b00,0x6b00,0x6b00,0x6b00,0x6b00,0x6b00,0x0000,0x0000,0x0000,
/* Character 110 (0x6e):	[n]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0x6e00,0x7300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x0000,0x0000,0x0000,
/* Character 111 (0x6f):	[o]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0x3e00,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 112 (0x70):	[p]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0x6e00,0x7300,0x6300,0x6300,0x6300,0x6300,0x7300,0x6e00,0x6000,0x6000,0x0000,
/* Character 113 (0x71):	[q]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0x3b00,0x6700,0x6300,0x6300,0x6300,0x6300,0x6700,0x3b00,0x0300,0x0300,0x0000,
/* Character 114 (0x72):	[r]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0x3700,0x3800,0x3000,0x3000,0x3000,0x3000,0x3000,0x3000,0x0000,0x0000,0x0000,
/* Character 115 (0x73):	[s]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0x3e00,0x6300,0x6000,0x3e00,0x0300,0x0300,0x6300,0x3e00,0x0000,0x0000,0x0000,
/* Character 116 (0x74):	[t]		*/
 0x0000,0x0000,0x1800,0x1800,0x1800,0x7f00,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x0f00,0x0000,0x0000,0x0000,
/* Character 117 (0x75):	[u]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0x6300,0x6300,0x6300,0x6300,0x6300,0x6300,0x6700,0x3b00,0x0000,0x0000,0x0000,
/* Character 118 (0x76):	[v]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0xc300,0xc300,0x6600,0x6600,0x3c00,0x3c00,0x1800,0x1800,0x0000,0x0000,0x0000,
/* Character 119 (0x77):	[w]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0x6b00,0x6b00,0x6b00,0x6b00,0x6b00,0x3600,0x3600,0x3600,0x0000,0x0000,0x0000,
/* Character 120 (0x78):	[x]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0x6300,0x6300,0x3600,0x1c00,0x1c00,0x3600,0x6300,0x6300,0x0000,0x0000,0x0000,
/* Character 121 (0x79):	[y]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0xc300,0xc300,0x6600,0x6600,0x3c00,0x3c00,0x1800,0x1800,0x3000,0xe000,0x0000,
/* Character 122 (0x7a):	[z]		*/
 0x0000,0x0000,0x0000,0x0000,0x0000,0x7f00,0x0300,0x0600,0x0c00,0x1800,0x3000,0x6000,0x7f00,0x0000,0x0000,0x0000,
/* Character 123 (0x7b):	[{]		*/
 0x0600,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x1800,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0600,0x0000,
/* Character 124 (0x7c):	[|]		*/
 0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x0000,
/* Character 125 (0x7d):	[}]		*/
 0x3000,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x0c00,0x1800,0x1800,0x1800,0x1800,0x1800,0x1800,0x3000,0x0000,
/* Character 126 (0x7e):	[~]		*/
 0x0000,0x0000,0x0000,0x0000,0x6000,0xd600,0xd600,0x0c00,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
};

#if defined(CONFIG_LCD_COLOR_DEPTH_16)
unsigned short*         fbpMem = 0;
#else
unsigned int*           fbpMem = 0;
#endif

void lcd_PutPixel(int x, int y, unsigned int color)
{
        unsigned int convert= (0xff)<<24 | color;

 
        if ((x < PANEL_W) && (y < PANEL_H))
                *(fbpMem+(y*(PANEL_W))+x) = convert;
}


void lcd_Line(int x1, int y1, int x2, int y2, unsigned int color)
{
        int dx,dy,e;
 
    dx=x2-x1;
        dy=y2-y1;
   
        if(dx>=0)
        {
                if(dy >= 0) // dy>=0
                {
                        if(dx>=dy) // 1/8 octant
                        {
                                e=dy-dx/2;
                                while(x1<=x2)
                                {
                                        lcd_PutPixel(x1,y1,color);
                                        if(e>0){y1+=1;e-=dx;}
                                        x1+=1;
                                        e+=dy;
                                }
                        }
                        else            // 2/8 octant
                        {
                                e=dx-dy/2;
                                while(y1<=y2)
                                {
                                        lcd_PutPixel(x1,y1,color);
                                        if(e>0){x1+=1;e-=dy;}
                                        y1+=1;
                                        e+=dx;
                                }
                       }
                }
                else               // dy<0
                {
                        dy=-dy;   // dy=abs(dy)
 
                        if(dx>=dy) // 8/8 octant
                        {
                                e=dy-dx/2;
                                while(x1<=x2)
                                {
                                        lcd_PutPixel(x1,y1,color);
                                        if(e>0){y1-=1;e-=dx;}
                                        x1+=1;
                                        e+=dy;
                                }
                        }
                        else            // 7/8 octant
                        {
                                e=dx-dy/2;
                                while(y1>=y2)
                                {
                                        lcd_PutPixel(x1,y1,color);
                                        if(e>0){x1+=1;e-=dy;}
                                        y1-=1;
                                        e+=dx;
                                }
                        }
                }
        }

       else //dx<0
        {
                dx=-dx;         //dx=abs(dx)
                if(dy >= 0) // dy>=0
                {
                        if(dx>=dy) // 4/8 octant
                        {
                                e=dy-dx/2;
                                while(x1>=x2)
                                {
                                        lcd_PutPixel(x1,y1,color);
                                        if(e>0){y1+=1;e-=dx;}
                                        x1-=1;
                                        e+=dy;
                                }
                        }
                        else            // 3/8 octant
                        {
                                e=dx-dy/2;
                                while(y1<=y2)
                                {
                                        lcd_PutPixel(x1,y1,color);
                                        if(e>0){x1-=1;e-=dy;}
                                        y1+=1;
                                        e+=dx;
                                }
                        }
                }
                else               // dy<0
                {
                        dy=-dy;   // dy=abs(dy)
                         if(dx>=dy) // 5/8 octant
                        {
                                e=dy-dx/2;
                                while(x1>=x2)
                                {
                                        lcd_PutPixel(x1,y1,color);
                                        if(e>0){y1-=1;e-=dx;}
                                        x1-=1;
                                        e+=dy;
                                }
                        }
                        else            // 6/8 octant
                        {
                                e=dx-dy/2;
                                while(y1>=y2)
                                {
                                        lcd_PutPixel(x1,y1,color);
                                        if(e>0){x1-=1;e-=dy;}
                                        y1-=1;
                                        e+=dx;
                                }
                        }
                }
        }
}

void lcd_FilledRectangle(int x1, int y1, int x2, int y2, unsigned int color)
{
    int i;
 
    for(i=y1;i<=y2;i++)
        {
                lcd_Line(x1,i,x2,i,color);
        }
}

void lcd_Rectangle(int x1, int y1, int x2, int y2, unsigned int color)
{
    lcd_Line(x1,y1,x2,y1,color);
    lcd_Line(x2,y1,x2,y2,color);
    lcd_Line(x1,y2,x2,y2,color);
    lcd_Line(x1,y1,x1,y2,color);
}

void lcd_PutPixelx2(int hx, int hy, unsigned int color)
{
	int x=hx*2;
	int y=hy*2;

	int i,j;

	unsigned int convert;
	convert = (0xff)<<24 | color;

	if ((hx < PANEL_W) && (hy < PANEL_H))
	{
		for(i=0;i<4;i++)
		{
			for(j=0;j<4;j++)
			{
				*(fbpMem+((y+i)*(PANEL_W))+x+j) = convert;
			}
		}
	}
}

void lcd_ClearScr(unsigned int color)
{

	int i,j;

    for(j=0;j<PANEL_H;j++)
    	for(i=0;i<PANEL_W;i++)
        	lcd_PutPixel(i,j,color);

}


void lcd_PutPixel_font(int x, int y,unsigned int color, int fontSize)
{

if (fontSize) 
	lcd_PutPixelx2(x,y,color);
else
	lcd_PutPixel(x,y,color);
}

/****************************************************************************
** module name	: lcd_draw_font()
** description		: 
****************************************************************************/
void lcd_draw_font(int x,int y,int fgc,int bgc,unsigned char len, unsigned char * data, unsigned char fontSize)
{
	int i,j,k;
	unsigned char *font_ptr;
	unsigned char *buf;
	unsigned char x1,c;
	unsigned char tmp_buf[40];
	unsigned int disp_data;
	x1=x/8;
	font_ptr = (unsigned char *)&font_eng_8x16[0];

	for(k=0;k<len;k++) {
		tmp_buf[k] = *(data+k);
//		khb_dbg_msg(" [tmp_buf=%02x] [x1=%d] [ptr=%d]\n",tmp_buf[k],x1,32*(tmp_buf[k]-0x20));
		if(tmp_buf[k]>0x20 && tmp_buf[k]<0x7F)	
			buf=font_ptr+(32*(tmp_buf[k]-0x20));
		else
			buf=font_ptr;
		
		for(j=y,i=0;i<32;j++,i+=2) {
			c = *(buf+i+1);
			(c&0x80) ? (disp_data = fgc) : (disp_data = bgc); 
			lcd_PutPixel_font(x+k*8,j,disp_data, fontSize);
			(c&0x40) ? (disp_data = fgc) : (disp_data = bgc); 
			lcd_PutPixel_font(x+k*8+1,j,disp_data, fontSize);
			(c&0x20) ? (disp_data = fgc) : (disp_data = bgc); 
			lcd_PutPixel_font(x+k*8+2,j,disp_data, fontSize);
			(c&0x10) ? (disp_data = fgc) : (disp_data = bgc); 
			lcd_PutPixel_font(x+k*8+3,j,disp_data, fontSize);
			(c&0x08) ? (disp_data = fgc) : (disp_data = bgc); 
			lcd_PutPixel_font(x+k*8+4,j,disp_data, fontSize);
			(c&0x04) ? (disp_data = fgc) : (disp_data = bgc); 
			lcd_PutPixel_font(x+k*8+5,j,disp_data, fontSize);
			(c&0x02) ? (disp_data = fgc) : (disp_data = bgc); 
			lcd_PutPixel_font(x+k*8+6,j,disp_data, fontSize);
			(c&0x01) ? (disp_data = fgc) : (disp_data = bgc); 
			lcd_PutPixel_font(x+k*8+7,j,disp_data, fontSize);
		}	
	}
}
/*
 * AY/YM emulator include file
 * (C) Matt Westcott "gasman"
 * GPL-2.0 License
 * https://raw.githubusercontent.com/gasman/libayemu
 */

#ifndef _AYEMU_ay8912_h
#define _AYEMU_ay8912_h

#include <stddef.h>

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */

/* Make sure the correct platform symbols are defined */
#if !defined(WIN32) && defined(_WIN32)
#define WIN32
#endif /* Windows */

/* Some compilers use a special export keyword */
#ifndef DECLSPEC
# ifdef __BEOS__
#  if defined(__GNUC__)
#   define DECLSPEC	__declspec(dllexport)
#  else
#   define DECLSPEC	__declspec(export)
#  endif
# else
# ifdef WIN32
#  ifdef __BORLANDC__
#   ifdef BUILD_SDL
#    define DECLSPEC 
#   else
#    define DECLSPEC __declspec(dllimport)
#   endif
#  else
#   define DECLSPEC	__declspec(dllexport)
#  endif
# else
#  define DECLSPEC
# endif
# endif
#endif

#define EXTERN extern DECLSPEC

/* typedefs for 32-bit architecture */
typedef unsigned char	Uint8;
typedef signed char	Sint8;
typedef unsigned short	Uint16;
typedef signed short	Sint16;
typedef unsigned int	Uint32;
typedef signed int	Sint32;

BEGIN_C_DECLS

/** Types of stereo.
		The codes of stereo types used for generage sound. */
typedef enum
{
	AYEMU_MONO = 0,
	AYEMU_ABC,
	AYEMU_ACB,
	AYEMU_BAC,
	AYEMU_BCA,
	AYEMU_CAB,
	AYEMU_CBA,
	AYEMU_STEREO_CUSTOM = 255
} ayemu_stereo_t;

/** Sound chip type.
		Constant for identify used chip for emulation */
typedef enum {
	AYEMU_AY,			/**< default AY chip (lion17 for now) */
	AYEMU_YM,			/**< default YM chip (lion17 for now) */
	AYEMU_AY_LION17,		/**< emulate AY with Lion17 table */
	AYEMU_YM_LION17,		/**< emulate YM with Lion17 table */
	AYEMU_AY_KAY,			/**< emulate AY with HACKER KAY table */
	AYEMU_YM_KAY,			/**< emulate YM with HACKER KAY table */
	AYEMU_AY_LOG,			/**< emulate AY with logariphmic table */
	AYEMU_YM_LOG,			/**< emulate YM with logariphmic table */
	AYEMU_AY_CUSTOM,		/**< use AY with custom table. */
	AYEMU_YM_CUSTOM		/**< use YM with custom table. */
} ayemu_chip_t;

/** parsed by #ayemu_set_regs() AY registers data \internal */
typedef struct
{
	int tone_a;           /**< R0, R1 */
	int tone_b;		/**< R2, R3 */	
	int tone_c;		/**< R4, R5 */
	int noise;		/**< R6 */
	int R7_tone_a;	/**< R7 bit 0 */
	int R7_tone_b;	/**< R7 bit 1 */
	int R7_tone_c;	/**< R7 bit 2 */
	int R7_noise_a;	/**< R7 bit 3 */
	int R7_noise_b;	/**< R7 bit 4 */
	int R7_noise_c;	/**< R7 bit 5 */
	int vol_a;		/**< R8 bits 3-0 */
	int vol_b;		/**< R9 bits 3-0 */
	int vol_c;		/**< R10 bits 3-0 */
	int env_a;		/**< R8 bit 4 */
	int env_b;		/**< R9 bit 4 */
	int env_c;		/**< R10 bit 4 */
	int env_freq;		/**< R11, R12 */
	int env_style;	/**< R13 */
}
ayemu_regdata_t;


/** Output sound format \internal */
typedef struct
{
	int freq;			/**< sound freq */
	int channels;			/**< channels (1-mono, 2-stereo) */
	int bpc;			/**< bits (8 or 16) */
}
ayemu_sndfmt_t;

/**
 * \defgroup libayemu Functions for emulate AY/YM chip
 */
/*@{*/

/** Data structure for sound chip emulation \internal
 * 
 */
typedef struct
{
	/* emulator settings */
	int table[32];		/**< table of volumes for chip */
	ayemu_chip_t type;		/**< general chip type (\b AYEMU_AY or \b AYEMU_YM) */
	int ChipFreq;			/**< chip emulator frequency */
	int eq[6];			/**< volumes for channels.
					 Array contains 6 elements: 
					 A left, A right, B left, B right, C left and C right;
					 range -100...100 */
	ayemu_regdata_t regs;		/**< parsed registers data */
	ayemu_sndfmt_t sndfmt;	/**< output sound format */

	/* flags */
	int magic;			/**< structure initialized flag */
	int default_chip_flag;	/**< =1 after init, resets in #ayemu_set_chip_type() */
	int default_stereo_flag;	/**< =1 after init, resets in #ayemu_set_stereo() */
	int default_sound_format_flag; /**< =1 after init, resets in #ayemu_set_sound_format() */
	int dirty;			/**< dirty flag. Sets if any emulator properties changed */

	int bit_a;			/**< state of channel A generator */
	int bit_b;			/**< state of channel B generator */
	int bit_c;			/**< state of channel C generator */
	int bit_n;			/**< current generator state */
	int cnt_a;			/**< back counter of A */
	int cnt_b;			/**< back counter of B */
	int cnt_c;			/**< back counter of C */
	int cnt_n;			/**< back counter of noise generator */
	int cnt_e;			/**< back counter of envelop generator */
	int ChipTacts_per_outcount;   /**< chip's counts per one sound signal count */
	int Amp_Global;		/**< scale factor for amplitude */
	int vols[6][32];              /**< stereo type (channel volumes) and chip table.
					 This cache calculated by #table and #eq  */
	int EnvNum;		        /**< number of current envilopment (0...15) */
	int env_pos;			/**< current position in envelop (0...127) */
	int Cur_Seed;		        /**< random numbers counter */
}
ayemu_ay_t;

EXTERN void
ayemu_init(ayemu_ay_t *ay);

EXTERN void
ayemu_reset(ayemu_ay_t *ay);

EXTERN int 
ayemu_set_chip_type(ayemu_ay_t *ay, ayemu_chip_t chip, int *custom_table);

EXTERN void
ayemu_set_chip_freq(ayemu_ay_t *ay, int chipfreq);

EXTERN int
ayemu_set_stereo(ayemu_ay_t *ay, ayemu_stereo_t stereo, int *custom_eq);

EXTERN int
ayemu_set_sound_format (ayemu_ay_t *ay, int freq, int chans, int bits);

EXTERN void 
ayemu_set_regs (ayemu_ay_t *ay, unsigned char *regs);

EXTERN void 
ayemu_set_reg (ayemu_ay_t *ay, int reg, unsigned char value);

EXTERN unsigned char 
ayemu_get_reg(ayemu_ay_t *ay, int reg);

EXTERN void*
ayemu_gen_sound (ayemu_ay_t *ay, void *buf, size_t frame_count);


/*@}*/

END_C_DECLS

#endif

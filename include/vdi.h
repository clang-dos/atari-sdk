/*
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (c) 2026, clang-dos
 *
 * vdi.h: the GEM VDI (Virtual Device Interface), TOS's graphics library.
 * The names and arguments follow the GEM documentation and gemlib. The
 * functions are in libgem.a.
 */
#ifndef _VDI_H
#define _VDI_H

#ifdef __cplusplus
extern "C" {
#endif

/* A memory form: a bitmap in screen format or the standard format. */
typedef struct {
    void *fd_addr; /* NULL is the screen */
    short fd_w;
    short fd_h;
    short fd_wdwidth; /* width in 16-bit words */
    short fd_stand;   /* 1 standard format, 0 device format */
    short fd_nplanes;
    short fd_r1, fd_r2, fd_r3;
} MFDB;

/* Writing modes. */
#define MD_REPLACE 1
#define MD_TRANS 2
#define MD_XOR 3
#define MD_ERASE 4

/* Fill interiors. */
#define FIS_HOLLOW 0
#define FIS_SOLID 1
#define FIS_PATTERN 2
#define FIS_HATCH 3
#define FIS_USER 4

/* Colours. */
#define G_WHITE 0
#define G_BLACK 1
#define G_RED 2
#define G_GREEN 3
#define G_BLUE 4
#define G_CYAN 5
#define G_YELLOW 6
#define G_MAGENTA 7
#define G_LWHITE 8
#define G_LBLACK 9
#define G_LRED 10
#define G_LGREEN 11
#define G_LBLUE 12
#define G_LCYAN 13
#define G_LYELLOW 14
#define G_LMAGENTA 15
#define WHITE G_WHITE
#define BLACK G_BLACK
#define RED G_RED
#define GREEN G_GREEN
#define BLUE G_BLUE
#define CYAN G_CYAN
#define YELLOW G_YELLOW
#define MAGENTA G_MAGENTA

/* Line types. */
#define SOLID 1
#define LONGDASH 2
#define DOT 3
#define DASHDOT 4
#define DASH 5
#define DASHDOTDOT 6
#define USERLINE 7

/* Text effects. */
#define TXT_NORMAL 0x0000
#define TXT_THICKENED 0x0001
#define TXT_LIGHT 0x0002
#define TXT_SKEWED 0x0004
#define TXT_UNDERLINED 0x0008
#define TXT_OUTLINED 0x0010
#define TXT_SHADOWED 0x0020

/* Text alignment. */
#define TA_LEFT 0
#define TA_CENTER 1
#define TA_RIGHT 2
#define TA_BASE 0
#define TA_HALF 1
#define TA_ASCENT 2
#define TA_BOTTOM 3
#define TA_DESCENT 4
#define TA_TOP 5

/* vro_cpyfm() logic operations. */
#define ALL_WHITE 0
#define S_AND_D 1
#define S_AND_NOTD 2
#define S_ONLY 3
#define NOTS_AND_D 4
#define D_ONLY 5
#define S_XOR_D 6
#define S_OR_D 7
#define NOT_SORD 8
#define NOT_SXORD 9
#define D_INVERT 10
#define S_OR_NOTD 11
#define NOT_D 12
#define NOTS_OR_D 13
#define NOT_SANDD 14
#define ALL_BLACK 15

/* Control */
void  v_opnwk(short work_in[11], short *handle, short work_out[57]);
void  v_clswk(short handle);
void  v_opnvwk(short work_in[11], short *handle, short work_out[57]);
void  v_clsvwk(short handle);
void  v_clrwk(short handle);
void  v_updwk(short handle);
void  vs_clip(short handle, short clip_flag, short pxy[4]);
short vst_load_fonts(short handle, short select);
void  vst_unload_fonts(short handle, short select);

/* Output */
void  v_pline(short handle, short count, short pxy[]);
void  v_pmarker(short handle, short count, short pxy[]);
void  v_gtext(short handle, short x, short y, const char *str);
void  v_fillarea(short handle, short count, short pxy[]);
void  v_bar(short handle, short pxy[4]);
void  v_arc(short handle, short x, short y, short radius, short begang, short endang);
void  v_pieslice(short handle, short x, short y, short radius, short begang, short endang);
void  v_circle(short handle, short x, short y, short radius);
void  v_ellipse(short handle, short x, short y, short xrad, short yrad);
void  v_ellarc(short handle, short x, short y, short xrad, short yrad, short begang,
               short endang);
void  v_ellpie(short handle, short x, short y, short xrad, short yrad, short begang,
               short endang);
void  v_rbox(short handle, short pxy[4]);
void  v_rfbox(short handle, short pxy[4]);
void  v_justified(short handle, short x, short y, const char *str, short length, short wflag,
                  short cflag);
void  vr_recfl(short handle, short pxy[4]);

/* Attributes */
short vswr_mode(short handle, short mode);
void  vs_color(short handle, short index, short rgb[3]);
short vsl_type(short handle, short style);
void  vsl_udsty(short handle, short pattern);
short vsl_width(short handle, short width);
short vsl_color(short handle, short color);
void  vsl_ends(short handle, short begstyle, short endstyle);
short vsm_type(short handle, short symbol);
short vsm_height(short handle, short height);
short vsm_color(short handle, short color);
short vst_height(short handle, short height, short *charw, short *charh, short *cellw,
                 short *cellh);
short vst_point(short handle, short point, short *charw, short *charh, short *cellw,
                short *cellh);
short vst_rotation(short handle, short angle);
short vst_font(short handle, short font);
short vst_color(short handle, short color);
short vst_effects(short handle, short effects);
void  vst_alignment(short handle, short hin, short vin, short *hout, short *vout);
short vsf_interior(short handle, short style);
short vsf_style(short handle, short style);
short vsf_color(short handle, short color);
short vsf_perimeter(short handle, short visible);
void  vsf_udpat(short handle, short pattern[], short planes);

/* Raster */
void  vro_cpyfm(short handle, short mode, short pxy[8], MFDB *src, MFDB *dst);
void  vrt_cpyfm(short handle, short mode, short pxy[8], MFDB *src, MFDB *dst, short color[2]);
void  vr_trnfm(short handle, MFDB *src, MFDB *dst);
void  v_get_pixel(short handle, short x, short y, short *pel, short *index);

/* Input */
void  v_show_c(short handle, short reset);
void  v_hide_c(short handle);
void  vq_mouse(short handle, short *status, short *x, short *y);
void  vq_key_s(short handle, short *state);
void  vsc_form(short handle, short form[37]);
void  vex_timv(short handle, void *tim_addr, void **otim_addr, short *tim_conv);
void  vex_butv(short handle, void *pusrcode, void **psavcode);
void  vex_motv(short handle, void *pusrcode, void **psavcode);
void  vex_curv(short handle, void *pusrcode, void **psavcode);

/* Inquiry */
void  vq_extnd(short handle, short flag, short work_out[57]);
short vq_color(short handle, short index, short flag, short rgb[3]);
void  vqt_extent(short handle, const char *str, short extent[8]);
short vqt_name(short handle, short index, char name[33]);
void  vqt_attributes(short handle, short attrib[10]);

/* Escapes for the text screen */
void  vq_chcells(short handle, short *rows, short *cols);
void  v_exit_cur(short handle);
void  v_enter_cur(short handle);
void  v_curhome(short handle);
void  v_curtext(short handle, const char *str);
void  v_eeos(short handle);
void  v_eeol(short handle);

/* The low level call, for functions not listed here. */
extern short vdi_control[12];
extern short vdi_intin[256];
extern short vdi_ptsin[256];
extern short vdi_intout[64];
extern short vdi_ptsout[32];
void vdi(short opcode, short nptsin, short nintin, short subop, short handle);

#ifdef __cplusplus
}
#endif
#endif /* _VDI_H */

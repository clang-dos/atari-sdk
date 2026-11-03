/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, clang-dos
 *
 * GEM VDI bindings. Each call fills the control array (opcode, number of
 * points in, number of integers in, sub-opcode, workstation handle) and
 * the input arrays, puts the address of the parameter block in d1 and 115
 * in d0, and calls trap #2. The VDI writes the number of points and
 * integers it returns to control[2] and control[4].
 */
#include <string.h>
#include <vdi.h>

short vdi_control[12];
short vdi_intin[256];
short vdi_ptsin[256];
short vdi_intout[64];
short vdi_ptsout[32];

static void
vdi_trap(void **pb)
{
    __asm__ __volatile__("move.l %0,%%d1\n\t"
                         "moveq #115,%%d0\n\t"
                         "trap #2"
                         :
                         : "r"(pb)
                         : "d0", "d1", "d2", "a0", "a1", "a2", "cc", "memory");
}

static void
vdi_arrays(short *intin, short *ptsin, short *intout, short *ptsout)
{
    void *pb[5] = { vdi_control, intin, ptsin, intout, ptsout };
    vdi_trap(pb);
}

void
vdi(short opcode, short nptsin, short nintin, short subop, short handle)
{
    vdi_control[0] = opcode;
    vdi_control[1] = nptsin;
    vdi_control[3] = nintin;
    vdi_control[5] = subop;
    vdi_control[6] = handle;
    vdi_arrays(vdi_intin, vdi_ptsin, vdi_intout, vdi_ptsout);
}

#define CI vdi_intin
#define CP vdi_ptsin
#define OI vdi_intout
#define OP vdi_ptsout

/* Control */

static void
open_wk(short op, short work_in[11], short *handle, short work_out[57])
{
    vdi_control[0] = op;
    vdi_control[1] = 0;
    vdi_control[3] = 11;
    vdi_control[5] = 0;
    vdi_control[6] = *handle;
    vdi_arrays(work_in, vdi_ptsin, work_out, work_out + 45);
    *handle = vdi_control[6];
}

void
v_opnwk(short work_in[11], short *handle, short work_out[57])
{
    open_wk(1, work_in, handle, work_out);
}

void
v_clswk(short handle)
{
    vdi(2, 0, 0, 0, handle);
}

void
v_opnvwk(short work_in[11], short *handle, short work_out[57])
{
    open_wk(100, work_in, handle, work_out);
}

void
v_clsvwk(short handle)
{
    vdi(101, 0, 0, 0, handle);
}

void
v_clrwk(short handle)
{
    vdi(3, 0, 0, 0, handle);
}

void
v_updwk(short handle)
{
    vdi(4, 0, 0, 0, handle);
}

static void
pts(const short *pxy, int n)
{
    memcpy(CP, pxy, n * 2 * sizeof(short));
}

void
vs_clip(short handle, short clip_flag, short pxy[4])
{
    CI[0] = clip_flag;
    if (pxy)
        pts(pxy, 2);
    else
        memset(CP, 0, 4 * sizeof(short));
    vdi(129, 2, 1, 0, handle);
}

short
vst_load_fonts(short handle, short select)
{
    CI[0] = select;
    vdi(119, 0, 1, 0, handle);
    return OI[0];
}

void
vst_unload_fonts(short handle, short select)
{
    CI[0] = select;
    vdi(120, 0, 1, 0, handle);
}

/* Output */

static void
polyop(short op, short handle, short count, short pxy[])
{
    if (count > 128)
        count = 128;
    pts(pxy, count);
    vdi(op, count, 0, 0, handle);
}

void
v_pline(short handle, short count, short pxy[])
{
    polyop(6, handle, count, pxy);
}

void
v_pmarker(short handle, short count, short pxy[])
{
    polyop(7, handle, count, pxy);
}

void
v_fillarea(short handle, short count, short pxy[])
{
    polyop(9, handle, count, pxy);
}

static int
str_in(const char *s)
{
    int n = 0;
    while (s[n] && n < 255) {
        CI[n] = (unsigned char)s[n];
        n++;
    }
    return n;
}

void
v_gtext(short handle, short x, short y, const char *str)
{
    CP[0] = x;
    CP[1] = y;
    vdi(8, 1, str_in(str), 0, handle);
}

/* The generalized drawing primitives: opcode 11. */
void
v_bar(short handle, short pxy[4])
{
    pts(pxy, 2);
    vdi(11, 2, 0, 1, handle);
}

static void
arc(short sub, short handle, short x, short y, short radius, short begang, short endang)
{
    CI[0] = begang;
    CI[1] = endang;
    CP[0] = x;
    CP[1] = y;
    CP[2] = CP[3] = CP[4] = CP[5] = 0;
    CP[6] = radius;
    CP[7] = 0;
    vdi(11, 4, 2, sub, handle);
}

void
v_arc(short handle, short x, short y, short radius, short begang, short endang)
{
    arc(2, handle, x, y, radius, begang, endang);
}

void
v_pieslice(short handle, short x, short y, short radius, short begang, short endang)
{
    arc(3, handle, x, y, radius, begang, endang);
}

void
v_circle(short handle, short x, short y, short radius)
{
    CP[0] = x;
    CP[1] = y;
    CP[2] = CP[3] = 0;
    CP[4] = radius;
    CP[5] = 0;
    vdi(11, 3, 0, 4, handle);
}

void
v_ellipse(short handle, short x, short y, short xrad, short yrad)
{
    CP[0] = x;
    CP[1] = y;
    CP[2] = xrad;
    CP[3] = yrad;
    vdi(11, 2, 0, 5, handle);
}

static void
ellarc(short sub, short handle, short x, short y, short xrad, short yrad, short begang,
       short endang)
{
    CI[0] = begang;
    CI[1] = endang;
    CP[0] = x;
    CP[1] = y;
    CP[2] = xrad;
    CP[3] = yrad;
    vdi(11, 2, 2, sub, handle);
}

void
v_ellarc(short handle, short x, short y, short xrad, short yrad, short begang, short endang)
{
    ellarc(6, handle, x, y, xrad, yrad, begang, endang);
}

void
v_ellpie(short handle, short x, short y, short xrad, short yrad, short begang, short endang)
{
    ellarc(7, handle, x, y, xrad, yrad, begang, endang);
}

void
v_rbox(short handle, short pxy[4])
{
    pts(pxy, 2);
    vdi(11, 2, 0, 8, handle);
}

void
v_rfbox(short handle, short pxy[4])
{
    pts(pxy, 2);
    vdi(11, 2, 0, 9, handle);
}

void
v_justified(short handle, short x, short y, const char *str, short length, short wflag,
            short cflag)
{
    CI[0] = wflag;
    CI[1] = cflag;
    int n = 0;
    while (str[n] && n < 253) {
        CI[2 + n] = (unsigned char)str[n];
        n++;
    }
    CP[0] = x;
    CP[1] = y;
    CP[2] = length;
    CP[3] = 0;
    vdi(11, 2, 2 + n, 10, handle);
}

void
vr_recfl(short handle, short pxy[4])
{
    pts(pxy, 2);
    vdi(114, 2, 0, 0, handle);
}

/* Attributes */

static short
set1(short op, short handle, short v)
{
    CI[0] = v;
    vdi(op, 0, 1, 0, handle);
    return OI[0];
}

short
vswr_mode(short handle, short mode)
{
    return set1(32, handle, mode);
}

void
vs_color(short handle, short index, short rgb[3])
{
    CI[0] = index;
    CI[1] = rgb[0];
    CI[2] = rgb[1];
    CI[3] = rgb[2];
    vdi(14, 0, 4, 0, handle);
}

short
vsl_type(short handle, short style)
{
    return set1(15, handle, style);
}

void
vsl_udsty(short handle, short pattern)
{
    set1(113, handle, pattern);
}

short
vsl_width(short handle, short width)
{
    CP[0] = width;
    CP[1] = 0;
    vdi(16, 1, 0, 0, handle);
    return OP[0];
}

short
vsl_color(short handle, short color)
{
    return set1(17, handle, color);
}

void
vsl_ends(short handle, short begstyle, short endstyle)
{
    CI[0] = begstyle;
    CI[1] = endstyle;
    vdi(108, 0, 2, 0, handle);
}

short
vsm_type(short handle, short symbol)
{
    return set1(18, handle, symbol);
}

short
vsm_height(short handle, short height)
{
    CP[0] = 0;
    CP[1] = height;
    vdi(19, 1, 0, 0, handle);
    return OP[1];
}

short
vsm_color(short handle, short color)
{
    return set1(20, handle, color);
}

static void
char_size(short *charw, short *charh, short *cellw, short *cellh)
{
    if (charw)
        *charw = OP[0];
    if (charh)
        *charh = OP[1];
    if (cellw)
        *cellw = OP[2];
    if (cellh)
        *cellh = OP[3];
}

short
vst_height(short handle, short height, short *charw, short *charh, short *cellw, short *cellh)
{
    CP[0] = 0;
    CP[1] = height;
    vdi(12, 1, 0, 0, handle);
    char_size(charw, charh, cellw, cellh);
    return OP[1];
}

short
vst_point(short handle, short point, short *charw, short *charh, short *cellw, short *cellh)
{
    CI[0] = point;
    vdi(107, 0, 1, 0, handle);
    char_size(charw, charh, cellw, cellh);
    return OI[0];
}

short
vst_rotation(short handle, short angle)
{
    return set1(13, handle, angle);
}

short
vst_font(short handle, short font)
{
    return set1(21, handle, font);
}

short
vst_color(short handle, short color)
{
    return set1(22, handle, color);
}

short
vst_effects(short handle, short effects)
{
    return set1(106, handle, effects);
}

void
vst_alignment(short handle, short hin, short vin, short *hout, short *vout)
{
    CI[0] = hin;
    CI[1] = vin;
    vdi(39, 0, 2, 0, handle);
    if (hout)
        *hout = OI[0];
    if (vout)
        *vout = OI[1];
}

short
vsf_interior(short handle, short style)
{
    return set1(23, handle, style);
}

short
vsf_style(short handle, short style)
{
    return set1(24, handle, style);
}

short
vsf_color(short handle, short color)
{
    return set1(25, handle, color);
}

short
vsf_perimeter(short handle, short visible)
{
    return set1(104, handle, visible);
}

void
vsf_udpat(short handle, short pattern[], short planes)
{
    int n = 16 * planes;
    if (n > 256)
        n = 256;
    memcpy(CI, pattern, n * sizeof(short));
    vdi(112, 0, n, 0, handle);
}

/* Raster */

static void
mfdbs(MFDB *src, MFDB *dst)
{
    *(MFDB **)&vdi_control[7] = src;
    *(MFDB **)&vdi_control[9] = dst;
}

void
vro_cpyfm(short handle, short mode, short pxy[8], MFDB *src, MFDB *dst)
{
    CI[0] = mode;
    pts(pxy, 4);
    mfdbs(src, dst);
    vdi(109, 4, 1, 0, handle);
}

void
vrt_cpyfm(short handle, short mode, short pxy[8], MFDB *src, MFDB *dst, short color[2])
{
    CI[0] = mode;
    CI[1] = color[0];
    CI[2] = color[1];
    pts(pxy, 4);
    mfdbs(src, dst);
    vdi(121, 4, 3, 0, handle);
}

void
vr_trnfm(short handle, MFDB *src, MFDB *dst)
{
    mfdbs(src, dst);
    vdi(110, 0, 0, 0, handle);
}

void
v_get_pixel(short handle, short x, short y, short *pel, short *index)
{
    CP[0] = x;
    CP[1] = y;
    vdi(105, 1, 0, 0, handle);
    *pel = OI[0];
    *index = OI[1];
}

/* Input */

void
v_show_c(short handle, short reset)
{
    set1(122, handle, reset);
}

void
v_hide_c(short handle)
{
    vdi(123, 0, 0, 0, handle);
}

void
vq_mouse(short handle, short *status, short *x, short *y)
{
    vdi(124, 0, 0, 0, handle);
    *status = OI[0];
    *x = OP[0];
    *y = OP[1];
}

void
vq_key_s(short handle, short *state)
{
    vdi(128, 0, 0, 0, handle);
    *state = OI[0];
}

void
vsc_form(short handle, short form[37])
{
    memcpy(CI, form, 37 * sizeof(short));
    vdi(111, 0, 37, 0, handle);
}

/* The exchange calls pass the new vector in control[7..8] and return the
 * old one in control[9..10]. */
static void
vex(short op, short handle, void *new, void **old)
{
    *(void **)&vdi_control[7] = new;
    vdi(op, 0, 0, 0, handle);
    if (old)
        *old = *(void **)&vdi_control[9];
}

void
vex_timv(short handle, void *tim_addr, void **otim_addr, short *tim_conv)
{
    vex(118, handle, tim_addr, otim_addr);
    if (tim_conv)
        *tim_conv = OI[0];
}

void
vex_butv(short handle, void *pusrcode, void **psavcode)
{
    vex(125, handle, pusrcode, psavcode);
}

void
vex_motv(short handle, void *pusrcode, void **psavcode)
{
    vex(126, handle, pusrcode, psavcode);
}

void
vex_curv(short handle, void *pusrcode, void **psavcode)
{
    vex(127, handle, pusrcode, psavcode);
}

/* Inquiry */

void
vq_extnd(short handle, short flag, short work_out[57])
{
    vdi_control[0] = 102;
    vdi_control[1] = 0;
    vdi_control[3] = 1;
    vdi_control[5] = 0;
    vdi_control[6] = handle;
    CI[0] = flag;
    vdi_arrays(vdi_intin, vdi_ptsin, work_out, work_out + 45);
}

short
vq_color(short handle, short index, short flag, short rgb[3])
{
    CI[0] = index;
    CI[1] = flag;
    vdi(26, 0, 2, 0, handle);
    rgb[0] = OI[1];
    rgb[1] = OI[2];
    rgb[2] = OI[3];
    return OI[0];
}

void
vqt_extent(short handle, const char *str, short extent[8])
{
    vdi(116, 0, str_in(str), 0, handle);
    memcpy(extent, OP, 8 * sizeof(short));
}

short
vqt_name(short handle, short index, char name[33])
{
    CI[0] = index;
    vdi(130, 0, 1, 0, handle);
    for (int i = 0; i < 32; i++)
        name[i] = (char)OI[1 + i];
    name[32] = '\0';
    return OI[0];
}

void
vqt_attributes(short handle, short attrib[10])
{
    vdi(38, 0, 0, 0, handle);
    for (int i = 0; i < 6; i++)
        attrib[i] = OI[i];
    for (int i = 0; i < 4; i++)
        attrib[6 + i] = OP[i];
}

/* Escapes: opcode 5. */

void
vq_chcells(short handle, short *rows, short *cols)
{
    vdi(5, 0, 0, 1, handle);
    *rows = OI[0];
    *cols = OI[1];
}

void
v_exit_cur(short handle)
{
    vdi(5, 0, 0, 2, handle);
}

void
v_enter_cur(short handle)
{
    vdi(5, 0, 0, 3, handle);
}

void
v_curhome(short handle)
{
    vdi(5, 0, 0, 8, handle);
}

void
v_eeos(short handle)
{
    vdi(5, 0, 0, 9, handle);
}

void
v_eeol(short handle)
{
    vdi(5, 0, 0, 10, handle);
}

void
v_curtext(short handle, const char *str)
{
    vdi(5, 0, str_in(str), 12, handle);
}

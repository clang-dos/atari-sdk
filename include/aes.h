/*
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (c) 2026, clang-dos
 *
 * aes.h: the GEM AES (Application Environment Services), the window,
 * menu, dialog and event manager of TOS. The names and arguments follow
 * the GEM documentation and gemlib, so most GEM sources build unchanged.
 * The functions are in libgem.a, which the driver links when it's in the
 * sysroot.
 */
#ifndef _AES_H
#define _AES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    short g_x, g_y, g_w, g_h;
} GRECT;

typedef struct {
    char *te_ptext;
    char *te_ptmplt;
    char *te_pvalid;
    short te_font;
    short te_fontid;
    short te_just;
    short te_color;
    short te_fontsize;
    short te_thickness;
    short te_txtlen;
    short te_tmplen;
} TEDINFO;

typedef struct {
    short *ib_pmask;
    short *ib_pdata;
    char *ib_ptext;
    short ib_char;
    short ib_xchar, ib_ychar;
    short ib_xicon, ib_yicon, ib_wicon, ib_hicon;
    short ib_xtext, ib_ytext, ib_wtext, ib_htext;
} ICONBLK;

typedef struct {
    short *bi_pdata;
    short bi_wb;
    short bi_hl;
    short bi_x, bi_y;
    short bi_color;
} BITBLK;

typedef union {
    long index;
    char *free_string;
    TEDINFO *tedinfo;
    ICONBLK *iconblk;
    BITBLK *bitblk;
    void *userblk;
} OBSPEC;

typedef struct {
    short ob_next;
    short ob_head;
    short ob_tail;
    unsigned short ob_type;
    unsigned short ob_flags;
    unsigned short ob_state;
    OBSPEC ob_spec;
    short ob_x, ob_y, ob_width, ob_height;
} OBJECT;

typedef struct {
    short mf_xhot, mf_yhot;
    short mf_nplanes;
    short mf_fg, mf_bg;
    short mf_mask[16];
    short mf_data[16];
} MFORM;

/* The AES global array; aes_global[0] is the AES version. */
extern short aes_global[15];
extern short gl_apid;       /* this application's id, from appl_init() */
extern short gl_ap_version; /* the AES version, from appl_init() */

/* Object types. */
#define G_BOX 20
#define G_TEXT 21
#define G_BOXTEXT 22
#define G_IMAGE 23
#define G_USERDEF 24
#define G_IBOX 25
#define G_BUTTON 26
#define G_BOXCHAR 27
#define G_STRING 28
#define G_FTEXT 29
#define G_FBOXTEXT 30
#define G_ICON 31
#define G_TITLE 32
#define G_CICON 33

/* Object flags. */
#define OF_NONE 0x0000
#define OF_SELECTABLE 0x0001
#define OF_DEFAULT 0x0002
#define OF_EXIT 0x0004
#define OF_EDITABLE 0x0008
#define OF_RBUTTON 0x0010
#define OF_LASTOB 0x0020
#define OF_TOUCHEXIT 0x0040
#define OF_HIDETREE 0x0080
#define OF_INDIRECT 0x0100
#define NONE OF_NONE
#define SELECTABLE OF_SELECTABLE
#define DEFAULT OF_DEFAULT
#define EXIT OF_EXIT
#define EDITABLE OF_EDITABLE
#define RBUTTON OF_RBUTTON
#define LASTOB OF_LASTOB
#define TOUCHEXIT OF_TOUCHEXIT
#define HIDETREE OF_HIDETREE
#define INDIRECT OF_INDIRECT

/* Object states. */
#define OS_NORMAL 0x00
#define OS_SELECTED 0x01
#define OS_CROSSED 0x02
#define OS_CHECKED 0x04
#define OS_DISABLED 0x08
#define OS_OUTLINED 0x10
#define OS_SHADOWED 0x20
#define NORMAL OS_NORMAL
#define SELECTED OS_SELECTED
#define CROSSED OS_CROSSED
#define CHECKED OS_CHECKED
#define DISABLED OS_DISABLED
#define OUTLINED OS_OUTLINED
#define SHADOWED OS_SHADOWED

#define ROOT 0
#define NIL (-1)
#define MAX_DEPTH 8

/* Events for evnt_multi(). */
#define MU_KEYBD 0x0001
#define MU_BUTTON 0x0002
#define MU_M1 0x0004
#define MU_M2 0x0008
#define MU_MESAG 0x0010
#define MU_TIMER 0x0020

/* Messages. */
#define MN_SELECTED 10
#define WM_REDRAW 20
#define WM_TOPPED 21
#define WM_CLOSED 22
#define WM_FULLED 23
#define WM_ARROWED 24
#define WM_HSLID 25
#define WM_VSLID 26
#define WM_SIZED 27
#define WM_MOVED 28
#define WM_NEWTOP 29
#define WM_UNTOPPED 30
#define WM_ONTOP 31
#define WM_BOTTOMED 33
#define WM_ICONIFY 34
#define WM_UNICONIFY 35
#define WM_ALLICONIFY 36
#define AC_OPEN 40
#define AC_CLOSE 41
#define AP_TERM 50
#define AP_TFAIL 51
#define SHUT_COMPLETED 60
#define RESCHG_COMPLETED 61
#define AP_DRAGDROP 63

/* WM_ARROWED actions. */
#define WA_UPPAGE 0
#define WA_DNPAGE 1
#define WA_UPLINE 2
#define WA_DNLINE 3
#define WA_LFPAGE 4
#define WA_RTPAGE 5
#define WA_LFLINE 6
#define WA_RTLINE 7

/* Window parts for wind_create(). */
#define NAME 0x0001
#define CLOSER 0x0002
#define FULLER 0x0004
#define MOVER 0x0008
#define INFO 0x0010
#define SIZER 0x0020
#define UPARROW 0x0040
#define DNARROW 0x0080
#define VSLIDE 0x0100
#define LFARROW 0x0200
#define RTARROW 0x0400
#define HSLIDE 0x0800
#define SMALLER 0x4000

/* wind_get() and wind_set() fields. */
#define WF_KIND 1
#define WF_NAME 2
#define WF_INFO 3
#define WF_WORKXYWH 4
#define WF_CURRXYWH 5
#define WF_PREVXYWH 6
#define WF_FULLXYWH 7
#define WF_HSLIDE 8
#define WF_VSLIDE 9
#define WF_TOP 10
#define WF_FIRSTXYWH 11
#define WF_NEXTXYWH 12
#define WF_NEWDESK 14
#define WF_HSLSIZE 15
#define WF_VSLSIZE 16
#define WF_SCREEN 17
#define WF_COLOR 18
#define WF_DCOLOR 19
#define WF_BOTTOM 25
#define WF_ICONIFY 26
#define WF_UNICONIFY 27

/* wind_update(). */
#define END_UPDATE 0
#define BEG_UPDATE 1
#define END_MCTRL 2
#define BEG_MCTRL 3

/* wind_calc(). */
#define WC_BORDER 0
#define WC_WORK 1

/* graf_mouse(). */
#define ARROW 0
#define TEXT_CRSR 1
#define BUSYBEE 2
#define BUSY_BEE BUSYBEE
#define HOURGLASS BUSYBEE
#define POINT_HAND 3
#define FLAT_HAND 4
#define THIN_CROSS 5
#define THICK_CROSS 6
#define OUTLN_CROSS 7
#define USER_DEF 255
#define M_OFF 256
#define M_ON 257
#define M_SAVE 258
#define M_LAST 259
#define M_RESTORE 260

/* form_dial(). */
#define FMD_START 0
#define FMD_GROW 1
#define FMD_SHRINK 2
#define FMD_FINISH 3

/* rsrc_gaddr() types. */
#define R_TREE 0
#define R_OBJECT 1
#define R_TEDINFO 2
#define R_ICONBLK 3
#define R_BITBLK 4
#define R_STRING 5
#define R_IMAGEDATA 6
#define R_OBSPEC 7
#define R_TEPTEXT 8
#define R_TEPTMPLT 9
#define R_TEPVALID 10
#define R_IBPMASK 11
#define R_IBPDATA 12
#define R_IBPTEXT 13
#define R_BIPDATA 14
#define R_FRSTR 15
#define R_FRIMG 16

/* Mouse rectangle events. */
#define MO_ENTER 0
#define MO_LEAVE 1

/* The keyboard state bits. */
#define K_RSHIFT 0x0001
#define K_LSHIFT 0x0002
#define K_CTRL 0x0004
#define K_ALT 0x0008

/* Application library */
short appl_init(void);
short appl_exit(void);
short appl_read(short ap_id, short length, void *buf);
short appl_write(short ap_id, short length, const void *buf);
short appl_find(const char *name);
short appl_tplay(void *mem, short num, short scale);
short appl_trecord(void *mem, short count);
short appl_getinfo(short type, short *out1, short *out2, short *out3, short *out4);

/* Event library */
short evnt_keybd(void);
short evnt_button(short clicks, short mask, short state, short *mx, short *my, short *button,
                  short *kstate);
short evnt_mouse(short flag, short x, short y, short w, short h, short *mx, short *my,
                 short *button, short *kstate);
short evnt_mesag(short msg[8]);
short evnt_timer(unsigned long ms);
short evnt_multi(short type, short clicks, short mask, short state, short m1flag, short m1x,
                 short m1y, short m1w, short m1h, short m2flag, short m2x, short m2y, short m2w,
                 short m2h, short msg[8], unsigned long ms, short *mx, short *my, short *button,
                 short *kstate, short *key, short *nclicks);
short evnt_dclick(short rate, short flag);

/* Menu library */
short menu_bar(OBJECT *tree, short show);
short menu_icheck(OBJECT *tree, short item, short check);
short menu_ienable(OBJECT *tree, short item, short enable);
short menu_tnormal(OBJECT *tree, short title, short normal);
short menu_text(OBJECT *tree, short item, const char *text);
short menu_register(short ap_id, const char *name);

/* Object library */
short objc_add(OBJECT *tree, short parent, short child);
short objc_delete(OBJECT *tree, short obj);
short objc_draw(OBJECT *tree, short start, short depth, short cx, short cy, short cw, short ch);
short objc_find(OBJECT *tree, short start, short depth, short mx, short my);
short objc_offset(OBJECT *tree, short obj, short *x, short *y);
short objc_order(OBJECT *tree, short obj, short newpos);
short objc_edit(OBJECT *tree, short obj, short ch, short *idx, short kind);
short objc_change(OBJECT *tree, short obj, short resvd, short cx, short cy, short cw, short ch,
                  short newstate, short redraw);

/* Form library */
short form_do(OBJECT *tree, short start);
short form_dial(short flag, short lx, short ly, short lw, short lh, short bx, short by, short bw,
                short bh);
short form_alert(short defbutton, const char *str);
short form_error(short code);
short form_center(OBJECT *tree, short *x, short *y, short *w, short *h);
short form_keybd(OBJECT *tree, short obj, short next, short ch, short *nextobj, short *nextch);
short form_button(OBJECT *tree, short obj, short clicks, short *nextobj);

/* Graphics library */
short graf_rubberbox(short x, short y, short minw, short minh, short *w, short *h);
short graf_dragbox(short w, short h, short x, short y, short bx, short by, short bw, short bh,
                   short *nx, short *ny);
short graf_movebox(short w, short h, short sx, short sy, short dx, short dy);
short graf_growbox(short sx, short sy, short sw, short sh, short fx, short fy, short fw, short fh);
short graf_shrinkbox(short fx, short fy, short fw, short fh, short sx, short sy, short sw,
                     short sh);
short graf_watchbox(OBJECT *tree, short obj, short instate, short outstate);
short graf_slidebox(OBJECT *tree, short parent, short obj, short vertical);
short graf_handle(short *wchar, short *hchar, short *wbox, short *hbox);
short graf_mouse(short form, const MFORM *mform);
short graf_mkstate(short *mx, short *my, short *button, short *kstate);

/* Scrap library */
short scrp_read(char *path);
short scrp_write(const char *path);

/* File selector library */
short fsel_input(char *path, char *file, short *button);
short fsel_exinput(char *path, char *file, short *button, const char *title);

/* Window library */
short wind_create(short kind, short x, short y, short w, short h);
short wind_open(short handle, short x, short y, short w, short h);
short wind_close(short handle);
short wind_delete(short handle);
short wind_get(short handle, short field, short *w1, short *w2, short *w3, short *w4);
short wind_set(short handle, short field, short w1, short w2, short w3, short w4);
short wind_set_str(short handle, short field, const char *str);
short wind_find(short x, short y);
short wind_update(short code);
short wind_calc(short type, short kind, short x, short y, short w, short h, short *ox, short *oy,
                short *ow, short *oh);
void  wind_new(void);
#define wind_get_grect(h, f, r) wind_get(h, f, &(r)->g_x, &(r)->g_y, &(r)->g_w, &(r)->g_h)

/* Resource library */
short rsrc_load(const char *name);
short rsrc_free(void);
short rsrc_gaddr(short type, short index, void *addr);
short rsrc_saddr(short type, short index, void *addr);
short rsrc_obfix(OBJECT *tree, short obj);

/* Shell library */
short shel_read(char *cmd, char *tail);
short shel_write(short doex, short isgr, short iscr, const char *cmd, const char *tail);
short shel_find(char *path);
short shel_envrn(char **value, const char *name);

/* The low level call: the AES parameter block and its arrays, for calls
 * not listed here. aes_control[0] is the opcode, then the number of
 * intin, intout, addrin and addrout entries. */
extern short aes_control[5];
extern short aes_intin[16];
extern short aes_intout[16];
extern void *aes_addrin[16];
extern void *aes_addrout[16];
short aes(short opcode, short nintin, short nintout, short naddrin, short naddrout);

#ifdef __cplusplus
}
#endif
#endif /* _AES_H */

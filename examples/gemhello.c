// A GEM program: opens a window, draws in it with the VDI, shows an alert
// box and ends when the window's close box is clicked. Under an emulator
// with NatFeats on, it tells the host when the alert is up.
#include <gem.h>
#include <stddef.h>
#include <natfeats.h>

static short vdi_handle;

static void draw(short handle, GRECT *area) {
  GRECT work, r;
  wind_get_grect(handle, WF_WORKXYWH, &work);
  graf_mouse(M_OFF, NULL);
  wind_update(BEG_UPDATE);
  wind_get_grect(handle, WF_FIRSTXYWH, &r);
  while (r.g_w && r.g_h) {
    // Clip to the part of the rectangle list that needs drawing.
    short x1 = r.g_x > area->g_x ? r.g_x : area->g_x;
    short y1 = r.g_y > area->g_y ? r.g_y : area->g_y;
    short x2 = r.g_x + r.g_w < area->g_x + area->g_w ? r.g_x + r.g_w
                                                     : area->g_x + area->g_w;
    short y2 = r.g_y + r.g_h < area->g_y + area->g_h ? r.g_y + r.g_h
                                                     : area->g_y + area->g_h;
    if (x2 > x1 && y2 > y1) {
      short clip[4] = {x1, y1, (short)(x2 - 1), (short)(y2 - 1)};
      vs_clip(vdi_handle, 1, clip);
      short box[4] = {work.g_x, work.g_y, (short)(work.g_x + work.g_w - 1),
                      (short)(work.g_y + work.g_h - 1)};
      vsf_interior(vdi_handle, FIS_SOLID);
      vsf_color(vdi_handle, G_WHITE);
      vr_recfl(vdi_handle, box);
      vsf_interior(vdi_handle, FIS_HATCH);
      vsf_style(vdi_handle, 3);
      vsf_color(vdi_handle, G_BLACK);
      v_circle(vdi_handle, work.g_x + work.g_w / 2, work.g_y + work.g_h / 2,
               work.g_h / 3);
      short line[4] = {work.g_x + 8, (short)(work.g_y + work.g_h - 8),
                       (short)(work.g_x + work.g_w - 8), work.g_y + 24};
      vsl_width(vdi_handle, 3);
      v_pline(vdi_handle, 2, line);
      vswr_mode(vdi_handle, MD_TRANS);
      v_gtext(vdi_handle, work.g_x + 8, work.g_y + 16,
              "Hello from clang on the Atari ST!");
      vswr_mode(vdi_handle, MD_REPLACE);
      vs_clip(vdi_handle, 0, clip);
    }
    wind_get_grect(handle, WF_NEXTXYWH, &r);
  }
  wind_update(END_UPDATE);
  graf_mouse(M_ON, NULL);
}

int main(void) {
  if (appl_init() < 0)
    return 1;
  short wchar, hchar, wbox, hbox;
  vdi_handle = graf_handle(&wchar, &hchar, &wbox, &hbox);
  short work_in[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2}, work_out[57];
  v_opnvwk(work_in, &vdi_handle, work_out);
  graf_mouse(ARROW, NULL);

  GRECT desk;
  wind_get_grect(0, WF_WORKXYWH, &desk);
  short kind = NAME | CLOSER | MOVER;
  short handle = wind_create(kind, desk.g_x, desk.g_y, desk.g_w, desk.g_h);
  wind_set_str(handle, WF_NAME, " clang-dos ");
  wind_open(handle, desk.g_x + desk.g_w / 8, desk.g_y + desk.g_h / 8,
            desk.g_w * 3 / 4, desk.g_h * 3 / 4);
  short msg[8], mx, my, mb, ks, key, nc;
  int alerted = 0;
  for (int done = 0; !done;) {
    short ev = evnt_multi(MU_MESAG, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, msg,
                          0, &mx, &my, &mb, &ks, &key, &nc);
    if (ev & MU_MESAG) {
      switch (msg[0]) {
      case WM_REDRAW:
        draw(msg[3], (GRECT *)&msg[4]);
        if (!alerted) {
          alerted = 1;
          nf_print("gemhello: alert\n"); // for the test script
          form_alert(1, "[1][Hello from clang!|This is a GEM alert box.][ OK ]");
        }
        break;
      case WM_TOPPED:
        wind_set(msg[3], WF_TOP, 0, 0, 0, 0);
        break;
      case WM_MOVED:
        wind_set(msg[3], WF_CURRXYWH, msg[4], msg[5], msg[6], msg[7]);
        break;
      case WM_CLOSED:
        done = 1;
        break;
      }
    }
  }
  wind_close(handle);
  wind_delete(handle);
  v_clsvwk(vdi_handle);
  appl_exit();
  return 0;
}

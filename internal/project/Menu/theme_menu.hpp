#pragma once

#include <windows.h>
#include <cstdio>
#include <cwchar>
#include <string>
#include <vector>

// Forward decls (defined in hooks.cpp)
void save_config();
void load_config();
void set_current_config(const std::string& name);
std::string get_current_config();
std::vector<std::string> list_configs();

namespace theme_menu
{
    // ------------------------------------------------------------------ types

    using color_t = flinearcolor;

    struct ctx
    {
        ucanvas*  canvas = nullptr;
        uobject*  font   = nullptr;
        fvector2d win;
        fvector2d mouse;
        bool      clicked = false;
        bool      held    = false;
        float     col_x   = 0.f;
        float     y       = 0.f;
        float     y0      = 0.f;
        float     col_w   = 0.f;
    };

    // ------------------------------------------------------------------ theme

    static color_t accent;
    static color_t accent_dim;
    static color_t accent_soft;
    static color_t accent_text;

    static color_t c_bg      = { 0.020f, 0.020f, 0.022f, 0.985f };
    static const color_t c_panel   = { 0.034f, 0.034f, 0.037f, 1.000f };
    static const color_t c_elem    = { 0.050f, 0.050f, 0.054f, 1.000f };
    static const color_t c_elem2   = { 0.075f, 0.075f, 0.080f, 1.000f };
    static const color_t c_border  = { 0.140f, 0.140f, 0.150f, 1.000f };
    static const color_t c_text    = { 0.902f, 0.918f, 0.957f, 1.000f };
    static const color_t c_textdim = { 0.549f, 0.580f, 0.643f, 1.000f };
    static const color_t c_hover   = { 1.000f, 1.000f, 1.000f, 0.060f };

    static color_t g_cursor    = { 1.000f, 1.000f, 1.000f, 1.000f };
    static color_t g_cursor_dim = { 0.780f, 0.780f, 0.780f, 1.000f };

    static color_t col(float r, float g, float b, float a = 1.f)
    {
        return { r / 255.f, g / 255.f, b / 255.f, a };
    }

    static void apply_theme(int idx)
    {
        switch (idx % 5)
        {
        case 0: accent = col(0, 205, 255); break;       // Cyan
        case 1: accent = col(130, 96, 255); break;      // Violet
        case 2: accent = col(0, 226, 145); break;       // Green
        case 3: accent = col(255, 74, 122); break;      // Rose
        default: accent = col(255, 196, 66); break;     // Gold
        }
        accent_dim   = { accent.r * 0.45f, accent.g * 0.45f, accent.b * 0.45f, 1.0f };
        accent_soft  = { accent.r, accent.g, accent.b, 0.35f };
        accent_text  = { accent.r, accent.g, accent.b, 1.0f };
    }

    // ------------------------------------------------------------------ prims

    static void rect(ctx& c, float x, float y, float w, float h, color_t cc)
    {
        for (float i = 0.f; i < h; i += 1.f)
            c.canvas->k2_drawline({ x, y + i }, { x + w, y + i }, 1.f, cc);
    }

    static void line(ctx& c, float x1, float y1, float x2, float y2, float t, color_t cc)
    {
        c.canvas->k2_drawline({ x1, y1 }, { x2, y2 }, t, cc);
    }

    static void outline(ctx& c, float x, float y, float w, float h, float t, color_t cc)
    {
        c.canvas->k2_drawline({ x, y }, { x + w, y }, t, cc);
        c.canvas->k2_drawline({ x + w, y }, { x + w, y + h }, t, cc);
        c.canvas->k2_drawline({ x + w, y + h }, { x, y + h }, t, cc);
        c.canvas->k2_drawline({ x, y + h }, { x, y }, t, cc);
    }

    static void grad_v(ctx& c, float x, float y, float w, float h, color_t top, color_t bot)
    {
        for (float i = 0.f; i < h; i += 1.f)
        {
            float t = h > 0.f ? i / h : 0.f;
            color_t cc;
            cc.r = top.r + (bot.r - top.r) * t;
            cc.g = top.g + (bot.g - top.g) * t;
            cc.b = top.b + (bot.b - top.b) * t;
            cc.a = top.a + (bot.a - top.a) * t;
            c.canvas->k2_drawline({ x, y + i }, { x + w, y + i }, 1.f, cc);
        }
    }

    static void text(ctx& c, const wchar_t* s, float x, float y, float sc, color_t cc, bool center = false)
    {
        c.canvas->k2_drawtext(c.font, s, { x, y }, { sc, sc }, cc, 0.f,
            { 0.f, 0.f, 0.f, 0.65f }, { 1.f, 1.f }, center, false, true, { 0.f, 0.f, 0.f, 0.85f });
    }

    static float text_w(ctx& c, const wchar_t* s, float sc)
    {
        return c.canvas->get_string_size(c.font, s, { sc, sc }).x;
    }

    static bool in_zone(const ctx& c, float x, float y, float w, float h)
    {
        return c.mouse.x > x && c.mouse.x < x + w && c.mouse.y > y && c.mouse.y < y + h;
    }

    static bool click_zone(const ctx& c, float x, float y, float w, float h)
    {
        return in_zone(c, x, y, w, h) && c.clicked;
    }

    // ------------------------------------------------------------------ state

    static int  g_tab = 0;
    static int  g_combo_open = -1;
    static int  g_combo_id   = 0;
    static int  g_bind_key   = -1;
    static int  g_hotkey_id  = 0;
    static int  g_theme      = 0;
    static int  g_color_open = -1;

    static fvector2d g_win_pos = { 0, 0 };
    static bool      g_win_inited = false;
    static bool      g_dragging   = false;
    static fvector2d g_drag_off  = { 0, 0 };

    // ------------------------------------------------------------------ keys

    static const wchar_t* key_name(int vk)
    {
        switch (vk)
        {
        case 0x01: return L"LMB";      case 0x02: return L"RMB";      case 0x04: return L"MMB";
        case 0x08: return L"Backspace";case 0x09: return L"Tab";      case 0x0D: return L"Enter";
        case 0x10: return L"Shift";    case 0x11: return L"Ctrl";     case 0x12: return L"Alt";
        case 0x14: return L"CapsLock"; case 0x1B: return L"Esc";      case 0x20: return L"Space";
        case 0x21: return L"PageUp";   case 0x22: return L"PageDown"; case 0x23: return L"End";
        case 0x24: return L"Home";     case 0x25: return L"Left";     case 0x26: return L"Up";
        case 0x27: return L"Right";    case 0x28: return L"Down";     case 0x2D: return L"Insert";
        case 0x2E: return L"Delete";   case 0x2F: return L"Help";
        default:
            if (vk >= 'A' && vk <= 'Z')
            {
                static wchar_t s[2] = { 0, 0 };
                s[0] = (wchar_t)vk;
                return s;
            }
            if (vk >= '0' && vk <= '9')
            {
                static wchar_t d[2] = { 0, 0 };
                d[0] = (wchar_t)vk;
                return d;
            }
            if (vk >= 0x60 && vk <= 0x69)
            {
                static wchar_t n[2] = { 0, 0 };
                n[0] = (wchar_t)(L'0' + (vk - 0x60));
                return n;
            }
            if (vk >= 0x70 && vk <= 0x87)
            {
                static wchar_t f[8] = { 0 };
                swprintf(f, 8, L"F%d", vk - 0x70 + 1);
                return f;
            }
            return L"Key";
        }
    }

    // ------------------------------------------------------------------ widgets

    static void section(ctx& c, const wchar_t* title)
    {
        float x = c.col_x + 2.f, y = c.y;
        rect(c, x, y + 2.f, 3.f, 13.f, accent);
        text(c, title, x + 9.f, y, 0.66f, c_textdim);
        line(c, x + 9.f, y + 16.f, x + c.col_w - 2.f, y + 16.f, 1.f, c_border);
        c.y += 24.f;
    }

    static void checkbox(ctx& c, const wchar_t* label, bool* val)
    {
        float x = c.col_x, y = c.y;
        float bh = 15.f;
        bool hover = in_zone(c, x + 2.f, y, c.col_w - 4.f, 22.f);

        if (hover) rect(c, x + 2.f, y + 1.f, c.col_w - 4.f, 22.f, c_hover);

        color_t box = *val ? accent : c_elem2;
        rect(c, x + 6.f, y + 3.f, bh, bh, box);
        if (hover && !*val) rect(c, x + 6.f, y + 3.f, bh, bh, c_elem2);
        outline(c, x + 6.f, y + 3.f, bh, bh, 1.f, *val ? accent : c_border);

        if (*val)
        {
            line(c, x + 9.f, y + 10.f, x + 13.f, y + 14.f, 2.f, c_text);
            line(c, x + 13.f, y + 14.f, x + 19.f, y + 6.f, 2.f, c_text);
        }

        text(c, label, x + 30.f, y + 2.f, 0.72f, *val ? c_text : c_textdim);

        if (click_zone(c, x + 2.f, y, c.col_w - 4.f, 22.f))
            *val = !*val;

        c.y += 24.f;
    }

    static void slider(ctx& c, const wchar_t* label, float* val, float min, float max, const wchar_t* fmt)
    {
        float x = c.col_x, y = c.y;

        wchar_t buf[64];
        swprintf(buf, 64, fmt, *val);
        float tw = text_w(c, buf, 0.66f);
        text(c, buf, x + c.col_w - 6.f - tw, y + 1.f, 0.66f, accent_text);
        text(c, label, x + 6.f, y + 1.f, 0.70f, c_textdim);

        float sx = x + 6.f, sy = y + 21.f, sw = c.col_w - 12.f, sh = 4.f;
        rect(c, sx, sy, sw, sh, c_elem);

        float t = (max > min) ? (*val - min) / (max - min) : 0.f;
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;

        rect(c, sx, sy, sw * t, sh, accent);
        float kx = sx + sw * t;
        rect(c, kx - 3.f, sy - 3.f, 6.f, 10.f, c_text);
        outline(c, kx - 3.f, sy - 3.f, 6.f, 10.f, 1.f, accent);

        if (in_zone(c, sx, sy - 6.f, sw, 16.f) && c.held)
        {
            float nt = (c.mouse.x - sx) / sw;
            if (nt < 0.f) nt = 0.f;
            if (nt > 1.f) nt = 1.f;
            *val = min + nt * (max - min);
        }

        c.y += 38.f;
    }

    static void hotkey(ctx& c, const wchar_t* label, int* key)
    {
        float x = c.col_x, y = c.y;
        int my_id = g_hotkey_id++;

        text(c, label, x + 6.f, y + 3.f, 0.70f, c_textdim);

        float bx = x + c.col_w - 90.f, by = y + 1.f, bw = 84.f, bh = 22.f;
        bool box_hover = in_zone(c, bx, by, bw, bh);
        bool binding = (g_bind_key == my_id);

        rect(c, bx, by, bw, bh, binding ? accent_dim : c_elem2);
        if (box_hover && !binding) rect(c, bx, by, bw, bh, c_hover);
        outline(c, bx, by, bw, bh, 1.f, binding ? accent : c_border);

        const wchar_t* shown = binding ? L"..." : key_name(*key);
        text(c, shown, bx + 8.f, by + 3.f, 0.68f, binding ? c_text : accent_text);

        if (click_zone(c, bx, by, bw, bh))
            g_bind_key = binding ? -1 : my_id;

        if (binding)
        {
            for (int vk = 0x08; vk <= 0xFE; vk++)
            {
                if (vk == VK_LBUTTON || vk == VK_RBUTTON || vk == VK_MBUTTON ||
                    vk == VK_XBUTTON1 || vk == VK_XBUTTON2 || vk == VK_LSHIFT ||
                    vk == VK_RSHIFT || vk == VK_LCONTROL || vk == VK_RCONTROL ||
                    vk == VK_LMENU || vk == VK_RMENU)
                    continue;
                if ((GetAsyncKeyState(vk) & 1) != 0)
                {
                    *key = vk;
                    g_bind_key = -1;
                    break;
                }
            }
        }

        if (binding && c.clicked && !box_hover)
            g_bind_key = -1;

        c.y += 24.f;
    }

    static void combo(ctx& c, const wchar_t* label, int* val, const wchar_t** items, int count)
    {
        float x = c.col_x, y = c.y;
        int my_id = g_combo_id++;

        text(c, label, x + 6.f, y + 3.f, 0.68f, c_textdim);

        float bx = x + c.col_w - 128.f, by = y + 1.f, bw = 122.f, bh = 22.f;
        bool box_hover = in_zone(c, bx, by, bw, bh);
        bool open = (g_combo_open == my_id);

        rect(c, bx, by, bw, bh, c_elem2);
        if (box_hover && !open) rect(c, bx, by, bw, bh, c_hover);
        outline(c, bx, by, bw, bh, 1.f, open ? accent : c_border);

        if (*val < 0) *val = 0;
        if (*val >= count) *val = count - 1;

        text(c, items[*val], bx + 8.f, by + 3.f, 0.66f, c_text);
        line(c, bx + bw - 14.f, by + 8.f, bx + bw - 14.f, by + 14.f, 1.f, accent_text);
        line(c, bx + bw - 10.f, by + 14.f, bx + bw - 14.f, by + 8.f, 1.f, accent_text);

        if (click_zone(c, bx, by, bw, bh))
            g_combo_open = open ? -1 : my_id;

        if (open)
        {
            float dy = by + bh + 2.f;
            float dh = bh * (float)count;

            rect(c, bx, dy, bw, dh, c_panel);
            outline(c, bx, dy, bw, dh, 1.f, c_border);

            if (c.clicked && !box_hover && !in_zone(c, bx, dy, bw, dh))
                g_combo_open = -1;

            for (int i = 0; i < count; i++)
            {
                float iy = dy + (float)i * bh;
                bool ihover = in_zone(c, bx, iy, bw, bh);
                if (i == *val) rect(c, bx, iy, bw, bh, accent_dim);
                else if (ihover) rect(c, bx, iy, bw, bh, c_hover);
                text(c, items[i], bx + 8.f, iy + 3.f, 0.66f, (i == *val) ? c_text : c_textdim);
                if (click_zone(c, bx, iy, bw, bh))
                {
                    *val = i;
                    g_combo_open = -1;
                }
            }
        }

        c.y += 26.f;
    }

    static void config_selector(ctx& c, const wchar_t* label)
    {
        float x = c.col_x, y = c.y;
        static std::vector<std::string> cfg_list;
        static std::vector<std::wstring> cfg_names;
        static DWORD last_refresh = 0;
        static bool initialized = false;
        static int cfg_open = 0;

        DWORD now = GetTickCount();
        if (!initialized || now - last_refresh > 1000)
        {
            cfg_list = list_configs();
            cfg_names.clear();
            for (auto& s : cfg_list)
                cfg_names.push_back(std::wstring(s.begin(), s.end()));
            last_refresh = now;
            initialized = true;
        }

        text(c, label, x + 6.f, y + 2.f, 0.68f, c_textdim);

        float bx = x + 6.f, by = y + 22.f, bw = c.col_w - 12.f, bh = 22.f;
        bool box_hover = in_zone(c, bx, by, bw, bh);
        bool open = (cfg_open != 0);

        rect(c, bx, by, bw, bh, c_elem2);
        if (box_hover && !open) rect(c, bx, by, bw, bh, c_hover);
        outline(c, bx, by, bw, bh, 1.f, open ? accent : c_border);

        std::string cur = get_current_config();
        std::wstring curw(cur.begin(), cur.end());
        text(c, curw.c_str(), bx + 8.f, by + 3.f, 0.66f, c_text);
        line(c, bx + bw - 14.f, by + 8.f, bx + bw - 14.f, by + 14.f, 1.f, accent_text);
        line(c, bx + bw - 10.f, by + 14.f, bx + bw - 14.f, by + 8.f, 1.f, accent_text);

        if (click_zone(c, bx, by, bw, bh))
            cfg_open = open ? 0 : 1;

        if (open)
        {
            float dy = by + bh + 2.f;
            float dh = bh * (float)cfg_names.size();
            float maxh = c.win.y + 500.f - dy;
            if (dh > maxh) dh = maxh;

            rect(c, bx, dy, bw, dh, c_panel);
            outline(c, bx, dy, bw, dh, 1.f, c_border);

            if (c.clicked && !box_hover && !in_zone(c, bx, dy, bw, dh))
                cfg_open = 0;

            for (int i = 0; i < (int)cfg_names.size(); i++)
            {
                float iy = dy + (float)i * bh;
                if (iy >= dy + dh) break;
                bool ihover = in_zone(c, bx, iy, bw, bh);
                bool selected = (curw == cfg_names[i]);
                if (selected) rect(c, bx, iy, bw, bh, accent_dim);
                else if (ihover) rect(c, bx, iy, bw, bh, c_hover);
                text(c, cfg_names[i].c_str(), bx + 8.f, iy + 3.f, 0.66f, selected ? c_text : c_textdim);
                if (click_zone(c, bx, iy, bw, bh))
                {
                    set_current_config(cfg_list[i]);
                    cfg_open = 0;
                }
            }
        }

        c.y += 34.f;
    }

    static void color_picker(ctx& c, const wchar_t* label, color_t* col, int id)
    {
        float x = c.col_x, y = c.y;
        bool open = (g_color_open == id);

        text(c, label, x + 6.f, y + 3.f, 0.68f, c_textdim);
        float bxx = x + c.col_w - 46.f;
        rect(c, bxx, y + 2.f, 22.f, 15.f, *col);
        outline(c, bxx, y + 2.f, 22.f, 15.f, 1.f, c_border);

        if (click_zone(c, bxx, y + 2.f, 22.f, 15.f))
            g_color_open = open ? -1 : id;

        c.y += 22.f;

        if (open)
        {
            slider(c, L"Red", &col->r, 0.f, 1.f, L"%.2f");
            slider(c, L"Green", &col->g, 0.f, 1.f, L"%.2f");
            slider(c, L"Blue", &col->b, 0.f, 1.f, L"%.2f");
            c.y -= 6.f;
        }
    }

    static void button(ctx& c, const wchar_t* label, void (*fn)())
    {
        float x = c.col_x + 6.f, y = c.y, w = c.col_w - 12.f, h = 26.f;
        bool hover = in_zone(c, x, y, w, h);

        grad_v(c, x, y, w, h, accent_dim, accent_dim);
        if (hover) rect(c, x, y, w, h, c_hover);
        outline(c, x, y, w, h, 1.f, accent);
        text(c, label, x + w / 2.f, y + 5.f, 0.70f, c_text, true);

        if (click_zone(c, x, y, w, h))
            fn();

        c.y += 32.f;
    }

    static void begin_col(ctx& c, float x)
    {
        c.col_x = x;
        c.y = c.y0;
    }

    // ------------------------------------------------------------------ tabs

    static const int TAB_COUNT = 8;
    static const wchar_t* TAB_NAMES[TAB_COUNT] =
    {
        L"RAGE", L"ANTI-AIM", L"VISUALS", L"WORLD", L"MISC", L"SKINS", L"SELF", L"CONFIG"
    };

    static void tab_rage(ctx& c)
    {
        begin_col(c, c.col_x);
        section(c, L"AIMBOT");
        checkbox(c, L"Aimbot", &globals::aimbot::a1mbot);
        checkbox(c, L"Visibility Check", &globals::aimbot::v1sh_ch3ck);
        checkbox(c, L"Recoil Control", &globals::aimbot::reco1l_contr0l);
        checkbox(c, L"No Spread", &globals::aimbot::spread_comp);
        checkbox(c, L"Auto Shoot", &globals::aimbot::autoshot);
        checkbox(c, L"Auto Scope", &globals::aimbot::auto_scope);
        checkbox(c, L"Prediction", &globals::aimbot::pvc);
        checkbox(c, L"Wall Penetrate", &globals::aimbot::wall_penetrate);
        checkbox(c, L"Smoke Penetrate", &globals::aimbot::smoke_pen);
        checkbox(c, L"Rage Bot", &globals::aimbot::rage);
        if (globals::aimbot::rage)
        {
            hotkey(c, L"Rage Key", &globals::aimbot::rage_key);
        }
        section(c, L"TARGETING");
        checkbox(c, L"Draw FOV", &globals::aimbot::draw_f0v);
        checkbox(c, L"360 Degree FOV", &globals::aimbot::enable_360_fov);
        hotkey(c, L"Aim Key", &globals::aimbot::a1m_k3y);
        slider(c, L"Smooth", &globals::aimbot::a1m_sm00th, 1.f, 100.f, L"%.0f");
        slider(c, L"FOV", &globals::aimbot::a1m_f0v, 1.f, 360.f, L"%.0f");
        slider(c, L"Max Aim Distance", &globals::aimbot::max_aim_distance, 100.f, 100000.f, L"%.0f");

        begin_col(c, c.col_x + c.col_w + 12.f);
        section(c, L"SELECTION");
        static const wchar_t* hitboxes[] = { L"Head", L"Chest", L"Stomach", L"Legs" };
        combo(c, L"Hitbox", &globals::aimbot::a1m_b0ne, hitboxes, 4);
        static const wchar_t* target_sel[] = { L"Distance", L"Crosshair", L"Combined" };
        combo(c, L"Target Selection", &globals::aimbot::target_selection, target_sel, 3);
        section(c, L"ADVANCED");
        checkbox(c, L"Aim Assist", &globals::aimbot::aim_assist);
        slider(c, L"Min Damage", &globals::aimbot::min_damage, 0.f, 100.f, L"%.0f");
        slider(c, L"Spread Delay", &globals::aimbot::spread_comp_delay, 0.f, 500.f, L"%.0f");
        slider(c, L"Auto Shoot Delay", &globals::aimbot::autoshoot_delay, 0.f, 100.f, L"%.0f");
    }

    static void tab_antiaim(ctx& c)
    {
        begin_col(c, c.col_x);
        section(c, L"MAIN");
        checkbox(c, L"Anti-Aim", &globals::misc::aa);
        hotkey(c, L"AA Toggle Key", &globals::misc::aa_toggle_key);
        static const wchar_t* aa_modes[] = { L"Normal", L"Spin", L"Backwards", L"3-Way", L"Jitter", L"Desync" };
        combo(c, L"Mode", &globals::misc::aa_mode, aa_modes, 6);

        int mode = globals::misc::aa_mode;
        globals::misc::aa_spin     = (mode == 1);
        globals::misc::aa_backwards = (mode == 2);
        globals::misc::aa_threeway = (mode == 3);
        globals::misc::aa_jitter   = (mode == 4);
        globals::misc::aa_desync   = (mode == 5);

        checkbox(c, L"Fast Duck", &globals::misc::fakeduck);
        checkbox(c, L"Fast Crouch", &globals::misc::fastcrouch);
        checkbox(c, L"Freestanding", &globals::misc::freestanding);
        checkbox(c, L"Desync Move", &globals::misc::jitter_move);
        section(c, L"ANGLES");
        checkbox(c, L"Pitch Override", &globals::misc::pitch_enabled);
        if (globals::misc::pitch_enabled)
            slider(c, L"Pitch Value", &globals::misc::pitch_value, -90.f, 90.f, L"%.0f");
        slider(c, L"Yaw Offset", &globals::misc::yaw_add, -180.f, 180.f, L"%.0f");
        checkbox(c, L"Jitter", &globals::misc::jitter_enabled);
        if (globals::misc::jitter_enabled)
            slider(c, L"Jitter Range", &globals::misc::jitter_range, 0.f, 360.f, L"%.0f");

        begin_col(c, c.col_x + c.col_w + 12.f);
        section(c, L"EXTENDED");
        checkbox(c, L"Manual Anti-Aim", &globals::misc::manual_aa);
        if (globals::misc::manual_aa)
        {
            hotkey(c, L"AA Left Key", &globals::misc::aa_left_key);
            hotkey(c, L"AA Back Key", &globals::misc::aa_back_key);
            hotkey(c, L"AA Right Key", &globals::misc::aa_right_key);
        }
        checkbox(c, L"Center Jitter", &globals::misc::atomic_aa);
        if (globals::misc::atomic_aa)
        {
            slider(c, L"Center Speed", &globals::misc::atomic_speed, 0.1f, 3.f, L"%.1f");
            static const wchar_t* at_modes[] = { L"Normal", L"Inverter", L"Flicker" };
            combo(c, L"Center Mode", &globals::misc::atomic_mode, at_modes, 3);
        }
        checkbox(c, L"Jitter On Back", &globals::misc::jitter_on_back);
        slider(c, L"Desync Range", &globals::misc::desync_range, 0.f, 360.f, L"%.0f");
        slider(c, L"Fake Lag", &globals::misc::fake_lag_ticks, 0.f, 64.f, L"%.0f");
        section(c, L"RESOLVER");
        checkbox(c, L"Prediction Resolver", &globals::misc::prediction_breaker);
        if (globals::misc::prediction_breaker)
            slider(c, L"Resolver Intensity", &globals::misc::breaker_intensity, 0.5f, 5.f, L"%.1f");
    }

    static void tab_visuals(ctx& c)
    {
        begin_col(c, c.col_x);
        section(c, L"PLAYERS");
        checkbox(c, L"2D Box", &globals::visuals::box2d);
        checkbox(c, L"3D Box", &globals::visuals::box3d);
        checkbox(c, L"Corner Box", &globals::visuals::cornerbox);
        checkbox(c, L"Skeleton", &globals::visuals::sk3let0n);
        checkbox(c, L"Snaplines", &globals::visuals::snapl1ne);
        checkbox(c, L"Head Box", &globals::visuals::headb0x);
        checkbox(c, L"Health Bar", &globals::visuals::h3althbar);
        checkbox(c, L"Health Text", &globals::visuals::HealthText);
        checkbox(c, L"Distance", &globals::visuals::dstc);

        begin_col(c, c.col_x + c.col_w + 12.f);
        section(c, L"INFO");
        checkbox(c, L"Agent Name", &globals::visuals::agent_name);
        checkbox(c, L"Agent Icon", &globals::visuals::agenticon);
        checkbox(c, L"Weapon ESP", &globals::visuals::weaponesp);
        checkbox(c, L"Chinese Hat", &globals::visuals::chinese_hat);
        checkbox(c, L"Spectators List", &globals::visuals::spectators_list);
        checkbox(c, L"Offscreen ESP", &globals::visuals::offscreen_esp);
        if (globals::visuals::offscreen_esp)
        {
            slider(c, L"Offscreen Radius", &globals::visuals::offscreen_radius, 50.f, 500.f, L"%.0f");
            slider(c, L"Arrow Size", &globals::visuals::offscreen_size, 10.f, 50.f, L"%.0f");
        }
        section(c, L"CHAMS");
        checkbox(c, L"Normal Chams", &globals::visuals::chamsvsbd);
        if (globals::visuals::chamsvsbd)
            globals::visuals::visible_check_ch = true;
        else
            globals::visuals::visible_check_ch = false;
        checkbox(c, L"Hand Chams", &globals::misc::handchams);
        checkbox(c, L"Rainbow Chams", &globals::visuals::rainbow_chams);
    }

    static void tab_world(ctx& c)
    {
        begin_col(c, c.col_x);
        section(c, L"SKYBOX");
        checkbox(c, L"Enable Skybox", &globals::misc::skybox);
        checkbox(c, L"Rainbow Skybox", &globals::misc::skyboxrgb);
        static const wchar_t* sky_presets[] = { L"Default", L"Night", L"Galaxy", L"Neon" };
        combo(c, L"Preset", &globals::misc::skybox_preset_index, sky_presets, 4);

        static int last_sky = -1;
        if (last_sky != globals::misc::skybox_preset_index)
        {
            switch (globals::misc::skybox_preset_index)
            {
            case 1:
                globals::misc::Overall = { 0.1f, 0.1f, 0.2f };
                globals::misc::Zenith = { 0.0f, 0.0f, 0.1f };
                globals::misc::Horizon = { 0.1f, 0.1f, 0.2f };
                globals::misc::Cloud = { 0.2f, 0.2f, 0.3f };
                globals::misc::SkySunColor = { 0.5f, 0.5f, 0.6f };
                globals::misc::CloudSpeed = 1.0f;
                globals::misc::CloudOpacity = 1.0f;
                globals::misc::StarsBrightness = 2.0f;
                globals::misc::SkyNoisePower1 = 1.0f;
                globals::misc::SkyNoisePower2 = 1.0f;
                globals::misc::SkySunRadius = 0.5f;
                globals::misc::SkySunBrightness = 0.5f;
                globals::misc::SkySunHeight = -1.0f;
                break;
            case 2:
                globals::misc::Overall = { 0.00f, 0.00f, 0.00f };
                globals::misc::Zenith = { 0.00f, 0.00f, 0.00f };
                globals::misc::Horizon = { 0.00f, 3.55f, 5.00f };
                globals::misc::Cloud = { 5.00f, 5.00f, 5.00f };
                globals::misc::SkySunColor = { 5.00f, 5.00f, 5.00f };
                globals::misc::CloudSpeed = 10.00f;
                globals::misc::CloudOpacity = 2.00f;
                globals::misc::StarsBrightness = 5.00f;
                globals::misc::SkyNoisePower1 = 10.00f;
                globals::misc::SkyNoisePower2 = 10.00f;
                globals::misc::SkySunRadius = 5.00f;
                globals::misc::SkySunBrightness = 0.00f;
                globals::misc::SkySunHeight = -5.00f;
                break;
            case 3:
                globals::misc::Overall = { 5.00f, 5.00f, 4.28f };
                globals::misc::Zenith = { 5.00f, 3.58f, 5.00f };
                globals::misc::Horizon = { 5.00f, 0.10f, 5.00f };
                globals::misc::Cloud = { 5.00f, 5.00f, 5.00f };
                globals::misc::SkySunColor = { 5.00f, 5.00f, 5.00f };
                globals::misc::CloudSpeed = 10.00f;
                globals::misc::CloudOpacity = 2.00f;
                globals::misc::StarsBrightness = 3.65f;
                globals::misc::SkyNoisePower1 = 10.00f;
                globals::misc::SkyNoisePower2 = 0.00f;
                globals::misc::SkySunRadius = 4.51f;
                globals::misc::SkySunBrightness = 0.00f;
                globals::misc::SkySunHeight = 5.00f;
                break;
            default:
                globals::misc::Overall = { 1.0f, 1.0f, 1.0f };
                globals::misc::Zenith = { 0.7f, 0.8f, 1.0f };
                globals::misc::Horizon = { 0.9f, 0.9f, 0.9f };
                globals::misc::Cloud = { 1.0f, 1.0f, 1.0f };
                globals::misc::SkySunColor = { 1.0f, 1.0f, 0.9f };
                globals::misc::CloudSpeed = 1.0f;
                globals::misc::CloudOpacity = 1.0f;
                globals::misc::StarsBrightness = 1.0f;
                globals::misc::SkyNoisePower1 = 1.0f;
                globals::misc::SkyNoisePower2 = 1.0f;
                globals::misc::SkySunRadius = 1.0f;
                globals::misc::SkySunBrightness = 1.0f;
                globals::misc::SkySunHeight = 0.0f;
                break;
            }
            last_sky = globals::misc::skybox_preset_index;
        }

        begin_col(c, c.col_x + c.col_w + 12.f);
        section(c, L"TRAPS");
        checkbox(c, L"Spike ESP", &globals::misc::gdg);
        checkbox(c, L"Spike Timer", &globals::misc::spktimer);
        checkbox(c, L"World ESP", &globals::misc::cpp);
        checkbox(c, L"Cypher Traps", &globals::misc::world_esp);
        checkbox(c, L"Gadgets", &globals::misc::gadgets);
        section(c, L"PLAYER ESP");
        checkbox(c, L"Ignore Dormants", &globals::visuals::dormant);
        checkbox(c, L"Visible Check", &globals::visuals::visiblecheck);
    }

    static void tab_misc(ctx& c)
    {
        begin_col(c, c.col_x);
        section(c, L"MOVEMENT");
        checkbox(c, L"Bunny Hop", &globals::misc::bunnyhop);
        checkbox(c, L"Fast Crouch", &globals::misc::fastcrouch);
        checkbox(c, L"Anti Flash", &globals::misc::antiflash);
        checkbox(c, L"Skip Tutorial", &globals::misc::disconnect_server);
        section(c, L"MISC");
        checkbox(c, L"FPS Counter", &globals::misc::fps_counter);
        checkbox(c, L"Hotkeys HUD", &globals::misc::hotkeys_hud);
        checkbox(c, L"Free Cam", &globals::misc::freecam_enabled);
        if (globals::misc::freecam_enabled)
        {
            hotkey(c, L"Freecam Key", &globals::misc::freecam_key);
            slider(c, L"Freecam Speed", &globals::misc::freecam_speed, 100.f, 5000.f, L"%.0f");
        }
        checkbox(c, L"Aspect Ratio", &globals::misc::aspect_ratio_enabled);
        slider(c, L"Aspect Ratio Value", &globals::misc::aspect_ratio_value, 0.5f, 3.f, L"%.2f");

        begin_col(c, c.col_x + c.col_w + 12.f);
        section(c, L"CAMERA");
        checkbox(c, L"Third Person", &globals::misc::tperson);
        hotkey(c, L"Third Person Key", &globals::misc::tperson_key);
        slider(c, L"Thirdperson Distance", &globals::misc::PlayerDistance, 10.f, 1000.f, L"%.0f");
        checkbox(c, L"FOV Changer", &globals::misc::FovChangor);
        if (globals::misc::FovChangor)
            slider(c, L"FOV", &globals::misc::Fovchangerfloat, 70.f, 120.f, L"%.0f");
        slider(c, L"Arm Width", &globals::misc::arm_width, 0.1f, 15.f, L"%.0f");
        slider(c, L"Arm Height", &globals::misc::arm_height, 0.1f, 15.f, L"%.0f");
        slider(c, L"Arm Length", &globals::misc::arm_length, 0.1f, 15.f, L"%.0f");
    }

    static void tab_skins(ctx& c)
    {
        begin_col(c, c.col_x);
        section(c, L"SKINS");
        checkbox(c, L"Unlock All Skins", &globals::misc::sk1n_chang3r);
        checkbox(c, L"Apply Custom Skins", &globals::misc::playerchamsself);
        checkbox(c, L"Gun Chams", &globals::misc::customgun);
        checkbox(c, L"Gun Materials", &globals::misc::rdy);
        checkbox(c, L"Viewmodel Changer", &globals::misc::ViewModelChanger);
        checkbox(c, L"Custom CS2 Skins", &globals::misc::custom_obj_enabled);
        checkbox(c, L"Advanced Resolver", &globals::aimbot::vsr);
        section(c, L"FINISHER");
        checkbox(c, L"Finisher", &globals::misc::finisher);
        if (globals::misc::finisher)
            checkbox(c, L"Only Last Kill", &globals::misc::onlylastkill);

        begin_col(c, c.col_x + c.col_w + 12.f);
        section(c, L"SOUNDS");
        checkbox(c, L"Kill Sounds", &globals::misc::killsound);
        if (globals::misc::killsound)
        {
            static const wchar_t* sounds[] = { L"Neverlose Hit", L"Unstoppable Hit", L"Success Hit" };
            combo(c, L"Sound", &globals::misc::killsound_selection, sounds, 3);
        }
        checkbox(c, L"Killsay", &globals::misc::killsays);
        checkbox(c, L"Dynamic Hitsound", &globals::misc::hitsound);
        section(c, L"OTHER");
        checkbox(c, L"Chat Spammer (F2)", &globals::misc::chat_spammer);
        checkbox(c, L"Kill Counter", &globals::misc::show_kill_counter);
        checkbox(c, L"Big Gun", &globals::misc::BigGun3p);
        checkbox(c, L"Self Resizer", &globals::misc::BigSelf);
    }

    static void tab_self(ctx& c)
    {
        begin_col(c, c.col_x);
        section(c, L"SELF");
        checkbox(c, L"Self Chams", &globals::visuals::self_chams);
        checkbox(c, L"Galaxy Self", &globals::visuals::self_galaxy_enabled);
        checkbox(c, L"Self Wireframe", &globals::visuals::self_wireframe);
        checkbox(c, L"Hand Wireframe", &globals::visuals::wireframe_hands);
        checkbox(c, L"Weapon Wireframe", &globals::visuals::wireframe_weapon);
        checkbox(c, L"Mosca Wireframe", &globals::misc::mosca_wireframe);
        checkbox(c, L"Self Chinese Hat", &globals::visuals::chinese_hat_self);

        begin_col(c, c.col_x + c.col_w + 12.f);
        section(c, L"OUTLINE");
        checkbox(c, L"Outline Enemy", &globals::visuals::outline_enabled);
        checkbox(c, L"Dynamic Fresnel", &globals::visuals::dynamic_fresnel_outline);
        checkbox(c, L"Crystal Chams", &globals::visuals::crystal_chams_enabled);
        checkbox(c, L"Galaxy Gun", &globals::visuals::galaxy_gun);
        checkbox(c, L"Rainbow Fresnel", &globals::visuals::rainbow_chams);
        section(c, L"STATE");
        if (globals::visuals::outline_enabled)
            text(c, L"OUTLINE ACTIVE", c.col_x + 6.f, c.y + 2.f, 0.72f, col(0, 226, 145));
        else
            text(c, L"OUTLINE DISABLED", c.col_x + 6.f, c.y + 2.f, 0.72f, col(255, 74, 94));
        c.y += 24.f;
    }

    static void tab_config(ctx& c)
    {
        begin_col(c, c.col_x);
        section(c, L"CONFIG");
        config_selector(c, L"Config File");
        button(c, L"Save Config", &save_config);
        button(c, L"Load Config", &load_config);
        hotkey(c, L"Menu Key", &globals::misc::kky);
        checkbox(c, L"Unload", &globals::misc::nld);

        begin_col(c, c.col_x + c.col_w + 12.f);
        section(c, L"APPEARANCE");
        static const wchar_t* themes[] = { L"Cyan", L"Violet", L"Green", L"Rose", L"Gold" };
        combo(c, L"Theme", &g_theme, themes, 5);
        color_picker(c, L"Background", &c_bg, 1);
        color_picker(c, L"Cursor Color", &g_cursor, 2);
        section(c, L"ABOUT");
        text(c, L"vcc-library.uk", c.col_x + 6.f, c.y + 2.f, 0.80f, accent_text);
        c.y += 22.f;
        text(c, L"Version 1.0.0", c.col_x + 6.f, c.y + 2.f, 0.66f, c_textdim);
        c.y += 20.f;
        text(c, L"Canvas Overlay", c.col_x + 6.f, c.y + 2.f, 0.66f, c_textdim);
        c.y += 20.f;
        text(c, L"Toggle: INSERT", c.col_x + 6.f, c.y + 2.f, 0.66f, c_textdim);
    }

    static void draw_cursor(ctx& c)
    {
        float cx = c.mouse.x, cy = c.mouse.y;
        for (int i = 0; i < 16; i++)
        {
            float y = cy + (float)i;
            float right = cx + 1.f + (float)i * 0.75f;
            if (i > 12) right = cx + 3.f;
            c.canvas->k2_drawline({ cx, y }, { right, y }, 1.f, g_cursor);
        }
        line(c, cx, cy, cx + 12.f, cy + 11.f, 1.f, g_cursor_dim);
        line(c, cx + 12.f, cy + 11.f, cx + 5.f, cy + 11.f, 1.f, g_cursor_dim);
        line(c, cx + 5.f, cy + 11.f, cx, cy, 1.f, g_cursor_dim);
    }

    // ------------------------------------------------------------------ render

    static void render(ucanvas* canvas, uobject* font)
    {
        if (!canvas || !font) return;

        POINT mp;
        GetCursorPos(&mp);
        fvector2d mouse = { (float)mp.x, (float)mp.y };

        static bool s_prev_held = false;
        bool held    = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        bool clicked = held && !s_prev_held;
        s_prev_held = held;

        apply_theme(g_theme);
        g_cursor_dim = { g_cursor.r * 0.78f, g_cursor.g * 0.78f, g_cursor.b * 0.78f, 1.0f };

        const float WW = 720.f, WH = 500.f, HW = 44.f, SW = 168.f;

        fvector2d screen = canvas->get_screen_size();

        if (!g_win_inited)
        {
            g_win_pos = { (screen.x - WW) * 0.5f, (screen.y - WH) * 0.5f };
            if (g_win_pos.x < 0) g_win_pos.x = 0;
            if (g_win_pos.y < 0) g_win_pos.y = 0;
            g_win_inited = true;
        }

        fvector2d win = g_win_pos;

        // drag
        if (clicked && in_zone(ctx{ canvas, font, win, mouse, clicked, held, 0, 0, 0, 0 }, win.x, win.y, WW, HW))
        {
            g_dragging = true;
            g_drag_off = { mouse.x - win.x, mouse.y - win.y };
        }
        if (g_dragging)
        {
            if (held)
                win = { mouse.x - g_drag_off.x, mouse.y - g_drag_off.y };
            else
                g_dragging = false;
        }
        if (win.x < -WW + 60.f) win.x = -WW + 60.f;
        if (win.y < 0.f) win.y = 0.f;
        if (win.x > screen.x - 60.f) win.x = screen.x - 60.f;
        g_win_pos = win;

        ctx c;
        c.canvas = canvas;
        c.font = font;
        c.win = win;
        c.mouse = mouse;
        c.clicked = clicked;
        c.held = held;
        c.y0 = win.y + HW + 10.f;
        c.col_w = (WW - SW - 28.f - 12.f) * 0.5f;

        float cw = c.col_w;
        float cx0 = win.x + SW + 14.f;
        float cx1 = cx0 + cw + 12.f;

        // window bg
        rect(c, win.x, win.y, WW, WH, c_bg);
        outline(c, win.x, win.y, WW, WH, 1.f, c_border);

        // header
        grad_v(c, win.x, win.y, WW, HW, { 0.050f, 0.050f, 0.052f, 1.f }, { 0.018f, 0.018f, 0.020f, 1.f });
        rect(c, win.x, win.y, WW, 2.f, accent);
        text(c, L"vcc-library.uk", win.x + 16.f, win.y + 10.f, 0.95f, c_text);
        text(c, L"ON TOP", win.x + 16.f + text_w(c, L"vcc-library.uk", 0.95f) + 8.f, win.y + 13.f, 0.75f, accent_text);
        wchar_t head_right[64];
        swprintf(head_right, 64, L"v1.0.0");
        float hrw = text_w(c, head_right, 0.62f);
        text(c, head_right, win.x + WW - 14.f - hrw, win.y + 15.f, 0.62f, c_textdim);

        // sidebar
        rect(c, win.x, win.y + HW, SW, WH - HW, c_panel);
        line(c, win.x + SW, win.y + HW, win.x + SW, win.y + WH, 1.f, c_border);

        const float tab_h = 30.f;
        float tab_x = win.x + 6.f, tab_w = SW - 12.f;
        float tab_y = win.y + HW + 8.f;

        for (int i = 0; i < TAB_COUNT; i++)
        {
            float ty = tab_y + (float)i * (tab_h + 4.f);
            bool active = (g_tab == i);
            bool hover = in_zone(c, tab_x, ty, tab_w, tab_h);

            if (active)
            {
                grad_v(c, tab_x, ty, tab_w, tab_h, accent_dim, accent_dim);
                rect(c, tab_x, ty, 3.f, tab_h, accent);
            }
            else if (hover)
            {
                rect(c, tab_x, ty, tab_w, tab_h, c_hover);
            }

            text(c, TAB_NAMES[i], tab_x + 12.f, ty + 6.f, 0.72f, active ? c_text : (hover ? c_text : c_textdim));

            if (active)
                line(c, tab_x, ty + tab_h - 1.f, tab_x + tab_w, ty + tab_h - 1.f, 1.f, accent);

            if (click_zone(c, tab_x, ty, tab_w, tab_h))
            {
                g_tab = i;
                g_combo_open = -1;
                g_bind_key = -1;
            }
        }

        // content
        g_combo_id = 0;
        g_hotkey_id = 0;

        c.col_x = cx0;

        switch (g_tab)
        {
        case 0: tab_rage(c); break;
        case 1: tab_antiaim(c); break;
        case 2: tab_visuals(c); break;
        case 3: tab_world(c); break;
        case 4: tab_misc(c); break;
        case 5: tab_skins(c); break;
        case 6: tab_self(c); break;
        default: tab_config(c); break;
        }

        // close combo if clicked outside content region
        if (g_combo_open != -1 && clicked && !in_zone(c, cx0 - 4.f, win.y + HW + 4.f, (cx1 + cw - cx0) + 8.f, WH - HW - 12.f))
            g_combo_open = -1;

        draw_cursor(c);
    }
}

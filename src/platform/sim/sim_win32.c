#ifdef _WIN32
#define _WIN32_WINNT 0x0600 /* GetTickCount64 */
#include "platform/sim/sim_platform.h"

#include <string.h>
#include <windows.h>

enum { SCALE = 2 };

static sim_ctx *g_sim;
static HWND g_window;
static uint64_t g_keys;
static bool g_closed;
static uint32_t g_rgb[AE_SCREEN_W * AE_SCREEN_H];
static BITMAPINFO g_bmi;

static int key_from_vk(WPARAM vk)
{
    switch (vk) {
    case VK_LEFT: return AE_KEY_LEFT;
    case VK_UP: return AE_KEY_UP;
    case VK_DOWN: return AE_KEY_DOWN;
    case VK_RIGHT: return AE_KEY_RIGHT;
    case VK_RETURN: return AE_KEY_OK;
    case VK_ESCAPE: return AE_KEY_BACK;
    case VK_BACK: return AE_KEY_BACKSPACE;
    case VK_SHIFT: return AE_KEY_SHIFT;
    case VK_TAB: return AE_KEY_TOOLBOX;
    case 'V': return AE_KEY_VAR;
    default: return -1;
    }
}

static void present(HDC dc)
{
    for (int i = 0; i < AE_SCREEN_W * AE_SCREEN_H; i++) {
        uint16_t c = g_sim->fb[i];
        g_rgb[i] = ((uint32_t)((c >> 11) * 255 / 31) << 16) | ((uint32_t)(((c >> 5) & 0x3F) * 255 / 63) << 8) |
                   (uint32_t)((c & 0x1F) * 255 / 31);
    }
    StretchDIBits(dc, 0, 0, AE_SCREEN_W * SCALE, AE_SCREEN_H * SCALE, 0, 0, AE_SCREEN_W, AE_SCREEN_H, g_rgb, &g_bmi,
                  DIB_RGB_COLORS, SRCCOPY);
}

static LRESULT CALLBACK wnd_proc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
    int key;
    switch (msg) {
    case WM_CLOSE:
        g_closed = true;
        return 0;
    case WM_KEYDOWN:
    case WM_KEYUP:
        key = key_from_vk(wp);
        if (key >= 0) {
            if (msg == WM_KEYDOWN) {
                g_keys |= 1ull << key;
            } else {
                g_keys &= ~(1ull << key);
            }
        }
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(w, &ps);
        present(dc);
        EndPaint(w, &ps);
        return 0;
    }
    default:
        return DefWindowProcA(w, msg, wp, lp);
    }
}

static void pump(void)
{
    MSG m;
    while (PeekMessageA(&m, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&m);
        DispatchMessageA(&m);
    }
}

static uint64_t w_keys(void *ctx)
{
    (void)ctx;
    pump();
    return g_keys;
}

static uint64_t w_time(void *ctx)
{
    (void)ctx;
    return GetTickCount64();
}

static void w_sleep(void *ctx, uint32_t ms)
{
    (void)ctx;
    HDC dc = GetDC(g_window);
    present(dc);
    ReleaseDC(g_window, dc);
    pump();
    Sleep(ms);
}

static bool w_quit(void *ctx)
{
    (void)ctx;
    return g_closed;
}

bool sim_window_open(sim_ctx *s, ae_hal *hal)
{
    WNDCLASSA wc;
    RECT rc = {0, 0, AE_SCREEN_W * SCALE, AE_SCREEN_H * SCALE};
    ae_hal headless = sim_headless_hal(s);

    g_sim = s;
    memset(&wc, 0, sizeof wc);
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "AethraSim";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    if (!RegisterClassA(&wc)) {
        return false;
    }
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    g_window = CreateWindowA("AethraSim", "AETHRA simulator", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                             rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    if (g_window == NULL) {
        return false;
    }
    g_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    g_bmi.bmiHeader.biWidth = AE_SCREEN_W;
    g_bmi.bmiHeader.biHeight = -AE_SCREEN_H;
    g_bmi.bmiHeader.biPlanes = 1;
    g_bmi.bmiHeader.biBitCount = 32;
    g_bmi.bmiHeader.biCompression = BI_RGB;

    *hal = headless;
    hal->keys_scan = w_keys;
    hal->time_ms = w_time;
    hal->sleep_ms = w_sleep;
    hal->should_quit = w_quit;
    return true;
}
#endif

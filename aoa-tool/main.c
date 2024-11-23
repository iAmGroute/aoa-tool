#include "scrcpy.h"
#include "events.h"
#include "keyboard_aoa.h"
#include "util/log.h"

#include <SDL2/SDL.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define INPUT_BUFFER_SIZE 4000

static void
sc_usb_on_disconnected(struct sc_usb *usb, void *userdata) {
    (void) usb;
    (void) userdata;

    sc_push_event(SC_EVENT_USB_DEVICE_DISCONNECTED);
}

bool set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        return false;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        return false;
    }
    return true;
}

size_t find_line(char *buf) {
    char *newline = strchr(buf, '\n');
    return newline == NULL ? 0 : (size_t)(newline - buf) + 1;
}

bool process_line(const char *line, struct sc_key_event *event) {
    SDL_Keycode key;
    Uint16      scan;
    Uint16      mod;
    if (line[0] == '#') {
        LOGI("%s", line);
        return false;
    }
    int n = sscanf(line, "%d %hu %hu", &key, &scan, &mod);
    if (n != 3 || scan >= SDL_NUM_SCANCODES) {
        LOGE("Bad input line: %s", line);
        return false;
    } else {
        event->action     = SC_ACTION_DOWN;
        event->keycode    = sc_keycode_from_sdl((SDL_Keycode)key);
        event->scancode   = sc_scancode_from_sdl(scan);
        event->mods_state = sc_mods_state_from_sdl(mod);
        event->repeat     = false;
        return true;
    }
}

void send_key(struct sc_key_processor *key_processor, const struct sc_key_event *event) {
    assert(key_processor->ops->process_key);
    key_processor->ops->process_key(key_processor, event, SC_SEQUENCE_INVALID);
}

int event_loop(struct sc_key_processor *key_processor, int key_interval) {
    if (!set_nonblock(0)) return 1;

    char buf[INPUT_BUFFER_SIZE] = {0};
    size_t buf_len = 0;      // excluding the '\0'
    size_t buf_line_len = 0; // including the '\n'

    struct sc_key_event last_event = {
        .action = SC_ACTION_UP,
    };
    SDL_Event event = {0};
    for (;;) {
        if (SDL_WaitEventTimeout(&event, key_interval)) {
            switch (event.type) {
                case SC_EVENT_USB_DEVICE_DISCONNECTED: return 1;
                case SC_EVENT_AOA_OPEN_ERROR:          return 1;
                case SDL_QUIT:                         return 0;
            }
        } else {
            if (last_event.action == SC_ACTION_DOWN) {
                last_event.action =  SC_ACTION_UP;
                send_key(key_processor, &last_event);
            } else {
                // try to get from buffer

                bool line_ok = false;
                while (!line_ok) {
                    line_ok = true;

                    // maybe we can read a new line
                    if (buf_line_len == 0) {
                        ssize_t n = read(0, &buf[buf_len], sizeof(buf) - buf_len - 1);
                        if (n > 0) {
                            buf_len += n;
                            buf[buf_len] = '\0';
                            buf_line_len = find_line(buf);
                        } else if (n == 0) {
                            return 0;
                        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            perror("read");
                            return 1;
                        }
                    }

                    // maybe we can process a line
                    if (buf_line_len > 0) {
                        buf[buf_line_len-1] = '\0';
                        if (buf_line_len > 1) {
                            line_ok = process_line(buf, &last_event);
                            if (line_ok) send_key(key_processor, &last_event);
                        } else {
                            // empty lines (len==1) do not need to be parsed
                            // as a side-effect, they will act as a time delay
                            LOGI("%s", "(delay)");
                        }
                        buf_len -= buf_line_len;
                        memmove(buf, &buf[buf_line_len], buf_len);
                        buf[buf_len] = '\0';
                        buf_line_len = find_line(buf);
                    }
                }
            }
        }
    }
    return 1;
}

int main(int argc, char **argv) {
    int ret = 1;

    int key_interval = 120;
    if (argc > 1) {
        if (argc > 2) {
            LOGE(
                "Too many arguments.\n"
                "Usage: %s [KEY_INTERVAL_MILLISECONDS=300]\n",
                argv[0]
            );
            return 1;
        }
        int n = sscanf(argv[1], "%u", &key_interval);
        if (n != 1 || key_interval < 10 || key_interval > 10000) {
            LOGE(
                "Bad argument: KEY_INTERVAL_MILLISECONDS=%s",
                argv[1]
            );
            return 1;
        }
        if (key_interval > 200) {
            LOGW(
                "Argument KEY_INTERVAL_MILLISECONDS=%d is too large and key events may be interpretted as long press",
                key_interval
            );
            return 1;
        }
    }

    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER)) {
        LOGE("Could not initialize SDL: %s", SDL_GetError());
        return SCRCPY_EXIT_FAILURE;
    }

    atexit(SDL_Quit);

    struct sc_usb usb;
    struct sc_aoa aoa;
    struct sc_keyboard_aoa keyboard;

    bool usb_device_initialized = false;
    bool usb_connected          = false;
    bool aoa_initialized        = false;
    bool keyboard_initialized   = false;
    bool aoa_started            = false;

    bool ok = false;

    static const struct sc_usb_callbacks cbs = {
        .on_disconnected = sc_usb_on_disconnected,
    };
    ok = sc_usb_init(&usb);
    if (!ok) return 1;

    struct sc_usb_device usb_device;
    ok = sc_usb_select_device(&usb, NULL, &usb_device);
    if (!ok) goto end;

    usb_device_initialized = true;

    ok = sc_usb_connect(&usb, usb_device.device, &cbs, NULL);
    if (!ok) goto end;
    usb_connected = true;

    ok = sc_aoa_init(&aoa, &usb, NULL);
    if (!ok) goto end;
    aoa_initialized = true;

    ok = sc_keyboard_aoa_init(&keyboard, &aoa);
    if (!ok) goto end;
    keyboard_initialized = true;

    ok = sc_aoa_start(&aoa);
    if (!ok) goto end;
    aoa_started = true;

    // usb_device not needed anymore
    sc_usb_device_destroy(&usb_device);
    usb_device_initialized = false;

    ret = event_loop(&keyboard.key_processor, key_interval);
    LOGD("quit...");

end:
    if (aoa_started) {
        sc_aoa_stop(&aoa);
    }
    sc_usb_stop(&usb);

    if (keyboard_initialized) {
        sc_keyboard_aoa_destroy(&keyboard);
    }

    if (aoa_initialized) {
        sc_aoa_join(&aoa);
        sc_aoa_destroy(&aoa);
    }

    sc_usb_join(&usb);

    if (usb_connected) {
        sc_usb_disconnect(&usb);
    }

    if (usb_device_initialized) {
        sc_usb_device_destroy(&usb_device);
    }

    sc_usb_destroy(&usb);

    return ret;
}

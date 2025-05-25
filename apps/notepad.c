/**
 * ToaruOS Notepad - GUIメモ帳アプリ
 * 保存: Ctrl+S / 読込: Ctrl+O / コピー: Ctrl+C / ペースト: Ctrl+V / 色変更: Ctrl+L
 */
#include <toaru/yutani.h>
#include <toaru/graphics.h>
#include <toaru/decorations.h>
#include <toaru/text.h>
#include <toaru/menu.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 65536

static yutani_t * yctx;
static yutani_window_t * window;
static gfx_context_t * ctx;

static char text_buffer[BUFFER_SIZE];
static int text_length = 0;

static uint32_t text_color = rgb(0, 0, 0);

void redraw() {
	draw_fill(ctx, rgb(255,255,255));
	struct decor_bounds bounds;
	decor_get_bounds(window, &bounds);

	tt_set_size("sans", 14);
	tt_set_foreground(text_color);
	tt_draw_string(ctx, bounds.left_width + 5, bounds.top_height + 20, text_buffer);

	render_decorations(window, ctx, "Notepad");
	flip(ctx);
	yutani_flip(yctx, window);
}

void save_file() {
	FILE * f = fopen("/tmp/notepad.txt", "w");
	if (f) {
		fwrite(text_buffer, 1, text_length, f);
		fclose(f);
	}
}

void load_file() {
	FILE * f = fopen("/tmp/notepad.txt", "r");
	if (f) {
		text_length = fread(text_buffer, 1, BUFFER_SIZE - 1, f);
		text_buffer[text_length] = '\0';
		fclose(f);
	}
}

void copy_clipboard() {
	FILE * f = fopen("/dev/clipboard", "w");
	if (f) {
		fwrite(text_buffer, 1, text_length, f);
		fclose(f);
	}
}

void paste_clipboard() {
	FILE * f = fopen("/dev/clipboard", "r");
	if (f) {
		text_length = fread(text_buffer, 1, BUFFER_SIZE - 1, f);
		text_buffer[text_length] = '\0';
		fclose(f);
	}
}

void change_color() {
	static int i = 0;
	i = (i + 1) % 4;
	switch (i) {
		case 0: text_color = rgb(0,0,0); break;
		case 1: text_color = rgb(255,0,0); break;
		case 2: text_color = rgb(0,128,0); break;
		case 3: text_color = rgb(0,0,255); break;
	}
}

int main(int argc, char ** argv) {
	yctx = yutani_init();
	init_decorations();

	int width = 400, height = 300;
	struct decor_bounds bounds;
	decor_get_bounds(NULL, &bounds);

	window = yutani_window_create(yctx, width + bounds.width, height + bounds.height);
	yutani_window_move(yctx, window, 100, 100);
	yutani_window_advertise_icon(yctx, window, "Notepad", "text");

	ctx = init_graphics_yutani_double_buffer(window);

	redraw();

	while (1) {
		yutani_msg_t * msg = yutani_poll(yctx);
		switch (msg->type) {
			case YUTANI_MSG_KEY_EVENT: {
				struct yutani_msg_key_event * ke = (void*)msg->data;
				if (ke->event.action == KEY_ACTION_DOWN) {
					if (ke->event.modifiers & KEY_MOD_CTRL) {
						switch (ke->event.keycode) {
							case 's': save_file(); break;
							case 'o': load_file(); break;
							case 'c': copy_clipboard(); break;
							case 'v': paste_clipboard(); break;
							case 'l': change_color(); break;
						}
					} else if (ke->event.key) {
						if (ke->event.key == '\b') {
							if (text_length > 0) text_buffer[--text_length] = '\0';
						} else if (text_length < BUFFER_SIZE - 1) {
							text_buffer[text_length++] = ke->event.key;
							text_buffer[text_length] = '\0';
						}
					}
					redraw();
				}
			} break;
			case YUTANI_MSG_WINDOW_CLOSE:
			case YUTANI_MSG_SESSION_END:
				goto done;
			default: break;
		}
		free(msg);
	}
done:
	yutani_close(yctx, window);
	return 0;
}

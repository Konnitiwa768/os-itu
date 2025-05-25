#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <toaru/graphics.h>
#include <toaru/yutani.h>
#include <toaru/yutani-window.h>
#include <toaru/trace.h>
#include <toaru/decorations.h>
#include <toaru/spinlock.h>
#include <toaru/pex.h>
#include <time.h>

#define FRAME_COUNT 120
#define FRAME_DELAY_USEC 41666 // ~24 FPS

int main(int argc, char *argv[]) {
	yutani_t * yctx = yutani_init();
	if (!yctx) return 1;

	int width  = 640;
	int height = 360;

	struct decor_bounds bounds;
	decor_get_bounds(width, height, &bounds);

	yutani_window_t * window = yutani_window_create(yctx, width + bounds.width, height + bounds.height);
	yutani_window_move(yctx, window, 50, 50);
	yutani_window_advertise_icon(yctx, window, "Video Player", "vdplay");

	gfx_context_t * ctx = init_graphics_yutani_double_buffer(window);

	decor_set_default(window, "ビデオプレイヤー");
	render_decorations(window, ctx, "ビデオプレイヤー");

	// 描画ループ
	for (int frame = 1; frame <= FRAME_COUNT; ++frame) {
		char path[256];
		snprintf(path, sizeof(path), "/usr/share/video/frames/frame_%04d.bmp", frame);

		sprite_t * image = decode_bmp(path);
		if (!image) continue;

		draw_fill(ctx, rgb(0,0,0));
		draw_sprite_scaled(ctx, image, bounds.left, bounds.top, width, height);

		free(image);

		render_decorations(window, ctx, "Video Player");
		flip(ctx);
		usleep(FRAME_DELAY_USEC);
	}

	// イベントを待ち続ける（ビデオ終了後もウィンドウ保持）
	while (1) {
		yutani_msg_t * m = yutani_poll(yctx);
		switch (m->type) {
			case YUTANI_MSG_WELCOME:
			case YUTANI_MSG_RESIZE_OFFER:
			case YUTANI_MSG_KEY_EVENT:
			case YUTANI_MSG_MOUSE_EVENT:
				break;
			case YUTANI_MSG_WINDOW_CLOSE:
				yutani_close(yctx, window);
				return 0;
		}
		free(m);
	}

	return 0;
}

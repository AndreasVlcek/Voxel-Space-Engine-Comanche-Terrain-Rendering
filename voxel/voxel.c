#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "dos/dos.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

/****************************************************************************/
/* BUFFERS FOR HEIGHTMAP & COLORMAP                                         */
/****************************************************************************/
uint8_t* heightmap = NULL; // Buffer/array to hold height values (1024*1024)
uint8_t* colormap = NULL; // Buffer/array to hold color values (1024*1024)

typedef struct {
	float x;		// x position on the map
	float y;		// y position on the map
	float zfar;		// distance of the camera looking forward
} camera_t;

camera_t camera = {
	.x = 512,
	.y = 512,
	.zfar = 400
};

/****************************************************************************/
/* MAIN                                                                     */
/****************************************************************************/
int main(int argc, char* args[]) {
	setvideomode(videomode_320x200);
	
	// TODO: load the GIF files and load colormap and heightmap buffers
	uint8_t palette[256 * 3];
	int map_width, map_height, pal_count;
	
	colormap = loadgif("maps/colormap.gif", &map_width, &map_height, &pal_count, palette);
	heightmap = loadgif("maps/heightmap.gif", &map_width, &map_height, NULL, NULL);
	
	for (int i = 0; i < pal_count; i++) {
		setpal(i, palette[3 * i + 0], palette[3 * i + 1], palette[3 * i +2]);
	}
	setpal(0, 36, 36, 56);
	
	setdoublebuffer(1);
	uint8_t* framebuffer = screenbuffer();
	
	while(!shuttingdown()) {
		waitvbl();
		clearscreen();

		float plx = -camera.zfar;
		float ply = +camera.zfar;
		
		float prx = +camera.zfar;
		float pry = +camera.zfar;
		
		// Loop 320 rays from left to right
		for (int i = 0; i < SCREEN_WIDTH; i++) {
			float delta_x = (plx + (prx - plx) / SCREEN_WIDTH * i) / camera.zfar;
			float delta_y = (ply + (pry - ply) / SCREEN_WIDTH * i) / camera.zfar;
			
			float rx = camera.x;
			float ry = camera.y;
			
			float max_height = SCREEN_HEIGHT;
			
			for (int z = 1; z < camera.zfar; z++) {
				rx += delta_x;
				ry -= delta_y;
				
				// Find the offset that we have to go and fetch values from the heightmap
				int mapoffset = ((1024 * (int)(ry)) + (int)(rx));
				
				int heightonscreen = (int)((100.0 - heightmap[mapoffset]) / z * 100.0);
				
				if (heightonscreen < 0) {
					heightonscreen = 0;
				}
				if (heightonscreen > SCREEN_HEIGHT) {
					heightonscreen = SCREEN_HEIGHT - 1;
				}
				
				// Only render terrain pixels if the new projected height is taller than the previous max-height
				if (heightonscreen < max_height) {
					// Draw pixels from previous max-height until the new projected height
					for (int y = heightonscreen; y < max_height; y++) {
						framebuffer[(SCREEN_WIDTH * y) + i] = (uint8_t)colormap[mapoffset];
					}
					max_height = heightonscreen;
				}
			}
		}
		
		framebuffer = swapbuffers();
		
		if (keystate(KEY_ESCAPE))
			break;
	}
	
	return EXIT_SUCCESS;
}
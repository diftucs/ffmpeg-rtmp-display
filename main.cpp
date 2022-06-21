/*
	Copyright © 2022 diftucs

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_thread.h>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
}

int main(int argc, char **argv)
{
	// Init input AVFormatContext. Will contain the input AVStream
	AVFormatContext *inputFormatContext = avformat_alloc_context();
	// Init input AVStream according to properties read from the input
	avformat_open_input(&inputFormatContext, "rtmp://localhost/publishlive/livestream", NULL, NULL);
	avformat_find_stream_info(inputFormatContext, NULL); // Will hang waiting for input

	// Init an AVCodecContext with the codec needed to decode the input AVStream
	AVCodecContext *inputCodecContext = avcodec_alloc_context3(avcodec_find_decoder(inputFormatContext->streams[0]->codecpar->codec_id));
	// Apply the input AVStream's parameters to the codec
	avcodec_parameters_to_context(inputCodecContext, inputFormatContext->streams[0]->codecpar);
	// Initialize the input codec
	avcodec_open2(inputCodecContext, NULL, NULL);

	// Allocate memory needed for decoding
	AVPacket *inputPacket = av_packet_alloc();
	AVFrame *inputFrame = av_frame_alloc();
	AVFrame *outputFrame = av_frame_alloc();

	// Allocate buffer for outputFrame since av_frame_alloc only allocates memory for the frame itself
	outputFrame->format = inputCodecContext->pix_fmt;
	outputFrame->width = inputCodecContext->width;
	outputFrame->height = inputCodecContext->height;
	av_frame_get_buffer(outputFrame, 0);

	// Init SDL library
	SDL_Init(SDL_INIT_VIDEO);
	// SDL display window
	SDL_Window *window = SDL_CreateWindow("Stream display", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, inputFormatContext->streams[0]->codecpar->width, inputFormatContext->streams[0]->codecpar->height, SDL_WINDOW_RESIZABLE);
	// SDL rendering context
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	// What SDL draws on the window through the rendering context
	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, inputFormatContext->streams[0]->codecpar->width, inputFormatContext->streams[0]->codecpar->height);

	// Init FFmpeg scaling to fit frames to the SDL texture
	SwsContext *sws_ctx = sws_getContext(inputFormatContext->streams[0]->codecpar->width,
										 inputFormatContext->streams[0]->codecpar->height,
										 static_cast<AVPixelFormat>(inputFormatContext->streams[0]->codecpar->format),
										 inputFormatContext->streams[0]->codecpar->width,
										 inputFormatContext->streams[0]->codecpar->height,
										 static_cast<AVPixelFormat>(inputFormatContext->streams[0]->codecpar->format),
										 SWS_BILINEAR,
										 NULL,
										 NULL,
										 NULL);

	SDL_Event event;
	while (1)
	{
		// Handle SDL window resizing
		while (SDL_PollEvent(&event))
		{
			if (event.type = SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				// Resize the SDL texture to fit the SDL window
				SDL_DestroyTexture(texture);
				texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STATIC, event.window.data1, event.window.data2);

				// Change the target resolution for the FFmpeg rescaler to fit the new SDL texture resolution
				sws_ctx = sws_getCachedContext(sws_ctx,
											   inputFormatContext->streams[0]->codecpar->width,
											   inputFormatContext->streams[0]->codecpar->height,
											   static_cast<AVPixelFormat>(inputFormatContext->streams[0]->codecpar->format),
											   event.window.data1,
											   event.window.data2,
											   static_cast<AVPixelFormat>(inputFormatContext->streams[0]->codecpar->format),
											   SWS_BILINEAR,
											   NULL,
											   NULL,
											   NULL);
			}
		}

		// Get packet from input
		av_read_frame(inputFormatContext, inputPacket);

		// Send packet to decoder
		avcodec_send_packet(inputCodecContext, inputPacket);
		// Fetch frame from decoder and loop if more data is needed
		if (avcodec_receive_frame(inputCodecContext, inputFrame) == AVERROR(EAGAIN))
			continue;

		// Scale frame with FFmpeg to fit the texture
		sws_scale(sws_ctx, inputFrame->data, inputFrame->linesize, 0, inputFormatContext->streams[0]->codecpar->height, outputFrame->data, outputFrame->linesize);

		// Draw the new data to the texture
		SDL_UpdateYUVTexture(texture, NULL, outputFrame->data[0], outputFrame->linesize[0],
							 outputFrame->data[1], outputFrame->linesize[1],
							 outputFrame->data[2], outputFrame->linesize[2]);

		// Clear the renderer's old data and copy the new texture to it's backbuffer
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);

		// Apply the rendering context's backbuffer
		SDL_RenderPresent(renderer);

		// Draw the renderer's data onto the window
		SDL_UpdateWindowSurface(window);

		// Free the packet buffers since they are not reused
		av_packet_unref(inputPacket);
	}

	return 0;
}

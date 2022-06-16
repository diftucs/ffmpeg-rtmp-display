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

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
}

int main(int argc, char **argv)
{
	// Init input AVFormatContext. Will contain the input AVStream
	AVFormatContext *inputFormatContext = avformat_alloc_context();
	// Init input AVStream according to properties read from the input
	avformat_open_input(&inputFormatContext, "rtmp://localhost/publishlive/livestream", NULL, NULL);
	avformat_find_stream_info(inputFormatContext, NULL); // Will hang waiting for input

	// Init the output AVFormatContext. Will contain the output AVStream
	AVFormatContext *outputFormatContext;
	avformat_alloc_output_context2(&outputFormatContext, NULL, "flv", NULL);

	// Init the output AVStream
	AVStream *outputStream = avformat_new_stream(outputFormatContext, NULL);
	outputStream->codecpar = inputFormatContext->streams[0]->codecpar;

	// Open the output stream
	avio_open(&outputFormatContext->pb, "/tmp/saved.flv", AVIO_FLAG_WRITE);

	// Write output header
	avformat_write_header(outputFormatContext, NULL);

	// Allocate memory for encoded pre- and post-converted packets
	AVPacket *inputPacket = av_packet_alloc();

	for (int i = 0; i < 250; i++)
	{
		// Get packet from input
		av_read_frame(inputFormatContext, inputPacket);

		// Write packet to output
		av_interleaved_write_frame(outputFormatContext, inputPacket);

		// Free the packet buffers since they are not reused
		av_packet_unref(inputPacket);
	}

	// Write end of stream
	av_write_trailer(outputFormatContext);

	return 0;
}

#include "Audio/Audio.h"

#include "Assets/SoundWave.h"
#include "Maths.h"
#include "Stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <vorbis/vorbisenc.h>

// Most of this vorbis encoding / decoding code was taken from the official libvorbis samples.

#define READ 1024
static signed char readbuffer[READ * 4 + 44]; /* out of the data segment, not the stack */

void AUD_EncodeVorbis(Stream& inStream, Stream& outStream, PcmFormat format)
{
    ogg_stream_state os; /* take physical pages, weld into a logical
                        stream of packets */
    ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
    ogg_packet       op; /* one raw packet of data for decode */

    vorbis_info      vi; /* struct that stores all the static vorbis bitstream
                            settings */
    vorbis_comment   vc; /* struct that stores all the user comments */

    vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
    vorbis_block     vb; /* local working space for packet->PCM decode */

    int eos = 0, ret;

    vorbis_info_init(&vi);
    ret = vorbis_encode_init_vbr(&vi, (long)format.mNumChannels, (long) format.mSampleRate, 0.1f);

    int bytesPerSample = (int)format.mBytesPerSample;
    int bytesPerFrame = (int)(format.mNumChannels * format.mBytesPerSample);

    if (ret)
        exit(1);

    /* add a comment */
    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "Octave");

    /* set up the analysis state and auxiliary encoding storage */
    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);

    /* set up our packet->stream encoder */
    /* pick a random serial number; that way we can more likely build
       chained streams just by concatenation */
    //srand(time(NULL));
    ogg_stream_init(&os, Maths::RandRange(0, RAND_MAX));

    /* Vorbis streams begin with three headers; the initial header (with
    most of the codec setup parameters) which is mandated by the Ogg
    bitstream spec.  The second header holds any comment fields.  The
    third header holds the bitstream codebook.  We merely need to
    make the headers, then pass them to libvorbis one at a time;
    libvorbis handles the additional Ogg bitstream constraints */

    {
        ogg_packet header;
        ogg_packet header_comm;
        ogg_packet header_code;

        vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
        ogg_stream_packetin(&os, &header); /* automatically placed in its own
                                             page */
        ogg_stream_packetin(&os, &header_comm);
        ogg_stream_packetin(&os, &header_code);

        /* This ensures the actual
         * audio data will start on a new page, as per spec
         */
        while (!eos) {
            int result = ogg_stream_flush(&os, &og);
            if (result == 0)break;
            outStream.WriteBytes(og.header, og.header_len);
            outStream.WriteBytes(og.body, og.body_len);
        }

    }


    while (!eos) {
        long i;
        long bytes = (long)inStream.ReadBytesMax((uint8_t*)readbuffer, READ * bytesPerFrame);

        if (bytes == 0) {
            /* end of file.  this can be done implicitly in the mainline,
               but it's easier to see here in non-clever fashion.
               Tell the library we're at end of stream so that it can handle
               the last frame and mark end of stream in the output properly */
            vorbis_analysis_wrote(&vd, 0);

        }
        else {
            /* data to encode */

            /* expose the buffer to submit data */
            float **buffer = vorbis_analysis_buffer(&vd, READ);

            if (format.mNumChannels == 1)
            {
                if (format.mBytesPerSample == 1)
                {
                    for (i = 0; i < bytes; i++) {
                        int16_t samp16 = (int16_t) readbuffer[i];
                        samp16 = (samp16 - 0x80) << 8;

                        char* pSamp16 = (char*)&samp16;
                        buffer[0][i] = ((pSamp16[1] << 8) |
                            (0x00ff & (int)pSamp16[0])) / 32768.f;
                    }
                }
                else
                {
                    for (i = 0; i < bytes / 2; i++) {
                        buffer[0][i] = ((readbuffer[i * 2 + 1] << 8) |
                            (0x00ff & (int)readbuffer[i * 2])) / 32768.f;
                    }
                }
            }
            else
            {
                /* uninterleave samples */
                if (format.mBytesPerSample == 1)
                {
                    for (i = 0; i < bytes / 2; i++) {
                        int16_t samps16[2] = { (int16_t)readbuffer[i * 2], (int16_t)readbuffer[i * 2 + 1] };
                        samps16[0] = (samps16[0] - 0x80) << 8;
                        samps16[1] = (samps16[1] - 0x80) << 8;

                        char* pSamp16 = (char*)samps16;

                        buffer[0][i] = ((pSamp16[1] << 8) |
                            (0x00ff & (int)pSamp16[0])) / 32768.f;
                        buffer[1][i] = ((pSamp16[3] << 8) |
                            (0x00ff & (int)pSamp16[2])) / 32768.f;
                    }
                }
                else
                {
                    for (i = 0; i < bytes / 4; i++) {
                        buffer[0][i] = ((readbuffer[i * 4 + 1] << 8) |
                            (0x00ff & (int)readbuffer[i * 4])) / 32768.f;
                        buffer[1][i] = ((readbuffer[i * 4 + 3] << 8) |
                            (0x00ff & (int)readbuffer[i * 4 + 2])) / 32768.f;
                    }
                }
            }

            /* tell the library how much we actually submitted */
            vorbis_analysis_wrote(&vd, i);
        }

        /* vorbis does some data preanalysis, then divvies up blocks for
           more involved (potentially parallel) processing.  Get a single
           block for encoding now */
        while (vorbis_analysis_blockout(&vd, &vb) == 1) {

            /* analysis, assume we want to use bitrate management */
            vorbis_analysis(&vb, NULL);
            vorbis_bitrate_addblock(&vb);

            while (vorbis_bitrate_flushpacket(&vd, &op)) {

                /* weld the packet into the bitstream */
                ogg_stream_packetin(&os, &op);

                /* write out pages (if any) */
                while (!eos) {
                    int result = ogg_stream_pageout(&os, &og);
                    if (result == 0)break;
                    outStream.WriteBytes(og.header, og.header_len);
                    outStream.WriteBytes(og.body, og.body_len);

                    /* this could be set above, but for illustrative purposes, I do
                       it here (to show that vorbis does know where the stream ends) */

                    if (ogg_page_eos(&og))eos = 1;
                }
            }
        }
    }

    /* clean up and exit.  vorbis_info_clear() must be called last */

    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);

    /* ogg_page and ogg_packet structs always point to storage in
       libvorbis.  They're never freed or manipulated directly */

    LogDebug("Encoded Vorbis: %d bytes -> %d bytes", inStream.GetSize(), outStream.GetSize());
}

static ogg_int16_t convbuffer[4096]; /* take 8k out of the data segment, not the stack */
static uint8_t convbuffer8[4096];
static int convsize = 4096;

void AUD_DecodeVorbis(Stream& inStream, Stream& outStream, PcmFormat format)
{
    ogg_sync_state   oy; /* sync and verify incoming physical bitstream */
    ogg_stream_state os; /* take physical pages, weld into a logical
                            stream of packets */
    ogg_page         og; /* one Ogg bitstream page. Vorbis packets are inside */
    ogg_packet       op; /* one raw packet of data for decode */

    vorbis_info      vi; /* struct that stores all the static vorbis bitstream
                            settings */
    vorbis_comment   vc; /* struct that stores all the bitstream user comments */
    vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
    vorbis_block     vb; /* local working space for packet->PCM decode */

    char *buffer;
    int  bytes;

    /********** Decode setup ************/

    ogg_sync_init(&oy); /* Now we can read pages */

    while (1) { /* we repeat if the bitstream is chained */
        int eos = 0;
        int i;

        /* grab some data at the head of the stream. We want the first page
           (which is guaranteed to be small and only contain the Vorbis
           stream initial header) We need the first page to get the stream
           serialno. */

           /* submit a 4k block to libvorbis' Ogg layer */
        buffer = ogg_sync_buffer(&oy, 4096);
        bytes = inStream.ReadBytesMax((uint8_t*)buffer, 4096);
        ogg_sync_wrote(&oy, bytes);

        /* Get the first page. */
        if (ogg_sync_pageout(&oy, &og) != 1) {
            /* have we simply run out of data?  If so, we're done. */
            if (bytes < 4096)break;

            /* error case.  Must not be Vorbis data */
            LogError("Input does not appear to be an Ogg bitstream.");
            OCT_ASSERT(0);
        }

        /* Get the serial number and set up the rest of decode. */
        /* serialno first; use it to set up a logical stream */
        ogg_stream_init(&os, ogg_page_serialno(&og));

        /* extract the initial header from the first page and verify that the
           Ogg bitstream is in fact Vorbis data */

           /* I handle the initial header first instead of just having the code
              read all three Vorbis headers at once because reading the initial
              header is an easy way to identify a Vorbis bitstream and it's
              useful to see that functionality seperated out. */

        vorbis_info_init(&vi);
        vorbis_comment_init(&vc);
        if (ogg_stream_pagein(&os, &og) < 0) {
            /* error; stream version mismatch perhaps */
            LogError("Error reading first page of Ogg bitstream data.");
            OCT_ASSERT(0);
        }

        if (ogg_stream_packetout(&os, &op) != 1) {
            /* no page? must not be vorbis */
            LogError("Error reading initial header packet.");
            OCT_ASSERT(0);
        }

        if (vorbis_synthesis_headerin(&vi, &vc, &op) < 0) {
            /* error case; not a vorbis header */
            LogError("This Ogg bitstream does not contain Vorbis audio data.");
            OCT_ASSERT(0);
        }

        /* At this point, we're sure we're Vorbis. We've set up the logical
           (Ogg) bitstream decoder. Get the comment and codebook headers and
           set up the Vorbis decoder */

           /* The next two packets in order are the comment and codebook headers.
              They're likely large and may span multiple pages. Thus we read
              and submit data until we get our two packets, watching that no
              pages are missing. If a page is missing, error out; losing a
              header page is the only place where missing data is fatal. */

        i = 0;
        while (i < 2) {
            while (i < 2) {
                int result = ogg_sync_pageout(&oy, &og);
                if (result == 0)break; /* Need more data */
                /* Don't complain about missing or corrupt data yet. We'll
                   catch it at the packet output phase */
                if (result == 1) {
                    ogg_stream_pagein(&os, &og); /* we can ignore any errors here
                                                   as they'll also become apparent
                                                   at packetout */
                    while (i < 2) {
                        result = ogg_stream_packetout(&os, &op);
                        if (result == 0)break;
                        if (result < 0) {
                            /* Uh oh; data at some point was corrupted or missing!
                               We can't tolerate that in a header.  Die. */
                            LogError("Corrupt secondary header.");
                            OCT_ASSERT(0);
                        }
                        result = vorbis_synthesis_headerin(&vi, &vc, &op);
                        if (result < 0) {
                            LogError("Corrupt secondary header.");
                            OCT_ASSERT(0);
                        }
                        i++;
                    }
                }
            }
            /* no harm in not checking before adding more */
            buffer = ogg_sync_buffer(&oy, 4096);
            bytes = inStream.ReadBytesMax((uint8_t*)buffer, 4096);
            if (bytes == 0 && i < 2) {
                LogError("End of file before finding all Vorbis headers!");
                OCT_ASSERT(0);
            }
            ogg_sync_wrote(&oy, bytes);
        }

        /* Throw the comments plus a few lines about the bitstream we're
           decoding */
        {
            char **ptr = vc.user_comments;
            while (*ptr) {
                LogDebug("%s", *ptr);
                ++ptr;
            }
            LogDebug("Bitstream is %d channel, %ldHz", vi.channels, vi.rate);
            LogDebug("Encoded by: %s", vc.vendor);

            OCT_ASSERT(vi.channels == (int)format.mNumChannels);
            OCT_ASSERT(vi.rate == (int)format.mSampleRate);
        }

        convsize = 4096 / vi.channels;

        /* OK, got and parsed all three headers. Initialize the Vorbis
           packet->PCM decoder. */
        if (vorbis_synthesis_init(&vd, &vi) == 0) { /* central decode state */
            vorbis_block_init(&vd, &vb);          /* local state for most of the decode
                                                    so multiple block decodes can
                                                    proceed in parallel. We could init
                                                    multiple vorbis_block structures
                                                    for vd here */

                                                    /* The rest is just a straight decode loop until end of stream */
            while (!eos) {
                while (!eos) {
                    int result = ogg_sync_pageout(&oy, &og);
                    if (result == 0)break; /* need more data */
                    if (result < 0) { /* missing or corrupt data at this page position */
                        LogError("Corrupt or missing data in bitstream; continuing...");
                    }
                    else {
                        ogg_stream_pagein(&os, &og); /* can safely ignore errors at
                                                       this point */
                        while (1) {
                            result = ogg_stream_packetout(&os, &op);

                            if (result == 0)break; /* need more data */
                            if (result < 0) { /* missing or corrupt data at this page position */
                              /* no reason to complain; already complained above */
                            }
                            else {
                                /* we have a packet.  Decode it */
                                float **pcm;
                                int samples;

                                if (vorbis_synthesis(&vb, &op) == 0) /* test for success! */
                                    vorbis_synthesis_blockin(&vd, &vb);
                                /*

                                **pcm is a multichannel float vector.  In stereo, for
                                example, pcm[0] is left, and pcm[1] is right.  samples is
                                the size of each channel.  Convert the float values
                                (-1.<=range<=1.) to whatever PCM format and write it out */

                                while ((samples = vorbis_synthesis_pcmout(&vd, &pcm)) > 0) {
                                    int j;
                                    int clipflag = 0;
                                    int bout = (samples < convsize ? samples : convsize);

                                    /* convert floats to 16 bit signed ints (host order) and
                                       interleave */
                                    for (i = 0; i < vi.channels; i++) {
                                        ogg_int16_t *ptr = convbuffer + i;
                                        float  *mono = pcm[i];
                                        for (j = 0; j < bout; j++) {
#if 1
                                            int val = (int) floor(mono[j] * 32767.f + .5f);
#else /* optional dither */
                                            int val = mono[j] * 32767.f + drand48() - 0.5f;
#endif
                                            /* might as well guard against clipping */
                                            if (val > 32767) {
                                                val = 32767;
                                                clipflag = 1;
                                            }
                                            if (val < -32768) {
                                                val = -32768;
                                                clipflag = 1;
                                            }
                                            *ptr = val;
                                            ptr += vi.channels;
                                        }
                                    }

                                    // Disabling this. Remove false to log clipping frames.
                                    if (clipflag && false)
                                    {
                                        LogWarning("Clipping in frame %ld", (long)(vd.sequence));
                                    }

                                    uint8_t* srcBuffer = (uint8_t*)convbuffer;

                                    if (format.mBytesPerSample == 1)
                                    {
                                        // Need to convert 16 bit signed to 8 bit unsigned.
                                        for (int32_t i = 0; i < vi.channels * bout; ++i)
                                        {
                                            convbuffer8[i] = (uint8_t)((convbuffer[i] + 32768) >> 8);
                                        }

                                        srcBuffer = convbuffer8;
                                    }

                                    outStream.WriteBytes(srcBuffer, format.mBytesPerSample * vi.channels * bout);

                                    vorbis_synthesis_read(&vd, bout); /* tell libvorbis how
                                                                        many samples we
                                                                        actually consumed */
                                }
                            }
                        }
                        if (ogg_page_eos(&og))eos = 1;
                    }
                }
                if (!eos) {
                    buffer = ogg_sync_buffer(&oy, 4096);
                    bytes = inStream.ReadBytesMax((uint8_t*)buffer,4096);
                    ogg_sync_wrote(&oy, bytes);
                    if (bytes == 0)eos = 1;
                }
            }

            /* ogg_page and ogg_packet structs always point to storage in
               libvorbis.  They're never freed or manipulated directly */

            vorbis_block_clear(&vb);
            vorbis_dsp_clear(&vd);
        }
        else {
            LogError("Error: Corrupt header during playback initialization.");
        }

        /* clean up this logical bitstream; before exit we see if we're
           followed by another [chained] */

        ogg_stream_clear(&os);
        vorbis_comment_clear(&vc);
        vorbis_info_clear(&vi);  /* must be called last */
    }

    /* OK, clean up the framer */
    ogg_sync_clear(&oy);

    LogDebug("Decoded Vorbis: %d bytes -> %d bytes", inStream.GetSize(), outStream.GetSize());
}


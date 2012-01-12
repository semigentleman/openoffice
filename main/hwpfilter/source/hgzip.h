/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



/* $Id: hgzip.h,v 1.5 2008-06-04 09:57:59 vg Exp $ */

#ifndef _HWPGZIP_H_
#define _HWPGZIP_H_

// DVO: add zlib/ prefix
#ifdef SYSTEM_ZLIB
#include <zlib.h>
#else
#include "zlib/zlib.h"
#endif

class HStream;
/**
 * @short Structure for using z_stream
 */
struct gz_stream
{
    z_stream stream;
/**
 * The error code of z_stream operation
 */
    int      z_err;
/**
 * EOF of the input file
 */
    int      z_eof;
/**
 * Stream
 */
    HStream*     _inputstream;
/**
 * Input buffer
 */
    Byte     *inbuf;
/**
 * Output buffer
 */
    Byte     *outbuf;
/**
 * Crc32 of uncompressed data
 */
    uLong    crc;
/**
 * Stream
 */
    char     *msg;
/**
 * It becomes one when the input file type is not gz file
 */
    int      transparent;
/**
 * 'w' or 'r'
 */
    char     mode;
};

/**
 *  Opens a gzipped stream for reading.
 *    gz_open returns NULL if the stream could not be opened or if there was
 *  insufficient memory to allocate the (de)compression state; errno
 *  can be checked to distinguish the two cases (if errno is zero, the
 *  zlib error is Z_MEM_ERROR).
 * @param _stream Reference of stream object having binary data.
 */
gz_stream   *gz_open    ( HStream& _stream );
/**
 * Flushes all pending output if necessary, closes the compressed stream
 * and deallocates all the (de)compression state
 */
int     gz_close    ( gz_stream *file );
/**
 * Reads the given number of uncompressed bytes from the compressed stream
 * @param file Gzipped stream
 * @param buf Buffer to have the data to be read
 * @param len Length of data to be read
 * @returns The number of bytes actually read
 */
int     gz_read     ( gz_stream *file, voidp  buf, unsigned len );
/**
 * Flushes all pending output into the compressed file
 * gz_flush should be called only when strictly necessary because it can
 * degrade compression
 * @param flush Is as in the deflate() function
 */
int     gz_flush    ( gz_stream *file, int flush );
#endif                                            /* _HWPGZIP_H_ */

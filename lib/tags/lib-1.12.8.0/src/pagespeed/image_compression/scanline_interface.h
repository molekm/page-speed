/**
 * Copyright 2011 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Author: Satyanarayana Manyam

#ifndef PAGESPEED_IMAGE_COMPRESSION_SCANLINE_INTERFACE_H_
#define PAGESPEED_IMAGE_COMPRESSION_SCANLINE_INTERFACE_H_

#include "base/basictypes.h"

namespace pagespeed {

namespace image_compression {

enum PixelFormat {
  UNSUPPORTED,  // Indicates not supported for reading the image.
  RGB_888,      // RGB triplets, 8 bits per pixel
  GRAY_8        // Grayscale, 8 bits per pixel
};

class ScanlineReaderInterface {
 public:
  ScanlineReaderInterface() {}
  virtual ~ScanlineReaderInterface() {}

  // Reset the scanline reader to its initial state.
  virtual void Reset() = 0;

  // Returns number of bytes that required to store a scanline.
  virtual size_t GetBytesPerScanline() = 0;

  // Returns true if there are more scanlines to read.
  virtual bool HasMoreScanLines() = 0;

  // Reads the next available scanline. Returns false if the
  // scan fails.
  virtual bool ReadNextScanline(void** out_scanline_bytes) = 0;

  // Returns the height of the image.
  virtual size_t GetImageHeight() = 0;

  // Returns the width of the image.
  virtual size_t GetImageWidth() = 0;

  // Returns the pixel format that need to be used by writer.
  virtual PixelFormat GetPixelFormat() = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(ScanlineReaderInterface);
};

class ScanlineWriterInterface {
 public:
  ScanlineWriterInterface() {}
  virtual ~ScanlineWriterInterface() {}

  // Initialize the basic parameter for writing the image.
  virtual bool Init(const size_t width, const size_t height,
                    PixelFormat pixel_format) = 0;

  // Writes the current scan line with data provided. Returns false
  // if the write fails.
  virtual bool WriteNextScanline(void *scanline_bytes) = 0;

  // Finalizes write structure once all scanlines are written.
  virtual bool FinalizeWrite() = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(ScanlineWriterInterface);
};

}  // namespace image_compression

}  // namespace pagespeed

#endif  // PAGESPEED_IMAGE_COMPRESSION_SCANLINE_INTERFACE_H_

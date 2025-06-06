/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* Based on code from eos https://github.com/DrMcCoy/xoreos/
 * relicensed under GPLv2+ with permission from DrMcCoy and clone2727
 */

/*
 * TGA decoder used in engines:
 *	- titanic
 *  - wintermute
 *  - zvision
 */

#ifndef IMAGE_TGA_H
#define IMAGE_TGA_H

#include "graphics/palette.h"
#include "graphics/surface.h"
#include "image/image_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Image {

/**
 * @defgroup image_tga TGA (TARGA) decoder
 * @ingroup image
 *
 * @brief Decoder for TGA images.
 * @{
 */

/** TarGa image-decoder
 * The following variations of TGA are supported:
 * - Type 1 - Color-mapped images in 16/24/32 bpp with 8 bit indexes
 * - Type 2 - 16/24/32 bpp Top AND Bottom origined.
 * - Type 3 - Black/White images, 8bpp.
 * - Type 9 - RLE-encoded color-mapped images. (8 bit indexes only)
 * - Type 10 - RLE-encoded TrueColor, 24/32bpp.
 * - Type 11 - RLE-encoded Black/White, 8bpp.
 *
 * No images are returned with a palette, instead they are converted
 * to 16 bpp for Type 1, or 32 bpp for Black/White-images.
 */
class TGADecoder : public ImageDecoder {
public:
	TGADecoder();
	virtual ~TGADecoder();
	virtual void destroy() override;
	const Graphics::Surface *getSurface() const override { return &_surface; }
	const Graphics::Palette &getPalette() const override { return _colorMap; }
	virtual bool loadStream(Common::SeekableReadStream &stream) override;
private:
	// Format-spec from:
	//http://www.ludorg.net/amnesia/TGA_File_Format_Spec.html
	enum {
	    TYPE_CMAP = 1,
	    TYPE_TRUECOLOR = 2,
	    TYPE_BW = 3,
	    TYPE_RLE_CMAP = 9,
	    TYPE_RLE_TRUECOLOR = 10,
	    TYPE_RLE_BW = 11
	};

	// Color-map:
	bool _colorMapSize;
	Graphics::Palette _colorMap;
	int16 _colorMapOrigin;
	int16 _colorMapLength;
	byte _colorMapEntryLength;

	// Origin may be at the top, or bottom
	bool _originTop;

	Graphics::PixelFormat _format;
	Graphics::Surface _surface;
	// Loading helpers
	bool readHeader(Common::SeekableReadStream &tga, byte &imageType, byte &pixelDepth);
	bool readData(Common::SeekableReadStream &tga, byte imageType, byte pixelDepth);
	bool readDataColorMapped(Common::SeekableReadStream &tga, byte imageType, byte indexDepth);
	bool readDataRLE(Common::SeekableReadStream &tga, byte imageType, byte pixelDepth);
	bool readColorMap(Common::SeekableReadStream &tga, byte imageType, byte pixelDepth);
};
/** @} */
} // End of namespace Image

#endif

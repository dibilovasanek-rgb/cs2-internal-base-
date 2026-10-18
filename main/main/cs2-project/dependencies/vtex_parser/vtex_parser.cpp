#include "vtex_parser.h"
#include <vector>
#include <fstream>
#include <span>
#define LZ4_DISABLE_DEPRECATE_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "../lz4/lz4.h"
#include "../../sdk/game_class/interfaces/steam/steamtypes.h"
#include "../../core/interfaces/interface.h"
#include "../../sdk/game_class/interfaces/c_base_file_system.h"
#include "../../dependencies/imgui/stb_image.h"

int MipLevelSize(int size, std::uint32_t level)
{
	size >>= (int)level;

	return max(size, 1);	
}

int CalculateBufferSizeForMipLevel(std::uint32_t mipLevel, std::uint32_t width, std::uint32_t height)
{
	auto bytesPerPixel = 4;
	auto width_ = MipLevelSize(width, mipLevel);
	auto height_ = MipLevelSize(height, mipLevel);

	return width_ * height_ * 1 * bytesPerPixel;
}

int SkipMipmaps(int desiredMipLevel, int num_mip_levels, int width, int height, int* compressed_mips)
{
	if (num_mip_levels < 2)
		return 0;

	int size_ = 0;

	for (int j = num_mip_levels - 1; j > desiredMipLevel; j--)
	{
		int size = CalculateBufferSizeForMipLevel(j, width, height);

		if (compressed_mips)
		{
			int compressedSize = compressed_mips[j];

			if (size > compressedSize)
			{
				size = compressedSize;
			}
		}


		size_ += size;
	}

	return size_;
}

VTexParser::ImageData VTexParser::Load(std::string_view filename) {

	auto handle = g_interface->m_file_system->Open(filename.data(), ("r"), ("GAME"));

	auto size = g_interface->m_file_system->Size(handle);
	std::vector<char> image(size);
	g_interface->m_file_system->Read(image.data(), size, handle);
	g_interface->m_file_system->Close(handle);

	if (image.empty())
		return {};

	char* blockPos = nullptr;

	int hdrPos = 0;
	for (int i = 0; i < image.size(); i++) {
		if (*(uint32_t*)(image.data() + i) == *(uint32_t*)("DATA")) {
			hdrPos = *(uint32_t*)(image.data() + i + 4) + i + 4;
			break;
		}
	}

	memstream s((char*)image.data());
	s.pos = hdrPos;

	VTexHeader hdr;
	s >> hdr;

	std::vector<int32> CompressedMips;

	auto ActualWidth = hdr.width,
		ActualHeight = hdr.height;

	bool is_compessed = false;

	std::uint32_t mips = 0;

	int extraDataSize = 0;
	if (hdr.extra_data_count > 0)
	{
		s.seekg(hdr.extra_data_offset - 8); // 8 is 2 uint32s we just read

		for (auto i = 0; i < hdr.extra_data_count; i++)
		{
			VTexExtraData type{};
			auto offset = 0, size = 0;
			s >> type >> offset >> size;
			offset -= 8;

			extraDataSize += size;

			auto prevOffset = s.pos;

			s.pos += offset;

			if (type == VTexExtraData::METADATA)
			{
				s.pos += 2;
				uint16_t nw, nh;
				s >> nw >> nh;
				if (nw > 0 && nh > 0 && ActualWidth >= nw && ActualHeight >= nh)
				{
					ActualWidth = nw;
					ActualHeight = nh;
				}
			}
			else if (type == VTexExtraData::COMPRESSED_MIP_SIZE)
			{
				std::uint32_t int1, mipsoffset;
				s >> int1 >> mipsoffset >> mips;

				is_compessed = int1 == 1; // TODO: Verify whether this int is the one that actually controls compression

				CompressedMips.resize(mips);

				s.pos += mipsoffset - 8;

				for (int mip = 0; mip < mips; mip++)
				{
					s >> CompressedMips[mip];
				}

			}

			s.pos = prevOffset;
		}
	}

	s.pos += extraDataSize;
	s.pos += SkipMipmaps(0, mips, ActualWidth, ActualWidth, CompressedMips.data());

	auto inputData = s.pos + s.data;
	auto dataSize = image.size() - s.pos;

	ImageData ret;
	ret.w = ActualWidth;
	ret.h = ActualHeight;

	using enum VTexFormat;
	switch (hdr.format) {
	case RGBA8:
	case BGRA8:
	{
		if (!is_compessed)
			return ret;

		auto uncompressed_size = CalculateBufferSizeForMipLevel(0, ret.w, ret.h);

		s.seekg(8);

		auto raw_data = s.pos + s.data;

		auto compressedSize = CompressedMips[0];

		std::span<char> buffer(raw_data, compressedSize);

		auto span = std::as_const(buffer).subspan(0, compressedSize);

		s.read(span.data(), compressedSize);

		ret.data.resize(uncompressed_size);

		LZ4_decompress_safe(span.data(), (char*)ret.data.data(), compressedSize, uncompressed_size);

		if (hdr.format == VTexFormat::BGRA8)
		{
			for (int i = 0; i < uncompressed_size; i += 4) {
				std::swap(ret.data[i], ret.data[i + 2]);
			}
		}

		break;
	}
	case PNG_RGBA8: {
		int x{}, y{}, comp{};

		stbi_set_flip_vertically_on_load(0);
		stbi_set_flip_vertically_on_load_thread(0);
		auto png = stbi_load_from_memory((stbi_uc*)inputData, dataSize, &x, &y, &comp, 4);

		ret.data.resize(x * y * 4);
		memcpy(ret.data.data(), png, x * y * 4);

		stbi_image_free(png);
		break;
	}
	case DXT5:
	{
		auto outSize = ActualWidth * ActualHeight * 4;
		ret.data.resize(outSize);

		DXT5Decoder((uint8_t*)inputData, ActualWidth, ActualHeight, hdr.width, hdr.height).Decode((uint8_t*)ret.data.data());
		// BGRA shenanigans... again?
		for (int i = 0; i < outSize; i += 4) {
			std::swap(ret.data.data()[i], ret.data.data()[i + 2]);
		}
	}
	}

	return ret;
}

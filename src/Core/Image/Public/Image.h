#pragma once
#include <vector>
#include "Log/Public/Log.h"
#include "Structs/Public/DataStructures.h"
#include "Serializer/Public/DataReader.h"

namespace ASE
{
	enum class PixelType : uint8_t
	{
		RGB = 1,
		RGBA = 2,
		Greyscale = 3,
		Indexed = 4
	};

	struct ImageSpecification
	{

	public:
		ImageSpecification() = default;
		ImageSpecification(const  ImageSpecification&) = default;

		ImageSpecification(uint32_t width, uint32_t height, int channels, uint8_t type, AsepriteFileData Data)
			:Width(width), Height(height), Channels(channels), Type((PixelType)type), Size({ (int32_t)width,(int32_t)height }), FileData(Data)
		{
			switch (Type)
			{
			case PixelType::RGBA:
			{
				PixelsPerByte = 0;
				break;
			}
			case PixelType::RGB:
			{
				PixelsPerByte = 0;
				break;
			}
			case PixelType::Greyscale:
			{
				CoreLogger::Warn("There is currently no implementation for textures with 2 channels!");
				PixelsPerByte = 0;
			}
			default:
			{
				CoreLogger::Error("Pixel Type is invalid for making images!");
				break;
			}

			}
		};

		//Use this constructor if you aren't sure how many channels there should be
		ImageSpecification(uint32_t width, uint32_t height, uint8_t type, AsepriteFileData Data)
			:Width(width), Height(height), Type((PixelType)type), Size({ (int32_t)width,(int32_t)height }), FileData(Data)
		{
			switch (Type)
			{
			case PixelType::RGBA:
			{
				Channels = 4;
				PixelsPerByte = 0;
				break;
			}
			case PixelType::RGB:
			{
				Channels = 3;
				PixelsPerByte = 0;
				break;
			}
			case PixelType::Greyscale:
			{
				CoreLogger::Warn("There is currently no implementation for textures with 2 channels!");
				Channels = 2;
				PixelsPerByte = 0;
			}
			default:
			{
				CoreLogger::Error("Pixel Type is invalid for making images!");
				break;
			}

			}

			Bounds = { 0,0, Size.Width, Size.Height };
		};


		~ImageSpecification() = default;

		std::pair<uint32_t, uint32_t> GetWidthHeight() { return { Width,Height }; }
		uint32_t GetWidth() const { return Width; }
		uint32_t GetWidth() { return Width; }
		uint32_t GetHeight() { return Height; }
		uint32_t GetHeight() const { return Height; }
		int GetChannels() { return Channels; }
		PixelType GetPixelType() { return Type; }
		PixelType GetPixelType() const { return Type; }
		AGERect& GetBounds() { return Bounds; }
		AGESize& GetSize() { return Size; }
		int GetPixelsPerByte() { return PixelsPerByte; }
		uint32_t GetWidthBytes()
		{
			uint32_t bpp;

			switch (Type)
			{
			case PixelType::RGBA:
			{
				bpp = 32;
				break;
			}
			case PixelType::RGB:
			{
				bpp = 24;
				break;
			}
			case PixelType::Greyscale:
			{
				bpp = 8;
				break;
			}
			case PixelType::Indexed:
			{
				bpp = 8;
				break;
			}
			default:
			{
				CoreLogger::Warn("Pixel Type Not Set for Image Specification... Using default 32 bits per pixel (RGBA)");
				bpp = 32;
				break;
			}
			}

			return Width * bpp;
		}

		AsepriteFileData& GetFileData() { return FileData; }


		void SetFileData(const AsepriteFileData& Data) { FileData = Data; }
		void SetWidthHeight(const std::pair<uint32_t, uint32_t>& WidthHeight) { Width = WidthHeight.first; Height = WidthHeight.second; }
		void SetWidth(const uint32_t width) { Width = width; }
		void SetHeight(const uint32_t height) { Height = height; }
		void SetChannels(int channels) { Channels = channels; }
		void SetPixelType(const PixelType& type) { Type = type; }
		void SetSize(const AGESize& size) { Size = Size; }
		void SetBounds(const AGERect& bounds) { Bounds = bounds; }


	private:
		uint32_t Width = 0;
		uint32_t Height = 0;
		int Channels = 4;
		PixelType Type = PixelType::RGBA;
		AsepriteFileData FileData;
		AGESize Size = { 0,0 };
		AGERect Bounds = { 0,0,0,0 };
		int PixelsPerByte = 0;

	};

	class Image
	{
	public:
		Image() = default;
		Image(ImageSpecification& Spec, bool FlipVerticallyOnLoad = false)
			:m_Spec(Spec), bShouldFlip(FlipVerticallyOnLoad)
		{
			size_t ForRows;
			size_t NumOfPixels;
			size_t Size;

			switch (Spec.GetPixelType())
			{
			case PixelType::RGBA:
			case PixelType::RGB:
			{
				//https://github.com/aseprite/aseprite/blob/main/src/doc/image_impl.h

				m_RowBytes = sizeof(uint32_t) * m_Spec.GetWidth();
				size_t ForRows = sizeof(uint32_t*) * Spec.GetHeight();
				size_t Size = ForRows + m_RowBytes * Spec.GetHeight();
				m_ByteSize = Size;

				ResizeImage(Spec.GetPixelType(), Size);


				std::fill(m_Buffer, m_Buffer + (Size), 0);

				m_RGBRows = (uint32_t**)m_Buffer;
				m_RGBBits = (uint32_t*)(m_Buffer + ForRows);


				auto Addr = m_RGBBits;
				for (int y = 0; y < Spec.GetHeight(); ++y)
				{
					m_RGBRows[y] = Addr;
					Addr = (uint32_t*)(((uint8_t*)Addr) + m_RowBytes);
				}
				break;
			}
			case PixelType::Greyscale:
			{
				//https://github.com/aseprite/aseprite/blob/main/src/doc/image_impl.h

				m_RowBytes = sizeof(uint16_t) * m_Spec.GetWidth();
				size_t ForRows = sizeof(uint16_t*) * Spec.GetHeight();
				size_t Size = ForRows + m_RowBytes * Spec.GetHeight();
				m_ByteSize = Size;

				ResizeImage(Spec.GetPixelType(), Size);


				std::fill(m_Buffer, m_Buffer + Size, 0);

				m_GSRows = (uint16_t**)m_Buffer;
				m_GSBits = (uint16_t*)(m_Buffer + ForRows);


				auto Addr = m_GSBits;
				for (int y = 0; y < Spec.GetHeight(); ++y)
				{
					m_GSRows[y] = Addr;
					Addr = (uint16_t*)(((uint8_t*)Addr) + m_RowBytes);
				}
				break;
			}
			default:
			{
				CoreLogger::Warn("Unable to make Image from this PixelType");
				break;
			}
			}

			for (auto& F : m_Spec.GetFileData().Frames)
			{
				for (auto& L : F.Layers)
				{
					for (auto& C : L.CelChunks)
					{
						for (auto& PD : C.PixelDatas)
						{
							InflateChunk(PD.Pixels, C.x, C.y);
						}
					}
				}
			}
		}
		Image(const Image& Other) = default;
		Image(const Image&& Other) noexcept
		{
			m_Spec = Other.m_Spec;
			m_RowBytes = Other.m_RowBytes;
			m_ByteSize = Other.m_ByteSize;
			m_Buffer = Other.m_Buffer;
			m_RGBImage = Other.m_RGBImage;
			m_GSImage = Other.m_GSImage;
			m_RGBRows = Other.m_RGBRows;
			m_RGBBits = Other.m_RGBBits;
			m_GSRows = Other.m_GSRows;
			m_GSBits = Other.m_GSBits;
		};

		virtual ~Image() = default;

		template<typename T>
		std::vector<T>& GetPixelData()
		{
			if (std::is_same<T, uint32_t>::value)
			{
				return m_RGBPixelData;
			}

			if (std::is_same<T, uint16_t>::value)
			{
				return m_GSPixelData;
			}
		}
		ImageSpecification& GetImageSpec() { return m_Spec; }
		const ImageSpecification& GetImageSpec() const { return m_Spec; }
		uint32_t* GetImageBuffer() { return m_RGBImage; }
		const uint32_t* GetImageBuffer() const { return m_RGBImage; }

		size_t GetImageByteSize()
		{
			return m_ByteSize;
		}

		void SetImageSpec(const ImageSpecification& Spec) { m_Spec = Spec; }
		template<typename T>

		void SetPixelData(const std::vector<T>& Data)
		{
			if (std::is_same<T, uint32_t>::value)
			{
				m_RGBPixelData = Data;
			}

			if (std::is_same<T, uint16_t>::value)
			{
				m_GSPixelData = Data;
			}
		}

		template<typename T>
		T GetPixel(int x, int y)
		{
			if (std::is_same<T, uint32_t>::value)
			{
				return GetRGBAddress(x, y);
			}

			if (std::is_same<T, uint16_t>::value)
			{
				return GetGSAddress(x, y);
			}
		}

	private:

		void DrawHorizontalLine(int x1, int y, int x2, Vector4 Color)
		{
			uint32_t* Start = GetRGBAddress(x1, y);
			int Width = x2 - x1 + 1;

			std::fill(Start, Start + Width, (uint32_t)Color);
		}

		void FillRect(int x1, int y1, int x2, int y2, Vector4 Color)
		{
			DrawHorizontalLine(x1, y1, x2, Color);

			uint32_t* FirstPixel = GetRGBAddress(x1, y1);
			int Width = x2 - x1 + 1;

			for (int y = y1; y <= y2; ++y)
			{
				std::copy(FirstPixel, FirstPixel + Width, GetRGBAddress(x1, y));
			}
		}

		void BlendRect(int x1, int y1, int x2, int y2, Vector4 Color, int Alpha)
		{
			FillRect(x1, y1, x2, y2, Color);
		}

		void SetPixel(int x, int y, uint32_t Data)
		{
			*GetRGBAddress(x, y) = Data;
		}

		void ClearImage(Vector4 Color)
		{
			for (int y = 0; y < m_Spec.GetHeight(); ++y)
			{
				uint32_t* First = GetRGBAddress(0, y);
				std::fill(First, First + m_Spec.GetWidth(), (uint32_t)Color);
			}
		}

		uint32_t* GetRGBLineAddress(int y)
		{
			return m_RGBRows[y];
		}

		uint16_t* GetGSLineAddress(int y)
		{
			return m_GSRows[y];
		}


		uint32_t* GetRGBAddress(int x, int y)
		{
			return GetRGBLineAddress(y) + x;
		}

		uint16_t* GetGSAddress(int x, int y)
		{
			return GetGSLineAddress(y) + x;
		}

		template<typename T>
		T* ReadImage(T* Addr, uint32_t Width, T** Buffer)
		{
			T* Img;

			if (bShouldFlip)
			{
				Img = new T[(m_Spec.GetWidth() * m_Spec.GetHeight()) * 2];
				for (int i = m_Spec.GetHeight() - 1; i >= 0; --i)
				{
					const T* Begin = Buffer[i];
					const T* End = Begin + m_Spec.GetWidth();

					std::copy(Begin, End, Img + m_Spec.GetWidth() * ((m_Spec.GetHeight() - 1) - i));

				}

				return Img;

			}
			else
			{
				Img = new T[(m_Spec.GetWidth() * m_Spec.GetHeight()) * 2];

				for (uint32_t i = 0; i < m_Spec.GetHeight(); ++i)
				{
					const T* Begin = Buffer[i];
					const T* End = Begin + m_Spec.GetWidth();

					std::copy(Begin, End, Img + m_Spec.GetWidth() * i);
				}

				return Img;

			}
		}

		void ResizeImage(PixelType Type, size_t BufferSize)
		{
			switch (Type)
			{
			case PixelType::RGBA:
			case PixelType::RGB:
			{
				m_RGBImage = new uint32_t[(m_Spec.GetWidth() * m_Spec.GetHeight())];
				break;
			}
			case PixelType::Greyscale:
			{
				m_GSImage = new uint16_t[m_Spec.GetWidth() * m_Spec.GetHeight()];
				break;
			}
			}
			m_Buffer = new uint8_t[BufferSize];
		}

		void InflateChunk(std::vector<uint8_t>& Data, int x, int y)
		{
			//https://github.com/aseprite/aseprite/blob/8e91d22b704d6d1e95e1482544318cee9f166c4d/src/doc/image_io.cpp

			MemoryStreamReader Stream(Data.data(), Data.size());
			int AvailBytes = Data.size();

			z_stream ZStream;
			ZStream.zalloc = (alloc_func)0;
			ZStream.zfree = (free_func)0;
			ZStream.opaque = (voidpf)0;

			int err = inflateInit(&ZStream);
			if (err != Z_OK)
			{
				CoreLogger::Error("Error in inflateInit()");
			}

			int Remain = AvailBytes;
			std::vector<uint8_t> compressed(4096);

			int Y = 0;
			uint8_t* Addr = (uint8_t*)GetRGBAddress(0, 0);
			uint8_t* AddrEnd = (uint8_t*)GetRGBAddress(0, 0) + m_Spec.GetHeight() * m_RowBytes;
			int uncompressed_offset = 0;

			while (Remain > 0)
			{
				int Len = std::min(Remain, int(compressed.size()));


				uint64_t StartPositon = Stream.GetStreamPosition();
				Stream.ReadBytes(&compressed[0], Len);
				if (!Stream.IsStreamGood())
				{
					CoreLogger::Error("Error Reading Aseprite Image Data!");
				}

				size_t bytesRead = Stream.GetStreamPosition() - StartPositon;
				if (bytesRead == 0)
				{
					break;
				}

				Remain -= bytesRead;

				ZStream.next_in = (Bytef*)&compressed[0];
				ZStream.avail_in = (uInt)bytesRead;

				do
				{

					//if (Addr == AddrEnd)
					//{
					//	if (Y < m_Spec.GetHeight())
					//	{
					//		Addr = (uint8_t*)GetRGBAddress(0, Y++);
					//		AddrEnd = Addr + m_Spec.GetWidthBytes();
					//	}
					//}
					ZStream.next_out = (Bytef*)Addr;
					ZStream.avail_out = AddrEnd - Addr;

					err = inflate(&ZStream, Z_NO_FLUSH);
					if (err != Z_OK && err != Z_STREAM_END && err != Z_BUF_ERROR)
					{
						CoreLogger::Error("Error in inflate");
						CoreLogger::Error("\tError:{}", err);
					}


					int uncompressed_bytes = (int)((AddrEnd - Addr) - ZStream.avail_out);
					if (uncompressed_bytes > 0)
					{
						if (uncompressed_offset + uncompressed_bytes > m_Spec.GetHeight() * m_RowBytes)
						{
							throw std::exception("Bad compressed image.");
						}
						uncompressed_offset += uncompressed_bytes;
						Addr += uncompressed_bytes;
					}

				} while (ZStream.avail_in != 0 && ZStream.avail_out == 0);
			}


			err = inflateEnd(&ZStream);

			if (err != Z_OK)
			{
				CoreLogger::Error("Zlib Error in inflateEnd()");
				CoreLogger::Error("\t {}", ZStream.msg);
			}
		}


		template<typename T>
		inline bool IsSameColor(const T A, const T B)
		{
			if (((A >> 24) & 0xff) == 0)
			{
				if (((B >> 24) & 0xff) == 0)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else if (((B >> 24) & 0xff) == 0)
			{
				return false;
			}
			else
			{
				return A == B;
			}
		}

		// LAF Base Library
		// Copyright (c) 2001-2016 David Capello
		//
		// This file is released under the terms of the MIT license.
		// Read LICENSE.txt for more information.
		//Copying the implementation from laf, since I don't need the entire library as of yet

#ifdef __cplusplus
#ifdef __STDCPP_DEFAULT_NEW_ALIGNMENT__
		static constexpr size_t BaseAlignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__;
#else
		static constexpr size_t BaseAlignment = 1;
#endif

		constexpr size_t AlignSize(const size_t N, const size_t Alignment = BaseAlignment)
		{
			size_t Remaining = (N % Alignment);
			size_t Aligned_N = N + (Remaining ? (Alignment - Remaining) : 0);

			if (Aligned_N > Alignment)
			{
				return Aligned_N;
			}
			return Alignment;
		}
#endif

	private:

		//This is our stride
		size_t m_RowBytes = 0;
		size_t m_ByteSize = 0;


		uint8_t* m_Buffer;

		uint32_t* m_RGBImage;
		uint16_t* m_GSImage;

		uint32_t** m_RGBRows;
		uint32_t* m_RGBBits;

		uint16_t** m_GSRows;
		uint16_t* m_GSBits;

		ImageSpecification m_Spec;

		bool bShouldFlip = false;


	public:
		Image& Image::operator=(const Image& Other)
		{
			m_Spec = Other.m_Spec;
			m_RowBytes = Other.m_RowBytes;
			m_ByteSize = Other.m_ByteSize;
			m_Buffer = Other.m_Buffer;
			m_RGBImage = Other.m_RGBImage;
			m_GSImage = Other.m_GSImage;
			m_RGBRows = Other.m_RGBRows;
			m_RGBBits = Other.m_RGBBits;
			m_GSRows = Other.m_GSRows;
			m_GSBits = Other.m_GSBits;


			return *this;
		}
	};
}
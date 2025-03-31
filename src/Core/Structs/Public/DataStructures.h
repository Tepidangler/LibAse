#pragma once
#include <zlib.h>
#include <cstddef>
#include <variant>
#include <map>
#include <cmath>
#include <immintrin.h>


namespace ASE
{
	struct Vector4
	{
	public:

		float x, y, z, w;

		Vector4()
		{
			x = 0;
			y = 0;
			z = 0;
			w = 0;
		}
		Vector4(float a)
		{
			x = a;
			y = a;
			z = a;
			w = a;
		}
		Vector4(float a, float b, float c, float d)
		{
			x = a;
			y = b;
			z = c;
			w = d;
		}
		Vector4(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
		{
			std::byte tmpbyte = std::byte(a);
			int tmpint = std::to_integer<int>(tmpbyte);
			x = ((100.f * (float)tmpint) / 255.f) * .01f;

			tmpbyte = std::byte(b);
			tmpint = std::to_integer<int>(tmpbyte);
			y = ((100.f * (float)tmpint) / 255.f) * .01f;

			tmpbyte = std::byte(c);
			tmpint = std::to_integer<int>(tmpbyte);
			z = ((100.f * (float)tmpint) / 255.f) * .01f;

			tmpbyte = std::byte(d);
			tmpint = std::to_integer<int>(tmpbyte);

			w = ((100.f * (float)tmpint) / 255.f) * .01f;
		}

		//https://stackoverflow.com/questions/22244629/efficient-way-to-convert-from-premultiplied-float-rgba-to-8-bit-rgba
		operator uint32_t()
		{
			double rgb[4] = { x,y,z, 0 };
			__m128 alpha = _mm_set1_ps(w);
			__m128i* converted = new __m128i();

			__m128 tmp1 = _mm256_cvtpd_ps(_mm256_load_pd(&rgb[0]));

			__m128 fact = _mm_set1_ps(w > 0 ? 255.f / w : 0);

			tmp1 = _mm_mul_ps(fact, tmp1); //rbg0
			alpha = _mm_mul_ps(_mm_set1_ps(255.f), _mm_set1_ps(w)); //alpha
			tmp1 = _mm_insert_ps(tmp1, alpha, _MM_MK_INSERTPS_NDX(1, 3, 0x00000400));

			__m128i tmp1i = _mm_cvtps_epi32(tmp1);

			_mm_store_si128((__m128i*)converted, tmp1i);
			uint32_t out = converted->m128i_u32[0] | (converted->m128i_u32[1] << 8) | (converted->m128i_u32[2] << 16) | (converted->m128i_u32[3] << 24);
			return out;
		}

		operator uint32_t* ()
		{
			double rgb[4] = { x,y,z, 0 };
			__m128 alpha = _mm_set1_ps(w);
			uint32_t out[4];

			__m128 tmp1 = _mm256_cvtpd_ps(_mm256_load_pd(&rgb[0]));

			__m128 fact = _mm_set1_ps(w > 0 ? 255.f / w : 0);

			tmp1 = _mm_mul_ps(fact, tmp1); //rbg0
			alpha = _mm_mul_ps(_mm_set1_ps(255.f), _mm_set1_ps(w)); //alpha
			tmp1 = _mm_insert_ps(tmp1, alpha, _MM_MK_INSERTPS_NDX(1, 3, 0x00000400));

			__m128i tmp1i = _mm_cvtps_epi32(tmp1);

			_mm_store_si128((__m128i*)out, tmp1i);

			return out;
		}

		float& Vector4::operator [](int i)
		{
			return ((&x)[i]);
		}
		const float& Vector4::operator [](int i) const
		{
			return ((&x)[i]);
		}

		Vector4 Vector4::operator+(const Vector4& vec) const {
			return Vector4(x + vec.x, y + vec.y, z + vec.z, vec.w);
		}

		void Vector4::operator+=(const Vector4& vec) {
			x += vec.x;
			y += vec.y;
			z += vec.z;
			w += vec.w;
		}

		Vector4 Vector4::operator-(const Vector4& vec) const {
			return Vector4(x - vec.x, y - vec.y, z - vec.z, w - vec.w);
		}

		void Vector4::operator-=(const Vector4& vec) {
			x -= vec.x;
			y -= vec.y;
			z -= vec.z;
			w -= vec.w;
		}

		Vector4 Vector4::operator*(float scalar) const {
			return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
		}

		void Vector4::operator*=(float scalar) {
			x *= scalar;
			y *= scalar;
			z *= scalar;
			w *= scalar;
		}

		Vector4 Vector4::operator/(float scalar) const {
			return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
		}

		void Vector4::operator/=(float scalar) {
			x /= scalar;
			y /= scalar;
			z /= scalar;
			w /= scalar;
		}


		float dot(const Vector4& vec) const {
			float DotProduct = (x * vec.x) + (y * vec.y) + (z * vec.z) + (w * vec.w);
			return DotProduct;
		}

		float norm(const Vector4& vec) const {
			float Magnitude = sqrt(
				pow((x - vec.x), 2.f) +
				pow((y - vec.y), 2.f) +
				pow((z - vec.z), 2.f) +
				pow((w - vec.w), 2.f)
			);

			return Magnitude;
		}

		float magnitude() const {
			return norm(Vector4());
		}


		bool Vector4::operator==(const Vector4& vec) {
			return x == vec.x && y == vec.y && z == vec.z && w == vec.w;
		}

		bool Vector4::operator!=(const Vector4& vec) {
			return x != vec.x || y != vec.y || z != vec.z || w != vec.w;
		}
	};

	struct AGEPixel
	{
	public:
		uint8_t RGBA[4];
		uint32_t U32RBGA[4];

		AGEPixel() = default;

		AGEPixel(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
		{
			RGBA[0] = a;
			RGBA[1] = b;
			RGBA[2] = c;
			RGBA[3] = d;

			//Convert to Uint32_t 

			U32RBGA[0] = RGBA[0];
			U32RBGA[1] = RGBA[1];
			U32RBGA[2] = RGBA[2];
			U32RBGA[3] = RGBA[3];

		}

		AGEPixel(float a, float b, float c, float d)
		{
			RGBA[0] = a;
			RGBA[1] = b;
			RGBA[2] = c;
			RGBA[3] = d;

			float tmp[4] = { a,b,c,d };
			uint32_t* tmp1 = ConvertFloatToU32(tmp);
			//Convert to Uint32_t 

			U32RBGA[0] = tmp1[0];
			U32RBGA[1] = tmp1[1];
			U32RBGA[2] = tmp1[2];
			U32RBGA[3] = tmp1[3];

		}

		operator Bytef* ()
		{
			return (Bytef*)RGBA[0];
		}

		operator uint32_t()
		{
			return ((RGBA[0] << 0) | (RGBA[1] << 8) | (RGBA[2] << 16) | (RGBA[3] << 24));
		}

	private:
		uint32_t* ConvertFloatToU32(float* Bytes)
		{
			double rgb[4] = { Bytes[0], Bytes[1], Bytes[2], 0 };
			__m128 alpha = _mm_set1_ps(Bytes[3]);
			uint32_t out[4];

			__m128 tmp1 = _mm256_cvtpd_ps(_mm256_load_pd(&rgb[0]));

			__m128 fact = _mm_set1_ps(Bytes[3] > 0 ? 255.f / Bytes[3] : 0);

			tmp1 = _mm_mul_ps(fact, tmp1); //rbg0
			alpha = _mm_mul_ps(_mm_set1_ps(255.f), _mm_set1_ps(Bytes[3])); //alpha
			tmp1 = _mm_insert_ps(tmp1, alpha, _MM_MK_INSERTPS_NDX(1, 3, 0x00000400));

			__m128i tmp1i = _mm_cvtps_epi32(tmp1);

			_mm_store_si128((__m128i*)out, tmp1i);

			return out;
		}
	};

	struct AsepriteVariant;

	struct AGEPoint
	{
		AGEPoint() = default;
		AGEPoint(int32_t x, int32_t y)
			:X(x), Y(y) {}
		~AGEPoint() = default;
		int32_t X;
		int32_t Y;

		bool operator==(const AGEPoint& Other)
		{
			bool x = X == Other.X;
			bool y = Y == Other.Y;

			return x && y;
		}

		bool operator!=(const AGEPoint& Other)
		{
			bool x = X == Other.X;
			bool y = Y == Other.Y;

			return !x || !y;
		}

	};
	struct AGESize
	{
		AGESize() = default;
		AGESize(int32_t width, int32_t height)
			:Width(width), Height(height) {}
		~AGESize() = default;

		int32_t Width;
		int32_t Height;
	};
	struct AGERect
	{
	public:
		AGERect() = default;
		AGERect(AGEPoint Point, int32_t width, int32_t height)
			: XY(Point), Width(width), Height(height) {}

		AGERect(int32_t x, int32_t y, int32_t width, int32_t height)
			:XY({ x,y }), Width(width), Height(height) {}
		~AGERect() = default;

		AGEPoint XY;
		int32_t Width;
		int32_t Height;

		bool Contains(AGEPoint Point)
		{
			return XY == Point;
		}
	};



	enum class AsepriteChunkType : uint16_t
	{
		OldPaletteChunk1 = 0x0004,
		OldPaletteChunk2 = 0x0011,
		LayerChunk = 0x2004,
		CelChunk = 0x2005,
		CelExtraChunk = 0x2006,
		ColorProfileChunk = 0x2007,
		ExternalFilesChunk = 0x2008,
		MaskChunk = 0x2016,
		TagsChunk = 0x2018,
		NewPaletteChunk = 0x2019,
		UserDataChunk = 0x2020,
		SliceChunk = 0x2022,
		TilesetChunk = 0x2023
	};

	enum class AsepritePropertyTypes : uint16_t
	{
		Boolean = 0x0001,
		Int8 = 0x0002,
		Int16 = 0x0004,
		Uint16 = 0x0005,
		Int32 = 0x0006,
		Uint32 = 0x0007,
		Int64 = 0x0008,
		Uint64 = 0x0009,
		Fixed = 0x000A,
		Float = 0x000B,
		Double = 0x000C,
		String = 0x000D,
		Point = 0x000E,
		Size = 0x000F,
		Rect = 0x0010,
		Vector = 0x0011,
		NestedMapProps = 0x0012,
		UUID = 0x0013

	};
	using Vector = std::vector<AsepriteVariant>;

	using VariantBase = std::variant < std::nullptr_t,
		bool,
		int8_t, uint8_t,
		int16_t, uint16_t,
		int32_t, uint32_t,
		int64_t, uint64_t,
		float, double,
		std::string,
		AGEPoint, AGESize, AGERect,
		Vector, std::map<std::string, AsepriteVariant>,
		uint8_t*>;

	struct AsepriteVariant : public VariantBase
	{
		//Copied this implementation right out of the Aseprite user_data.h

		AsepriteVariant() = default;
		AsepriteVariant(const AsepriteVariant& v) = default;

		template<typename T>
		AsepriteVariant(T&& v) : VariantBase(std::forward<T>(v)) { }

		// Avoid using Variant.operator=(const char*) because the "const
		// char*" is converted to a bool implicitly by MSVC.
		AsepriteVariant& operator=(const char*) = delete;

		template<typename T>
		AsepriteVariant& operator=(T&& v) {
			VariantBase::operator=(std::forward<T>(v));
			return *this;
		}

		const uint16_t type() const {
			return index();
		}
	};

	struct AsepritePropertyData
	{
		uint16_t Type;
		bool Boolean;
		int8_t Int8;
		uint8_t Byte;
		int16_t Int16;
		uint16_t Uint16;
		int32_t Int32;
		uint32_t Uint32;
		int64_t Int64;
		uint64_t Uint64;
		double  Fixed;
		float Float;
		double Double;
		std::string String;
		AGEPoint Point;
		AGESize Size;
		AGERect Rect;
		Vector Vec;
	};



	struct AsepritePixelData
	{
		AsepriteChunkType ChunkType;
		uint16_t Flags;
		std::vector<uint8_t> Pixels;
		std::string Name;
	};

	struct AsepriteSliceKey
	{
		uint32_t FrameNumber;
		int32_t SliceX;
		int32_t SliceY;
		uint32_t SliceWidth;
		uint32_t SliceHeight;

		int32_t CenterX;
		int32_t CenterY;
		uint32_t CenterWidth;
		uint32_t CenterHeight;

		int32_t PivotX;
		int32_t PivotY;
	};

	struct AsepriteExternalFileEntry
	{
		uint32_t ID;
		uint8_t Type;
		std::string Name;
	};

	struct AsepriteTag
	{
		uint16_t FromFrame;
		uint16_t ToFrame;
		uint8_t LoopDirection;
		uint16_t RepeatTimes;

		uint8_t RGB[3]; //Deprecated in newer versions
		std::string Name;
	};

	struct AsepriteUserProps
	{
		std::string Name;
		uint16_t Type;
		uint32_t NumofElementsInVec;
		uint16_t ElementsType;
		//Probably have to do some sort of templated vector thing TODO: Look into that in the future
		AsepritePropertyData PropData;


		std::map<uint32_t, uint8_t[]> NestedPropsMap;
		uint8_t UUID[16];


	};

	struct AsepriteUserData
	{
		uint32_t Flags;
		std::string Text;
		uint8_t RGBA[4];
		uint32_t Size;
		uint32_t NumOfPropMaps;
		std::vector<std::pair<uint32_t, uint32_t>> PropMapKeyPairs;
		std::map<uint32_t, std::vector<AsepriteUserProps>> UserProps;

	};

	struct AsepriteCelChunk
	{
	public:
		AsepriteCelChunk() = default;
		~AsepriteCelChunk() = default;


		uint16_t LayerIndex;
		int16_t x;
		int16_t y;
		uint8_t Opacity;
		uint16_t CelType;
		int16_t zIndex;
		uint16_t Width;
		uint16_t Height;
		uint16_t FramePosition; // Frame position to link with
		std::vector<AsepritePixelData> PixelDatas;

		int order() const
		{
			return LayerIndex + zIndex;
		}
	};

	struct AsepriteLayer
	{
	public:
		AsepriteLayer() = default;
		AsepriteLayer(int LIndex, int ZIndex)
			:Layerindex(LIndex), zIndex(ZIndex) {}
		AsepriteLayer(const AsepriteLayer&) = default;

		int Layerindex;
		int zIndex;

		uint16_t Flags;
		uint16_t Type;
		uint16_t Child;
		uint16_t BlendMode;
		uint8_t Opacity;
		std::string Name;
		std::vector<AsepriteCelChunk> CelChunks;

	};

	struct AsepriteOldPaletteChunk
	{
		AsepriteChunkType Type;
		uint16_t NumOfPackets;
		uint8_t NumOfPalettesToSkip;
		uint8_t NumOfColors;
		std::vector<uint8_t*> Colors;

	};

	struct AsepriteColorProfileChunk
	{
		uint16_t Type;
		uint16_t Flags;
		double Gamma;
		uint32_t ICCProfileDataLength;
		std::vector<std::byte> ICCProfileData; // More info: http://www.color.org/ICC1V42.pdf

	};

	struct AsepriteExternalFilesChunk
	{
		uint32_t NumOfEntries;
		std::vector<AsepriteExternalFileEntry> Entries;

	};

	struct AsepriteMaskChunk
	{
		int16_t x;
		int16_t y;
		uint16_t Width;
		uint16_t Height;
		std::string Name;
		std::vector<std::byte> Data; //Bit Map Data
	};

	struct AsepriteTagsChunk
	{
		uint16_t NumOfTags;
		std::vector<AsepriteTag> Tags;
	};

	struct AsepritePaletteChunk
	{
		uint32_t Size;
		uint32_t FirstIndexToChange;
		uint32_t LastIndexToChange;
		std::vector<AsepritePixelData> Entries;
	};

	struct AsepriteSliceChunk
	{
		uint32_t NumOfSliceKeys;
		uint32_t Flags;
		std::string Name;
		std::vector<AsepriteSliceKey> Slices;
	};

	struct AsepriteChunk
	{
	public:
		AsepriteChunk() = default;
		AsepriteChunk(const AsepriteChunk&) = default;

		uint32_t Size;
		AsepriteChunkType Type;
		std::vector<std::byte> Data;
	};

	struct AsepriteHeader
	{
	public:
		AsepriteHeader() = default;
		AsepriteHeader(const AsepriteHeader&) = default;
		//HeaderData
		uint32_t FileSize;
		uint16_t MagicNumber = 0xA5E0;
		uint16_t Frames;
		uint16_t Width;
		uint16_t Height;
		uint16_t Depth;
		uint32_t Flags;
		uint16_t Speed; // Deprecated, but we're leaving just incase anyone is using older versions where this is still valid or needed
		uint32_t whoknows = 0;
		uint32_t whoknowsagain = 0;
		uint8_t  EntryIndex;
		//skip 3 uint8_t
		uint16_t NumOfColors;
		uint8_t PixelWidth;
		uint8_t PixelHeight;
		int16_t x;
		int16_t y;
		uint16_t GridWidth;
		uint16_t GridHeight;
	};

	struct AsepriteFrameData
	{
	public:
		AsepriteFrameData() = default;
		AsepriteFrameData(const AsepriteFrameData&) = default;
		//FrameData
		uint32_t BytesInFrame;
		uint16_t MagicNumber = 0xF1FA;
		uint16_t NumOfChunks;
		uint16_t FrameDuration;
		//two uint8_t which will be 0
		uint32_t NewNumOfChunks; // If 0 use NumOfChunks

		std::vector<AsepriteChunk> ChunkData;
		std::vector<AsepriteLayer> Layers;
		std::vector<AsepriteOldPaletteChunk> OldPaletteChunks;
		std::vector<AsepritePaletteChunk> NewPaletteChunks;
	};

	struct AsepriteFileData
	{
	public:
		AsepriteFileData() = default;
		AsepriteFileData(const AsepriteHeader& HeaderData)
			:Header(HeaderData) {}
		AsepriteFileData(const AsepriteFileData&) = default;

		AsepriteHeader Header;
		std::vector<AsepriteFrameData> Frames;

	};
	
}

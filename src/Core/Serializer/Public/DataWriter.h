#pragma once
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include <unordered_map>
#include "Log/Public/Log.h"
#include "Serializer/Public/DataReader.h"


namespace ASE
{
	struct Buffer
	{
		void* Data;
		uint64_t Size;

		Buffer()
			: Data(nullptr), Size(0)
		{

		}

		Buffer(const void* data, uint64_t size = 0)
			:Data((void*)data), Size(size)
		{

		}

		static Buffer Copy(const Buffer& Other)
		{
			Buffer buffer;
			buffer.Allocate(Other.Size);
			memcpy(buffer.Data, Other.Data, Other.Size);
			return buffer;
		}

		static Buffer Copy(const void* data, uint64_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}

		void Allocate(uint64_t size)
		{
			delete[](char*)Data;
			Data = nullptr;

			if (size == 0)
			{
				return;
			}

			Data = new char* [size];
			Size = size;
		}

		void Release()
		{
			delete[](char*) Data;
			Data = nullptr;
			Size = 0;
		}

		void ZeroInitialize()
		{
			if (Data)
			{
				memset(Data, 0, Size);
			}
		}

		template<typename T>
		T& Read(uint64_t offset = 0)
		{
			return *(T*)((char*)Data + offset);
		}

		char* ReadBytes(uint64_t size, uint64_t offset) const
		{
			ASE_CORE_ASSERT(offset + size <= Size, "Buffer Overflow!");
			char* buf = new char[size];
			memcpy(buf, (char*)Data + offset, size);
			return buf;
		}

		void Write(const void* data, uint64_t size, uint64_t offset = 0)
		{
			ASE_CORE_ASSERT(offset + size <= Size, "Buffer Overflow!");
			memcpy((char*)Data + offset, data, size);
		}

		operator bool() const
		{
			return (bool)Data;
		}

		char& operator[](int Index)
		{
			return ((char*)Data)[Index];
		}
	};

	class DataWriter
	{
	public:

		virtual ~DataWriter() = default;

		virtual bool IsStreamGood() const = 0;
		virtual uint64_t GetStreamPosition() = 0;
		virtual void SetStreamPosition(uint64_t Pos) = 0;
		virtual bool WriteData(const char* Data, size_t Size) = 0;

		operator bool() const { return IsStreamGood(); }

		void WriteBuffer(Buffer buffer, bool WriteSize = true)
		{
			if (WriteSize)
			{
				WriteData((char*)&buffer.Size, sizeof(uint64_t));
			}

			WriteData((char*)buffer.Data, buffer.Size);
		}
		void WriteZero(uint64_t Size)
		{
			char Zero = 0;
			for (uint64_t i = 0; i < Size; i++)
			{
				WriteData(&Zero, 1);
			}
		}
		void WriteString(const std::string& String)
		{
			//Lol this isn't how it works
			size_t Size = String.size();
			WriteData((char*)&Size, sizeof(size_t));
			WriteData((char*)String.data(), sizeof(char) * String.size()); //This should probably be String.length() as we should be multiplying the size of a char (1 byte) by the number of chars in the string
		}


		template<typename T>
		void WriteRaw(const T& Type)
		{
			bool Success = WriteData((char*)&Type, sizeof(T));
			AGE_GAME_ASSERT(Success, "Failed to Write Data");
		}

		template<typename T>
		void WriteObject(const T& Obj)
		{
			T::Serialize(this, Obj);
		}

		template<typename Key, typename Value>
		void WriteMap(const std::map<Key, Value>& Map, bool WriteSize = true)
		{
			if (WriteSize)
			{
				WriteRaw<uint32_t>((uint32_t)Map.size());
			}

			for (const auto& [K, V] : Map)
			{
				if constexpr (std::is_trivial<Key>())
				{
					WriteRaw<Key>(K);
				}
				else
				{
					WriteObject<Key>(K);
				}

				if constexpr (std::is_trivial<Value>())
				{
					WriteRaw<Value>(V);
				}
				else
				{
					WriteObject<Value>(V);
				}
			}
		}

		template<typename Key, typename Value>
		void WriteMap(const std::unordered_map<Key, Value>& Map, bool WriteSize = true)
		{
			if (WriteSize)
			{
				WriteRaw<uint32_t>((uint32_t)Map.size());
			}

			for (const auto& [K, V] : Map)
			{
				if constexpr (std::is_trivial<Key>())
				{
					WriteRaw<Key>(K);
				}
				else
				{
					WriteObject<Key>(K);
				}

				if constexpr (std::is_trivial<Value>())
				{
					WriteRaw<Value>(V);
				}
				else
				{
					WriteObject<Value>(V);
				}
			}
		}

		template<typename Value>
		void WriteMap(const std::unordered_map<std::string, Value>& Map, bool WriteSize = true)
		{
			if (WriteSize)
			{
				WriteRaw<uint32_t>((uint32_t)Map.size());
			}

			for (const auto& [K, V] : Map)
			{
				WriteString(K);

				if constexpr (std::is_trivial<Value>())
				{
					WriteRaw<Value>(V);
				}
				else
				{
					WriteObject<Value>(V);
				}
			}
		}

		template<typename T>
		void WriteArray(const std::vector<T>& Array, bool WriteSize = true)
		{
			if (WriteSize)
			{
				WriteRaw<uint32_t>((uint32_t)Array.size());
			}

			for (const auto& E : Array)
			{
				if constexpr (std::is_trivial<T>())
				{
					WriteRaw<T>(E);
				}
				else
				{
					WriteObject<T>(E);
				}
			}

		}
	};


	class FileStreamWriter : public DataWriter
	{
	public:

		FileStreamWriter() = default;
		FileStreamWriter(const std::filesystem::path& Path)
			:m_Path(Path)
		{
			m_Stream = std::ofstream(Path, std::ofstream::out | std::ofstream::binary);
		}
		FileStreamWriter(const FileStreamWriter&) = delete;

		virtual ~FileStreamWriter()
		{
			m_Stream.close();
		}

		bool IsStreamGood() const final { return m_Stream.good(); }
		uint64_t GetStreamPosition() final { return m_Stream.tellp(); }
		void SetStreamPosition(uint64_t Pos) final { m_Stream.seekp(Pos); }
		bool WriteData(const char* Data, size_t Size) final
		{
			m_Stream.write(Data, Size);
			return true;
		}

	private:

		std::filesystem::path m_Path;
		std::ofstream m_Stream;
	};

	class MemoryStreamWriter : public DataWriter
	{
	public:

		MemoryStreamWriter(void* Addr)
			:m_Addr(Addr)
		{
			m_Stream = std::stringstream(std::stringstream::out | std::stringstream::binary);
		}
		MemoryStreamWriter(const MemoryStreamWriter&) = delete;

		virtual ~MemoryStreamWriter()
		{
			m_Stream.clear();
		}

		bool IsStreamGood() const final { return m_Stream.good(); }
		uint64_t GetStreamPosition() final { return m_Stream.tellp(); }
		void SetStreamPosition(uint64_t Pos) final { m_Stream.seekp(Pos); }
		bool WriteData(const char* Data, size_t Size) final
		{
			m_Stream.write(Data, Size);
			return true;
		}

	private:

		void* m_Addr;
		std::stringstream m_Stream;
	};


}
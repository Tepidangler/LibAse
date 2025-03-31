#pragma once
#include <sstream>
#include <filesystem>

namespace ASE
{
	class DataReader
	{
	public:
		virtual ~DataReader() = default;

		virtual bool IsStreamGood() const = 0;
		virtual uint64_t GetStreamPosition() = 0;
		virtual void SetStreamPosition(uint64_t Pos) = 0;
		virtual bool ReadData(char* Data, size_t Size) = 0;
		virtual bool ReadBytes(std::vector<std::byte>& Data, size_t Size) = 0;
		virtual bool ReadBytes(uint8_t* Data, size_t Size) = 0;

		operator bool() const { return IsStreamGood(); }

		void ReadBuffer(char* Data, size_t Size)
		{
			uint32_t size = 0;
			if (Size == 0)
			{
				ReadRaw<uint32_t>(size);
				ReadData(Data, (size_t)size);
				return;
			}
			ReadData(Data, Size);
		}
		void ReadString(std::string& String, size_t Size)
		{
			ReadData(&String[0], Size);
		}


		template<typename T>
		void ReadRaw(T& Type)
		{
			bool Success = ReadData((char*)&Type, sizeof(T));
			ASE_CORE_ASSERT(Success, "Failed to Read Data");

		}

		template<typename T>
		void ReadObject(T& Obj)
		{
			T::Deserialize(this, Obj);
		}

		template<typename Key, typename Value>
		void ReadMap(std::map<Key, Value>& Map, uint32_t Size = 0)
		{
			if (Size == 0)
			{
				ReadRaw<uint32_t>(Size);
			}

			for (uint32_t i = 0; i < Size; i++)
			{
				Key K;
				if constexpr (std::is_trivial<Key>())
				{
					ReadRaw<Key>(K);
				}
				else
				{
					ReadObject<Key>(K);
				}

				if constexpr (std::is_trivial<Value>())
				{
					ReadRaw<Value>(Map[K]);
				}
				else
				{
					ReadObject<Value>(Map[K]);
				}
			}
		}

		template<typename Key, typename Value>
		void ReadMap(std::unordered_map<Key, Value>& Map, uint32_t Size = 0)
		{
			if (Size == 0)
			{
				ReadRaw<uint32_t>(Size);
			}

			for (uint32_t i = 0; i < Size; i++)
			{
				Key K;
				if constexpr (std::is_trivial<Key>())
				{
					ReadRaw<Key>(K);
				}
				else
				{
					ReadObject<Key>(K);
				}

				if constexpr (std::is_trivial<Value>())
				{
					ReadRaw<Value>(Map[K]);
				}
				else
				{
					ReadObject<Value>(Map[K]);
				}
			}
		}

		template<typename Value>
		void ReadMap(std::unordered_map<std::string, Value>& Map, uint32_t Size = 0)
		{
			if (Size == 0)
			{
				ReadRaw<uint32_t>(Size);
			}

			for (uint32_t i = 0; i < Size; i++)
			{
				Key K;
				if constexpr (std::is_trivial<Key>())
				{
					ReadRaw<Key>(K);
				}
				else
				{
					ReadObject<Key>(K);
				}

				if constexpr (std::is_trivial<Value>())
				{
					ReadRaw<Value>(Map[K]);
				}
				else
				{
					ReadObject<Value>(Map[K]);
				}
			}
		}

		template<typename T>
		void ReadArray(std::vector<T>& Array, uint32_t Size = 0)
		{
			if (Size == 0)
			{
				ReadRaw<uint32_t>(Size);
				Array.resize(Size);
			}
			else
			{
				Array.resize(Size);
			}

			for (uint32_t i = 0; i < Size; i++)
			{
				if constexpr (std::is_trivial<T>())
				{
					ReadRaw<T>(Array[i]);
				}
				else
				{
					ReadObject<T>(Array[i]);
				}
			}

		}

	};


	class FileStreamReader : public DataReader
	{
	public:
		FileStreamReader() = default;
		FileStreamReader(const std::filesystem::path& Path)
		{
			m_Stream = std::ifstream(Path, std::ifstream::in | std::ifstream::binary);
		}
		FileStreamReader(const FileStreamReader&) = delete;

		virtual ~FileStreamReader()
		{
			m_Stream.close();
		}

		bool IsStreamGood() const final { return m_Stream.good(); }
		uint64_t GetStreamPosition() final { return m_Stream.tellg(); }
		void SetStreamPosition(uint64_t Pos) final { m_Stream.seekg(Pos); }
		bool ReadData(char* Data, size_t Size) final
		{
			m_Stream.read(Data, Size);
			return true;
		}
		bool ReadBytes(std::vector<std::byte>& Data, size_t Size) final
		{
			m_Stream.read(reinterpret_cast<char*>(Data.data()), Size);
			return true;
		}
		bool ReadBytes(uint8_t* Data, size_t Size) final
		{
			m_Stream.read(reinterpret_cast<char*>(Data), Size);
			return true;
		}


	private:

		std::filesystem::path m_Path;
		std::ifstream m_Stream;

	};

	class MemoryStreamReader : public DataReader
	{
	public:

		MemoryStreamReader(void* Addr, size_t Size)
		{
			std::string s((char*)m_Addr, Size);
			m_Stream = std::istringstream(s);
		}
		MemoryStreamReader(const MemoryStreamReader&) = delete;

		virtual ~MemoryStreamReader()
		{
			m_Stream.clear();
		}

		bool IsStreamGood() const final { return m_Stream.good(); }
		uint64_t GetStreamPosition() final { return m_Stream.tellg(); }
		void SetStreamPosition(uint64_t Pos) final { m_Stream.seekg(Pos); }
		bool ReadData(char* Data, size_t Size) final
		{
			m_Stream.read(Data, Size);
			return true;
		}
		bool ReadBytes(std::vector<std::byte>& Data, size_t Size) final
		{
			m_Stream.read(reinterpret_cast<char*>(Data.data()), Size);
			return true;
		}
		bool ReadBytes(uint8_t* Data, size_t Size) final
		{
			m_Stream.read(reinterpret_cast<char*>(Data), Size);
			return true;
		}

	private:

		void* m_Addr;
		std::string m_String;
		std::istringstream m_Stream;
	};
}
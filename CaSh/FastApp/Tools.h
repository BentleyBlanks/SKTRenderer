#pragma once 
#include "RBMath/Inc/RBMath.h"
#include "Util.h"

#pragma region MURMURHASH
// MurmurHash2 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#define MURMUR_M 0x5bd1e995
#define MURMUR_R 24
#define mmix(_h, _k) { _k *= MURMUR_M; _k ^= _k >> MURMUR_R; _k *= MURMUR_M; _h *= MURMUR_M; _h ^= _k; }

class HashMurmur2A
{
public:
  void begin(u32 _seed = 0)
  {
    m_hash = _seed;
    m_tail = 0;
    m_count = 0;
    m_size = 0;
  }

  void add(const void* _data, int _len)
  {
    addAligned(_data, _len);
  }

  void addAligned(const void* _data, int _len)
  {
    const u8* data = (const u8*)_data;
    m_size += _len;

    mixTail(data, _len);

    while (_len >= 4)
    {
      u32 kk = *(u32*)data;

      mmix(m_hash, kk);

      data += 4;
      _len -= 4;
    }

    mixTail(data, _len);
  }

  void addUnaligned(const void* _data, int _len)
  {
    const u8* data = (const u8*)_data;
    m_size += _len;

    mixTail(data, _len);

    while (_len >= 4)
    {
      u32 kk;
      readUnaligned(data, kk);

      mmix(m_hash, kk);

      data += 4;
      _len -= 4;
    }

    mixTail(data, _len);
  }

  template<typename Ty>
  void add(Ty _value)
  {
    add(&_value, sizeof(Ty));
  }

  u32 end()
  {
    mmix(m_hash, m_tail);
    mmix(m_hash, m_size);

    m_hash ^= m_hash >> 13;
    m_hash *= MURMUR_M;
    m_hash ^= m_hash >> 15;

    return m_hash;
  }

private:
  static void readUnaligned(const void* _data, u32& _out)
  {
    const u8* data = (const u8*)_data;
    {
      _out = 0
        | data[0]
        | data[1] << 8
        | data[2] << 16
        | data[3] << 24
        ;
    }
  }

  void mixTail(const u8*& _data, int& _len)
  {
    while (_len && ((_len<4) || m_count))
    {
      m_tail |= (*_data++) << (m_count * 8);

      m_count++;
      _len--;

      if (m_count == 4)
      {
        mmix(m_hash, m_tail);
        m_tail = 0;
        m_count = 0;
      }
    }
  }

  u32 m_hash;
  u32 m_tail;
  u32 m_count;
  u32 m_size;
};

#undef MURMUR_M
#undef MURMUR_R
#undef mmix

inline u32 hashMurmur2A(const void* _data, u32 _size)
{
  HashMurmur2A murmur;
  murmur.begin();
  murmur.add(_data, (int)_size);
  return murmur.end();
}

template <typename Ty>
inline u32 hashMurmur2A(const Ty& _data)
{
  //BX_STATIC_ASSERT(BX_TYPE_IS_POD(Ty));
  return hashMurmur2A(&_data, sizeof(Ty));
}
#pragma endregion


#pragma region Memory



struct Whence
{
  enum Enum
  {
    Begin,
    Current,
    End,
  };
};

struct __declspec(novtable) ReaderI
{
  virtual ~ReaderI() = 0;
  virtual i32 read(void* _data, i32 _size) = 0;
};

inline ReaderI::~ReaderI()
{
}

struct __declspec(novtable) WriterI
{
  virtual ~WriterI() = 0;
  virtual int32 write(const void* _data, int32 _size) = 0;
};

inline WriterI::~WriterI()
{
}

struct __declspec(novtable) SeekerI
{
  virtual ~SeekerI() = 0;
  virtual int64 seek(int64 _offset = 0, Whence::Enum _whence = Whence::Current) = 0;
};

inline SeekerI::~SeekerI()
{
}

/// Read data.
inline int32 read(ReaderI* _reader, void* _data, int32 _size)
{
  return _reader->read(_data, _size);
}

/// Read value.
template<typename Ty>
inline int32 read(ReaderI* _reader, Ty& _value)
{
  return _reader->read(&_value, sizeof(Ty));
}

/// Read value and converts it to host endianess. _fromLittleEndian specifies
/// underlying stream endianess.
template<typename Ty>
inline int32 readHE(ReaderI* _reader, Ty& _value, bool _fromLittleEndian)
{
  Ty value;
  int32 result = _reader->read(&value, sizeof(Ty));
  return result;
}

/// Write data.
inline int32 write(WriterI* _writer, const void* _data, int32_t _size)
{
  return _writer->write(_data, _size);
}

/// Write repeat the same value.
inline int32 writeRep(WriterI* _writer, uint8_t _byte, int32 _size)
{
  /*
  const uint32_t tmp0 = uint32_sels(64 - _size, 64, _size);
  const uint32_t tmp1 = uint32_sels(256 - _size, 256, tmp0);
  const uint32_t blockSize = uint32_sels(1024 - _size, 1024, tmp1);
  uint8_t* temp = (uint8_t*)alloca(blockSize);
  memset(temp, _byte, blockSize);

  int32_t size = 0;
  while (0 < _size)
  {
    int32_t bytes = write(_writer, temp, uint32_min(blockSize, _size));
    size += bytes;
    _size -= bytes;
  }

  return size;
  */
  return 0;
}

/// Write value.
template<typename Ty>
inline int32 write(WriterI* _writer, const Ty& _value)
{
  return _writer->write(&_value, sizeof(Ty));
}

/// Write value as little endian.
template<typename Ty>
inline int32 writeLE(WriterI* _writer, const Ty& _value)
{
  Ty value = (_value);
  int32 result = _writer->write(&value, sizeof(Ty));
  return result;
}

/// Write value as big endian.
template<typename Ty>
inline int32 writeBE(WriterI* _writer, const Ty& _value)
{
  int32 result = _writer->write(&value, sizeof(Ty));
  return result;
}

/// Write formated string.
inline int32 writePrintf(WriterI* _writer, const char* _format, ...)
{
  va_list argList;
  va_start(argList, _format);

  char temp[2048];
  char* out = temp;
  int32 max = sizeof(temp);
  int32 len = vsnprintf(out, max, _format, argList);
  if (len > max)
  {
    out = (char*)alloca(len);
    len = vsnprintf(out, len, _format, argList);
  }

  int32 size = write(_writer, out, len);

  va_end(argList);

  return size;
}

/// Skip _offset bytes forward.
inline int64 skip(SeekerI* _seeker, int64 _offset)
{
  return _seeker->seek(_offset, Whence::Current);
}

/// Seek to any position in file.
inline int64 seek(SeekerI* _seeker, int64 _offset = 0, Whence::Enum _whence = Whence::Current)
{
  return _seeker->seek(_offset, _whence);
}

/// Returns size of file.
inline int64 getSize(SeekerI* _seeker)
{
  int64 offset = _seeker->seek();
  int64 size = _seeker->seek(0, Whence::End);
  _seeker->seek(offset, Whence::Begin);
  return size;
}

struct __declspec(novtable) ReaderSeekerI : public ReaderI, public SeekerI
{
};

/// Peek data.
inline int32 peek(ReaderSeekerI* _reader, void* _data, int32 _size)
{
  int64 offset = seek(_reader);
  int32 size = _reader->read(_data, _size);
  seek(_reader, offset, Whence::Begin);
  return size;
}

/// Peek value.
template<typename Ty>
inline int32 peek(ReaderSeekerI* _reader, Ty& _value)
{
  return peek(_reader, &_value, sizeof(Ty));
}

struct __declspec(novtable) WriterSeekerI : public WriterI, public SeekerI
{
};

struct __declspec(novtable) ReaderOpenI
{
  virtual ~ReaderOpenI() = 0;
  virtual bool open(const char* _filePath) = 0;
};

inline ReaderOpenI::~ReaderOpenI()
{
}

struct __declspec(novtable) WriterOpenI
{
  virtual ~WriterOpenI() = 0;
  virtual bool open(const char* _filePath, bool _append) = 0;
};

inline WriterOpenI::~WriterOpenI()
{
}

struct __declspec(novtable) CloserI
{
  virtual ~CloserI() = 0;
  virtual void close() = 0;
};

inline CloserI::~CloserI()
{
}

struct __declspec(novtable) FileReaderI : public ReaderOpenI, public CloserI, public ReaderSeekerI
{
};

struct __declspec(novtable) FileWriterI : public WriterOpenI, public CloserI, public WriterSeekerI
{
};

inline bool open(ReaderOpenI* _reader, const char* _filePath)
{
  return _reader->open(_filePath);
}

inline bool open(WriterOpenI* _writer, const char* _filePath, bool _append = false)
{
  return _writer->open(_filePath, _append);
}

inline void close(CloserI* _reader)
{
  _reader->close();
}

struct __declspec(novtable) MemoryBlockI
{
  virtual void* more(uint32 _size = 0) = 0;
  virtual uint32 getSize() = 0;
};

class StaticMemoryBlock : public MemoryBlockI
{
public:
  StaticMemoryBlock(void* _data, uint32 _size)
    : m_data(_data)
    , m_size(_size)
  {
  }

  virtual ~StaticMemoryBlock()
  {
  }

  virtual void* more(uint32 /*_size*/ = 0) 
  {
    return m_data;
  }

    virtual uint32 getSize() 
  {
    return m_size;
  }

private:
  void* m_data;
  uint32 m_size;
};

class MemoryBlock : public MemoryBlockI
{
public:
  MemoryBlock(AllocatorI* _allocator)
    : m_allocator(_allocator)
    , m_data(NULL)
    , m_size(0)
  {
  }

  virtual ~MemoryBlock()
  {
    BX_FREE(m_allocator, m_data);
  }

  virtual void* more(uint32 _size = 0) 
  {
    if (0 < _size)
    {
      m_size += _size;
      m_data = BX_REALLOC(m_allocator, m_data, m_size);
    }

    return m_data;
  }

    virtual uint32 getSize()
  {
    return m_size;
  }

private:
  AllocatorI* m_allocator;
  void* m_data;
  uint32 m_size;
};

class SizerWriter : public WriterSeekerI
{
public:
  SizerWriter()
    : m_pos(0)
    , m_top(0)
  {
  }

  virtual ~SizerWriter()
  {
  }

  virtual int64 seek(int64 _offset = 0, Whence::Enum _whence = Whence::Current)
  {
    switch (_whence)
    {
    case Whence::Begin:
      m_pos = RBMath::clamp<i64>(_offset, 0, m_top);
      break;

    case Whence::Current:
      m_pos = RBMath::clamp<i64>(m_pos + _offset, 0, m_top);
      break;

    case Whence::End:
      m_pos = RBMath::clamp<i64>(m_top - _offset, 0, m_top);
      break;
    }

    return m_pos;
  }

    virtual int32 write(const void* /*_data*/, int32 _size) 
  {
    int32 morecore = int32(m_pos - m_top) + _size;

    if (0 < morecore)
    {
      m_top += morecore;
    }

    int64_t remainder = m_top - m_pos;
    int32_t size = RBMath::get_min((uint32)_size, uint32(RBMath::get_min(remainder, (i64)INT32_MAX)));
    m_pos += size;
    if (size != _size)
    {
      CHECK(!"SizerWriter: write truncated.");
    }
    return size;
  }

private:
  int64 m_pos;
  int64 m_top;
};

class MemoryReader : public ReaderSeekerI
{
public:
  MemoryReader(const void* _data, uint32 _size)
    : m_data((const uint8*)_data)
    , m_pos(0)
    , m_top(_size)
  {
  }

  virtual ~MemoryReader()
  {
  }

  virtual int64 seek(int64 _offset, Whence::Enum _whence) 
  {
    switch (_whence)
    {
    case Whence::Begin:
      m_pos = RBMath::clamp(_offset, (i64)0, m_top);
      break;

    case Whence::Current:
      m_pos = RBMath::clamp(m_pos + _offset, (i64)0, m_top);
      break;

    case Whence::End:
      m_pos = RBMath::clamp(m_top - _offset, (i64)0, m_top);
      break;
    }

    return m_pos;
  }

    virtual int32 read(void* _data, int32 _size) 
  {
    CHECK(!"Reader/Writer interface calling functions must handle errors.");

    int64 remainder = m_top - m_pos;
    int32 size = RBMath::get_min((u32)_size, uint32(RBMath::get_min(remainder, (i64)INT32_MAX)));
    memcpy(_data, &m_data[m_pos], size);
    m_pos += size;
    if (size != _size)
    {
      CHECK("MemoryReader: read truncated.");
    }
    return size;
  }

    const uint8* getDataPtr() const
  {
    return &m_data[m_pos];
  }

  int64 getPos() const
  {
    return m_pos;
  }

  int64 remaining() const
  {
    return m_top - m_pos;
  }

private:
  const uint8* m_data;
  int64 m_pos;
  int64 m_top;
};

class MemoryWriter : public WriterSeekerI
{
public:
  MemoryWriter(MemoryBlockI* _memBlock)
    : m_memBlock(_memBlock)
    , m_data(NULL)
    , m_pos(0)
    , m_top(0)
    , m_size(0)
  {
  }

  virtual ~MemoryWriter()
  {
  }

  virtual int64 seek(int64 _offset = 0, Whence::Enum _whence = Whence::Current)
  {
    switch (_whence)
    {
    case Whence::Begin:
      m_pos = RBMath::clamp(_offset, (i64)0, m_top);
      break;

    case Whence::Current:
      m_pos = RBMath::clamp<i64>(m_pos + _offset, 0, m_top);
      break;

    case Whence::End:
      m_pos = RBMath::clamp<i64>(m_top - _offset, 0, m_top);
      break;
    }

    return m_pos;
  }

    virtual int32 write(const void* _data, int32 _size)
  {
    CHECK(!"Reader/Writer interface calling functions must handle errors.");

    int32 morecore = int32(m_pos - m_size) + _size;

    if (0 < morecore)
    {
      morecore = Align(morecore, 0xfff);
      m_data = (uint8_t*)m_memBlock->more(morecore);
      m_size = m_memBlock->getSize();
    }

    int64_t remainder = m_size - m_pos;
    int32_t size = RBMath::get_min<u32>(_size, uint32_t(RBMath::get_min<i64>(remainder, MAX_I32)));
    memcpy(&m_data[m_pos], _data, size);
    m_pos += size;
    m_top = RBMath::get_max<i64>(m_top, m_pos);
    if (size != _size)
    {
      CHECK(!"MemoryWriter: write truncated.");
    }
    return size;
  }

private:
  MemoryBlockI* m_memBlock;
  uint8* m_data;
  int64 m_pos;
  int64 m_top;
  int64 m_size;
};

class StaticMemoryBlockWriter : public MemoryWriter
{
public:
  StaticMemoryBlockWriter(void* _data, uint32 _size)
    : MemoryWriter(&m_smb)
    , m_smb(_data, _size)
  {
  }

  ~StaticMemoryBlockWriter()
  {
  }

private:
  StaticMemoryBlock m_smb;
};

#pragma endregion

#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include "data-buffer.hpp"

DataBuffer::DataBuffer(void *memory, int fd)
{
    this->memory = static_cast<std::byte *>(memory);
    this->fd = fd;
}

void *DataBuffer::GetLatest()
{
    DataBufferHeader *header = reinterpret_cast<DataBufferHeader *>(this->memory);

    if (header->writeIndex == -1)
        return nullptr;

    // We already returned this buffer.
    if (this->lastBufferReceived == header->writeIndex)
        return nullptr;

    this->lastBufferReceived = header->writeIndex;

    return this->memory + header->dataOffset + (header->elementSize * (header->writeIndex));
}

void DataBuffer::Close()
{
    struct stat sb;
    fstat(this->fd, &sb);

    munmap(this->memory, sb.st_size);

    close(this->fd);
}
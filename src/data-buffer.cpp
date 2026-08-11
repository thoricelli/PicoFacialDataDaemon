#include "data-buffer.hpp"

DataBuffer::DataBuffer(void *memory)
{
    this->memory = static_cast<std::byte *>(memory);
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
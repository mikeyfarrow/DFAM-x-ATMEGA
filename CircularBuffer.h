#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdint.h> // Use standard integer types

#define OVERWRITE_MODE 1

template <typename T, uint8_t SIZE>
class CircularBuffer
{
public:
	T buffer[SIZE];
    uint8_t write_idx;
    uint8_t read_idx;

public:
    CircularBuffer();

    bool put(T item);
    bool get(T* val_ptr);
    uint8_t ready() const;
};

template <typename T, uint8_t SIZE>
CircularBuffer<T, SIZE>::CircularBuffer() : write_idx(0), read_idx(0) { }

template <typename T, uint8_t SIZE>
bool CircularBuffer<T, SIZE>::put(T item)
{
#ifndef OVERWRITE_MODE
	if ((write_idx + 1) % SIZE == read_idx)
	{
		// buffer is full, avoid overflow
		return 0;
	}
#endif
	
	buffer[write_idx] = item;
	write_idx = (write_idx + 1) % SIZE;
	return 1;
}

template <typename T, uint8_t SIZE>
bool CircularBuffer<T, SIZE>::get(T* val_ptr)
{
	if (read_idx == write_idx)
	{
		return 0; // buffer is empty
	}
	
	*val_ptr = buffer[read_idx];
	read_idx = (read_idx + 1) % SIZE;
	return 1;
}

template <typename T, uint8_t SIZE>
uint8_t CircularBuffer<T, SIZE>::ready() const
{
	return write_idx - read_idx;
}


#endif // CIRCULAR_BUFFER_H
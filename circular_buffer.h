/*
 * circular_buffer.h
 *
 * Created: 7/7/2024 9:57:28 AM
 *  Author: mikey
 */ 


#ifndef CIRCBUFFER_H_
#define CIRCBUFFER_H_

#define BUFFER_MAX_SIZE 100

class Circular_Buffer
{
	private:
	uint8_t buffer[BUFFER_MAX_SIZE];
	uint8_t write_idx = 0;
	uint8_t read_idx = 0;
	
	public:
	Circular_Buffer() {	};
	
	/*
		put - adds the item to the buffer and returns 1. If the add fails
		   because the buffer is full, then we do not add the item and
		   return 0
	*/
	uint8_t put(uint8_t item)
	{
		if ((write_idx + 1) % BUFFER_MAX_SIZE == read_idx)
		{
			// buffer is full, avoid overflow
			return 0;
		}
		
		buffer[write_idx] = item;
		write_idx = (write_idx + 1) % BUFFER_MAX_SIZE;
		return 1;
	};
	
	/*
		get - if there is a value in the buffer, it is saved to the address
		   pointed to by val_ptr and returns 1. If there is no data in the
		   buffer, then returns 0.
	*/
	uint8_t get(uint8_t* val_ptr)
	{
		if (read_idx == write_idx)
		{
			return 0; // buffer is empty
		}
		
		*val_ptr = buffer[read_idx];
		read_idx = (read_idx + 1) % BUFFER_MAX_SIZE;
		return 1;
	};
};





#endif /* CIRCBUFFER_H_ */
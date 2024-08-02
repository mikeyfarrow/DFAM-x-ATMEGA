/*
 * Serializable.h
 *
 * Created: 8/1/2024 6:51:39 PM
 *  Author: mikey
 */ 


#ifndef SERIALIZABLE_H_
#define SERIALIZABLE_H_


class Serializable
{
	public:
	virtual void serialize(uint8_t* buffer) const = 0;
	virtual void deserialize(const uint8_t* buffer) = 0;
	virtual size_t size_bytes() const = 0;
};


#endif /* SERIALIZABLE_H_ */
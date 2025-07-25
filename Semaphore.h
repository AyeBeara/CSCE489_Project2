#ifndef SEMAPHORE_H
#define SEMAPHORE_H

class Semaphore 
{
public:

	Semaphore(int count, int buffer_size);
	~Semaphore();

	void wait();
	void signal();

private:
	int count;
	int buffer_size;

	int buffer[];
};

#endif

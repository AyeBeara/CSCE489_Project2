#ifndef SEMAPHORE_H
#define SEMAPHORE_H

class Semaphore 
{
public:

	Semaphore(int count);
	~Semaphore();

	void wait();
	void signal();
	int getCount();
	int getSize();

private:
	int size;
	int count;
};

#endif

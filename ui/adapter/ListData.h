#ifndef _LIST_DATA_
#define _LIST_DATA_

class ListData {
public:
	virtual int size() = 0;
	virtual int type(int pos) = 0;
};

#endif
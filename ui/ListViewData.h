#pragma once

#include <vector>
#include <string>
#include "ListData.h"
class ListViewData :public ListData{
public:
	int size() override {
		return mData.size();
	}
	int type(int pos) override {
		return pos % 2 ? 1 : 0;
	}
	std::string& getString(int pos) {
		return mData[pos];
	}
private:
	std::vector<std::string> mData{ "a","A","b","B","c","C","d","D","e","E","f","F","g","G","h","H","i","I","j","J","k","K","l","L" };
};


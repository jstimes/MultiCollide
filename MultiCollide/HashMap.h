#pragma once

#include <math.h>
#include <iostream>

struct HashedNode {
	unsigned int hashcode;
	float key;
	float value;
	HashedNode *next;

	HashedNode(long aHash, float aKey, float aValue) {
		hashcode = aHash;
		key = aKey;
		value = aValue;
		next = nullptr;
	}
};

class HashMap {

private:
	HashedNode **map;

	int prevMapSize;

	int mapSize;

	int keyCount;

public:

	HashMap() {
		keyCount = 0;
		prevMapSize = mapSize = 2999;
		map = new HashedNode*[mapSize];
		
		for (int i = 0; i < mapSize; i++) {
			map[i] = nullptr;
		}
	}

	void Put(float aKey, float aValue) {
		unsigned int hash = hashcode(aKey);
		
		int index = hash % mapSize;

		int len = 0;
		if (map[index] == nullptr) {
			map[index] = new HashedNode(hash, aKey, aValue);
		}
		else {
			HashedNode *prev = nullptr;
			HashedNode *cur = map[index];
			while (cur != nullptr && !floatComparison(cur->key, aKey)) {
				len++;
				prev = cur;
				cur = cur->next;
			}

			if (cur == nullptr) {
				prev->next = new HashedNode(hash, aKey, aValue);
			}
			else {
				//Just setting a new value
				cur->value = aValue;
				return;
			}
		}

		keyCount++;
		/*std::cout << "Bucket length: " << len << std::endl;

		if (keyCount % 1000 == 0) {
			this->Report();
		}*/

		if (keyCount == 2999) {
			mapSize = 6007;
			resize();
		}
	}

	void Report() {
		for (int i = 0; i < mapSize; i++) {
			int count = 0;
			HashedNode *cur = map[i];

			while (cur != nullptr) {
				cur = cur->next;
				count++;
			}
			if (count > 10) {
				std::cout << std::endl;
			}
			std::cout << "bucket " << i << ": " << count << " entries" << std::endl;
			if (count > 10) {
				std::cout << std::endl;
			}
		}
	}

	float Get(float aKey) {
		unsigned int hash = hashcode(aKey);

		int index = hash % mapSize;

		HashedNode *cur = map[index];
		while (cur != nullptr && !floatComparison(cur->key, aKey)) {
			cur = cur->next;
		}

		if (cur == nullptr) {
			std::cout << "Key not in HashMap !!!" << std::endl;
			return NAN;
		}

		return map[index]->value;
	}

	bool ContainsKey(float aKey) {
		unsigned int hash = hashcode(aKey);

		int index = hash % mapSize;

		HashedNode *cur = map[index];
		while (cur != nullptr && !floatComparison(cur->key, aKey)) {
			cur = cur->next;
		}

	    return cur != nullptr;
	}

	~HashMap() {
		for (int i = 0; i < mapSize; i++) {
			HashedNode *node = map[i];
			while (node != nullptr) {
				HashedNode *temp = node;
				node = node->next;
				delete temp;
			}
		}
		delete[] map;
	}

private:

	void resize() {
		std::cout << "RESIZING HASHMAP" << std::endl;
		HashedNode **newMap = new HashedNode*[mapSize];
		
		int i;
		for (i = 0; i < mapSize; i++) {
			newMap[i] = nullptr;
		}

		for (i = 0; i < prevMapSize; i++) {
			if (map[i] != nullptr) {
				HashedNode *cur = map[i];
				while (cur != nullptr) {
					int newIndex = cur->hashcode % mapSize;

					HashedNode *temp = newMap[newIndex];
					newMap[newIndex] = cur;
					cur = cur->next;
					newMap[newIndex]->next = temp;					
				}
			}
		}

		delete[] map;
		map = newMap;
		prevMapSize = mapSize;
	}

	bool floatComparison(float a, float b) {
		return fabs(a - b) < .0000001f;
	}

	unsigned int hashcode(float f)
	{
		unsigned int ui;
		memcpy(&ui, &f, sizeof(float));
		return ui & 0xfffff000;
	}
};
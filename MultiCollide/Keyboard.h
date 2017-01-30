#pragma once

#pragma once

#include <vector>
#include "ShapeUtils.h"

class Keyboard {


public:

	class Listener {
	public: 
		virtual void KeyPressed(char c) = 0;
	};


private:
	std::vector<Listener*> listeners;

	Keyboard() {}

	static char charForKey(int glfwKey) {
		switch (glfwKey) {
		case GLFW_KEY_0:
			return '0';
		case GLFW_KEY_1:
			return '1';
		case GLFW_KEY_2:
			return '2';
		case GLFW_KEY_3:
			return '3';
		case GLFW_KEY_4:
			return '4';
		case GLFW_KEY_5:
			return '5';
		case GLFW_KEY_6:
			return '6';
		case GLFW_KEY_7:
			return '7';
		case GLFW_KEY_8:
			return '8';
		case GLFW_KEY_9:
			return '9';
		case GLFW_KEY_PERIOD:
			return '.';
		case GLFW_KEY_MINUS:
			return '-';
		case GLFW_KEY_BACKSPACE:
			return -5;
		case GLFW_KEY_DELETE:
			return -6;
		case GLFW_KEY_LEFT:
		case GLFW_KEY_RIGHT:
		case GLFW_KEY_UP:
		case GLFW_KEY_DOWN:
		default:
			return -1;
		}
	}


public:

	Keyboard(Keyboard const&) = delete;
	void operator=(Keyboard const&) = delete;

	static Keyboard& GetInstance() {
		static Keyboard keyboard;
		return keyboard;
	}

	/*static bool getR(bool flip) {
		static bool R = false;
		if (flip) {
			R = !R;
			std::cout << "R is down" << std::endl;
		}
		return R;
	}*/

	static void Input(int glfwKey) {
		Keyboard &keyboard = GetInstance();
		char c = charForKey(glfwKey);
		std::cout << c << std::endl;

		/*if (c == 'R') {
			getR(true);
		}*/

		if(c > 0){
			for (std::vector<Listener*>::iterator it = keyboard.listeners.begin(); it != keyboard.listeners.end(); it++) {
				(*it)->KeyPressed(c);
			}
		}
	}

	static void AddListener(Listener *listener) {
		Keyboard &keyboard = GetInstance();
		keyboard.listeners.push_back(listener);
	}

	static void RemoveListener(Listener *listener) {
		Keyboard &keyboard = GetInstance();
		for (std::vector<Listener*>::iterator it = keyboard.listeners.begin(); it != keyboard.listeners.end(); it++) {
			if ((*it) == listener) {
				it = keyboard.listeners.erase(it);
			}
		}
	}

	static void RemoveAllListeners() {
		Keyboard &keyboard = GetInstance();
		keyboard.listeners.clear();
	}

};
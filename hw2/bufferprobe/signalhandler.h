#ifndef __SIGNALHANDLER_H
#define __SIGNALHANDLER_H

#include <signal.h>
#include <unistd.h>

class SignalHandler {
public:
	SignalHandler(int signalToCatch)
	{
		signal(signalToCatch, SignalHandler::signalCallback);
	}

	static void signalCallback(int) {
		_receivedSignal = true;
	}

	bool receivedSignal() const {
		return _receivedSignal;
	}

	static bool _receivedSignal;

private:
};

bool SignalHandler::_receivedSignal = false;

class AlarmInterrupter {
public:
	AlarmInterrupter()
	{
		struct sigaction _action;
		_action.sa_handler = alarmCallback;

		if (sigfillset(&_action.sa_mask) < 0)
			throw std::runtime_error("sigfillset()");

		_action.sa_flags = 0;

		if (sigaction(SIGALRM, &_action, 0) < 0)
			throw std::runtime_error("sigaction");
	}

	void set(unsigned int seconds) const {
		alarm(seconds);
	}

	void unset() const {
		set(0);
	}

	static void alarmCallback(int) { }
};

#endif // __SIGNALHANDLER_H

#ifndef KONTROLLER_SERVICE_H
#define KONTROLLER_SERVICE_H

#include "KontrollerSock/Server.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <memory>
#include <thread>

class KontrollerService {
public:
	KontrollerService();

	void onStart(SERVICE_STATUS_HANDLE handle);

	void onStop();

	void onShutdown();

private:
	void updateServiceState(DWORD dwCurrentState);

	SERVICE_STATUS_HANDLE serviceStatusHandle;
	SERVICE_STATUS serviceStatus;
	DWORD checkPoint;

	std::thread thread;
	std::unique_ptr<KontrollerSock::Server> kontrollerServer;
};

#endif

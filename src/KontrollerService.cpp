#include "KontrollerService.h"

#include <cassert>

KontrollerService::KontrollerService() : serviceStatusHandle(nullptr), serviceStatus{}, checkPoint(0) {
	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	serviceStatus.dwWin32ExitCode = NO_ERROR;
	serviceStatus.dwServiceSpecificExitCode = 0;
	serviceStatus.dwCheckPoint = checkPoint++;
	serviceStatus.dwWaitHint = 0;
}

void KontrollerService::onStart(SERVICE_STATUS_HANDLE handle) {
	assert(handle != nullptr && serviceStatusHandle == nullptr && !kontrollerServer);

	serviceStatusHandle = handle;
	updateServiceState(SERVICE_START_PENDING);

	kontrollerServer = std::make_unique<KontrollerSock::Server>();

	thread = std::thread([this]() {
		updateServiceState(SERVICE_RUNNING);
		kontrollerServer->run();
	});
}

void KontrollerService::onStop() {
	updateServiceState(SERVICE_STOP_PENDING);

	kontrollerServer->shutDown();
	thread.join();
	kontrollerServer = {};

	updateServiceState(SERVICE_STOPPED);
}

void KontrollerService::onShutdown() {
	if (kontrollerServer) {
		onStop();
	}

	serviceStatusHandle = nullptr;
}

void KontrollerService::updateServiceState(DWORD dwCurrentState) {
	serviceStatus.dwCurrentState = dwCurrentState;
	serviceStatus.dwCheckPoint = ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED)) ? 0 : checkPoint++;

	SetServiceStatus(serviceStatusHandle, &serviceStatus);
}

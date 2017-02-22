#include "KontrollerService.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <chrono>
#include <string>
#include <thread>

namespace {

char* kServiceName = "KontrollerServer";
char* kServiceDisplayName = "Kontroller Server";
char* kServiceDescription = "Communicates with the KORG nanoKONTROL2 and hosts a socket server, allowing multiple outside processes to obtain MIDI data.";
const DWORD kServiceStartType = SERVICE_AUTO_START;
char* kServiceDependencies = "";
char* kServiceAccount = "NT AUTHORITY\\LocalService";
char* kServicePassword = nullptr;

KontrollerService kontrollerService;

bool installService(PSTR pszServiceName, PSTR pszDisplayName, PSTR pszDescription, DWORD dwStartType, PSTR pszDependencies, PSTR pszAccount, PSTR pszPassword) {
   char path[MAX_PATH];
   if (GetModuleFileName(nullptr, path, ARRAYSIZE(path)) == 0) {
      printf("GetModuleFileName failed with error: 0x%08lx\n", GetLastError());
      return false;
   }

   // Open the local default service control manager database
   SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
   if (schSCManager == nullptr) {
      printf("OpenSCManager failed with error: 0x%08lx\n", GetLastError());
      return false;
   }

   // Install the service into SCM by calling CreateService
   DWORD access = SERVICE_QUERY_STATUS | SERVICE_CHANGE_CONFIG;
   SC_HANDLE schService = CreateService(
      schSCManager,                   // SCManager database
      pszServiceName,                 // Name of service
      pszDisplayName,                 // Name to display
      access,                        // Desired access
      SERVICE_WIN32_OWN_PROCESS,      // Service type
      dwStartType,                    // Service start type
      SERVICE_ERROR_NORMAL,           // Error control type
      path,                           // Service's binary
      nullptr,                        // No load ordering group
      nullptr,                        // No tag identifier
      pszDependencies,                // Dependencies
      pszAccount,                     // Service running account
      pszPassword                     // Password of the account
   );
   if (schService == nullptr) {
      printf("CreateService failed with error 0x%08lx\n", GetLastError());

      CloseServiceHandle(schSCManager);
      return false;
   }

   // Set the service description
   SERVICE_DESCRIPTION serviceDescription;
   serviceDescription.lpDescription = pszDescription;
   if (!ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &serviceDescription)) {
      printf("ChangeServiceConfig2 failed with error: 0x%08lx\n", GetLastError());
   }

   printf("%s has been installed\n", pszServiceName);

   CloseServiceHandle(schSCManager);
   CloseServiceHandle(schService);

   return true;
}

bool uninstallService(PSTR pszServiceName) {
   // Open the local default service control manager database
   SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
   if (schSCManager == nullptr) {
      printf("OpenSCManager failed with error: 0x%08lx\n", GetLastError());
      return false;
   }

   // Open the service with delete, stop, and query status permissions
   SC_HANDLE schService = OpenService(schSCManager, pszServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
   if (schService == nullptr) {
      printf("OpenService failed with error: 0x%08lx\n", GetLastError());
      
      CloseServiceHandle(schSCManager);
      return false;
   }

   // Try to stop the service
   SERVICE_STATUS ssSvcStatus = {};
   if (ControlService(schService, SERVICE_CONTROL_STOP, &ssSvcStatus)) {
      printf("Stopping %s", pszServiceName);
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));

      while (QueryServiceStatus(schService, &ssSvcStatus)) {
         if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING) {
            printf(".");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
         } else {
            break;
         }
      }

      if (ssSvcStatus.dwCurrentState == SERVICE_STOPPED) {
         printf("\n%s has been stopped\n", pszServiceName);
      } else {
         printf("\n%s failed to stop\n", pszServiceName);
      }
   }

   // Now remove the service by calling DeleteService.
   if (!DeleteService(schService)) {
      printf("DeleteService failed with error: 0x%08lx\n", GetLastError());
      
      CloseServiceHandle(schSCManager);
      CloseServiceHandle(schService);
      return false;
   }

   printf("%s has been uninstalled\n", pszServiceName);

   CloseServiceHandle(schSCManager);
   CloseServiceHandle(schService);

   return true;
}

} // namespace

void WINAPI serviceControlHandler(DWORD dwCtrl) {
   switch (dwCtrl) {
   case SERVICE_CONTROL_STOP: kontrollerService.onStop(); break;
   case SERVICE_CONTROL_PAUSE: break;
   case SERVICE_CONTROL_CONTINUE: break;
   case SERVICE_CONTROL_SHUTDOWN: kontrollerService.onShutdown(); break;
   case SERVICE_CONTROL_INTERROGATE: break;
   default: break;
   }
}

VOID WINAPI ServiceMain(DWORD dwArgc, LPSTR *lpszArgv) {
   SERVICE_STATUS_HANDLE handle = RegisterServiceCtrlHandler(kServiceName, serviceControlHandler);
   if (!handle) {
      fprintf(stderr, "RegisterServiceCtrlHandler failed\n");
      return;
   }

   kontrollerService.onStart(handle);
}

int main(int argc, char* argv[]) {
   bool success = true;

   if (argc > 1) {
      std::string command(argv[1]);
      if (command == "install") {
         success = installService(kServiceName, kServiceDisplayName, kServiceDescription, kServiceStartType, kServiceDependencies, kServiceAccount, kServicePassword);
      } else if (command == "uninstall") {
         success = uninstallService(kServiceName);
      } else {
         printf("Usage: %s [install | uninstall]\n", argv[0]);
      }

      return success ? 0 : 1;
   }

   SERVICE_TABLE_ENTRY serviceTable[] =
   {
      { kServiceName, ServiceMain },
      { nullptr, nullptr }
   };

   success = StartServiceCtrlDispatcher(serviceTable) != 0;
   return success ? 0 : 1;
}

#include <stdio.h>
#include <time.h>
#include "sysinf.h"
#if defined( _WIN32)
#include <windows.h>
#elif defined( __APPLE__) || defined( __unix__)
#include <sys/utsname.h>
#endif

void sys_info_print_repl(){
  printf("\n");
  printf("IDO 0.0.0 on ");
#if defined( _WIN32) || defined(_WIN64)
  SYSTEM_INFO sysInfo;
  OSVERSIONINFO osvi;
  
  GetSystemInfo(&sysInfo);
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  
  if (GetVersionEx(&osvi)) {
    printf("Windows(");
    printf("build %ld ", osvi.dwBuildNumber);
    printf("versions %ld.%ld) ", osvi.dwMajorVersion, osvi.dwMinorVersion);
  }
  switch (sysInfo.wProcessorArchitecture) {
  case PROCESSOR_ARCHITECTURE_AMD64:
    printf("[x64]");
    break;
  case PROCESSOR_ARCHITECTURE_ARM:
    printf(" [ARM]");
    break;
  case PROCESSOR_ARCHITECTURE_IA64:
    printf(" [Intel Itanium]");
    break;
  case PROCESSOR_ARCHITECTURE_INTEL:
    printf(" [x86]");
    break;
  default: break;
  }
  
#elif defined(__APPLE__) || defined(__unix__)
  struct utsname buf;

  if(uname(&buf) == 0){
    printf("%s", buf.sysname); 
    printf(" (%s", buf.nodename); 
    printf(" %s", buf.version); 
    printf(" %s", buf.release); 
    printf(" %s)", buf.machine);
  } else {
    printf("Unix-like system");
  }
#endif
  time_t t;
  time(&t);
  printf(" %s", ctime(&t));
  printf("Happy hacking.\n\n");
}

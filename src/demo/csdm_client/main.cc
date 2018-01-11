#include <iostream>
#include "csdm_client.h"

int main() {
  CsdmClient csdm_client;

  std::string version;
  if (csdm_client.GetVersion(&version)) {
    std::cout << version << std::endl;
  }

  return 0;
}

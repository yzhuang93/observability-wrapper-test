#include <iostream>
#include <unistd.h>

#include "nutanix_observability.h"

int main() {
  std::cout << "Init NutanixObservability\n";
  nutanix::NutanixObservability::InitNutanixObservability("yiyang-test-idf");
  std::cout << "start span\n";
  auto span = nutanix::NutanixObservability::StartSpan("test span init");
  std::cout << "end span\n";
  span.End();

  return 0;
}

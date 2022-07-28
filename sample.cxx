#include <iostream>
#include <unistd.h>

#include "nutanix_observability.h"

int main() {
  nutanix::NutanixObservability::InitNutanixObservability("yiyang-test");
  auto span_a = nutanix::NutanixObservability::StartSpan("span_a");
  {
    auto span_b = nutanix::NutanixObservability::StartSpan("span_b");
    span_b->End();
  }
  span_a->End();

  return 0;
}

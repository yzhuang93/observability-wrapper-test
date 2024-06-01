#include <iostream>
#include <fstream>
#include <unistd.h>
#include "json/json.h"
#include <json/value.h>

int main() {
  std::string collector_endpoint = "";

  std::ifstream file_input("../config.json");
  Json::Reader reader;
  Json::Value data;
  if (reader.parse(file_input, data)) {
    collector_endpoint =
      data["observability"]["distributed_tracing"]["opentelemetry"]
        ["collector_endpoint"].asString();
    file_input.close();
  }
  std::cout << "Collector_endpoint: " << collector_endpoint << "\n";

  return 0;
}

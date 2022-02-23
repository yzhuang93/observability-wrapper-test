#include <iostream>
#include <unistd.h>

#include "opentelemetry/exporters/jaeger/jaeger_exporter.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"
#include "opentelemetry/context/runtime_context.h"
#include "opentelemetry/context/propagation/global_propagator.h"
#include "opentelemetry/context/propagation/text_map_propagator.h"
#include "opentelemetry/trace/propagation/http_trace_context.h"
#include "opentelemetry/ext/http/client/http_client.h"
#include "tracer_common.h"

namespace trace     = opentelemetry::trace;
namespace nostd     = opentelemetry::nostd;
namespace sdktrace   = opentelemetry::sdk::trace;
namespace jaeger    = opentelemetry::exporter::jaeger;

int main() {
  std::cout << "start span\n";
  initTracer();

  // Get tracer
  auto tracer = get_tracer();
  auto span = tracer->StartSpan("Sample-test");
  auto scope = tracer->WithActiveSpan(span);
  std::string log_prefix = get_log_prefix();
  std::cout<<log_prefix<<"\n";
  span->End();

  return 0;
}

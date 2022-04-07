/*
 * Copyright (c) 2022 Nutanix Inc. All rights reserved.
 *
 * Author: mudit.wadhwa@nutanix.com
 */

#ifndef _UTIL_BASE_NUTANIX_SPAN_H_
#define _UTIL_BASE_NUTANIX_SPAN_H_
#ifndef HAVE_ABSEIL
#define HAVE_ABSEIL
#endif

#include <cstring>
#include <string>
#include <iostream>
#include <vector>

#include "opentelemetry/context/propagation/global_propagator.h"
#include "opentelemetry/context/propagation/text_map_propagator.h"
#include "opentelemetry/exporters/jaeger/jaeger_exporter.h"
#include "opentelemetry/exporters/ostream/span_exporter.h"
#include "opentelemetry/ext/http/client/http_client.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/propagation/http_trace_context.h"
#include "opentelemetry/trace/provider.h"

namespace trace = opentelemetry::trace;
namespace nostd = opentelemetry::nostd;
namespace sdktrace = opentelemetry::sdk::trace;
namespace jaeger = opentelemetry::exporter::jaeger;
using namespace std;
namespace nutanix {
// Forward Declaration.
class NutanixObservability;
class NutanixSpan {
  public:
    NutanixSpan(nostd::shared_ptr<trace::Span>& span,\
                trace::Scope* scope);
    // NutanixSpan();
    void MakeCurrent();
    void End() const;
    void SetTag(string tag_name, string tag_value);

  private:
    friend NutanixObservability;
    nostd::shared_ptr<trace::Span> actual_span;

    // Scope controls how long a span is active.
    trace::Scope* scope_;
};
} // namespace

#endif // _UTIL_BASE_NUTANIX_SPAN_H_
/*
 * Copyright (c) 2022 Nutanix Inc. All rights reserved.
 *
 * Author: mudit.wadhwa@nutanix.com
 *
 * This file provides a Nutanix Span wrapper that encapsulates opentelemetry
 * span. It exposes methods related to lifecycles of span and adding data to
 * span.
 * Example usage:
 * Once a span is started by any of the methods inside NutanixObservability
 * class, we can add tags, status and error tags to the span.
 * // Start a root span.
 * auto span = NutanixObservability::StartSpan("demo_span");
 * // Set some tag in the span.
 * span->SetTag("tag.name", "tag.value");
 * // Set the status of request in the span.
 * span->SetStatus("kNoError");
 * // Set the error tag to true if request has error. Used to generate error
 * // spans.
 * span->SetErrorTag("kNoError");
 *
 */

#ifndef _UTIL_BASE_NUTANIX_SPAN_H_
#define _UTIL_BASE_NUTANIX_SPAN_H_
#ifndef HAVE_ABSEIL
#define HAVE_ABSEIL
#endif

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "opentelemetry/context/propagation/global_propagator.h"
#include "opentelemetry/context/propagation/text_map_propagator.h"
#include "opentelemetry/exporters/jaeger/jaeger_exporter.h"
#include "opentelemetry/exporters/ostream/span_exporter.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include "opentelemetry/sdk/common/env_variables.h"
#include "opentelemetry/sdk/resource/resource_detector.h"
#include "opentelemetry/sdk/trace/batch_span_processor.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/propagation/http_trace_context.h"
#include "opentelemetry/trace/propagation/jaeger.h"
#include "opentelemetry/trace/provider.h"
#include "opentelemetry/trace/span_metadata.h"

namespace context = opentelemetry::context;
namespace jaeger = opentelemetry::exporter::jaeger;
namespace nostd = opentelemetry::nostd;
namespace sdktrace = opentelemetry::sdk::trace;
namespace trace = opentelemetry::trace;

using namespace std;

namespace nutanix {
// Forward Declaration.
class NutanixObservability;

class NutanixSpan {
 public:
    typedef shared_ptr<NutanixSpan> Ptr;
    typedef shared_ptr<const NutanixSpan> PtrConst;

    // Construct a NutanixSpan by taking in an opentelemetry span and a
    // scope object.
    NutanixSpan(const nostd::shared_ptr<trace::Span>& span,
                trace::Scope* scope);

    ~NutanixSpan() = default;

    // Public constants.
    static const string SPAN_KIND_SERVER;
    static const string SPAN_KIND_CLIENT;

    // Method to make current span active.
    void MakeCurrent();

    // Method to end span.
    void End();

    // Method to add tag(Attribute) to span.
    void SetTag(string tag_name, string tag_value);

    // Method to set status to span.
    void SetStatus(string status_code);

    // Method to add Error tag to span.
    void SetErrorTag(string error_msg);

    // Method to set span kind.
    void SetSpanKind(string span_kind);

    // Method to set http method.
    void SetHttpMethod(string http_method);

    // Method to set method name.
    void SetMethodName(string method_name);

    // Method to check if empty span.
    bool IsEmpty();

 private:
    friend NutanixObservability;

    // Fetch the SpanContext object from the span.
    trace::SpanContext GetContext() {
      return actual_span->GetContext();
    }

    // Returns context object fetched from the span.
    context::Context GetTraceContext(
      context::Context current_ctx) {
      return trace::SetSpan(current_ctx, actual_span);
    }

    // opentelemetry span.
    nostd::shared_ptr<trace::Span> actual_span;

    // Scope controls how long a span is active.
    trace::Scope* scope_;
};
} // namespace

#endif // _UTIL_BASE_NUTANIX_SPAN_H_

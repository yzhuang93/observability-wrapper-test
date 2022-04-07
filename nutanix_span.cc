/*
 * Copyright (c) 2022 Nutanix Inc. All rights reserved.
 *
 * Author: mudit.wadhwa@nutanix.com
 */

#include "nutanix_span.h"

namespace trace = opentelemetry::trace;
namespace nostd = opentelemetry::nostd;
namespace sdktrace = opentelemetry::sdk::trace;
namespace jaeger = opentelemetry::exporter::jaeger;


namespace nutanix {

NutanixSpan::NutanixSpan(nostd::shared_ptr<trace::Span>& span,
                         trace::Scope* scope) {
  actual_span = span;
  scope_ = scope;
}

void NutanixSpan::End() const {
  actual_span->End();
}

void NutanixSpan::MakeCurrent() {
  *scope_ = opentelemetry::trace::Tracer::WithActiveSpan(actual_span);
}
void NutanixSpan::SetTag(string tag_name, string tag_value) {
  actual_span->SetAttribute(tag_name, tag_value);
}

} // namespace

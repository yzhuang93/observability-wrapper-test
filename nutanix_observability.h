/*
 * Copyright (c) 2022 Nutanix Inc. All rights reserved.
 *
 * Author: mudit.wadhwa@nutanix.com
 */

#ifndef _UTIL_BASE_NUTANIX_OBSERVABILITY_H_
#define _UTIL_BASE_NUTANIX_OBSERVABILITY_H_

#include "nutanix_span.h"

namespace trace = opentelemetry::trace;
namespace nostd = opentelemetry::nostd;

namespace nutanix {

class NutanixObservability {
  public:
    // Initializes TracerProvider and sets tracer_.
    static void InitNutanixObservability(const string& service_name);

    // Method to start NutanixSpan.
    static NutanixSpan StartSpan(const string& span_name);

    // Method to create child span.
    static NutanixSpan CreateChildSpan(const string& span_name, NutanixSpan& parent_span);

    // This method makes the span passed as parameter active.
    static void MakeCurrent(NutanixSpan& span);

  private:
    static nostd::shared_ptr<trace::Tracer> tracer_;

    // Parses the config file and returns the endpoint.
    static string GetEndPoint();
};

} // namespace

#endif // _UTIL_BASE_NUTANIX_OBSERVABILITY_H_

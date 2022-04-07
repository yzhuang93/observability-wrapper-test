/*
 * Copyright (c) 2022 Nutanix Inc. All rights reserved.
 *
 * Author: mudit.wadhwa@nutanix.com
 */

#include "nutanix_observability.h"

namespace trace = opentelemetry::trace;
namespace nostd = opentelemetry::nostd;
namespace sdktrace = opentelemetry::sdk::trace;
namespace jaeger = opentelemetry::exporter::jaeger;

using namespace std;

namespace nutanix {

nostd::shared_ptr<trace::Tracer>  NutanixObservability::tracer_;

void NutanixObservability::InitNutanixObservability(
  const string& service_name) {
    
  // if (NutanixObservability::tracer_)
  //   return;

  jaeger::JaegerExporterOptions options;
  options.endpoint = NutanixObservability::GetEndPoint();
  options.transport_format = opentelemetry::exporter::jaeger::TransportFormat::kThriftHttp;

  // Create Jaeger exporter instance
  auto exporter  = std::unique_ptr<sdktrace::SpanExporter>(new jaeger::JaegerExporter(options));

  // simple processor
  auto simple_processor = std::unique_ptr<sdktrace::SpanProcessor>(new sdktrace::SimpleSpanProcessor(std::move(exporter)));

  // AlwaysOnSampler
  auto always_on_sampler = std::unique_ptr<sdktrace::AlwaysOnSampler>(new sdktrace::AlwaysOnSampler);

  auto resource = opentelemetry::sdk::resource::Resource::Create({
    {"service.name", service_name},
    {"service.instance.id", "instance-1"}
  });

  // Id generator
  auto id_generator = std::unique_ptr<sdktrace::IdGenerator>(new sdktrace::RandomIdGenerator());

//   nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracer_provider_;
//   (new sdktrace::TracerProvider(std::move(simple_processor), resource, std::move(always_on_sampler), std::move(id_generator)));

  auto tracer_provider_ = nostd::shared_ptr<opentelemetry::trace::TracerProvider>
  (new sdktrace::TracerProvider(std::move(simple_processor), resource, std::move(always_on_sampler), std::move(id_generator)));

  trace::Provider::SetTracerProvider(tracer_provider_);

  NutanixObservability::tracer_ = tracer_provider_->GetTracer("io.opentelemetry.undertow-1.4", "1.5.3");

}

string NutanixObservability::GetEndPoint() {
  return "http://prismloadtest3.umvm.nutanix.com:14268/api/traces";
}

NutanixSpan NutanixObservability::StartSpan(const string& span_name) {
  auto span = NutanixObservability::tracer_->StartSpan(span_name);
  auto scope = NutanixObservability::tracer_->WithActiveSpan(span);
  // return NutanixSpan(span/*, &scope*/);
  return NutanixSpan(span, &scope);
}

NutanixSpan NutanixObservability::CreateChildSpan(const string& span_name,
                                                  NutanixSpan& parent_span) {
  trace::StartSpanOptions options;
  options.parent = (parent_span.actual_span)->GetContext();
  auto child_span = NutanixObservability::tracer_->StartSpan(span_name, options);
  auto child_scope = NutanixObservability::tracer_->WithActiveSpan(child_span);
  return NutanixSpan(child_span, &child_scope);
}

} // namespace

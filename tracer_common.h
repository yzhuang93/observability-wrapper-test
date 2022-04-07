/**
 * @file tracer_common.h
 * @author Yiyang.Zhuang (yiyang.zhuang@nutanix.com)
 * @brief Old tracing utils class, not being used any more. Only keep for reference purpose
 * @version 0.1
 * @date 2022-04-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once
#include "opentelemetry/exporters/ostream/span_exporter.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"

#include "opentelemetry/context/propagation/global_propagator.h"
#include "opentelemetry/context/propagation/text_map_propagator.h"
#include "opentelemetry/trace/propagation/http_trace_context.h"

#include <cstring>
#include <iostream>
#include <vector>
#include "opentelemetry/ext/http/client/http_client.h"
#include "opentelemetry/nostd/shared_ptr.h"


namespace trace     = opentelemetry::trace;
namespace nostd     = opentelemetry::nostd;
namespace sdktrace  = opentelemetry::sdk::trace;
namespace jaeger    = opentelemetry::exporter::jaeger;

namespace
{

template <typename T>
class HttpTextMapCarrier : public opentelemetry::context::propagation::TextMapCarrier
{
public:
  HttpTextMapCarrier<T>(T &headers) : headers_(headers) {}
  HttpTextMapCarrier() = default;
  virtual opentelemetry::nostd::string_view Get(
      opentelemetry::nostd::string_view key) const noexcept override
  {
    std::string key_to_compare = key.data();
    // Header's first letter seems to be  automatically capitaliazed by our test http-server, so
    // compare accordingly.
    if (key == opentelemetry::trace::propagation::kTraceParent)
    {
      key_to_compare = "Traceparent";
    }
    else if (key == opentelemetry::trace::propagation::kTraceState)
    {
      key_to_compare = "Tracestate";
    }
    auto it = headers_.find(key_to_compare);
    if (it != headers_.end())
    {
      return it->second;
    }
    return "";
  }

  virtual void Set(opentelemetry::nostd::string_view key,
                   opentelemetry::nostd::string_view value) noexcept override
  {
    headers_.insert(std::pair<std::string, std::string>(std::string(key), std::string(value)));
  }

  T headers_;
};

void initTracer()
{
  //Init the tracer provider
  opentelemetry::exporter::jaeger::JaegerExporterOptions options;
  options.endpoint         = "http://prismloadtest3.umvm.nutanix.com:14268/api/traces";
  options.transport_format = opentelemetry::exporter::jaeger::TransportFormat::kThriftHttp;

  // Create Jaeger exporter instance
  auto exporter  = std::unique_ptr<sdktrace::SpanExporter>(new jaeger::JaegerExporter(options));

  // simple processor
  auto simple_processor = std::unique_ptr<sdktrace::SpanProcessor>(new sdktrace::SimpleSpanProcessor(std::move(exporter)));

  // AlwaysOnSampler
  auto always_on_sampler = std::unique_ptr<sdktrace::AlwaysOnSampler>(new sdktrace::AlwaysOnSampler);

  // define resource
  char host[256];
   char *IP;
   struct hostent *host_entry;
   int hostname;
   hostname = gethostname(host, sizeof(host)); //find the host name

  auto custom_resource = opentelemetry::sdk::resource::Resource::Create({
    {"service.name", "sample"},
    {"service.instance.id", "instance-1"}
  });
  auto otel_resource = opentelemetry::sdk::resource::OTELResourceDetector().Detect();
  auto resource = custom_resource.Merge(otel_resource);

  // Id generator
  auto id_generator = std::unique_ptr<sdktrace::IdGenerator>(new sdktrace::RandomIdGenerator());

  auto tracer_provider_ = nostd::shared_ptr<opentelemetry::trace::TracerProvider> 
  (new sdktrace::TracerProvider(std::move(simple_processor), resource, std::move(always_on_sampler), std::move(id_generator)));

  trace::Provider::SetTracerProvider(tracer_provider_);
}

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> get_tracer()
{
  auto provider = opentelemetry::trace::Provider::GetTracerProvider();
  return provider->GetTracer("test");
}

std::string get_log_prefix() {
  auto span_context = get_tracer()->GetCurrentSpan()->GetContext();
  char trace_id_char[32];
  char span_id_char[16];
  span_context.trace_id().ToLowerBase16(opentelemetry::nostd::span<char, 2 * 16>{&trace_id_char[0], 32});
  span_context.span_id().ToLowerBase16(opentelemetry::nostd::span<char, 2 * 8>{&span_id_char[0], 16});
  std::string trace_id(trace_id_char, 32);
  std::string span_id(span_id_char, 16);
  std::string log_prefix = "{ app=\"mercury\", trace_id=\"" + trace_id + "\", span_id=\"" + span_id + "\" }";

  return log_prefix;
}

} 
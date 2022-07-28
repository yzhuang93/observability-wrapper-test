/*
 * Copyright (c) 2022 Nutanix Inc. All rights reserved.
 *
 * Author: mudit.wadhwa@nutanix.com
 */


#include <arpa/inet.h>
#include <errno.h>
#include <exception>
#include <fstream>
#include <gflags/gflags.h>
#include "json/json.h"
#include <json/value.h>
#include <netdb.h>
#include <netinet/in.h>
#include <regex>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "nutanix_observability.h"

DEFINE_string(observability_config_file_path,
              "../config.json",
              "Path for file with observability configuration.");
DEFINE_string(observability_otel_library_name,
              "io.opentelemetry.undertow-1.4",
              "Name of the opentelemetry tracing library.");
DEFINE_string(observability_otel_library_version,
              "1.5.3",
              "Version of the opentelemetry tracing library.");

namespace nutanix {

nostd::shared_ptr<trace::Tracer>  NutanixObservability::tracer_;
bool NutanixObservability::is_tracing_enabled = false;
string NutanixObservability::collector_endpoint = "";
string NutanixObservability::hostname = "";
string NutanixObservability::ip_address = "";

//-----------------------------------------------------------------------------

void NutanixObservability::InitNutanixObservability(
  const string& service_name) {
  try {
    NutanixObservability::CheckTracingEnabled();

    if (!IsTracingEnabled()) {
      cout << "Observability is not enabled for this host, "
                << "skipping tracer initialization.\n";
      return;
    }

    if (NutanixObservability::tracer_ != nullptr) {
      cout << "Tracer has been initialized, skipping initialization.\n";
      return;
    }

    cout << "Initializing observability tracer with"
              << " service.name: " << service_name
              << ", hostname: " << NutanixObservability::hostname
              << "\n";

    jaeger::JaegerExporterOptions options;
    options.endpoint = NutanixObservability::GetEndPoint();
    options.transport_format =
      opentelemetry::exporter::jaeger::TransportFormat::kThriftHttp;

    // Create Jaeger exporter instance
    auto exporter  = std::unique_ptr<sdktrace::SpanExporter>(
      new jaeger::JaegerExporter(options));

    // Use of Batch Span Processor instead of simple span processor reduced the
    // latency caused by EndSpan() function.
    sdktrace::BatchSpanProcessorOptions options1{};
    auto simple_processor = std::unique_ptr<sdktrace::BatchSpanProcessor>(
      new sdktrace::BatchSpanProcessor(std::move(exporter), options1));

    // AlwaysOnSampler
    auto always_on_sampler = std::unique_ptr<sdktrace::AlwaysOnSampler>(
      new sdktrace::AlwaysOnSampler);

    // TODO: Set ip address correctly.
    // Currently localhost ip is getting set.
    auto resource = opentelemetry::sdk::resource::Resource::Create({
      {"service.name", service_name},
      {"service.instance.id", "instance-1"},
      {"hostname", NutanixObservability::hostname},
      {"ip", NutanixObservability::ip_address}
    });

    // Id generator
    auto id_generator = std::unique_ptr<sdktrace::IdGenerator>(
      new sdktrace::RandomIdGenerator());

    auto tracer_provider_ =
      nostd::shared_ptr<opentelemetry::trace::TracerProvider>
      (new sdktrace::TracerProvider(std::move(simple_processor), resource,
      std::move(always_on_sampler), std::move(id_generator)));

    trace::Provider::SetTracerProvider(tracer_provider_);

    // TODO: Support composite propagators.
    // JIRA: ENG-467787
    context::propagation::GlobalTextMapPropagator::SetGlobalPropagator(
      opentelemetry::nostd::
      shared_ptr<context::propagation::TextMapPropagator>(
          new opentelemetry::trace::propagation::JaegerPropagator()));

    NutanixObservability::tracer_ =
      tracer_provider_->GetTracer(FLAGS_observability_otel_library_name,
      FLAGS_observability_otel_library_version);
  } catch (const exception& ex) {
    is_tracing_enabled = false;
  }
}

//-----------------------------------------------------------------------------

void NutanixObservability::CheckTracingEnabled() {
  try {
    std::ifstream file_input(FLAGS_observability_config_file_path);
    Json::Reader reader;
    Json::Value data;
    if (reader.parse(file_input, data)) {
      NutanixObservability::is_tracing_enabled =
        (data["observability"]["distributed_tracing"]["enabled"].asString()
          == "true");
      NutanixObservability::collector_endpoint =
        data["observability"]["distributed_tracing"]["opentelemetry"]
          ["collector_endpoint"].asString();
      file_input.close();
    } else {
      cout << "Error occured while reading config file - "
           << FLAGS_observability_config_file_path << "\n";
    }

    char hostbuffer[256];
    int hostname;

    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    NutanixObservability::hostname = string(hostbuffer);
    // TODO: Check how to get the IP of the current host.
    // We could not use the ip_util because it is in util_net,
    // It will cause linking issue if service only links util_base
    // auto ip =
    //  net::IpUtil::ResolveHostnameToIp(NutanixObservability::hostname);
    // NutanixObservability::ip_address = ip.ToString();
    NutanixObservability::ip_address = "127.0.0.1";
  } catch (const exception& ex) {
    NutanixObservability::is_tracing_enabled = false;
  }
}

//-----------------------------------------------------------------------------

bool NutanixObservability::IsTracingEnabled() {
  return NutanixObservability::is_tracing_enabled;
}

//-----------------------------------------------------------------------------

string NutanixObservability::GetEndPoint() {
  string str = NutanixObservability::collector_endpoint;
  // Teamporary workaround for endpoint, once we migrate to open telemetry
  // agent, this needs to be changed.
  string endpoint = str.substr(0, str.find_last_of(":")) + ":14268/api/traces";
  return endpoint;
}

//-----------------------------------------------------------------------------

shared_ptr<NutanixSpan> NutanixObservability::StartSpan(
  const string& span_name) {
  NutanixSpan::Ptr empty_span = GetEmptySpan();
  try {
    if (NutanixObservability::tracer_ && IsTracingEnabled())  {
      auto span = NutanixObservability::tracer_->StartSpan(span_name);
      auto scope = NutanixObservability::tracer_->WithActiveSpan(span);
      return make_shared<NutanixSpan>(span, &scope);
    } else {
      return empty_span;
    }
  } catch (const exception& ex) {
    return empty_span;
  }
}

//-----------------------------------------------------------------------------

shared_ptr<NutanixSpan> NutanixObservability::CreateChildSpan(
  const string& span_name, shared_ptr<NutanixSpan> parent_span) {
  NutanixSpan::Ptr empty_span = GetEmptySpan();
  try {
    if (NutanixObservability::tracer_ && IsTracingEnabled()) {
      trace::StartSpanOptions options;
      if (!parent_span->IsEmpty()) {
        options.parent = (parent_span->actual_span)->GetContext();
      }
      auto child_span =
        NutanixObservability::tracer_->StartSpan(span_name, options);
      auto child_scope =
        NutanixObservability::tracer_->WithActiveSpan(child_span);
      return make_shared<NutanixSpan>(child_span, &child_scope);
    } else {
      return empty_span;
    }
  } catch (const exception& ex) {
    return empty_span;
  }
}

//-----------------------------------------------------------------------------

shared_ptr<NutanixSpan> NutanixObservability::StartSpanFromBytes(
  const string& span_name, const string& bytes) {
  try {
    HttpTextMapCarrier<std::unordered_map<std::string, std::string>> carrier;
    Json::Reader reader(Json::Features::all());
    Json::Value json_parsed_data;
    reader.parse(bytes, json_parsed_data);

    for (const auto& trace_mdata : json_parsed_data.getMemberNames()) {
      // tokenize the span context metadata with delimiter ":"
      // For example: e9076d71e689d41fa05c2874fcc8859a:24e4b557b66e721f:0:01
      // {trace-id}:{span-id}:{parent-span-id}:{flags}
      std::string trace_metadata = json_parsed_data[trace_mdata].asString();
      std::regex regex("\\:");
      std::vector<std::string> out(
            std::sregex_token_iterator(
              trace_metadata.begin(), trace_metadata.end(), regex, -1),
            std::sregex_token_iterator());

      std::string value;
      for (uint64_t i = 0; i < out.size(); i++) {
        string s;
        // Append 0 in front if 14 or 15 character TraceId/SpanId is received.
        // This is done so that spans are stiched correctly.
        if ((i == 1 || i == 0) && out[i].size() < 16) {
          s = string(16 - out[i].size(), '0') + out[i];
        } else {
          s = out[i];
        }
        if (i != out.size() - 1) {
          s = s + ":";
        }
        value = value + s;
      }
      carrier.Set(trace_mdata, value);
    }

    // embed bytes into carrier
    auto prop =
      context::propagation::GlobalTextMapPropagator::GetGlobalPropagator();
    auto current_ctx = context::RuntimeContext::GetCurrent();

    auto new_context = prop->Extract(carrier, current_ctx);
    auto span = opentelemetry::trace::GetSpan(new_context);
    auto scope = NutanixObservability::tracer_->WithActiveSpan(span);
    auto nspan = make_shared<NutanixSpan>(span, &scope);
    return NutanixObservability::CreateChildSpan(span_name, nspan);
  } catch (const exception& ex) {
    return GetEmptySpan();
  }
}

//-----------------------------------------------------------------------------

string NutanixObservability::GetBytesFromSpan(shared_ptr<NutanixSpan> span) {
  try {
    if (!span || span->IsEmpty()) {
      return "";
    }
    opentelemetry::context::Context current_ctx;
    current_ctx = span->GetTraceContext(current_ctx);

    HttpTextMapCarrier<std::unordered_map<std::string, std::string>> carrier;
    auto prop =
      context::propagation::GlobalTextMapPropagator::GetGlobalPropagator();
    prop->Inject(carrier, current_ctx);

    Json::Value json;

    for (auto& element : carrier.headers_) {
        json[element.first] = element.second;
    }
    Json::FastWriter fastWritter;
    std::string serialized = fastWritter.write(json);
    return serialized.c_str();
  } catch (const exception& ex) {
    return "";
  }
}

//-----------------------------------------------------------------------------

std::unordered_map<std::string, std::string> NutanixObservability::Inject(
  const shared_ptr<NutanixSpan> &span) {
  std::unordered_map<std::string, std::string> emptyMap;
  try {
    if (!span || span->IsEmpty()) {
      return emptyMap;
    }
    opentelemetry::context::Context current_ctx;
    current_ctx = span->GetTraceContext(current_ctx);
    HttpTextMapCarrier<std::unordered_map<std::string, std::string>> carrier;
    auto prop =
      context::propagation::GlobalTextMapPropagator::GetGlobalPropagator();
    prop->Inject(carrier, current_ctx);
    return carrier.headers_;
  } catch(const exception& ex) {
    return emptyMap;
  }
}

//-----------------------------------------------------------------------------

shared_ptr<NutanixSpan> NutanixObservability::Extract(
  const string &span_name,
  const context::propagation::TextMapCarrier &carrier) {
  try {
    auto prop =
      context::propagation::GlobalTextMapPropagator::GetGlobalPropagator();
    auto current_ctx = context::RuntimeContext::GetCurrent();
    auto new_context = prop->Extract(carrier, current_ctx);
    auto span_context =
      opentelemetry::trace::GetSpan(new_context)->GetContext();
    trace::StartSpanOptions options;
    if (span_context.IsValid()) {
      options.parent = span_context;
    }
    auto child_span =
      NutanixObservability::tracer_->StartSpan(span_name, options);
    auto child_scope =
      NutanixObservability::tracer_->WithActiveSpan(child_span);
    return make_shared<NutanixSpan>(child_span, &child_scope);
  } catch (const exception& ex) {
    return GetEmptySpan();
  }
}

//-----------------------------------------------------------------------------

shared_ptr<NutanixSpan> NutanixObservability::GetEmptySpan() {
  // Define a nullptr of nostd namespace
  nostd::shared_ptr<trace::Span> null_span(nullptr);
  return make_shared<NutanixSpan>(null_span, nullptr);
}

//-----------------------------------------------------------------------------

} // namespace

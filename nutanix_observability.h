/*
 * Copyright (c) 2022 Nutanix Inc. All rights reserved.
 *
 * Author: mudit.wadhwa@nutanix.com
 *
 * This file defines the NutanixObservability class that provides distributed
 * tracing facility for cpp services. Distributed Tracing is a method to
 * monitor and track service requests as they propagate through distributed
 * systems. A Trace consists of multiple spans stitched together. Every span
 * has it's own life cycle. The goal of this wrapper is to encapsulate the
 * framework and configuration being used. Currently we are using opentelemetry
 * as the tracing framework to support tracing. In future, we may change the
 * framework but no changes have to be done by service owners while onboarding
 * tracing using this wrapper library.
 * Distributed Tracing requires all the services involved to contribute spans
 * and stitch them together in order to form complete and meaningful traces.
 * We have a global configuration file to enable/disable tracing. The global
 * config file can be found at /home/nutanix/config/observability/config.json
 * on PC/PE. In order to enable/disable tracing we have a field "enabled" in
 * config file which we can set.
 *
 * Example usage:
 * Every cpp service need to initialize the tracer in the main function of
 * the service using InitNutanixObservability().
 * NutanixObservability::InitNutanixObservability("service_name");
 *
 * After initializing the tracer, services can start and end span based on
 * their use-case.
 * // To start a new root span with name "demo_span".
 *  auto span = NutanixObservability::StartSpan("demo_span");
 *
 * // To create a child span from a parent span.
 * auto child_span = NutanixObservability::CreateChildSpan("child_span", span);
 *
 * // Byte array is used to pass trace context across multiple services. Format
 * // of data stored in byte array:
 * // {"traceparent ":" version "-" trace-id "-" parent-id "-" trace-flags"}.
 * // Create child span from byte array
 * auto span = NutanixObservability::StartSpanFromBytes("span_name",
 *   byte_array);
 *
 * // Get trace context byte array from Nutanix Span.
 * auto bytes = NutanixObservability::GetBytesFromSpan(span);
 *
 * // To end span.
 * span->End();
 *
 */

#ifndef _UTIL_BASE_NUTANIX_OBSERVABILITY_H_
#define _UTIL_BASE_NUTANIX_OBSERVABILITY_H_

#include "nutanix_span.h"

namespace nutanix {

// This class defines static methods.
class NutanixObservability {
 public:
    // This method checks if tracing is enabled and initializes the tracer. It
    // sets the Jaeger Exporter endpoint and the transport format.
    // service_name has to be given by the service calling this method.
    // It uses batch span processor to send spans to collector endpoint. Also,
    // hostname, ip address, etc. are set as resource attributes.
    static void InitNutanixObservability(const string& service_name);

    // This method starts a new root span with the given "span_name" if tracing
    // is enabled on the setup and returns a shared pointer pointing to new
    // NutanixSpan created.
    static shared_ptr<NutanixSpan> StartSpan(const string& span_name);

    // This method creates a child span of parent_span with the given
    // "span_name" if tracing is enabled on the setup and returns a shared
    // pointer pointing to new NutanixSpan created.
    static shared_ptr<NutanixSpan> CreateChildSpan(
      const string& span_name, shared_ptr<NutanixSpan> span);

    // This method creates a child span from the context which is passed in
    // form of bytes, with the given "span_name" if tracing is enabled on the
    // setup and returns a shared pointer pointing to new NutanixSpan created.
    static shared_ptr<NutanixSpan> StartSpanFromBytes(
      const string& span_name, const string& bytes);

    // This method extracts byte array which contains information about
    // trace-id and span-id from the NutanixSpan object which is passed in
    // as input parameter. This byte array is set in trace_context field of
    // RpcRequestContext for trace context propagation across services.
    static string GetBytesFromSpan(shared_ptr<NutanixSpan> span);

    // Inject the span context as key value pair into a hashmap
    // "uber-trace-id":"e9076d71e689d41fa05c2874fcc8859a:24e4b557b66e721f:0:01"
    // This function should be used inside of StartClientSpan() function.
    static std::unordered_map<std::string, std::string> Inject(
      const shared_ptr<NutanixSpan> &span);

    // Extract the parent SpanContext from the carrier
    // and return the child span of it.
    // This function should be used inside of StartServerSpan() function.
    static shared_ptr<NutanixSpan> Extract(
      const string &span_name,
      const context::propagation::TextMapCarrier &carrier);

    // Return a NutanixSpan pointer which contains empty opentelemetry
    // span and scope, this is used to be returned instead of nullptr,
    // which allows service to skip the null check for span everywhere.
    static shared_ptr<NutanixSpan> GetEmptySpan();

    // Returns true if tracing is enabled on the setup.
    static bool IsTracingEnabled();

 private:
    // Pointer to Global Tracer object.
    static nostd::shared_ptr<trace::Tracer> tracer_;

    // Whether tracing is enabled or not.
    static bool is_tracing_enabled;

    // collector endpoint where spans are sent.
    static string collector_endpoint;

    // hostname of the setup.
    static string hostname;

    // ip address of the setup.
    static string ip_address;

    // Returns the collector endpoint.
    static string GetEndPoint();

    // Reads the config.json from FLAGS_observability_config_file_path and
    // sets the tracing_enabled and collector_endpoint accordingly.
    static void CheckTracingEnabled();
};

template <typename T>
class HttpTextMapCarrier :
public opentelemetry::context::propagation::TextMapCarrier {
 public:
    HttpTextMapCarrier<T>(T &headers) : headers_(headers) {}
    HttpTextMapCarrier() = default;
    opentelemetry::nostd::string_view Get(
        opentelemetry::nostd::string_view key) const noexcept override {
      std::string key_to_compare = key.data();
      auto it = headers_.find(key_to_compare);
      if (it != headers_.end()) {
        return it->second;
      }
      return "";
    }

    void Set(opentelemetry::nostd::string_view key,
            opentelemetry::nostd::string_view value) noexcept override {
      headers_.insert(std::pair<std::string, std::string>(std::string(key),
                      std::string(value)));
    }

    T headers_;
};

} // namespace

#endif // _UTIL_BASE_NUTANIX_OBSERVABILITY_H_

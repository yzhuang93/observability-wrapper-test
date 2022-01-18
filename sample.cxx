#include <iostream>

#include <yaml-cpp/yaml.h>

#include <opentracing/tracer.h>
#include <jaegertracing/Tracer.h>
#include <jaegertracing/SpanContext.h>
#include <curl/curl.h>
#include <opentracing/string_view.h>

using namespace opentracing;

using StrMap = std::unordered_map<std::string, std::string>;

size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

template <typename BaseWriter>
struct WriterMock : public BaseWriter {
    explicit WriterMock(StrMap& keyValuePairs)
        : _keyValuePairs(keyValuePairs)
    {
    }

    opentracing::expected<void>
    Set(opentracing::string_view key,
        opentracing::string_view value) const override
    {
        _keyValuePairs[key] = value;
        return opentracing::make_expected();
    }

    StrMap& _keyValuePairs;
};

void callMockAdonis(StrMap& headerMap) {
  auto curl = curl_easy_init();
  std::string response_string;
  std::string header_string;
  curl_easy_setopt(curl, CURLOPT_URL, "http://10.36.240.15:8888/api/mockasync/v4/config/cats?delay=1000");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);
  struct curl_slist* headers = NULL;
  for (auto& entry : headerMap) {
    std::string headerString = entry.first + ": " + entry.second;
    std::cout << "#headerstring: " << headerString << "\n";
    headers = curl_slist_append(headers, headerString.c_str());
  }
  headers = curl_slist_append(headers, "Cookie: NTNX_IGW_SESSION=eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiJ9.eyJ1c2VyX3Byb2ZpbGUiOiJ7XCJ1c2VybmFtZVwiOiBcImFkbWluXCIsIFwiZG9tYWluXCI6IG51bGwsIFwibGVnYWN5X2FkbWluX2F1dGhvcml0aWVzXCI6IFtcIlJPTEVfQ0xVU1RFUl9WSUVXRVJcIiwgXCJST0xFX1VTRVJfQURNSU5cIiwgXCJST0xFX0NMVVNURVJfQURNSU5cIiwgXCJST0xFX01VTFRJQ0xVU1RFUl9BRE1JTlwiXSwgXCJhdXRoZW50aWNhdGVkXCI6IHRydWUsIFwiX3Blcm1hbmVudFwiOiB0cnVlLCBcInJvbGVzXCI6IFtcIlByaXNtIFZpZXdlclwiLCBcIlN1cGVyIEFkbWluXCIsIFwiUHJpc20gQWRtaW5cIl0sIFwib3JpZ190b2tlbl9pc3NfdGltZVwiOiAxNjQyMTkxMjU4LCBcImxvZ191dWlkXCI6IFwiMGQ5ODE1MTAtYmRhZS00NjVkLWJkZjctMWVhMDcwZGUwYTE2XCIsIFwidXNlcnR5cGVcIjogXCJsb2NhbFwiLCBcImFwcF9kYXRhXCI6IHt9LCBcImF1dGhfaW5mb1wiOiB7XCJ1c2VybmFtZVwiOiBcImFkbWluXCIsIFwicmVtb3RlX2F1dGhvcml6YXRpb25cIjogbnVsbCwgXCJ1c2VyX2dyb3VwX3V1aWRzXCI6IG51bGwsIFwicmVtb3RlX2F1dGhfanNvblwiOiBudWxsLCBcInNlcnZpY2VfbmFtZVwiOiBudWxsLCBcInRva2VuX2F1ZGllbmNlXCI6IG51bGwsIFwidG9rZW5faXNzdWVyXCI6IG51bGwsIFwidXNlcl91dWlkXCI6IFwiMDAwMDAwMDAtMDAwMC0wMDAwLTAwMDAtMDAwMDAwMDAwMDAwXCIsIFwidGVuYW50X3V1aWRcIjogXCIwMDAwMDAwMC0wMDAwLTAwMDAtMDAwMC0wMDAwMDAwMDAwMDBcIn19IiwianRpIjoiM2ViMjZiZDktOGVmYS0zYzdlLTg1ZTEtYzExNTE3ODliZWNmIiwiaXNzIjoiQXRoZW5hIiwiaWF0IjoxNjQyMTkxMjU4LCJleHAiOjE2NDIxOTIxNTh9.F3mGS_Y2R0Kr2efHDND4iUgfOGv8G2QyTfu24iX-4YW34VSfXPHpcu6m23gBvj8DjCm5GOYgP62JT27BDs2w4gDTElEGyCmnJphq57iixhy15wY5EKmdZnSRG1BC7BGRS5PJNN-hqVbSMG8deFFL-s9_B2awTkonZRE-IRaNivRF0kS4bk127wF-vYt5-o_olxKkwpxmJLl7Y9_cZxCsvKabk--AgasK3XtWQnhxL0UQLnQKqyX4d5pkle3TOct_E9cOi6gHeTGIqHtM53AZnPnJcUbaRl2PtALjJTuBgewgnlNf3XXfBKR5to33Bg1fp2UrS5QT-rEBHwykA0pNpw");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_perform(curl);
  //Clean up
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  curl = NULL;
}

int main() {
  auto configYAML = YAML::LoadFile("../jaegerConfig.yaml");
  auto config = jaegertracing::Config::parse(configYAML);
  auto tracer = jaegertracing::Tracer::make(
      "cpp-sample-service", config, jaegertracing::logging::consoleLogger());
  opentracing::Tracer::InitGlobal(std::static_pointer_cast<opentracing::Tracer>(tracer));

// start a new span
  std::cout << "start span\n";
  auto span = opentracing::Tracer::Global()->StartSpan("tracedFunction");

  // start child span (to call adonis)
  auto child_span =tracer->StartSpan("childA", {ChildOf(&span->context())});
  // Inject uber-trace-id: 9b974a1ef09fb4dd:9b974a1ef09fb4dd:0000000000000000:1
  StrMap headerMap;
  WriterMock<opentracing::HTTPHeadersWriter> headerWriter(headerMap);
  tracer->Inject(child_span->context(), headerWriter); 
  // Call adonis with header
  callMockAdonis(headerMap); 

  // Finish Spans
  std::cout << "finishing child_span\n";
  child_span -> Finish();
  std::cout << "finishing span\n";
  span -> Finish();
  std::cout << "finish span\n";
  opentracing::Tracer::Global()->Close();
  std::cout << "close tracer\n";
  return 0;
}

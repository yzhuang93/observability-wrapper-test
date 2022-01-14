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
  curl_easy_setopt(curl, CURLOPT_URL, "http://10.33.64.109:9999/api/mockasync/v4/config/cats?size=50k&delay=100");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);
  struct curl_slist* headers = NULL;
  for (auto& entry : headerMap) {
    std::string headerString = entry.first + ": " + entry.second;
    std::cout << "#headerstring: " << headerString << "\n";
    headers = curl_slist_append(headers, headerString.c_str());
  }

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  curl = NULL;
}

int main() {
  auto configYAML = YAML::LoadFile("../jaegerConfig.yaml");
  auto config = jaegertracing::Config::parse(configYAML);
  auto tracer = jaegertracing::Tracer::make(
      "yiyang-sample-service", config, jaegertracing::logging::consoleLogger());
  opentracing::Tracer::InitGlobal(std::static_pointer_cast<opentracing::Tracer>(tracer));

  std::cout << "start span\n";
  auto span = opentracing::Tracer::Global()->StartSpan("tracedFunction");
  auto child_span =tracer->StartSpan("childA", {ChildOf(&span->context())});

  StrMap headerMap;
  WriterMock<opentracing::HTTPHeadersWriter> headerWriter(headerMap);
  tracer->Inject(child_span->context(), headerWriter);

  callMockAdonis(headerMap); 

  std::cout << "finishing child_span\n";
  child_span -> Finish();
  std::cout << "finishing span\n";
  span -> Finish();
  std::cout << "finish span\n";
  opentracing::Tracer::Global()->Close();
  std::cout << "close tracer\n";
  return 0;
}

/*
 * Copyright (c) 2022 Nutanix Inc. All rights reserved.
 *
 * Author: mudit.wadhwa@nutanix.com
 */

#include "nutanix_span.h"

namespace nutanix {

const std::string STR_K_NO_ERROR = "kNoError";
const std::string STR_ERROR = "error";
const std::string STR_RESPONSE_STATUS = "response.status";
const std::string STR_SPAN_KIND = "span.kind";
const std::string STR_HTTP_METHOD = "http.method";
const std::string STR_METHOD_NAME = "method.name";

// Public constants.
const std::string NutanixSpan::SPAN_KIND_SERVER = "server";
const std::string NutanixSpan::SPAN_KIND_CLIENT = "client";

//-----------------------------------------------------------------------------

NutanixSpan::NutanixSpan(const nostd::shared_ptr<trace::Span>& span,
                         trace::Scope* scope) {
  actual_span = span;
  scope_ = scope;
}

//-----------------------------------------------------------------------------

void NutanixSpan::End() {
  if (actual_span) {
    actual_span->End();
  }
}

//-----------------------------------------------------------------------------

void NutanixSpan::MakeCurrent() {
  if (actual_span) {
    if (scope_)
      *scope_ = trace::Tracer::WithActiveSpan(actual_span);
  } else {
    scope_ = nullptr;
  }
}

//-----------------------------------------------------------------------------

void NutanixSpan::SetTag(string tag_name, string tag_value) {
  try {
    if (actual_span) {
      actual_span->SetAttribute(tag_name, tag_value);
    }
  } catch (const exception& ex) {
  }
}

//-----------------------------------------------------------------------------

void NutanixSpan::SetStatus(string status_code) {
  try {
    if (actual_span) {
      if (status_code != STR_K_NO_ERROR)
        actual_span->SetStatus(opentelemetry::trace::StatusCode::kError);
      else
        actual_span->SetStatus(opentelemetry::trace::StatusCode::kOk);
    }
  } catch (const exception& ex) {
  }
}

//-----------------------------------------------------------------------------

void NutanixSpan::SetErrorTag(string error_msg) {
  try {
    if (actual_span) {
      if (error_msg != STR_K_NO_ERROR) {
        actual_span->SetAttribute(STR_ERROR, true);
        actual_span->SetAttribute(STR_RESPONSE_STATUS, error_msg);
      } else {
        actual_span->SetAttribute(STR_ERROR, false);
        actual_span->SetAttribute(STR_RESPONSE_STATUS, error_msg);
      }
    }
  } catch (const exception& ex) {
  }
}

//-----------------------------------------------------------------------------

void NutanixSpan::SetSpanKind(string span_kind) {
  if (actual_span) {
    actual_span->SetAttribute(STR_SPAN_KIND, span_kind);
  }
}

//-----------------------------------------------------------------------------

void NutanixSpan::SetHttpMethod(string http_method) {
  if (actual_span) {
    actual_span->SetAttribute(STR_HTTP_METHOD, http_method);
  }
}

//-----------------------------------------------------------------------------

void NutanixSpan::SetMethodName(string method_name) {
  if (actual_span) {
    actual_span->SetAttribute(STR_METHOD_NAME, method_name);
  }
}

//-----------------------------------------------------------------------------

bool NutanixSpan::IsEmpty() {
  return !actual_span || !scope_;
}

//-----------------------------------------------------------------------------

} // namespace

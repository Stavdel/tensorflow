/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include <memory>
#include <utility>
#include <vector>

#include "learning/brain/google/monitoring/metrics_exporter.h"
#include "tensorflow/compiler/mlir/lite/metrics/error_collector.h"
#include "tensorflow/lite/python/metrics_wrapper/metrics_wrapper.h"

namespace tflite {
namespace metrics_wrapper {

MetricsWrapper::MetricsWrapper(std::unique_ptr<MetricsExporter> exporter)
    : exporter_(std::move(exporter)) {}

MetricsWrapper::~MetricsWrapper() {}

MetricsWrapper* MetricsWrapper::CreateMetricsWrapper(
    const std::string& session_id) {
  MetricsExporter::Options options;
  options.export_at_exit = false;
  {
    // Add session_id to root label list.
    std::vector<streamz::Entity::Label> root_labels;
    streamz::Entity::Label session_id_label;
    session_id_label.set_key("session_id");
    session_id_label.set_string_value(session_id);
    root_labels.push_back(session_id_label);
    options.entity_labels = root_labels;
  }

  std::unique_ptr<MetricsExporter> exporter(new MetricsExporter(options));

  MetricsWrapper* wrapper = new MetricsWrapper(std::move(exporter));
  return wrapper;
}

PyObject* MetricsWrapper::ExportMetrics() {
  if (!exporter_) {
    PyErr_SetString(PyExc_ValueError, "MetricsExporter was not initialized.");
    return nullptr;
  }

  exporter_->ExportMetrics();

  Py_RETURN_NONE;
}

std::vector<std::string> GetCollectedErrors() {
  mlir::TFL::ErrorCollector* collector = mlir::TFL::GetErrorCollector();
  std::vector<std::string> result;
  for (const auto& error_data : collector->CollectedErrors()) {
    result.push_back(error_data.SerializeAsString());
  }
  return result;
}

}  // namespace metrics_wrapper
}  // namespace tflite

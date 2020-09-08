/* Copyright 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#define InitializePageLoadMetricsForWebContents \
  InitializePageLoadMetricsForWebContents_Chromium
#include "../../../../../chrome/browser/page_load_metrics/page_load_metrics_initialize.cc"
#undef InitializePageLoadMetricsForWebContents

#include "brave/components/brave_perf_predictor/browser/buildflags.h"

#if BUILDFLAG(ENABLE_BRAVE_PERF_PREDICTOR)
#include "brave/components/brave_perf_predictor/browser/perf_predictor_page_metrics_observer.h"
#endif

#if defined(OS_ANDROID)
#include "brave/browser/android/brave_perf_predictor_tab_helper_delegate_android_impl.h"
#endif

namespace chrome {

namespace {

class BravePageLoadMetricsEmbedder : public chrome::PageLoadMetricsEmbedder {
 public:
  explicit BravePageLoadMetricsEmbedder(content::WebContents* web_contents);
  ~BravePageLoadMetricsEmbedder() override;

 protected:
  // page_load_metrics::PageLoadMetricsEmbedderBase:
  void RegisterEmbedderObservers(
      ::page_load_metrics::PageLoadTracker* tracker) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(BravePageLoadMetricsEmbedder);
};

BravePageLoadMetricsEmbedder::BravePageLoadMetricsEmbedder(
    content::WebContents* web_contents)
    : chrome::PageLoadMetricsEmbedder(web_contents) {}

BravePageLoadMetricsEmbedder::~BravePageLoadMetricsEmbedder() = default;

void BravePageLoadMetricsEmbedder::RegisterEmbedderObservers(
    page_load_metrics::PageLoadTracker* tracker) {
  PageLoadMetricsEmbedder::RegisterEmbedderObservers(tracker);

#if BUILDFLAG(ENABLE_BRAVE_PERF_PREDICTOR)
  tracker->AddObserver(
#if defined(OS_ANDROID)
      std::make_unique<brave_perf_predictor::PerfPredictorPageMetricsObserver>(
          std::make_unique<
              brave::android::
                  BravePerfPredictorTabHelperDelegateAndroidImpl>()));
#else
      std::make_unique<
          brave_perf_predictor::PerfPredictorPageMetricsObserver>());
#endif
#endif
}

}  // namespace

void InitializePageLoadMetricsForWebContents(
    content::WebContents* web_contents) {
  // TODO(bug https://github.com/brave/brave-browser/issues/7784)
  // change
  // android_webview/browser/page_load_metrics/page_load_metrics_initialize.cc
  // as well to register Page Load Metrics Observers
  page_load_metrics::MetricsWebContentsObserver::CreateForWebContents(
      web_contents,
      std::make_unique<BravePageLoadMetricsEmbedder>(web_contents));
}

}  // namespace chrome

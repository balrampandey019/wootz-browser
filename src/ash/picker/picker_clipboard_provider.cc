// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/picker/picker_clipboard_provider.h"

#include "ash/clipboard/clipboard_history_item.h"
#include "ash/public/cpp/clipboard_history_controller.h"
#include "base/i18n/case_conversion.h"
#include "chromeos/crosapi/mojom/clipboard_history.mojom.h"

namespace ash {
namespace {
std::optional<PickerSearchResult::ClipboardData::DisplayFormat>
GetDisplayFormat(crosapi::mojom::ClipboardHistoryDisplayFormat format) {
  switch (format) {
    case crosapi::mojom::ClipboardHistoryDisplayFormat::kFile:
      return PickerSearchResult::ClipboardData::DisplayFormat::kFile;
    case crosapi::mojom::ClipboardHistoryDisplayFormat::kText:
      return PickerSearchResult::ClipboardData::DisplayFormat::kText;
    case crosapi::mojom::ClipboardHistoryDisplayFormat::kPng:
      return PickerSearchResult::ClipboardData::DisplayFormat::kImage;
    case crosapi::mojom::ClipboardHistoryDisplayFormat::kHtml:
      return PickerSearchResult::ClipboardData::DisplayFormat::kHtml;
    default:
      return std::nullopt;
  }
}

bool MatchQuery(const ClipboardHistoryItem& item, const std::u16string& query) {
  if (query.empty()) {
    return true;
  }
  if (item.display_format() !=
          crosapi::mojom::ClipboardHistoryDisplayFormat::kText &&
      item.display_format() !=
          crosapi::mojom::ClipboardHistoryDisplayFormat::kFile) {
    return false;
  }
  return base::i18n::ToLower(item.display_text())
             .find(base::i18n::ToLower(query)) != std::u16string::npos;
}
}

PickerClipboardProvider::PickerClipboardProvider(base::Clock* clock)
    : clock_(clock) {}

PickerClipboardProvider::~PickerClipboardProvider() = default;

void PickerClipboardProvider::FetchResults(OnFetchResultsCallback callback,
                                           const std::u16string& query,
                                           base::TimeDelta recency) {
  ash::ClipboardHistoryController* clipboard_history_controller =
      ash::ClipboardHistoryController::Get();
  if (clipboard_history_controller) {
    clipboard_history_controller->GetHistoryValues(base::BindOnce(
        &PickerClipboardProvider::OnFetchHistory,
        weak_ptr_factory_.GetWeakPtr(), std::move(callback), query, recency));
  }
}

void PickerClipboardProvider::OnFetchHistory(
    OnFetchResultsCallback callback,
    const std::u16string& query,
    base::TimeDelta recency,
    std::vector<ClipboardHistoryItem> items) {
  std::vector<PickerSearchResult> results;
  for (const auto& item : items) {
    if ((clock_->Now() - item.time_copied()) > recency) {
      continue;
    }
    if (!MatchQuery(item, query)) {
      continue;
    }
    if (std::optional<PickerSearchResult::ClipboardData::DisplayFormat>
            display_format = GetDisplayFormat(item.display_format());
        display_format.has_value()) {
      results.push_back(PickerSearchResult::Clipboard(
          item.id(), *display_format, item.display_text(),
          item.display_image()));
    }
  }
  std::move(callback).Run(std::move(results));
}
}  // namespace ash

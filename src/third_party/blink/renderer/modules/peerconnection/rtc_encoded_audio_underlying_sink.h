// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_PEERCONNECTION_RTC_ENCODED_AUDIO_UNDERLYING_SINK_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_PEERCONNECTION_RTC_ENCODED_AUDIO_UNDERLYING_SINK_H_

#include "base/threading/thread_checker.h"
#include "third_party/blink/renderer/core/streams/underlying_sink_base.h"
#include "third_party/blink/renderer/modules/modules_export.h"
#include "third_party/blink/renderer/platform/peerconnection/rtc_encoded_audio_stream_transformer.h"
#include "third_party/webrtc/api/frame_transformer_interface.h"

namespace blink {

class ExceptionState;
class RTCEncodedAudioStreamTransformer;

class MODULES_EXPORT RTCEncodedAudioUnderlyingSink final
    : public UnderlyingSinkBase {
 public:
  RTCEncodedAudioUnderlyingSink(
      ScriptState*,
      scoped_refptr<blink::RTCEncodedAudioStreamTransformer::Broker>);

  // UnderlyingSinkBase
  ScriptPromise<IDLUndefined> start(ScriptState*,
                                    WritableStreamDefaultController*,
                                    ExceptionState&) override;
  ScriptPromise<IDLUndefined> write(ScriptState*,
                                    ScriptValue chunk,
                                    WritableStreamDefaultController*,
                                    ExceptionState&) override;
  ScriptPromise<IDLUndefined> close(ScriptState*, ExceptionState&) override;
  ScriptPromise<IDLUndefined> abort(ScriptState*,
                                    ScriptValue reason,
                                    ExceptionState&) override;

  void Trace(Visitor*) const override;

 private:
  scoped_refptr<blink::RTCEncodedAudioStreamTransformer::Broker>
      transformer_broker_;
  THREAD_CHECKER(thread_checker_);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_PEERCONNECTION_RTC_ENCODED_AUDIO_UNDERLYING_SINK_H_

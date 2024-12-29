import sys
import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

# DeepStream 파이프라인 생성
def create_pipeline():
    Gst.init(None)

    # WebRTC를 위한 GStreamer 파이프라인 구성
    pipeline = Gst.parse_launch('videotestsrc ! videoconvert ! x264enc ! webrtcbin bundle-policy=max-bundle')

    webrtc_bin = pipeline.get_by_name('webrtcbin0')

    # WebRTC 연결을 위한 신호 처리 (예: offer, answer, ICE 후보 처리)
    def on_offer_created(webrtcbin, promise, user_data):
        offer = webrtcbin.emit('create-offer')
        promise.reply(offer)

    webrtc_bin.connect('on-offer-created', on_offer_created)

    return pipeline

def main():
    pipeline = create_pipeline()

    # 파이프라인 실행
    pipeline.set_state(Gst.State.PLAYING)

    # 이벤트 루프 실행
    loop = GLib.MainLoop()
    loop.run()

if __name__ == '__main__':
    main()

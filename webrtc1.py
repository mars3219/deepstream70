import asyncio
import websockets
import json
import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

# WebRTC 연결을 위한 메시지 처리
async def send_offer(websocket, offer_sdp):
    offer = {
        "type": "offer",
        "sdp": offer_sdp
    }
    await websocket.send(json.dumps(offer))

async def send_answer(websocket, answer_sdp):
    answer = {
        "type": "answer",
        "sdp": answer_sdp
    }
    await websocket.send(json.dumps(answer))

async def send_candidate(websocket, candidate):
    candidate_msg = {
        "type": "candidate",
        "candidate": candidate
    }
    await websocket.send(json.dumps(candidate_msg))

# WebRTC 파이프라인 설정
def create_pipeline():
    Gst.init(None)
    
    # RTSP 스트림 처리 (rtspsrc 사용) + decodebin 사용
    # pipeline = Gst.parse_launch(
    #     'rtspsrc location=rtsp://210.99.70.120:1935/live/cctv006.stream ! '
    #     'rtph264depay ! decodebin ! videoconvert ! vp8enc ! rtpvp8pay ! webrtcbin name=webrtcbin0 ! autovideosink'
    # )
    pipeline = Gst.parse_launch(
          'videotestsrc ! videoconvert ! tee name=t t. '
          '! queue ! autovideosink t. ! queue ! vp8enc ! rtpvp8pay ! webrtcbin name=webrtcbin0'
)

    webrtc_bin = pipeline.get_by_name('webrtcbin0')

    # WebRTC offer 생성 후 전송
    def on_create_offer(webrtcbin, promise, user_data):
        # offer 생성
        offer = webrtcbin.emit('create-offer')
        promise.reply(offer)
        offer_sdp = offer.sdp.as_text()
        print("Generated SDP Offer:", offer_sdp)
        # WebSocket을 통해 offer 전송
        asyncio.run(send_offer(user_data, offer_sdp))

    # 'create-offer' 신호를 처리하도록 연결
    webrtc_bin.connect('create-offer', on_create_offer)

    return pipeline

# WebSocket 서버와 연결 및 파이프라인 실행
async def run_webrtc():
    async with websockets.connect("ws://localhost:8765") as websocket:
        print("Connected to signaling server")

        # WebRTC 파이프라인 생성 및 실행
        pipeline = create_pipeline()
        pipeline.set_state(Gst.State.PLAYING)
        loop = GLib.MainLoop()
        loop.run()

        # 실제 ICE 후보가 생성되면 이를 서버로 전송
        def on_new_candidate(webrtcbin, mlineindex, candidate, user_data):
            candidate_info = {
                "candidate": candidate,
                "mlineindex": mlineindex
            }
            print("New ICE Candidate:", candidate_info)
            # WebSocket을 통해 ICE 후보를 서버로 전송
            asyncio.run(send_candidate(user_data, candidate_info))

        webrtc_bin = pipeline.get_by_name('webrtcbin0')
        webrtc_bin.connect('on-new-ice-candidate', on_new_candidate, websocket)

if __name__ == "__main__":
    asyncio.run(run_webrtc())

import asyncio
import websockets
import json
import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

# WebRTC 시그널링 서버와 연결
async def send_offer(websocket):
    # WebRTC offer 생성
    offer = {
        "type": "offer",
        "sdp": "dummy_sdp_offer"  # 실제 SDP offer 생성 코드 필요
    }
    await websocket.send(json.dumps(offer))

async def send_answer(websocket, answer):
    # WebRTC answer 생성
    answer_msg = {
        "type": "answer",
        "sdp": answer
    }
    await websocket.send(json.dumps(answer_msg))

async def send_candidate(websocket, candidate):
    # ICE 후보 전송
    candidate_msg = {
        "type": "candidate",
        "candidate": candidate
    }
    await websocket.send(json.dumps(candidate_msg))

# WebRTC 파이프라인 생성 및 설정
def create_pipeline():
    Gst.init(None)
    
    # WebRTC GStreamer 파이프라인 구성
    pipeline = Gst.parse_launch('videotestsrc ! videoconvert ! x264enc ! webrtcbin bundle-policy=max-bundle name=webrtcbin0')

    webrtc_bin = pipeline.get_by_name('webrtcbin0')

    # Offer 생성 및 전송
    def on_offer_created(webrtcbin, promise, user_data):
        offer = webrtcbin.emit('create-offer')
        promise.reply(offer)
        # 실제 WebSocket을 통해 offer 전송
        asyncio.run(send_offer(user_data))

    webrtc_bin.connect('on-offer-created', on_offer_created)

    return pipeline

# 시그널링 서버 연결 및 파이프라인 실행
async def run_webrtc():
    pipeline = create_pipeline()

    # WebSocket 연결
    async with websockets.connect("ws://localhost:8765") as websocket:
        print("Connected to signaling server")

        # WebRTC 파이프라인 실행
        pipeline.set_state(Gst.State.PLAYING)
        loop = GLib.MainLoop()
        loop.run()

if __name__ == "__main__":
    asyncio.run(run_webrtc())

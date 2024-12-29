import asyncio
import websockets
import json

# WebRTC 연결을 위한 시그널링 서버
class WebRTCSignalingServer:
    def __init__(self):
        self.clients = set()

    async def register(self, websocket):
        self.clients.add(websocket)
        print(f"Client connected: {websocket.remote_address}")

    async def unregister(self, websocket):
        self.clients.remove(websocket)
        print(f"Client disconnected: {websocket.remote_address}")

    async def handle_message(self, websocket, message):
        # 메시지 파싱
        data = json.loads(message)
        message_type = data.get("type")

        # 클라이언트로부터 offer, answer, ICE candidates 처리
        if message_type == "offer":
            print("Received offer")
            await self.broadcast({"type": "offer", "sdp": data["sdp"]})
        elif message_type == "answer":
            print("Received answer")
            await self.broadcast({"type": "answer", "sdp": data["sdp"]})
        elif message_type == "candidate":
            print("Received ICE candidate")
            await self.broadcast({"type": "candidate", "candidate": data["candidate"]})

    async def broadcast(self, message):
        # 모든 연결된 클라이언트에 메시지 전달
        if self.clients:  # 메시지가 비어있지 않으면
            message = json.dumps(message)
            await asyncio.wait([client.send(message) for client in self.clients])

    async def handler(self, websocket, path):
        await self.register(websocket)
        try:
            async for message in websocket:
                await self.handle_message(websocket, message)
        except websockets.exceptions.ConnectionClosed:
            print("Connection closed")
        finally:
            await self.unregister(websocket)

# WebSocket 서버 시작
async def start_server():
    server = WebRTCSignalingServer()
    async with websockets.serve(server.handler, "localhost", 8765):
        print("WebRTC signaling server started on ws://localhost:8765")
        await asyncio.Future()  # 서버가 종료되지 않도록 유지

# 서버 실행
if __name__ == "__main__":
    asyncio.run(start_server())

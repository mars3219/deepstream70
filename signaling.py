import asyncio
import websockets
import json

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
        data = json.loads(message)
        message_type = data.get("type")

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
        if self.clients:
            message = json.dumps(message)
            await asyncio.gather(*(client.send(message) for client in self.clients))

    async def handler(self, websocket):
        await self.register(websocket)
        try:
            async for message in websocket:
                await self.handle_message(websocket, message)
        except websockets.exceptions.ConnectionClosed:
            print("Connection closed")
        finally:
            await self.unregister(websocket)

    async def start_server(self):
        async with websockets.serve(self.handler, "localhost", 8765):
            print("WebRTC signaling server started on ws://localhost:8765")
            await asyncio.Future()  # 서버가 종료되지 않도록 유지

if __name__ == "__main__":
    server = WebRTCSignalingServer()
    try:
        loop = asyncio.get_event_loop()
        loop.run_until_complete(server.start_server())
    except KeyboardInterrupt:
        print("Server stopped.")

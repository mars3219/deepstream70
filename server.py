import http.server
import socketserver
import os

# 웹 서버 실행할 포트 번호
PORT = 8000

# HTML 파일이 있는 디렉터리 설정
# web_dir = os.path.join(os.path.dirname(__file__), 'web')
web_dir = "/workspace/web"
os.makedirs(web_dir, exist_ok=True)

# 웹 서버 설정
Handler = http.server.SimpleHTTPRequestHandler

# 현재 디렉터리의 'web' 폴더를 웹 서버의 루트 디렉터리로 설정
os.chdir(web_dir)

# 서버 실행
with socketserver.TCPServer(("", PORT), Handler) as httpd:
    print(f"Serving at port {PORT}")
    httpd.serve_forever()

#!/usr/bin/env python3
"""
商城库存管理系统 - Web可视化界面服务器
简单的HTTP服务器，用于提供静态文件服务并支持CORS
"""

import http.server
import socketserver
import os
import sys
from urllib.parse import urlparse

class CORSHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    """支持CORS的HTTP请求处理器"""

    def end_headers(self):
        # 添加CORS头
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        super().end_headers()

    def do_OPTIONS(self):
        # 处理预检请求
        self.send_response(200)
        self.end_headers()

    def guess_type(self, path):
        """改善MIME类型猜测"""
        mimetype = super().guess_type(path)
        if path.endswith('.js'):
            return 'application/javascript'
        elif path.endswith('.css'):
            return 'text/css'
        elif path.endswith('.txt'):
            return 'text/plain; charset=utf-8'
        return mimetype

def main():
    # 获取项目根目录
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    web_dir = os.path.join(project_root, 'web')

    # 切换到web目录
    os.chdir(web_dir)

    # 设置端口
    PORT = 8080

    # 检查端口是否可用
    try:
        with socketserver.TCPServer(("", PORT), CORSHTTPRequestHandler) as httpd:
            print(f"📊 商城库存管理系统 - Web可视化界面")
            print(f"🚀 服务器启动成功！")
            print(f"🌐 访问地址: http://localhost:{PORT}")
            print(f"📁 服务目录: {web_dir}")
            print(f"📈 数据目录: {os.path.join(project_root, 'data')}")
            print(f"\n💡 提示：")
            print(f"   - 请确保C++程序已运行并生成了数据文件")
            print(f"   - 在浏览器中打开上述地址查看可视化界面")
            print(f"   - 按 Ctrl+C 停止服务器")
            print(f"\n" + "="*60)

            try:
                httpd.serve_forever()
            except KeyboardInterrupt:
                print(f"\n\n🛑 服务器已停止")
                sys.exit(0)

    except OSError as e:
        if e.errno == 48:  # Address already in use
            print(f"❌ 端口 {PORT} 已被占用！")
            print(f"💡 请尝试以下解决方案：")
            print(f"   1. 关闭占用端口的程序")
            print(f"   2. 修改此脚本中的PORT变量")
            print(f"   3. 等待几分钟后重试")
        else:
            print(f"❌ 服务器启动失败：{e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
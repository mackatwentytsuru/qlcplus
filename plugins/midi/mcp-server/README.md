# MCU Model Context Protocol (MCP) サーバー

このプロジェクトは、Mackie Control Universal (MCU) MIDIプロトコルをラップするMCPサーバーです。これにより、AIアシスタント（Claudeなど）が物理的なMCUデバイスを制御できるようになります。特に **Behringer X-Touch Extender** でテストされています。

## インストール

QLC+本体とは別に配布するため、`pip` または `uvx` を使用してインストールします：

```bash
# uvxを使用する場合（Claude Desktopで推奨）
uvx mcu-mcp-server

# またはグローバルにインストール
pip install mcu-mcp-server
```

## Claude Desktopでの実行方法

以下の設定を `claude_desktop_config.json` に追加してください：

```json
{
  "mcpServers": {
    "qlc-mcu": {
      "command": "uvx",
      "args": ["mcu-mcp-server"]
    }
  }
}
```

## 主な機能

- モータライズドフェーダーの制御（14ビット精度）
- ボタンLEDの状態設定（オン/オフ/点滅）
- VPot LEDリングの制御
- LCDスクリブルストリップへのテキストの書き込み
- 実際のデバイス状態の読み取り、ハンドシェイク、および最近の入力の取得

正確なボタンマッピングと定数については、`mcu_protocol.py` を参照してください。
